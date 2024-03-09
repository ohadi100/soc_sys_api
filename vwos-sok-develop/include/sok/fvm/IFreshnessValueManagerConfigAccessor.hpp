/* Copyright (c) 2023 Volkswagen Group */

#ifndef I_FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP
#define I_FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP

#include <vector>
#include "FreshnessValueManagerDefinitions.hpp"
#include "FreshnessValueManagerError.hpp"

namespace sok
{
namespace fvm
{

class IFreshnessValueManagerConfigAccessor
{
public:
    virtual ~IFreshnessValueManagerConfigAccessor() = default;

    virtual bool Init() = 0;
    
    /**
     * @brief Get the SokFvConfigArrayInstance Entry By FvId
     * 
     * @param id id to retrieve the SokFvConfigArrayInstance for
     * @return FvmResult<SokFvConfigArrayInstance> if found, error code otherwise
     */
    virtual FvmResult<SokFvConfigInstance> GetSokFvConfigInstanceByFvId(SokFreshnessValueId id) const = 0;
    virtual FvmResult<ChallengeConfigInstance> GetChallengeConfigInstanceByFvId(SokFreshnessValueId id) const = 0;

    virtual FvmResult<SokFreshnessType> GetEntryTypeByFvId(SokFreshnessValueId id) const = 0;

    /**
     * @brief Get a list of all Freshness value IDs
     * 
     * @return list of all configured FV IDs
     */
    virtual std::vector<SokFreshnessValueId> GetAllFreshnessValueIds() const = 0;
    virtual std::vector<SokFreshnessValueId> GetAllAuthBroadcastFreshnessValueIds() const = 0;
    virtual std::vector<SokFreshnessValueId> GetAllChallengeFreshnessValueIds() const = 0;

    /**
     * @brief Get the Clients Array object (for FVM server)
     * 
     * @return std::vector<SokFvClientConfigArrayInstance> 
     */
    virtual FmServerClientsConfigMap GetClientsConfigMap() const = 0;

    /**
     * @brief Get the Ecu Name
     * 
     * @return std::string 
     */
    virtual std::string GetEcuName() const = 0;

    virtual std::string GetNetworkInterface() const = 0;
    virtual SignalConfig GetUnauthenticatedFvSignalConfig() const = 0;
    virtual SignalConfig GetAuthenticatedFvValueSignalConfig() const = 0;
    virtual SignalConfig GetAuthenticatedFvSignatureSignalConfig() const = 0;
    virtual SignalConfig GetAuthenticatedFvChallengeSignalConfig() const = 0;

    virtual SokKeyConfig GetSokKeyConfig() const = 0;
    virtual uint16_t GetEcuKeyIdForFvDistribution() const = 0;

};

} // namespace fvm
} // namespace sok

#endif // I_FRESHNESS_VALUE_MANAGER_CONFIG_ARRAY_ACCESSOR_HPP