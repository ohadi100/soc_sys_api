/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP
#define FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP

#include <vector>
#include "IFreshnessValueManagerConfigAccessor.hpp"
#include "FreshnessValueManagerError.hpp"
#include "FvmConfigParser.hpp"

namespace sok
{
namespace fvm
{

class FreshnessValueManagerConfigAccessor : public IFreshnessValueManagerConfigAccessor
{
public:
    static std::shared_ptr<FreshnessValueManagerConfigAccessor> GetInstance();

    /**
     * @brief Get the SokFvConfigArrayInstance Entry By FvId
     * 
     * @param id id to retrieve the SokFvConfigArrayInstance for
     * @return FvmResult<SokFvConfigArrayInstance> if found, error code otherwise
     */
    FvmResult<SokFvConfigInstance> GetSokFvConfigInstanceByFvId(SokFreshnessValueId id) const override;
    FvmResult<ChallengeConfigInstance> GetChallengeConfigInstanceByFvId(SokFreshnessValueId id) const override;

    FvmResult<SokFreshnessType> GetEntryTypeByFvId(SokFreshnessValueId id) const override;

    bool Init() override;

    /**
     * @brief Get a list of all Freshness value IDs
     * 
     * @return list of all configured FV IDs
     */
    std::vector<SokFreshnessValueId> GetAllFreshnessValueIds() const override;

    std::vector<SokFreshnessValueId> GetAllAuthBroadcastFreshnessValueIds() const override;
    std::vector<SokFreshnessValueId> GetAllChallengeFreshnessValueIds() const override;

    /**
     * @brief Get the Clients Array object (for FVM server)
     * 
     * @return std::vector<SokFvClientConfigArrayInstance> 
     */
    FmServerClientsConfigMap GetClientsConfigMap() const override;

    /**
     * @brief Get the Ecu Name
     * 
     * @return std::string 
     */
    std::string GetEcuName() const override;

    std::string GetNetworkInterface() const override;
    SignalConfig GetUnauthenticatedFvSignalConfig() const override;
    SignalConfig GetAuthenticatedFvValueSignalConfig() const override;
    SignalConfig GetAuthenticatedFvSignatureSignalConfig() const override;
    SignalConfig GetAuthenticatedFvChallengeSignalConfig() const override;

    SokKeyConfig GetSokKeyConfig() const override;
    uint16_t GetEcuKeyIdForFvDistribution() const override;

    FreshnessValueManagerConfigAccessor(const FreshnessValueManagerConfigAccessor&)            = delete;
    FreshnessValueManagerConfigAccessor(FreshnessValueManagerConfigAccessor&&)                 = delete;
    FreshnessValueManagerConfigAccessor& operator=(const FreshnessValueManagerConfigAccessor&) = delete;
    FreshnessValueManagerConfigAccessor& operator=(FreshnessValueManagerConfigAccessor&&)      = delete;

private:
    /*
     * Singleton class private constructor
     */
    FreshnessValueManagerConfigAccessor();

private:
    FvmConfigParser mParser;
    SokFmConfig mConfig;
    bool mInitialized;
};

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP