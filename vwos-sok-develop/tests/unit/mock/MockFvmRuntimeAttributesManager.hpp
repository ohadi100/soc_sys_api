/* Copyright (c) 2023 Volkswagen Group */

#ifndef MOCK_FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP
#define MOCK_FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP

#include <gmock/gmock.h>
#include "sok/fvm/IFvmRuntimeAttributesManager.hpp"

namespace sok
{
namespace fvm    
{

class MockFvmRuntimeAttributesManager : public IFvmRuntimeAttributesManager
{
public:
    MOCK_METHOD(bool, Init, (), (override));
    MOCK_METHOD(void, Reset, (), (override));
    MOCK_METHOD(bool, IsActive, (SokFreshnessValueId), (const, override));
    MOCK_METHOD(void, SetActive, (SokFreshnessValueId, uint64_t), (override));
    MOCK_METHOD(std::vector<uint8_t>, IncSessionCounter, (SokFreshnessValueId), (override));
    MOCK_METHOD(std::vector<uint8_t>, GetSessionCounter, (SokFreshnessValueId), (const, override));
    MOCK_METHOD(std::vector<uint8_t>, GetNextSessionCounter, (SokFreshnessValueId), (const, override));
    MOCK_METHOD(void, UpdateEvent, (EventType, SokFreshnessValueId, uint64_t), (override));
    MOCK_METHOD(uint64_t, GetEvent, (EventType, SokFreshnessValueId), (const, override));
};

class UTFvmRuntimeAttributesManager : public IFvmRuntimeAttributesManager
{
public:

    bool 
    Init() override
    {
        return mMockFvmAttrMgr->Init();
    }

    void 
    Reset() override
    {
        mMockFvmAttrMgr->Reset();
    }

    bool 
    IsActive(SokFreshnessValueId fvId) const override
    {
        return mMockFvmAttrMgr->IsActive(fvId);
    }

    void 
    SetActive(SokFreshnessValueId fvId, uint64_t  activationTime) override
    {
        mMockFvmAttrMgr->SetActive(fvId, activationTime);
    }

    std::vector<uint8_t> 
    IncSessionCounter(SokFreshnessValueId fvId) override
    {
        return mMockFvmAttrMgr->IncSessionCounter(fvId);
    }

    std::vector<uint8_t> 
    GetSessionCounter(SokFreshnessValueId fvId) const override
    {
        return mMockFvmAttrMgr->GetSessionCounter(fvId);
    }

    std::vector<uint8_t> 
    GetNextSessionCounter(SokFreshnessValueId fvId) const override
    {
        return mMockFvmAttrMgr->GetNextSessionCounter(fvId);
    }

    void 
    UpdateEvent(EventType type, SokFreshnessValueId fvId, uint64_t value) override
    {
        mMockFvmAttrMgr->UpdateEvent(type, fvId, value);
    }

    uint64_t 
    GetEvent(EventType type, SokFreshnessValueId fvId) const override
    {
        return mMockFvmAttrMgr->GetEvent(type, fvId);
    }

    static MockFvmRuntimeAttributesManager* mMockFvmAttrMgr;
};

}  // namespace fvm
}  // namespace sok

#endif  // MOCK_FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP