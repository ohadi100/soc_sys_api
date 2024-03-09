/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include <thread>
#include <numeric>
#include "sok/fvm/AFreshnessValueManagerImpl.hpp"
#include "sok/fvm/FreshnessValueManagerConstants.hpp"
#include "sok/fvm/FvmRuntimeAttributesManager.hpp"
#include "sok/common/SokUtilities.hpp"
#include "MockFreshnessValueManagerConfigAccessor.hpp"
#include "MockSignalManager.hpp"
#include "MockCsmAccessor.hpp"
#include "MockFvmRuntimeAttributesManager.hpp"

using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::_;
using ::testing::Return;
using namespace sok::fvm;
using namespace sok::common;

MockFreshnessValueManagerConfigAccessor* UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor;
MockSignalManager* UTSignalManager::mMockSm;
MockCsmAccessor* UTCsmAccessor::mMockCsm;
MockFvmRuntimeAttributesManager* UTFvmRuntimeAttributesManager::mMockFvmAttrMgr;

class AFreshnessValueManagerImplStub : public AFreshnessValueManagerImpl {
public:
    FvmErrorCode 
    MainFunction() noexcept override
    {
        incTimers();
        return FvmErrorCode::kSuccess;
    }

    bool 
    serverOrParticipantInit() noexcept override
    {
        return true;
    }

    bool 
    registerToSignals() noexcept
    {
        return AFreshnessValueManagerImpl::registerToSignals();
    }

    void setFv(uint64_t fv) 
    {
        mFV = fv;
        mIsFvValid = true;
    }

    void setTimeSinceInit(uint64_t time) 
    {
        mTimeSinceInit = time;
    }

    uint64_t getFv() 
    {
        return mFV;
    }

    uint32_t getClockCount() 
    {
        return mClockCount;
    }

    void setInitialized(bool status) 
    {
        mInitialized = status;
    }

    void setRealAttrMgr(std::vector<SokFvConfigInstance> const& configs) 
    {
        std::vector<SokFreshnessValueId> ids(configs.size());
        std::iota(ids.begin(), ids.end(), 0);
        EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAllFreshnessValueIds()).Times(1).WillOnce(Return(ids));
        for (auto && id : ids) {
            EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetSokFvConfigInstanceByFvId(id)).WillRepeatedly(Return(FvmResult<SokFvConfigInstance>(configs[id])));
        }
        mAttrMgr = std::make_shared<FvmRuntimeAttributesManager>();
        mAttrMgr->Init();
    }
};

class AFreshnessValueManagerTest : public ::testing::Test
{
public:
    AFreshnessValueManagerTest()
    {
        mTestChallengeSignalConfig = {
                PduConfig {
                    FrameConfig {
                        /*frameName = */ "SK_SOK_Gateway",
                        /*frameMaxPayloadSize = */ 16,
                        /*sourceIp = */ "::1",
                        /*destinationIp = */ "::1",
                        /*sourcePort = */ 1234,
                        /*destinationPort = */ 4321,
                    },
                    /*pduName = */ "SOK_CR_sample_01",
                    /*pduId = */ 0x34F,
                    /*pduLengthBytes = */ 8,
                },
                /*signalName = */ "SOK_CR_sample_challenge_signal",
                /*startByte = */ 0,
                /*signalLengthInBits = */ 64
            };
        mTestCrResponderConfigInstance = {
            SokFreshnessType::kVwSokFreshnessCrResponse,
            mTestChallengeSignalConfig
        };
        mTestChallengeConfigInstance = {
            SokFreshnessType::kVwSokFreshnessCrChallenge,
            mTestChallengeSignalConfig
        };
        UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor = new MockFreshnessValueManagerConfigAccessor();
        UTSignalManager::mMockSm = new MockSignalManager();
        UTCsmAccessor::mMockCsm = new MockCsmAccessor();
        UTFvmRuntimeAttributesManager::mMockFvmAttrMgr = new MockFvmRuntimeAttributesManager();

        mFvm = std::make_shared<AFreshnessValueManagerImplStub>();
    }

