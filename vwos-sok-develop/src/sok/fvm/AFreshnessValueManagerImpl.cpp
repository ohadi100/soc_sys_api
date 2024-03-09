/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/AFreshnessValueManagerImpl.hpp"
#include "sok/fvm/FreshnessValueManagerConstants.hpp"
#include "sok/common/SokUtilities.hpp"
#include "sok/common/SokCommonInternalFactory.hpp"
#include "sok/fvm/SokFmInternalFactory.hpp"
#include "sok/common/Logger.hpp"

namespace sok
{
namespace fvm
{

AFreshnessValueManagerImpl::AFreshnessValueManagerImpl()
: mInitialized(false)
, mIsFvValid(false)
, mTimeSinceInit(0)
, mClockCount(0)
, mFV(0)
, mActiveOutgoingChallenges()
, mActiveIncomingChallenges()
, mChallengeSignalToFvId()
, mFvRxCandidates()
, mCsmAccessor(common::SokCommonInternalFactory::CreateCsmAccessor())
, mSignalManager(SokFmInternalFactory::CreateSignalManager())
, mAttrMgr(SokFmInternalFactory::CreateFvmRuntimeAttributesManager())
, mFvmConfAccessor(SokFmInternalFactory::CreateFreshnessValueManagerConfigAccessor())
{
}

FvmResult<FVContainer>
AFreshnessValueManagerImpl::getChallengeForIncomingResponse(SokFreshnessValueId SecOCFreshnessValueID, FVContainer const& SecOCTruncatedFreshnessValue)
{
    (void)SecOCTruncatedFreshnessValue;
    auto activeChFindRes = mActiveOutgoingChallenges.find(SecOCFreshnessValueID);
    if (mActiveOutgoingChallenges.end() == activeChFindRes) {
        LOGE("No active challenge found for freshness value ID: " << SecOCFreshnessValueID);
        return FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
    }
    //todo: check if challenge is within timeout
    //todo: session counter for challenge
    LOGD("Returning challenge for incoming response");
    return FvmResult<FVContainer>(activeChFindRes->second.second);
}

FvmResult<FVContainer>
AFreshnessValueManagerImpl::getChallengeForOutgoingResponse(SokFreshnessValueId SecOCFreshnessValueID)
{
    auto activeChFindRes = mActiveIncomingChallenges.find(SecOCFreshnessValueID);
    if (mActiveIncomingChallenges.end() == activeChFindRes) {
        LOGE("No active challenge found for freshness value ID: " << SecOCFreshnessValueID);
        return FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
    }
    auto ret = activeChFindRes->second;
    //todo: session counter for challenge

    // challenge can now be discarded for replay protection
    mActiveIncomingChallenges.erase(activeChFindRes);
    LOGD("Returning challenge for outgoing response");
    return FvmResult<FVContainer>(ret.second);
}


FvmResult<FVContainer>
AFreshnessValueManagerImpl::getTxFv(SokFreshnessValueId SecOCFreshnessValueID, SokFreshnessType type)
{
    FVContainer ret;
    if (!mAttrMgr->IsActive(SecOCFreshnessValueID)) {
        // this is the first occurrence for this ID
        mAttrMgr->SetActive(SecOCFreshnessValueID, mTimeSinceInit);
        ret = common::UintToByteVector<uint64_t>(SOK_UPSTART_TIME);
    }
    else if (!mIsFvValid) {
        if (mTimeSinceInit <= SOK_FM_TIME_VALID_TIMEOUT_MS) {
            ret = common::UintToByteVector<uint64_t>(SOK_UPSTART_TIME);
        }
        else {
            LOGE("No valid Fv for the FvID: " << SecOCFreshnessValueID);
            ret = common::UintToByteVector<uint64_t>(SOK_INVALID_TIME);
        }
    }
    else {
        ret = common::UintToByteVector<uint64_t>(mFV);
        mAttrMgr->UpdateEvent(IFvmRuntimeAttributesManager::EventType::kSignReq, SecOCFreshnessValueID, mFV);
    }

    if (SokFreshnessType::kVwSokFreshnessValueSessionSender == type) {
        auto serializedCounter = mAttrMgr->IncSessionCounter(SecOCFreshnessValueID);
        ret.insert(ret.end(), serializedCounter.begin(), serializedCounter.end());
    }
    
    return FvmResult<FVContainer>(ret);
}

FvmResult<FVContainer>
AFreshnessValueManagerImpl::getRxFv(SokFreshnessValueId SecOCFreshnessValueID, SokFreshnessType type, FVContainer const& SecOCTruncatedFreshnessValue, uint16_t SecOCAuthVerifyAttempts)
{
    FVContainer ret;
    // Check if FV candidate list was built for this Fv Id
    auto rxCandList = mFvRxCandidates.find(SecOCFreshnessValueID);
    if (mFvRxCandidates.end() != rxCandList) {
        if (SecOCAuthVerifyAttempts >= rxCandList->second.size()) {
            LOGE("Invalid amount of verification attempts");
            return FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
        }
        else {
            ret = common::UintToByteVector<uint64_t>(rxCandList->second[SecOCAuthVerifyAttempts]);
            ret.insert(ret.end(), SecOCTruncatedFreshnessValue.begin(), SecOCTruncatedFreshnessValue.end());
            return FvmResult<FVContainer>(ret);
        }
    }
    // If there is no candidate list this must be the first attempt
    if (0 != SecOCAuthVerifyAttempts) {
        LOGE("Invalid amount of verification attempts");
        return FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
    }
    // if there is a session counter - check it and increment
    if (SokFreshnessType::kVwSokFreshnessValueSessionReceiver == type) {
        if (SecOCTruncatedFreshnessValue != mAttrMgr->GetNextSessionCounter(SecOCFreshnessValueID)) {
            LOGE("Session counter mismatch");
            return FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
        }
        mAttrMgr->IncSessionCounter(SecOCFreshnessValueID);
    }
    if (!mIsFvValid && (mTimeSinceInit > SOK_FM_TIME_VALID_TIMEOUT_MS)) {
        LOGE("No auth FV available");
        return FvmResult<FVContainer>(FvmErrorCode::kFVNotAvailable);
    }

    // build FV candidate list
    bool firstOccurrence = false;
    if (false == mAttrMgr->IsActive(SecOCFreshnessValueID)) {
        firstOccurrence = true;
        mAttrMgr->SetActive(SecOCFreshnessValueID, mTimeSinceInit);
    }
    // if we don't have valid FV and we are within the valid timeout return default FV
    if (!mIsFvValid && (mTimeSinceInit <= SOK_FM_TIME_VALID_TIMEOUT_MS)) {
        mFvRxCandidates[SecOCFreshnessValueID] = {SOK_UPSTART_TIME};
    }
    else if (mIsFvValid) {
        if (firstOccurrence) {
            mFvRxCandidates[SecOCFreshnessValueID] = {SOK_UPSTART_TIME, mFV, mFV-1, mFV+1};
        }
        else {
            auto firstActivity = mAttrMgr->GetEvent(IFvmRuntimeAttributesManager::EventType::kFirstActivity, SecOCFreshnessValueID);
            if (mFV < firstActivity) {
                return FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
            }
            if ((mTimeSinceInit - firstActivity) <= SOK_FM_TIME_VALID_TIMEOUT_MS) {
                mFvRxCandidates[SecOCFreshnessValueID] = {SOK_UPSTART_TIME, mFV, mFV-1, mFV+1};
            }
            else {
                mFvRxCandidates[SecOCFreshnessValueID] = {mFV, mFV-1, mFV+1};
            }
        }
    }
    
    mAttrMgr->UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, SecOCFreshnessValueID, mFV);
    ret = common::UintToByteVector<uint64_t>(mFvRxCandidates[SecOCFreshnessValueID][SecOCAuthVerifyAttempts]);
    ret.insert(ret.end(), SecOCTruncatedFreshnessValue.begin(), SecOCTruncatedFreshnessValue.end());
    return FvmResult<FVContainer>(ret);
}

FvmResult<FVContainer> 
AFreshnessValueManagerImpl::GetRxFreshness(SokFreshnessValueId SecOCFreshnessValueID, FVContainer const& SecOCTruncatedFreshnessValue, uint16_t SecOCAuthVerifyAttempts) noexcept
{
    LOGD("AFreshnessValueManagerImpl::GetRxFreshness");
    FvmResult<FVContainer> ret({});
    try {
        if (!mInitialized) {
            return FvmResult<FVContainer>(FvmErrorCode::kNotInitialized);
        }
        auto getEntryTypeRes = mFvmConfAccessor->GetEntryTypeByFvId(SecOCFreshnessValueID);
        if (getEntryTypeRes.isFailed()) {
            LOGE("Failed getting the config entry for the freshness value ID: " << SecOCFreshnessValueID);
            return FvmResult<FVContainer>(FvmErrorCode::kFvIdNotFound);
        }

        switch(getEntryTypeRes.getObject()) {
            case SokFreshnessType::kVwSokFreshnessCrChallenge:
                if (0 != SecOCAuthVerifyAttempts) {
                    LOGE("Invalid amount of verification attempts for FV ID of challenge type");
                    ret = FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
                    break;
                }
                ret = getChallengeForIncomingResponse(SecOCFreshnessValueID, SecOCTruncatedFreshnessValue);
                break;
            case SokFreshnessType::kVwSokFreshnessValueSessionReceiver: // fallthrough
            case SokFreshnessType::kVwSokFreshnessValue:
                ret = getRxFv(SecOCFreshnessValueID, getEntryTypeRes.getObject(), SecOCTruncatedFreshnessValue, SecOCAuthVerifyAttempts);
                break;
            default:
                LOGE("Freshness ID is not supported for Rx freshness: " << SecOCFreshnessValueID);
                ret = FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
                break;
        }
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        ret = FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
    } catch (...) {
        LOGE("exception");
        ret = FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
    }
    return ret;
}

FvmResult<FVContainer> 
AFreshnessValueManagerImpl::GetTxFreshness(SokFreshnessValueId SecOCFreshnessValueID) noexcept
{
    LOGD("AFreshnessValueManagerImpl::GetTxFreshness");
    FvmResult<FVContainer> ret({});
    try {
        if (!mInitialized) {
            return FvmResult<FVContainer>(FvmErrorCode::kNotInitialized);
        }
        auto getEntryTypeRes = mFvmConfAccessor->GetEntryTypeByFvId(SecOCFreshnessValueID);
        if (getEntryTypeRes.isFailed()) {
            LOGE("Failed getting the config entry for the freshness value ID: " << SecOCFreshnessValueID);
            return FvmResult<FVContainer>(FvmErrorCode::kFvIdNotFound);
        }

        switch(getEntryTypeRes.getObject()) {
            case SokFreshnessType::kVwSokFreshnessCrResponse:
                ret = getChallengeForOutgoingResponse(SecOCFreshnessValueID);
                break;
            case SokFreshnessType::kVwSokFreshnessValueSessionSender: // fallthrough
            case SokFreshnessType::kVwSokFreshnessValue:
                ret = getTxFv(SecOCFreshnessValueID, getEntryTypeRes.getObject());
                break;
            default:
                LOGE("Freshness ID is not supported for Tx freshness: " << SecOCFreshnessValueID);
                ret = FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
                break;
        }
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        ret = FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
    } catch (...) {
        LOGE("exception");
        ret = FvmResult<FVContainer>(FvmErrorCode::kGeneralError);
    }
    return ret;
}

void 
AFreshnessValueManagerImpl::VerificationStatusCallout(SecOC_VerificationStatusType verificationStatus) noexcept
{
    try {
        auto getEntryTypeRes = mFvmConfAccessor->GetEntryTypeByFvId(verificationStatus.fvId);
        if (getEntryTypeRes.isFailed()) {
            LOGE("Failed getting the config entry for the freshness value ID: " << verificationStatus.fvId);
            return;
        }
        switch(getEntryTypeRes.getObject()) {
            case SokFreshnessType::kVwSokFreshnessCrChallenge:
                updateVerificationStatusChallenge(verificationStatus.fvId, verificationStatus.verificationSucceeded);
                break;
            case SokFreshnessType::kVwSokFreshnessValueSessionReceiver: // fallthrough
            case SokFreshnessType::kVwSokFreshnessValue:
                updateVerificationStatusAuthBroadcast(verificationStatus.fvId, verificationStatus.verificationSucceeded);
                break;
            default:
                LOGE("Freshness ID is not supported for Rx freshness: " << verificationStatus.fvId);
                break;
        }
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
    } catch (...) {
        LOGE("exception");
    }
}

void 
AFreshnessValueManagerImpl::SPduTxConfirmation(SokFreshnessValueId SecOCFreshnessValueID) noexcept
{
    try {
        auto lastSignReq = mAttrMgr->GetEvent(IFvmRuntimeAttributesManager::EventType::kSignReq, SecOCFreshnessValueID);
        mAttrMgr->UpdateEvent(IFvmRuntimeAttributesManager::EventType::kSignSuccess, SecOCFreshnessValueID, lastSignReq);
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
    } catch (...) {
        LOGE("exception");
    }
}

FvmErrorCode 
AFreshnessValueManagerImpl::Init() noexcept
{
    try {
        if (mInitialized) {
            return FvmErrorCode::kAlreadyInitialized;
        }
        if (!mFvmConfAccessor->Init()) {
            return FvmErrorCode::kGeneralError;
        }

        auto keyConfig = mFvmConfAccessor->GetSokKeyConfig();
        for (auto&& keyId : keyConfig) {
            if (common::CsmErrorCode::kSuccess != mCsmAccessor->IsKeyExists(keyId.second)) {
                LOGE("Key with id: " << keyId.second << ", was not found");
                return FvmErrorCode::kKeyNotFound;
            }
        }

        if (!mAttrMgr->Init()) {
            return FvmErrorCode::kGeneralError;
        }

        if (!registerToSignals()) {
            return FvmErrorCode::kGeneralError;
        }

        if (!serverOrParticipantInit()) {
            return FvmErrorCode::kGeneralError;
        }

        mInitialized = true;

        return FvmErrorCode::kSuccess;
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        return FvmErrorCode::kGeneralError;
    } catch (...) {
        LOGE("exception");
        return FvmErrorCode::kGeneralError;
    }
}

FvmErrorCode 
AFreshnessValueManagerImpl::Deinit() noexcept
{
    try {
        if (!mInitialized) {
            LOGW("Fvm is not initialized, nothing to deinit");
            return FvmErrorCode::kSuccess;
        }
        mInitialized = false;
        mAttrMgr->Reset();
        mIsFvValid = false;
        mTimeSinceInit = 0;
        mClockCount = 0;
        mFV = 0;
        mChallengeSignalToFvId.clear();
        mFvRxCandidates.clear();
        return FvmErrorCode::kSuccess;
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
        return FvmErrorCode::kGeneralError;
    } catch (...) {
        LOGE("exception");
        return FvmErrorCode::kGeneralError;
    }
}

FvmErrorCode 
AFreshnessValueManagerImpl::TriggerCrRequest(SokFreshnessValueId SecOCFreshnessValueID)
{
    auto activeChFindRes = mActiveOutgoingChallenges.find(SecOCFreshnessValueID);
    if (mActiveOutgoingChallenges.end() != activeChFindRes && ((mTimeSinceInit - activeChFindRes->second.first) < SOK_FM_CHALLENGE_TIMEOUT_MS)) {
        LOGE("Active challenge for this FvId is still undergoing, previous challenge triggered: " << (mTimeSinceInit - activeChFindRes->second.first) << " MS ago");
        // todo: should be handled differently?
        return FvmErrorCode::kGeneralError;
    }

    auto getChEntryRes = mFvmConfAccessor->GetChallengeConfigInstanceByFvId(SecOCFreshnessValueID);
    if (getChEntryRes.isFailed()) {
        LOGE("Freshness value ID: " << SecOCFreshnessValueID << ", not supported for CR triggering");
        return FvmErrorCode::kFvIdNotFound;
    }

    auto genRes = mCsmAccessor->GenerateRandomBytes(CHALLENGE_LENGTH_BYTES);
    if (genRes.isFailed()) {
        LOGE("Failed generating random bytes for a challenge");
        return FvmErrorCode::kRngError;
    }
    
    if (FvmErrorCode::kSuccess != mSignalManager->Publish(getChEntryRes.getObject().challengeSignalConfig, genRes.getObject())) {
        LOGE("Failed publishing challenge signal");
        // todo: Retry?
        return FvmErrorCode::kGeneralError;
    }
    LOGI("Triggered challenge with ID: " << SecOCFreshnessValueID << " successfully, challenge: " << common::ByteVectorToUint<uint64_t>(genRes.getObject()));
    mActiveOutgoingChallenges[SecOCFreshnessValueID] = {mTimeSinceInit, genRes.getObject()};
    return FvmErrorCode::kSuccess;
}

FvmErrorCode 
AFreshnessValueManagerImpl::OfferCrRequest(SokFreshnessValueId SecOCFreshnessValueID, ChallengeReceivedIndicationCb const& cb)
{
    auto findRes = mFvmConfAccessor->GetEntryTypeByFvId(SecOCFreshnessValueID);
    if (findRes.isFailed() || (SokFreshnessType::kVwSokFreshnessCrResponse != findRes.getObject())) {
        LOGE("FV ID: " << SecOCFreshnessValueID << ", is not supported or not of CR response type");
        return FvmErrorCode::kFvIdNotFound;
    }
    mFvIdToNotificationCallback[SecOCFreshnessValueID] = cb;
    return FvmErrorCode::kSuccess;
}

void
AFreshnessValueManagerImpl::incTimers() noexcept
{
    try {
        if (!mInitialized) {
            LOGE("Fvm is not initialized");
            return;
        }
        mTimeSinceInit += SOK_FM_MAIN_FUNCTION_PERIOD_MS;
        mClockCount += SOK_FM_MAIN_FUNCTION_PERIOD_MS;
        if (mIsFvValid) {
            if (SOK_FM_TIME_INCREMENT_PERIOD_MS == mClockCount) {
                mClockCount = 0;
                mFV++;
            }
        }
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
    } catch (...) {
        LOGE("exception");
    }
}

bool 
AFreshnessValueManagerImpl::registerToSignals() noexcept
{
    try {
        auto Ids = mFvmConfAccessor->GetAllChallengeFreshnessValueIds();
        auto cb = [this](std::string const& signal, std::vector<uint8_t> const& value) {
            this->incomingChallengeSignalCb(signal, value);
        };
        for (auto&& id : Ids) {
            auto confRes = mFvmConfAccessor->GetChallengeConfigInstanceByFvId(id);
            if (SokFreshnessType::kVwSokFreshnessCrResponse == confRes.getObject().type) {
                if (FvmErrorCode::kSuccess != mSignalManager->Subscribe(confRes.getObject().challengeSignalConfig, cb)) {
                    LOGE("Failed registering for signal: " << confRes.getObject().challengeSignalConfig.name);
                    continue;
                }
                mChallengeSignalToFvId[confRes.getObject().challengeSignalConfig.name] = id;
            }
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
AFreshnessValueManagerImpl::incomingChallengeSignalCb(std::string const& signal, std::vector<uint8_t> const& challenge)
{
    if (CHALLENGE_LENGTH_BYTES != challenge.size()) {
        LOGE("Invalid challenge size: " << challenge.size());
        return;
    }
    auto fvIdRes = mChallengeSignalToFvId.find(signal);
    if (mChallengeSignalToFvId.end() == fvIdRes) {
        LOGE("Received unregistered signal: " << signal);
        return;
    }
    LOGI("Received challenge signal: " << signal << ", connected with FV ID: " << fvIdRes->second);

    auto cbFindRes = mFvIdToNotificationCallback.find(fvIdRes->second);
    if (mFvIdToNotificationCallback.end() == cbFindRes) {
        LOGE("No app notification callback was registered for challenge signal: " << signal);
        return;
    }

    auto activeChFindRes = mActiveIncomingChallenges.find(fvIdRes->second);
    if (mActiveIncomingChallenges.end() != activeChFindRes && ((mTimeSinceInit - activeChFindRes->second.first) < SOK_FM_CHALLENGE_TIMEOUT_MS)) {
        LOGE("Active challenge for this FvId is still undergoing, previous challenge triggered: " << (mTimeSinceInit - activeChFindRes->second.first) << " MS ago");
        // todo: should be handled differently?
        return;
    }
    mActiveIncomingChallenges[fvIdRes->second] = {mTimeSinceInit, challenge};
    LOGD("Triggering user's CB for incoming challenge: " << common::ByteVectorToUint<uint64_t>(challenge));
    cbFindRes->second(fvIdRes->second);
    LOGD("User's CB execution ended");
}

void 
AFreshnessValueManagerImpl::updateVerificationStatusChallenge(SokFreshnessValueId SecOCFreshnessValueID, bool verificationSucceeded)
{
    if (verificationSucceeded) {
        LOGD("Received confirmation for verification of a challenge response");
        mActiveOutgoingChallenges.erase(SecOCFreshnessValueID);
    }
}

void 
AFreshnessValueManagerImpl::updateVerificationStatusAuthBroadcast(SokFreshnessValueId SecOCFreshnessValueID, bool verificationSucceeded)
{
    if (!verificationSucceeded) {
        // todo: diagnostics stuff
    }
    else {
        auto lastVerifyReq = mAttrMgr->GetEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, SecOCFreshnessValueID);
        mAttrMgr->UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifySuccess, SecOCFreshnessValueID, lastVerifyReq);
        mFvRxCandidates.erase(SecOCFreshnessValueID);
    }
}

} // namespace fvm
} // namespace sok