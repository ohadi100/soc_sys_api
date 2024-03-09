/* Copyright (c) 2023 Volkswagen Group */

#ifndef I_FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP
#define I_FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP

#include "FreshnessValueManagerDefinitions.hpp"

namespace sok
{
namespace fvm
{

class IFvmRuntimeAttributesManager
{
public:
    enum class EventType : uint8_t {
        kSignReq = 0,
        kSignSuccess,
        kVerifyReq,
        kVerifySuccess,
        kVwSokFreshnessCrResponse,
        kFirstActivity,

        kEndEnum
    };
    
    virtual ~IFvmRuntimeAttributesManager() = default;
    /**
     * @brief Initialize the runtime attributes based on a predefined config
     * 
     * @return true on success
     * @return false on failure
     */
    virtual bool Init() = 0;

    /**
     * @brief Reset the current state
     * 
     */
    virtual void Reset() = 0;

    /**
     * @brief 
     * 
     * @param fvId ID to activate the attributes for
     * @return true 
     * @return false 
     */
    virtual bool IsActive(SokFreshnessValueId fvId) const = 0;

    /**
     * @brief Activates the entry for the given ID
     * 
     * @param fvId ID to activate the attributes for
     * @param activationTime integer representation of the activation time
     */
    virtual void SetActive(SokFreshnessValueId fvId, uint64_t activationTime) = 0;

    /**
     * @brief Increments the session counter for the provided ID
     * 
     * @param fvId ID to increment the session counter for
     * @return std::vector<uint8_t> serialized vector of the incremented counter
     */
    virtual std::vector<uint8_t> IncSessionCounter(SokFreshnessValueId fvId) = 0;

    /**
     * @brief Gets the session counter for the provided ID
     * 
     * @param fvId ID to get the session counter for
     * @return std::vector<uint8_t> serialized vector of the counter
     */
    virtual std::vector<uint8_t> GetSessionCounter(SokFreshnessValueId fvId) const = 0;

    /**
     * @brief Gets the next session counter for the provided ID
     * 
     * @param fvId ID to get the session counter for
     * @return std::vector<uint8_t> serialized vector of the counter + 1
     */
    virtual std::vector<uint8_t> GetNextSessionCounter(SokFreshnessValueId fvId) const = 0;

    /**
     * @brief Updates the value of a runtime attribute
     * 
     * @param type the type of the attribute to update
     * @param fvId the ID for the specific attribute
     * @param value the new value
     */
    virtual void UpdateEvent(EventType type, SokFreshnessValueId fvId, uint64_t value) = 0;

    /**
     * @brief Gets the value of a runtime attribute
     * 
     * @param type the type of the attribute
     * @param fvId the ID for the specific attribute
     * @return uint64_t the value of the attribute for the specif ID
     */
    virtual uint64_t GetEvent(EventType type, SokFreshnessValueId fvId) const = 0;
};

} // namespace fvm
} // namespace sok

#endif // I_FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP