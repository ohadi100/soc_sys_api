/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include <numeric>
#include <limits>
#include "sok/fvm/FvmRuntimeAttributesManager.hpp"
#include "sok/fvm/FreshnessValueManagerConstants.hpp"
#include "sok/common/SokUtilities.hpp"
#include "MockFreshnessValueManagerConfigAccessor.hpp"

using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::_;
using ::testing::Return;
using namespace sok::fvm;
using namespace sok::common;

class FvmRuntimeAttributesManagerTest : public ::testing::Test
{
public:

    FvmRuntimeAttributesManagerTest()
    {
        UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor = new MockFreshnessValueManagerConfigAccessor();
    }

    ~FvmRuntimeAttributesManagerTest()
    {
        delete UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor;
    }

    void initSuccessCalls(std::vector<SokFvConfigInstance> const& configs)
    {
        std::vector<SokFreshnessValueId> ids(configs.size());
        std::iota(ids.begin(), ids.end(), 0);
        EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAllFreshnessValueIds()).Times(1).WillOnce(Return(ids));
        for (auto && id : ids) {
            EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetSokFvConfigInstanceByFvId(id)).WillRepeatedly(Return(FvmResult<SokFvConfigInstance>(configs[id])));
        }
    }

    FvmRuntimeAttributesManager mFvmAttrMgr;
};

TEST_F(FvmRuntimeAttributesManagerTest, init_success)
{
    std::vector<SokFvConfigInstance> testConfig{
        SokFvConfigInstance{
            /*type=*/ SokFreshnessType::kVwSokFreshnessValueSessionReceiver,
            /*pduIdRef=*/ {},
            /*sessionCounterLength=*/ 1
        },
        SokFvConfigInstance{
            /*type=*/ SokFreshnessType::kVwSokFreshnessValueSessionSender,
            /*pduIdRef=*/ {},
            /*sessionCounterLength=*/ 2
        },
        };
    initSuccessCalls(testConfig);
    EXPECT_TRUE(mFvmAttrMgr.Init());
}


TEST_F(FvmRuntimeAttributesManagerTest, get_session_counter_success)
{
    std::vector<SokFvConfigInstance> testConfig{
        SokFvConfigInstance{
            /*type=*/ SokFreshnessType::kVwSokFreshnessValueSessionReceiver,
            /*pduIdRef=*/ {},
            /*sessionCounterLength=*/ 4
        }
    };
    std::vector<uint8_t> expectedSessionCounter = {0,0,0,0};
    initSuccessCalls(testConfig);
    EXPECT_TRUE(mFvmAttrMgr.Init());

    auto res = mFvmAttrMgr.GetSessionCounter(0);
    EXPECT_EQ(res, expectedSessionCounter);
}

TEST_F(FvmRuntimeAttributesManagerTest, inc_session_counter_success)
{
    SokFreshnessValueId testId = 0;
    std::vector<SokFvConfigInstance> testConfig{
        SokFvConfigInstance{
            /*type=*/ SokFreshnessType::kVwSokFreshnessValueSessionReceiver,
            /*pduIdRef=*/ {},
            /*sessionCounterLength=*/ 1
        }
    };
    initSuccessCalls(testConfig);
    EXPECT_TRUE(mFvmAttrMgr.Init());

    for (int i = 1 ; i <= std::numeric_limits<uint8_t>::max() ; i++) {
        auto res = mFvmAttrMgr.IncSessionCounter(testId);
        ASSERT_FALSE(res.empty());
        EXPECT_EQ(res, UintToByteVector<uint8_t>(static_cast<uint8_t>(i)));
    }

    // maxed out - should restart from 0
    auto res = mFvmAttrMgr.IncSessionCounter(testId);
    EXPECT_EQ(res, UintToByteVector<uint8_t>(0));
}