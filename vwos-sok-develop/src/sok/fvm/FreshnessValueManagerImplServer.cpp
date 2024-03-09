/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/FreshnessValueManagerImplServer.hpp"
#include "sok/fvm/FreshnessValueManagerConstants.hpp"
#include "sok/common/SokUtilities.hpp"
#include "sok/common/Logger.hpp"

namespace sok
{
namespace fvm
{
FreshnessValueManagerImplServer::FreshnessValueManagerImplServer()
: AFreshnessValueManagerImpl()
, mChallengesMutex()
, mNeedToSendAuthFvResponses(false)
, mNeedToBroadcastFv(true)
, mResponsePendingChallenges()
, mClientConfigMap()
{}

FvmErrorCode
FreshnessValueManagerImplServer::MainFunction() noexcept
{
    try {
        if (!mInitialized) {
            return FvmErrorCode::kNotInitialized;
        }

        FvmErrorCode ret = FvmErrorCode::kSuccess;
        if (mNeedToBroadcastFv) {
            auto actionRet = unauthenticatedBroadcast();
            if (FvmErrorCode::kSuccess != actionRet) {
                ret = actionRet;
            }
        }

        if (mNeedToSendAuthFvResponses) {
            auto actionRet = sendAuthenticFvResponses();
            if (FvmErrorCode::kSuccess != actionRet) {
                ret = actionRet;
            }
        }

        uint64_t cachedFv = mFV;
        incTimers();
        if (mFV != cachedFv) {
            mNeedToSendAuthFvResponses = true;
        }

        if ((mTimeSinceInit % SOK_FM_TIME_SEND_MS) == 0) {
            mNeedToBroadcastFv = true;
        }
        return ret;
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        return FvmErrorCode::kGeneralError;
    } catch (...) {
        LOGE("exception");
        return FvmErrorCode::kGeneralError;
    }
}

bool 
FreshnessValueManagerImplServer::serverOrParticipantInit() noexcept
{
    try {
        // generate random initial FV
        auto GenRes = mCsmAccessor->GenerateRandomBytes(FVM_SERVER_NUM_OF_BYTES_INITIAL_FV);
        if (GenRes.isFailed()) {
            LOGE("Failed generating random number for freshness value");
            return false;
        }
        auto randomBytes = GenRes.getObject();
        // prepend with zeros
        randomBytes.insert(randomBytes.begin(), {0});
        mFV = common::ByteVectorToUint<uint64_t>(randomBytes);
        mIsFvValid = true;

        auto AuthFvChallengeCb = [this](std::string const& signal, std::vector<uint8_t> const& challenge) {
            this->incomingAuthFvChallengeSignalsCb(signal, challenge);
        };
        
        mClientConfigMap = mFvmConfAccessor->GetClientsConfigMap();

        for (auto&& client : mClientConfigMap) {
            if (common::CsmErrorCode::kSuccess != mCsmAccessor->IsKeyExists(client.second.keyId)) {
                LOGE("Couldn't find key id: " << client.second.keyId << ", for authentic FV distribution with ECU: " << client.first);
                return false;
            }
            LOGD("Server subscribing for participant: "<< client.first << " incoming FV challenge signal: " << client.second.clientChallengeSignal.name);
            // register for signals
            auto ret = mSignalManager->Subscribe(client.second.clientChallengeSignal, AuthFvChallengeCb);
            if (FvmErrorCode::kSuccess != ret) {
                LOGE("Failed subscribing for incoming challenge signal: " << client.second.clientChallengeSignal.name)
                return false;
            }
            // store name & key ID 
            mClientNameToKeyId[client.first] = client.second.keyId;
        }

        return true;
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        return false;
    } catch (...) {
        LOGE("exception");
        return false;
    }
}

void 
FreshnessValueManagerImplServer::incomingAuthFvChallengeSignalsCb(std::string const& signal, std::vector<uint8_t> const& challenge)
{
    static const std::regex regex("SOK_Zeit_(\\w+)_Challenge");
    std::smatch match;
    if ((false == std::regex_search(signal.begin(), signal.end(), match, regex)) || (match.size() != 2)) {
        LOGE("invalid challenge signal received");
        return;
    }
    if (mClientNameToKeyId.end() == mClientNameToKeyId.find(match[1])) {
        LOGE("Challenge received from unsupported client: " << std::string(match[1]));
        return;
    }
    std::lock_guard<std::mutex> lock(mChallengesMutex);
    mResponsePendingChallenges[match[1]] = challenge;
}   

FvmErrorCode 
FreshnessValueManagerImplServer::unauthenticatedBroadcast()
{
    auto res = mSignalManager->Publish(mFvmConfAccessor->GetUnauthenticatedFvSignalConfig(), common::UintToByteVector<uint64_t>(mFV));
    if (FvmErrorCode::kSuccess != res) {
        LOGE("Failed broadcasting freshness value");
    }
    else {
        LOGD("Published un-authenticated FV: " << mFV);
        mNeedToBroadcastFv = false;
    }
    return res;
}

FvmErrorCode 
FreshnessValueManagerImplServer::sendAuthenticFvResponses()
{
    std::lock_guard<std::mutex> lock(mChallengesMutex);
    for (auto&& challengeEntry : mResponsePendingChallenges) {
        auto serializedFv = common::UintToByteVectorTrim<uint64_t>(mFV, FVM_SERVER_NUM_OF_BYTES_INITIAL_FV);
        auto data = challengeEntry.second;
        // todo: assuming that the signature is calculated over - challenge + auth FV. needs verification!!
        data.insert(data.end(), serializedFv.begin(), serializedFv.end());
        LOGD("creating authenticator for challenge from ECU: " << challengeEntry.first);
        auto macRes = mCsmAccessor->MacCreate(mClientNameToKeyId[challengeEntry.first], data, common::MacAlgorithm::kAes128Cmac);
        if (macRes.isFailed()) {
            LOGE("Failed creating MAC for response to FV request from ECU: " << challengeEntry.first);
            continue;
        }

        std::vector<uint8_t> macRes8Byte(macRes.getObject().begin(),macRes.getObject().begin() + AUTH_FV_SIGNATURE_SIZE_BYTES);
        LOGD("Sending FV: " << mFV << ", mac: " << common::ByteVectorToUint<uint64_t>(macRes8Byte))
        if (FvmErrorCode::kSuccess != mSignalManager->Publish(mClientConfigMap[challengeEntry.first].clientResponseValueSignal, serializedFv)) {
            LOGE("Failed sending FV signal to ECU: " << challengeEntry.first);
            continue;
        }

        std::string signatureSignal = "SOK_Zeit_" + challengeEntry.first + "_Signature";
        if (FvmErrorCode::kSuccess != mSignalManager->Publish(mClientConfigMap[challengeEntry.first].clientResponseSignatureSignal, macRes8Byte)) {
            LOGE("Failed sending signature signal to ECU: " << challengeEntry.first);
            continue;
        }
        LOGI("Sent FV and signature signals to ECU: " << challengeEntry.first << " successfully");
    }
    mResponsePendingChallenges.clear();
    mNeedToSendAuthFvResponses = false;
    return FvmErrorCode::kSuccess;
}

} // namespace fvm
} // namespace sok