/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/FvmRuntimeAttributesManager.hpp"
#include "sok/fvm/SokFmInternalFactory.hpp"

namespace sok
{
namespace fvm
{

FvmRuntimeAttributesManager::FvmRuntimeAttributesManager() 
: mConfigAccessor(SokFmInternalFactory::CreateFreshnessValueManagerConfigAccessor())
{
}
    
bool 
FvmRuntimeAttributesManager::Init()
{
    auto Ids = mConfigAccessor->GetAllFreshnessValueIds();
    for (auto&& id : Ids) {
        uint8_t sessionCounterLength = 0;
        auto entry = mConfigAccessor->GetSokFvConfigInstanceByFvId(id);
        if (entry.isSucceeded()) {
            sessionCounterLength = entry.getObject().sessionCounterLength;
        }
        mAttributesMap[id] = SokFmRuntimeAttributes{
            /*valueIsActive=*/ false,
            /*sessionCounterLength=*/ sessionCounterLength,
            /*firstUsed=*/ 0,
            /*latestVerifiedTime=*/ 0,
            /*lastRequestedForVerification=*/ 0,
            /*latestSignedTime=*/ 0,
            /*lastRequestedForSigning=*/ 0,
            /*sessionCounterLastUsed=*/ std::vector<uint8_t>(sessionCounterLength, 0)
            };   
    }
    return true;
}

void 
FvmRuntimeAttributesManager::Reset()
{
    mAttributesMap.clear();
}

bool 
FvmRuntimeAttributesManager::IsActive(SokFreshnessValueId fvId) const
{
    auto findRes = mAttributesMap.find(fvId);
    return mAttributesMap.end() != findRes ? findRes->second.valueIsActive : false;
}

void 
FvmRuntimeAttributesManager::SetActive(SokFreshnessValueId fvId, uint64_t activationTime)
{
    auto findRes = mAttributesMap.find(fvId);
    if ((mAttributesMap.end() != findRes) && (false == findRes->second.valueIsActive)) {
        findRes->second.valueIsActive = true;
        findRes->second.firstUsed = activationTime;
    }
}

std::vector<uint8_t> 
FvmRuntimeAttributesManager::IncSessionCounter(SokFreshnessValueId fvId)
{
    auto findRes = mAttributesMap.find(fvId);
    if ((mAttributesMap.end() != findRes) && (0 != findRes->second.sessionCounterLength)) {
        incrementByteArray(findRes->second.sessionCounterLastUsed);
        return findRes->second.sessionCounterLastUsed;
    }
    return {};
}

std::vector<uint8_t> 
FvmRuntimeAttributesManager::GetSessionCounter(SokFreshnessValueId fvId) const
{
    auto findRes = mAttributesMap.find(fvId);
    if ((mAttributesMap.end() != findRes) && (0 != findRes->second.sessionCounterLength)) {
        return findRes->second.sessionCounterLastUsed;
    }
    return {};
}

std::vector<uint8_t> 
FvmRuntimeAttributesManager::GetNextSessionCounter(SokFreshnessValueId fvId) const
{
    auto findRes = mAttributesMap.find(fvId);
    if ((mAttributesMap.end() != findRes) && (0 != findRes->second.sessionCounterLength)) {
        auto counterCopy = findRes->second.sessionCounterLastUsed;
        incrementByteArray(counterCopy);
        return counterCopy;
    }
    return {};
}

void 
FvmRuntimeAttributesManager::UpdateEvent(EventType type, SokFreshnessValueId fvId, uint64_t value)
{
    auto findRes = mAttributesMap.find(fvId);
    if (mAttributesMap.end() == findRes) {
        return;
    }

    switch (type) {
        case EventType::kSignReq:
            findRes->second.lastRequestedForSigning = value;
            break;
        case EventType::kSignSuccess:
            findRes->second.latestSignedTime = value;
            break;
        case EventType::kVerifyReq:
            findRes->second.lastRequestedForVerification = value;
            break;
        case EventType::kVerifySuccess:
            findRes->second.latestVerifiedTime = value;
            break;
        case EventType::kFirstActivity:
            findRes->second.firstUsed = value;
            break;
        default:
            break;
    }
}

uint64_t 
FvmRuntimeAttributesManager::GetEvent(EventType type, SokFreshnessValueId fvId) const
{
    auto findRes = mAttributesMap.find(fvId);
    if (mAttributesMap.end() == findRes) {
        return 0;
    }

    switch (type) {
        case EventType::kSignReq:
            return findRes->second.lastRequestedForSigning;
        case EventType::kSignSuccess:
            return findRes->second.latestSignedTime;
        case EventType::kVerifyReq:
            return findRes->second.lastRequestedForVerification;
        case EventType::kVerifySuccess:
            return findRes->second.latestVerifiedTime;    
        case EventType::kFirstActivity:
            return findRes->second.firstUsed;    
        default:
            return 0;
    }
}

void 
FvmRuntimeAttributesManager::incrementByteArray(std::vector<uint8_t>& vec) const
{
    for (auto it = vec.rbegin() ; it != vec.rend() ; it ++) {
        if (((*it)++) != 0) {
            break;
        }
    }
}

} // namespace fvm
} // namespace sok