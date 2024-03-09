/* Copyright (c) 2023 Volkswagen Group */

#ifndef MOCK_FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP
#define MOCK_FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP

#include <gmock/gmock.h>
#include "sok/fvm/IFreshnessValueManagerConfigAccessor.hpp"
#include "sok/fvm/FreshnessValueManagerConstants.hpp"

namespace sok
{
namespace fvm    
{

class MockFreshnessValueManagerConfigAccessor : public IFreshnessValueManagerConfigAccessor
{
public:
    MOCK_METHOD(bool, Init, (), (override));
    MOCK_METHOD(FvmResult<SokFvConfigInstance>, GetSokFvConfigInstanceByFvId, (SokFreshnessValueId), (const, override));
    MOCK_METHOD(FvmResult<ChallengeConfigInstance>, GetChallengeConfigInstanceByFvId, (SokFreshnessValueId), (const, override));
    MOCK_METHOD(FvmResult<SokFreshnessType>, GetEntryTypeByFvId, (SokFreshnessValueId), (const, override));
    MOCK_METHOD(std::vector<SokFreshnessValueId>, GetAllFreshnessValueIds, (), (const, override));
    MOCK_METHOD(std::vector<SokFreshnessValueId>, GetAllAuthBroadcastFreshnessValueIds, (), (const, override));
    MOCK_METHOD(std::vector<SokFreshnessValueId>, GetAllChallengeFreshnessValueIds, (), (const, override));
    MOCK_METHOD(FmServerClientsConfigMap, GetClientsConfigMap, (), (const, override));
    MOCK_METHOD(std::string, GetEcuName, (), (const, override));
    MOCK_METHOD(std::string, GetNetworkInterface, (), (const, override));
    MOCK_METHOD(SignalConfig, GetUnauthenticatedFvSignalConfig, (), (const, override));
    MOCK_METHOD(SignalConfig, GetAuthenticatedFvValueSignalConfig, (), (const, override));
    MOCK_METHOD(SignalConfig, GetAuthenticatedFvSignatureSignalConfig, (), (const, override));
    MOCK_METHOD(SignalConfig, GetAuthenticatedFvChallengeSignalConfig, (), (const, override));
    MOCK_METHOD(SokKeyConfig, GetSokKeyConfig, (), (const, override));
    MOCK_METHOD(uint16_t, GetEcuKeyIdForFvDistribution, (), (const, override));
};

class UTFreshnessValueManagerConfigAccessor : public IFreshnessValueManagerConfigAccessor
{
public:
    bool
    Init() override
    {
        return mMockFvConfAccessor->Init();
    }

    FvmResult<SokFvConfigInstance> 
    GetSokFvConfigInstanceByFvId(SokFreshnessValueId id) const override
    {
        return mMockFvConfAccessor->GetSokFvConfigInstanceByFvId(id);
    }
    FvmResult<ChallengeConfigInstance> 
    GetChallengeConfigInstanceByFvId(SokFreshnessValueId id) const override
    {
        return mMockFvConfAccessor->GetChallengeConfigInstanceByFvId(id);
    }
    FvmResult<SokFreshnessType> 
    GetEntryTypeByFvId(SokFreshnessValueId id) const override
    {
        return mMockFvConfAccessor->GetEntryTypeByFvId(id);
    }
    std::vector<SokFreshnessValueId> 
    GetAllFreshnessValueIds() const override
    {
        return mMockFvConfAccessor->GetAllFreshnessValueIds();
    }
    std::vector<SokFreshnessValueId> 
    GetAllAuthBroadcastFreshnessValueIds() const override
    {
        return mMockFvConfAccessor->GetAllAuthBroadcastFreshnessValueIds();
    }
    std::vector<SokFreshnessValueId> 
    GetAllChallengeFreshnessValueIds() const override
    {
        return mMockFvConfAccessor->GetAllChallengeFreshnessValueIds();
    }
    FmServerClientsConfigMap 
    GetClientsConfigMap() const override
    {
        return mMockFvConfAccessor->GetClientsConfigMap();
    }
    std::string 
    GetEcuName() const override
    {
        return mMockFvConfAccessor->GetEcuName();
    }
    std::string 
    GetNetworkInterface() const override
    {
        return mMockFvConfAccessor->GetNetworkInterface();
    }
    SignalConfig 
    GetUnauthenticatedFvSignalConfig() const override
    {
        return mMockFvConfAccessor->GetUnauthenticatedFvSignalConfig();
    }
    SignalConfig 
    GetAuthenticatedFvValueSignalConfig() const override
    {
        return mMockFvConfAccessor->GetAuthenticatedFvValueSignalConfig();
    }
    SignalConfig 
    GetAuthenticatedFvSignatureSignalConfig() const override
    {
        return mMockFvConfAccessor->GetAuthenticatedFvSignatureSignalConfig();
    }
    SignalConfig 
    GetAuthenticatedFvChallengeSignalConfig() const override
    {
        return mMockFvConfAccessor->GetAuthenticatedFvChallengeSignalConfig();
    }

    SokKeyConfig GetSokKeyConfig() const override 
    {
        return mMockFvConfAccessor->GetSokKeyConfig();
    }
    uint16_t GetEcuKeyIdForFvDistribution() const override 
    {
        return mMockFvConfAccessor->GetEcuKeyIdForFvDistribution();
    }

    static MockFreshnessValueManagerConfigAccessor* mMockFvConfAccessor;
};

}  // namespace fvm
}  // namespace sok

#endif  // MOCK_FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP