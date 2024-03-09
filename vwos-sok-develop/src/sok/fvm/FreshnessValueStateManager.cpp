/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/FreshnessValueStateManager.hpp"
#include "sok/common/Logger.hpp"

namespace sok
{
namespace fvm
{

FreshnessValueStateManager::FreshnessValueStateManager() : mFvActions(), mCurrentFvState(nullptr) 
{
}

FvmErrorCode FreshnessValueStateManager::enter() {

    auto ret = FvmErrorCode::kGeneralError;

    if (!mCurrentFvState){ 
        LOGE("The current state is unitialized");
    }
    else {
        auto found = mFvActions.find(*mCurrentFvState);
        if (found != mFvActions.end()) {
            auto stateAction = found->second;
            if (stateAction) {
                ret = stateAction();
            } else {
                LOGE("The action function of this state is not valid");
            }
        }
    } 
    return ret;
}

void FreshnessValueStateManager::transiteTo(FreshnessValueState const state) {
    if (mFvActions.find(state) != mFvActions.end()) {
        std::atomic_store(&mCurrentFvState, std::make_shared<FreshnessValueState>(state));
    } else {
        LOGE("Unable to transit to an unregistered state"); 
    }
}

void FreshnessValueStateManager::registerState(FreshnessValueState const state, StateAction const& action) {
    mFvActions.emplace(std::make_pair(state, action));
}

void FreshnessValueStateManager::reactToFVRes() {
   
    if (!mCurrentFvState) {
        LOGE("The current state is not initialized");
        return;
    }

    switch (*mCurrentFvState)
    {
    case FreshnessValueState::FVInProgress:
        transiteTo(FreshnessValueState::ProcessFV);
        break;

    case FreshnessValueState::ProcessFV:
        LOGE("An authentic freshness-value has already been received and waits to be processed");
        break;

    case FreshnessValueState::ProcessAnauthFV:
    case FreshnessValueState::Idle:
    case FreshnessValueState::RequestFV:
        LOGE("An authentic freshness-value received without a request");
        break;
    
    default:
        LOGE("FATAL DEVELOPMENT ERROR: The current state is not recognized by this function. Apparently the function is not updated.")
        break;
    }
}

void FreshnessValueStateManager::reactToUnauthenticFVRes() {
   
    if (!mCurrentFvState) {
        LOGE("The current state is not initialized");
        return;
    }

    switch (*mCurrentFvState)
    {
    case FreshnessValueState::RequestFV:
        LOGI("Ignore this unauthentic freshness-value because a valid authentic freshness-value does not yet exist");
        break;

    case FreshnessValueState::FVInProgress:
        LOGI("Ignore this unauthentic freshness-value because a valid authentic freshness-value is on the way");
        break;

    case FreshnessValueState::ProcessFV:
        LOGI("Ignore this unauthentic freshness-value because a valid authentic freshness-value is now processed");
        break;

    case FreshnessValueState::Idle:
        transiteTo(FreshnessValueState::ProcessAnauthFV);
        break;

    case FreshnessValueState::ProcessAnauthFV:
        LOGI("Ignore this unauthentic freshness-value because another unauthentic freshness-value is now processed");
        break;
    
    default:
        LOGE("FATAL DEVELOPMENT ERROR: The current state is not recognized by this function. Apparently the function is not updated.")
        break;
    }
}

} // namespace fvm 
} // namespace sok