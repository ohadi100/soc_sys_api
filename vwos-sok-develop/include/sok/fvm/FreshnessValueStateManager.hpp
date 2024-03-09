/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_STATE_MANAGER_HPP
#define FRESHNESS_VALUE_STATE_MANAGER_HPP

#include "AFreshnessValueManagerImpl.hpp"
#include <unordered_map>

namespace sok
{
namespace fvm
{

using StateAction = std::function<FvmErrorCode()>; 

enum class FreshnessValueState : uint8_t {
    RequestFV, 
    FVInProgress, 
    ProcessFV, 
    ProcessAnauthFV,
    Idle
};

class FreshnessValueStateManager {
public:
    FreshnessValueStateManager();
    virtual ~FreshnessValueStateManager() = default;

    /**
     * @brief enter the state, run the state action function
     * 
     * @return FvmErrorCode
     */
    FvmErrorCode enter();

    /**
     * @brief transit between states, assign the new state to mCurrentFvState
     * 
     * @param state FreshnessValueState enum describing the different states 
     */
    void transiteTo(FreshnessValueState const state);

    /**
     * @brief register a new state
     * 
     * @param state FreshnessValueState enum describing the different states 
     * @param action the state action function, will be called by enter()
     */
    void registerState(FreshnessValueState const state, StateAction const& action);
    
    /**
     * @brief react to an incoming authentic fv response according to the current state
     */
    void reactToFVRes();

    /**
     * @brief react to an incoming unauthentic fv response according to the current state
     */
    void reactToUnauthenticFVRes();

private:
    std::unordered_map<FreshnessValueState, StateAction> mFvActions;
    std::shared_ptr<FreshnessValueState> mCurrentFvState;
};

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_STATE_MANAGER_HPP