    ~AFreshnessValueManagerTest()
    {
        delete UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor;
        delete UTSignalManager::mMockSm;
        delete UTCsmAccessor::mMockCsm;
        delete UTFvmRuntimeAttributesManager::mMockFvmAttrMgr;
    }

    std::shared_ptr<AFreshnessValueManagerImplStub> mFvm;
    SignalConfig mTestChallengeSignalConfig;
    ChallengeConfigInstance mTestCrResponderConfigInstance;
    ChallengeConfigInstance mTestChallengeConfigInstance;
};

TEST_F(AFreshnessValueManagerTest, fv_increment_success)
{
    size_t mainFunctionCalls = 111;
    uint64_t initialFv = 0x1234567812345678;
    uint64_t expectedFv = initialFv + ((mainFunctionCalls * SOK_FM_MAIN_FUNCTION_PERIOD_MS) / SOK_FM_TIME_INCREMENT_PERIOD_MS);
    uint16_t expectedClockCount = static_cast<uint16_t>((mainFunctionCalls * SOK_FM_MAIN_FUNCTION_PERIOD_MS) % SOK_FM_TIME_INCREMENT_PERIOD_MS);

    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    for (size_t i = 0 ; i < mainFunctionCalls ; ++i) {
        ASSERT_TRUE(FvmErrorCode::kSuccess == mFvm->MainFunction());
    }

    EXPECT_EQ(mFvm->getFv(), expectedFv);
    EXPECT_EQ(mFvm->getClockCount(), expectedClockCount);
}

TEST_F(AFreshnessValueManagerTest, cr_responding_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    ISignalManager::SignalEventCallback cb;
    std::vector<uint8_t> sigValue({0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8});
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAllChallengeFreshnessValueIds()).Times(1).WillOnce(Return(std::vector<SokFreshnessValueId>{testId}));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetChallengeConfigInstanceByFvId(testId)).Times(1).WillOnce(Return(FvmResult<ChallengeConfigInstance>(mTestCrResponderConfigInstance)));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(mTestCrResponderConfigInstance.challengeSignalConfig, _)).Times(1).WillOnce(DoAll(SaveArg<1>(&cb), Return(FvmErrorCode::kSuccess)));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(3).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessCrResponse)));

    // register to signals
    ASSERT_TRUE(mFvm->registerToSignals());

    // offer CR service
    bool called = false;
    ChallengeReceivedIndicationCb appCb = [&called, &testId](SokFreshnessValueId id) {
        ASSERT_EQ(testId, id);
        called = true;
    };
    ASSERT_EQ(FvmErrorCode::kSuccess, mFvm->OfferCrRequest(testId,appCb));

    // invoke signal CB
    cb(mTestCrResponderConfigInstance.challengeSignalConfig.name, sigValue);
    EXPECT_TRUE(called);

    // get the FV (challenge received by signal)
    auto result = mFvm->GetTxFreshness(testId);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), sigValue);

    // second time challenge should have been discarded
    result = mFvm->GetTxFreshness(testId);
    EXPECT_FALSE(result.isSucceeded());
}

TEST_F(AFreshnessValueManagerTest, cr_trigger_and_receive_response_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<uint8_t> sigValue({0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8});
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetChallengeConfigInstanceByFvId(testId)).Times(1).WillOnce(Return(FvmResult<ChallengeConfigInstance>(mTestChallengeConfigInstance)));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, GenerateRandomBytes(CHALLENGE_LENGTH_BYTES)).Times(1).WillOnce(Return(CsmResult<std::vector<uint8_t>>(sigValue)));
    EXPECT_CALL(*UTSignalManager::mMockSm, Publish(mTestChallengeConfigInstance.challengeSignalConfig, sigValue)).Times(1).WillOnce(Return(FvmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(3).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessCrChallenge)));

    // trigger CR
    auto res= mFvm->TriggerCrRequest(testId);
    ASSERT_EQ(res, FvmErrorCode::kSuccess);

    // get the FV (generated challenge for respose verification)
    auto result = mFvm->GetRxFreshness(testId, {}, 0);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), sigValue);

    // notify successful verification
    mFvm->VerificationStatusCallout(SecOC_VerificationStatusType{testId, true});

    // second time challenge should have been discarded
    result = mFvm->GetRxFreshness(testId, {}, 0);
    EXPECT_FALSE(result.isSucceeded());
}

TEST_F(AFreshnessValueManagerTest, rx_fv_first_occurrence_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<std::vector<uint8_t>> expectedFvs{{UintToByteVector<uint64_t>(SOK_UPSTART_TIME)}, {UintToByteVector<uint64_t>(initialFv)}, {UintToByteVector<uint64_t>(initialFv - 1)}, {UintToByteVector<uint64_t>(initialFv + 1)}};
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(4).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValue)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, SetActive(testId, _)).Times(1);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, testId, initialFv)).Times(1);

    // get the FV
    for (int iter : {0,1,2,3}) {
        auto result = mFvm->GetRxFreshness(testId, {}, static_cast<uint16_t>(iter));
        ASSERT_TRUE(result.isSucceeded());
        EXPECT_EQ(result.getObject(), expectedFvs[static_cast<size_t>(iter)]);
    }
}

TEST_F(AFreshnessValueManagerTest, rx_fv_active_id_within_timeout_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<std::vector<uint8_t>> expectedFvs{{UintToByteVector<uint64_t>(SOK_UPSTART_TIME)}, {UintToByteVector<uint64_t>(initialFv)}, {UintToByteVector<uint64_t>(initialFv - 1)}, {UintToByteVector<uint64_t>(initialFv + 1)}};
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(4).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValue)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, GetEvent(IFvmRuntimeAttributesManager::EventType::kFirstActivity, testId)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, SetActive(testId, initialFv)).Times(0);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, testId, initialFv)).Times(1);

    // get the FV
    for (int iter : {0,1,2,3}) {
        auto result = mFvm->GetRxFreshness(testId, {}, static_cast<uint16_t>(iter));
        ASSERT_TRUE(result.isSucceeded());
        EXPECT_EQ(result.getObject(), expectedFvs[static_cast<size_t>(iter)]);
    }
}

TEST_F(AFreshnessValueManagerTest, rx_fv_active_id_out_of_timeout_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<std::vector<uint8_t>> expectedFvs{{UintToByteVector<uint64_t>(initialFv)}, {UintToByteVector<uint64_t>(initialFv - 1)}, {UintToByteVector<uint64_t>(initialFv + 1)}};
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(3).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValue)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, GetEvent(IFvmRuntimeAttributesManager::EventType::kFirstActivity, testId)).Times(1).WillOnce(Return(initialFv - (SOK_FM_TIME_VALID_TIMEOUT_MS + SOK_FM_TIME_INCREMENT_PERIOD_MS)/SOK_FM_TIME_INCREMENT_PERIOD_MS));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, SetActive(testId, initialFv)).Times(0);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, testId, initialFv)).Times(1);

    // get the FV
    for (int iter : {0,1,2}) {
        auto result = mFvm->GetRxFreshness(testId, {}, static_cast<uint16_t>(iter));
        ASSERT_TRUE(result.isSucceeded());
        EXPECT_EQ(result.getObject(), expectedFvs[static_cast<size_t>(iter)]);
    }
}

TEST_F(AFreshnessValueManagerTest, tx_no_valid_fv_within_timeout_success)
{
    // setup
    SokFreshnessValueId testId = 0;
    mFvm->setInitialized(true);
    mFvm->setTimeSinceInit(SOK_FM_TIME_VALID_TIMEOUT_MS - SOK_FM_TIME_INCREMENT_PERIOD_MS);
    auto expectedFV = UintToByteVector<uint64_t>(SOK_UPSTART_TIME);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(1).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValue)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(true));

    // get the FV
    auto result = mFvm->GetTxFreshness(testId);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFV);
}

