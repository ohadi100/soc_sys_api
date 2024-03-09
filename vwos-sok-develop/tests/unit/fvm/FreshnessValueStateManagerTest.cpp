/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include "sok/fvm/FreshnessValueStateManager.hpp"

using namespace sok::fvm;

TEST(FreshnessValuStateManagerTest, stateActionsAndTransactions) {

    FreshnessValueStateManager stateManager;
    std::string actionMsg;
    
    // uninitialised state - failure
    EXPECT_EQ(stateManager.enter(), FvmErrorCode::kGeneralError);
   
   /* RequestFV state */
   // register RequestFV state and transit to it - success
    stateManager.registerState(FreshnessValueState::RequestFV, [&]() -> FvmErrorCode {   
        actionMsg = "run RequestFV state";
        return FvmErrorCode::kSuccess;
    });
    stateManager.transiteTo(FreshnessValueState::RequestFV);
    EXPECT_EQ(stateManager.enter(), FvmErrorCode::kSuccess);
    EXPECT_EQ(actionMsg, "run RequestFV state");

    // try to move to unregistered state - stay in current state
    stateManager.transiteTo(FreshnessValueState::FVInProgress);
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run RequestFV state");

    // register other states
    stateManager.registerState(FreshnessValueState::FVInProgress, [&]() -> FvmErrorCode {   
        actionMsg = "run FVInProgress state";
        return FvmErrorCode::kSuccess;
    });
    stateManager.registerState(FreshnessValueState::ProcessFV, [&]() -> FvmErrorCode {   
        actionMsg = "run ProcessFV state";
        return FvmErrorCode::kSuccess;
    }); 
    stateManager.registerState(FreshnessValueState::ProcessAnauthFV, [&]() -> FvmErrorCode {   
        actionMsg = "run ProcessAnauthFV state";
        return FvmErrorCode::kSuccess;
    });     
    stateManager.registerState(FreshnessValueState::Idle, [&]() -> FvmErrorCode {   
        actionMsg = "run Idle state";
        return FvmErrorCode::kSuccess;
    }); 

    // react to auth/unauth fv res - stay in RequestFV state
    stateManager.reactToFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run RequestFV state");
    stateManager.reactToUnauthenticFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run RequestFV state");

    /* FVInProgress state */
    // transit to FVInProgress state - success
    stateManager.transiteTo(FreshnessValueState::FVInProgress);
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run FVInProgress state");

    // react to unauth fv res - stay in FVInProgress state
    stateManager.reactToUnauthenticFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run FVInProgress state");
    
    /* ProcessFV state */
    // react to unauth fv - move to ProcessFV state
    stateManager.reactToFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run ProcessFV state");

    // react to auth/unauth fv res - stay in ProcessFV state
    stateManager.reactToFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run ProcessFV state");
    stateManager.reactToUnauthenticFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run ProcessFV state");

    /* Idle state */
    // transit to Idle state - success
    stateManager.transiteTo(FreshnessValueState::Idle);
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run Idle state");

    // react to auth fv res - stay in Idle state
    stateManager.reactToFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run Idle state");

    /* ProcessAnauthFV state */
    // react to unauth fv res - move to ProcessAnauthFV state
    stateManager.reactToUnauthenticFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run ProcessAnauthFV state");
    
    // react to auth/unauth fv res - stay in ProcessAnauthFV state
    stateManager.reactToFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run ProcessAnauthFV state");
    stateManager.reactToUnauthenticFVRes();
    stateManager.enter();
    EXPECT_EQ(actionMsg, "run ProcessAnauthFV state");
}
