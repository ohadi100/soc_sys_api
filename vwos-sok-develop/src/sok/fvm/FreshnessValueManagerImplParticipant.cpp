/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/FreshnessValueManagerImplParticipant.hpp"
#include <cmath>
#include "sok/fvm/FreshnessValueManagerConstants.hpp"
#include "sok/common/SokUtilities.hpp"
#include "sok/common/Logger.hpp"

namespace sok
{
namespace fvm
{

FreshnessValueManagerImplParticipant::FreshnessValueManagerImplParticipant()
: AFreshnessValueManagerImpl()
, mFvStateManager(std::make_shared<FreshnessValueStateManager>())
, mTimeSinceAuthFvReq(65535)
, mActiveFvChallenge()
, mUnAuthFv()
, mCrAuthFvAndMac()
, mEcuKeyIdForFvDistribution()
{
}

FvmErrorCode 
FreshnessValueManagerImplParticipant::MainFunction() noexcept
{
    try {
        if (!mInitialized) {
            return FvmErrorCode::kNotInitialized;
        }

        auto retValue = mFvStateManager->enter();

        incTimers();

        return retValue;

    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        return FvmErrorCode::kGeneralError;
    } catch (...) {
        LOGE("exception");
        return FvmErrorCode::kGeneralError;
    }
}

bool 
FreshnessValueManagerImplParticipant::serverOrParticipantInit() noexcept
{
    try {
        mEcuKeyIdForFvDistribution = mFvmConfAccessor->GetEcuKeyIdForFvDistribution();
        if (common::CsmErrorCode::kSuccess != mCsmAccessor->IsKeyExists(mEcuKeyIdForFvDistribution)) {
            LOGE("Couldn't find key id: " << mEcuKeyIdForFvDistribution << ", for the authentic FV distribution");
            return false;
        }

        auto requestFvAction = [this]() -> FvmErrorCode {
            return this->requestAnAuthenticFv();
        };
        auto waitForAnAuthenticFvAction = [this]() -> FvmErrorCode {
            return this->waitForAnAuthenticFv();
        };
        auto processAnAuthenticFvAction = [this]() -> FvmErrorCode {
            return this->processAnAuthenticFv();
        };
        auto processAnUnauthenticFvAction = [this]() -> FvmErrorCode {
            return this->processAnUnauthenticFv();
        };
        auto IdleAction = [this]() -> FvmErrorCode {
            return FvmErrorCode::kSuccess;
        };

        mFvStateManager->registerState(FreshnessValueState::FVInProgress, waitForAnAuthenticFvAction);
        mFvStateManager->registerState(FreshnessValueState::RequestFV, requestFvAction);
        mFvStateManager->registerState(FreshnessValueState::ProcessFV, processAnAuthenticFvAction);
        mFvStateManager->registerState(FreshnessValueState::ProcessAnauthFV, processAnUnauthenticFvAction);
        mFvStateManager->registerState(FreshnessValueState::Idle, IdleAction);
        // Set initial state to RequestFV
        mFvStateManager->transiteTo(FreshnessValueState::RequestFV);

        auto AuthFvCb = [this](std::string const& signal, std::vector<uint8_t> const& value) {
            this->incomingAuthFvSignalsCb(signal, value);
        };
        auto unAuthFvCb = [this](std::string const& signal, std::vector<uint8_t> const& value) {
            this->incomingUnAuthFvSignalsCb(signal, value);
        };

        LOGI("Subscribing to FV distribution signals");
        auto ret1 = mSignalManager->Subscribe(mFvmConfAccessor->GetAuthenticatedFvValueSignalConfig(), AuthFvCb);
        auto ret2 = mSignalManager->Subscribe(mFvmConfAccessor->GetAuthenticatedFvSignatureSignalConfig(), AuthFvCb);
        auto ret3 = mSignalManager->Subscribe(mFvmConfAccessor->GetUnauthenticatedFvSignalConfig(), unAuthFvCb);

        return ((FvmErrorCode::kSuccess == ret1) && (FvmErrorCode::kSuccess == ret2) && (FvmErrorCode::kSuccess == ret3));
    
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        return false;
    } catch (...) {
        LOGE("exception");
        return false;
    }
}

void FreshnessValueManagerImplParticipant::incomingAuthFvSignalsCb(std::string const &signal, std::vector<uint8_t> const &value)
{
    static std::string const authFvSignalValueName = mFvmConfAccessor->GetAuthenticatedFvValueSignalConfig().name;
    static std::string const authFvSignalSigName = mFvmConfAccessor->GetAuthenticatedFvSignatureSignalConfig().name;

    std::lock_guard<std::mutex> lock(mRecFVMutex);
    if (authFvSignalValueName == signal) {
        LOGI("Received authenticated FV signal - with FV");
        mCrAuthFvAndMac.first = value;
    }
    else if (authFvSignalSigName == signal) {
        LOGI("Received authenticated FV signal - with MAC");
        mCrAuthFvAndMac.second = value;
    }
    else {
        LOGE("Received an invalid signal");
    }

    if (!mCrAuthFvAndMac.first.empty() && !mCrAuthFvAndMac.second.empty()) {
        mFvStateManager->reactToFVRes();
    } 
}

void 
FreshnessValueManagerImplParticipant::incomingUnAuthFvSignalsCb(std::string const& signal, std::vector<uint8_t> const& value)
{
    static std::string const unauthFvSignalName = mFvmConfAccessor->GetUnauthenticatedFvSignalConfig().name;
    
    std::lock_guard<std::mutex> lock(mRecUnauthFvMutex);
    if ((unauthFvSignalName == signal) && (value.size() == 8)) {
        LOGD("Received an unauthentic freshness value signal, FV: " << common::ByteVectorToUint<uint64_t>(value));
        mUnAuthFv = value;
        mFvStateManager->reactToUnauthenticFVRes();
    }

    else {
        LOGE("Received an invalid signal");
    }
}

FvmErrorCode 
FreshnessValueManagerImplParticipant::requestAnAuthenticFv() {
    
    auto genRes = mCsmAccessor->GenerateRandomBytes(CHALLENGE_LENGTH_BYTES);
    
    if (genRes.isFailed()) {
        LOGE("Failed generating random bytes for a challenge");
        return FvmErrorCode::kRngError;
    }

    LOGI("Requesting an authentic freshness value with a challenge");
    auto publishRes = mSignalManager->Publish(mFvmConfAccessor->GetAuthenticatedFvChallengeSignalConfig(), genRes.getObject());
    if (FvmErrorCode::kSuccess == publishRes) {
        mActiveFvChallenge = genRes.getObject();
        mTimeSinceAuthFvReq = 0;
        mFvStateManager->transiteTo(FreshnessValueState::FVInProgress);
    }
    else {
        LOGE("Failed publishing an authenticated FV request signal");
    }

    return publishRes;
}

FvmErrorCode 
FreshnessValueManagerImplParticipant::waitForAnAuthenticFv() {
   
    mTimeSinceAuthFvReq += SOK_FM_MAIN_FUNCTION_PERIOD_MS;

    if (mTimeSinceAuthFvReq > SOK_FM_TIME_REQUEST_TIMEOUT_MS) {
        std::lock_guard<std::mutex> lock(mRecFVMutex);
        mFvStateManager->transiteTo(FreshnessValueState::RequestFV);
    }

    return FvmErrorCode::kSuccess;
}

FvmErrorCode 
FreshnessValueManagerImplParticipant::processAnAuthenticFv() {
    
    std::lock_guard<std::mutex> lock(mRecFVMutex);
    LOGI("Processing an authentic FV:" << common::ByteVectorToUint<uint64_t>(mCrAuthFvAndMac.first) << ", mac: " << common::ByteVectorToUint<uint64_t>(mCrAuthFvAndMac.second));
    // todo: assuming that the signature is calculated over - challenge + auth FV. needs verification!!
    
    auto payloadForVerification = mActiveFvChallenge;
    payloadForVerification.insert(payloadForVerification.end(), mCrAuthFvAndMac.first.begin() + 1, mCrAuthFvAndMac.first.end());
    auto verifyRes = mCsmAccessor->MacVerify(mEcuKeyIdForFvDistribution, payloadForVerification, mCrAuthFvAndMac.second, common::MacAlgorithm::kAes128Cmac);
    
    if (common::CsmErrorCode::kSuccess == verifyRes) {
        mFV = common::ByteVectorToUint<uint64_t>(mCrAuthFvAndMac.first);
        mIsFvValid = true;
        mClockCount = 0;
        LOGI("An authentic freshness-value distribution was completed successfully, The updated FV is:" << mFV);
        mFvStateManager->transiteTo(FreshnessValueState::Idle);
    } 
    else {
        LOGE("Failed to verify the authentice freshness-value");
        mFvStateManager->transiteTo(FreshnessValueState::FVInProgress);
    }
    
    mCrAuthFvAndMac.first.clear();
    mCrAuthFvAndMac.second.clear();
    
    return FvmErrorCode::kSuccess;
}

FvmErrorCode 
FreshnessValueManagerImplParticipant::processAnUnauthenticFv() {
    std::lock_guard<std::mutex> lock(mRecUnauthFvMutex);
    auto unauthFv = common::ByteVectorToUint<uint64_t>(mUnAuthFv);
    
    int64_t jitter = static_cast<int64_t>((mFV - unauthFv) * SOK_FM_TIME_INCREMENT_PERIOD_MS);
    if ((jitter != 0) && (SOK_FM_TIME_JITTER_MAX_MS < std::abs(jitter))) {
        jitter = std::abs(jitter);
        jitter -= (mClockCount + SOK_FM_MAIN_FUNCTION_PERIOD_MS);
    }

    if (SOK_FM_TIME_JITTER_MAX_MS < std::abs(jitter)) {
        LOGI("Calculated exceeded jitter from un-authenticated FV broadcast. jitter: " << std::abs(jitter));
        mIsFvValid = false;
        mUnAuthFv.clear();
        mFvStateManager->transiteTo(FreshnessValueState::RequestFV);
    } else {
        mFvStateManager->transiteTo(FreshnessValueState::Idle);
    }

    return FvmErrorCode::kSuccess;
}


} // namespace fvm 
} // namespace sok