TEST_F(AFreshnessValueManagerTest, tx_no_valid_fv_out_of_timeout_success)
{
    // setup
    SokFreshnessValueId testId = 0;
    mFvm->setInitialized(true);
    mFvm->setTimeSinceInit(SOK_FM_TIME_VALID_TIMEOUT_MS + SOK_FM_TIME_INCREMENT_PERIOD_MS);
    auto expectedFV = UintToByteVector<uint64_t>(SOK_INVALID_TIME);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(1).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValue)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(true));

    // get the FV
    auto result = mFvm->GetTxFreshness(testId);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFV);
}

TEST_F(AFreshnessValueManagerTest, tx_valid_fv_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);
    auto expectedFV = UintToByteVector<uint64_t>(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(1).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValue)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kSignReq, testId, initialFv)).Times(1);

    // get the FV
    auto result = mFvm->GetTxFreshness(testId);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFV);
}

TEST_F(AFreshnessValueManagerTest, tx_valid_fv_session_counter_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);
    std::vector<uint8_t> nextCounter = {0x1};
    auto expectedFV = UintToByteVector<uint64_t>(initialFv);
    expectedFV.insert(expectedFV.end(), nextCounter.begin(), nextCounter.end());


    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(1).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValueSessionSender)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kSignReq, testId, initialFv)).Times(1);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IncSessionCounter(testId)).Times(1).WillOnce(Return(nextCounter));

    // get the FV
    auto result = mFvm->GetTxFreshness(testId);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFV);
}

TEST_F(AFreshnessValueManagerTest, rx_fv_active_id_out_of_timeout_session_counter_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<uint8_t> sessionCounter = {0x1};
    std::vector<uint8_t> expectedFv{UintToByteVector<uint64_t>(initialFv)};
    expectedFv.insert(expectedFv.end(), sessionCounter.begin(), sessionCounter.end());
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(1).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValueSessionReceiver)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, GetNextSessionCounter(testId)).Times(1).WillOnce(Return(sessionCounter));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IncSessionCounter(testId)).Times(1);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, GetEvent(IFvmRuntimeAttributesManager::EventType::kFirstActivity, testId)).Times(1).WillOnce(Return(initialFv - (SOK_FM_TIME_VALID_TIMEOUT_MS + SOK_FM_TIME_INCREMENT_PERIOD_MS)/SOK_FM_TIME_INCREMENT_PERIOD_MS));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, SetActive(testId, initialFv)).Times(0);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, testId, initialFv)).Times(1);

    // get the FV
    auto result = mFvm->GetRxFreshness(testId, sessionCounter, 0);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFv);
}

TEST_F(AFreshnessValueManagerTest, rx_fv_active_id_out_of_timeout_confirmation_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<uint8_t> expectedFv{UintToByteVector<uint64_t>(initialFv)};
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(3).WillRepeatedly(Return(FvmResult<SokFreshnessType>(SokFreshnessType::kVwSokFreshnessValue)));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, IsActive(testId)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, GetEvent(IFvmRuntimeAttributesManager::EventType::kFirstActivity, testId)).Times(2).WillRepeatedly(Return(initialFv - (SOK_FM_TIME_VALID_TIMEOUT_MS + SOK_FM_TIME_INCREMENT_PERIOD_MS)/SOK_FM_TIME_INCREMENT_PERIOD_MS));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, SetActive(testId, initialFv)).Times(0);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, testId, initialFv)).Times(2);
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, GetEvent(IFvmRuntimeAttributesManager::EventType::kVerifyReq, testId)).Times(1).WillOnce(Return(initialFv));
    EXPECT_CALL(*UTFvmRuntimeAttributesManager::mMockFvmAttrMgr, UpdateEvent(IFvmRuntimeAttributesManager::EventType::kVerifySuccess, testId, initialFv)).Times(1);

    // get the FV
    auto result = mFvm->GetRxFreshness(testId, {}, 0);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFv);

    // confirm on verification
    mFvm->VerificationStatusCallout(SecOC_VerificationStatusType{testId, true});

    // get the FV
    result = mFvm->GetRxFreshness(testId, {}, 0);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFv);
}

TEST_F(AFreshnessValueManagerTest, attr_mgr_integration_rx_fv_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<std::vector<uint8_t>> expectedFvs{{UintToByteVector<uint64_t>(SOK_UPSTART_TIME)}, {UintToByteVector<uint64_t>(initialFv + (SOK_FM_TIME_VALID_TIMEOUT_MS / SOK_FM_TIME_INCREMENT_PERIOD_MS) + 1)}};
    std::vector<SokFvConfigInstance> testConfig{
        SokFvConfigInstance{
            /*type=*/ SokFreshnessType::kVwSokFreshnessValueSessionReceiver, 
            /*pduIdRef=*/ {},
            /*sessionCounterLength=*/ 1
        },
        };
    std::vector<uint8_t> firstSessionCounter = {0x1};
    std::vector<uint8_t> secondSessionCounter = {0x2};
    expectedFvs[0].insert(expectedFvs[0].end(), firstSessionCounter.begin(), firstSessionCounter.end());
    expectedFvs[1].insert(expectedFvs[1].end(), secondSessionCounter.begin(), secondSessionCounter.end());
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);
    mFvm->setRealAttrMgr(testConfig);
    
    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(3).WillRepeatedly(Return(FvmResult<SokFreshnessType>(testConfig[testId].type)));

    // get the FV
    auto result = mFvm->GetRxFreshness(testId, firstSessionCounter, 0);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFvs[0]);

    // confirm on verification
    mFvm->VerificationStatusCallout(SecOC_VerificationStatusType{testId, true});

    // pass some time 
    for (int i = 0 ; i < ((SOK_FM_TIME_VALID_TIMEOUT_MS + SOK_FM_TIME_INCREMENT_PERIOD_MS)/ SOK_FM_MAIN_FUNCTION_PERIOD_MS) ; i++) {
        mFvm->MainFunction();
    }

    // get the FV
    result = mFvm->GetRxFreshness(testId, secondSessionCounter, 0);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFvs[1]);
}

TEST_F(AFreshnessValueManagerTest, attr_mgr_integration_tx_fv_success)
{
    // setup
    uint64_t initialFv = 0x1234567812345678;
    SokFreshnessValueId testId = 0;
    std::vector<SokFvConfigInstance> testConfig{
        SokFvConfigInstance{
        /*type=*/ SokFreshnessType::kVwSokFreshnessValueSessionSender,
        /*pduIdRef=*/ {},
        /*sessionCounterLength=*/ 1
        },
        };
    mFvm->setInitialized(true);
    mFvm->setFv(initialFv);
    auto expectedFV1 = UintToByteVector<uint64_t>(SOK_UPSTART_TIME);
    expectedFV1.emplace_back(0x1);
    auto expectedFV2 = UintToByteVector<uint64_t>(initialFv);
    expectedFV2.emplace_back(0x2);
    mFvm->setRealAttrMgr(testConfig);

    // expected mock calls
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEntryTypeByFvId(testId)).Times(2).WillRepeatedly(Return(FvmResult<SokFreshnessType>(testConfig[0].type)));

    // get the FV
    auto result = mFvm->GetTxFreshness(testId);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFV1);

    // get the FV
    result = mFvm->GetTxFreshness(testId);
    ASSERT_TRUE(result.isSucceeded());
    EXPECT_EQ(result.getObject(), expectedFV2);
}