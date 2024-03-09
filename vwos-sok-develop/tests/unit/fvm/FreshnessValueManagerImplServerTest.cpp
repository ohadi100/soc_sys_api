/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include <thread>
#include "sok/fvm/FreshnessValueManagerImplServer.hpp"
#include "sok/fvm/FreshnessValueManagerConstants.hpp"
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

class FreshnessValueManagerImplServerStub : public FreshnessValueManagerImplServer {
public:

    bool 
    stub_serverOrParticipantInit()
    {
        return FreshnessValueManagerImplServer::serverOrParticipantInit();
    }

    void setFv(uint64_t fv) {
        mFV = fv;
        mIsFvValid = true;
    }

    void setTimeSinceInit(uint64_t time) {
        mTimeSinceInit = time;
    }

    uint64_t getFv() {
        return mFV;
    }

    void setClockCount(uint16_t time) {
        mClockCount = time;
    }

    uint32_t getClockCount() {
        return mClockCount;
    }

    void setInitialized(bool status) {
        mInitialized = status;
    }

    FvmErrorCode stub_sendAuthenticFvResponses() {
        return FreshnessValueManagerImplServer::sendAuthenticFvResponses();
    }
};

class FreshnessValueManagerImplServerTest : public ::testing::Test
{
public:
    FreshnessValueManagerImplServerTest()
    {
        UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor = new MockFreshnessValueManagerConfigAccessor();
        UTSignalManager::mMockSm = new MockSignalManager();
        UTCsmAccessor::mMockCsm = new MockCsmAccessor();
        UTFvmRuntimeAttributesManager::mMockFvmAttrMgr = new MockFvmRuntimeAttributesManager();

        FrameConfig testFrame;
        testFrame.name = "TETS_FRAME_NAME";
        testFrame.maxPayloadSizeBytes = 16;
        testFrame.sourceIp = "fd53:7cb8:383:2::1";
        testFrame.destinationIp = "::1";
        testFrame.sourcePort = 1234;
        testFrame.destinationPort = 4321;
        PduConfig testPdu;
        testPdu.frameConfig = testFrame;
        testPdu.name = "TEST_PDU_NAME";
        testPdu.id = 34;
        testPdu.lengthBytes = 8;
        mTestSignal.pduConfig = testPdu;
        mTestSignal.name = "TEST_SIGNAL_NAME";
        mTestSignal.startByte = 0;
        mTestSignal.lengthInBits = 64;
        mTestClientConfig["ECU1"] = {mTestSignal, mTestSignal, mTestSignal, 123};

        EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetClientsConfigMap()).Times(1).WillOnce(Return(mTestClientConfig));
        mFvm = std::make_shared<FreshnessValueManagerImplServerStub>();
    }

    ~FreshnessValueManagerImplServerTest()
    {
        delete UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor;
        delete UTSignalManager::mMockSm;
        delete UTCsmAccessor::mMockCsm;
        delete UTFvmRuntimeAttributesManager::mMockFvmAttrMgr;
    }

    std::shared_ptr<FreshnessValueManagerImplServerStub> mFvm;
    FmServerClientsConfigMap mTestClientConfig;
    SignalConfig mTestSignal;
};

TEST_F(FreshnessValueManagerImplServerTest, server_init_success)
{   
    std::vector<uint8_t> retRandom(FVM_SERVER_NUM_OF_BYTES_INITIAL_FV, 0);
    retRandom[FVM_SERVER_NUM_OF_BYTES_INITIAL_FV - 1] = 1;
    uint64_t expectedInitialFv = 1;

    EXPECT_CALL(*UTCsmAccessor::mMockCsm, GenerateRandomBytes(FVM_SERVER_NUM_OF_BYTES_INITIAL_FV)).Times(1).WillOnce(Return(CsmResult<std::vector<uint8_t>>(retRandom)));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(_)).Times(static_cast<int>(mTestClientConfig.size())).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(_, _)).Times(static_cast<int>(mTestClientConfig.size())).WillRepeatedly(Return(FvmErrorCode::kSuccess));

    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());
    EXPECT_EQ(mFvm->getFv(), expectedInitialFv);
}

TEST_F(FreshnessValueManagerImplServerTest, main_first_iteration_success)
{   
    std::vector<uint8_t> retRandom(FVM_SERVER_NUM_OF_BYTES_INITIAL_FV, 0);
    retRandom[FVM_SERVER_NUM_OF_BYTES_INITIAL_FV - 1] = 1;
    uint64_t expectedInitialFv = 1;
    mFvm->setInitialized(true);

    EXPECT_CALL(*UTCsmAccessor::mMockCsm, GenerateRandomBytes(FVM_SERVER_NUM_OF_BYTES_INITIAL_FV)).Times(1).WillOnce(Return(CsmResult<std::vector<uint8_t>>(retRandom)));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(_)).Times(static_cast<int>(mTestClientConfig.size())).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(_, _)).Times(static_cast<int>(mTestClientConfig.size())).WillRepeatedly(Return(FvmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetUnauthenticatedFvSignalConfig()).Times(1).WillOnce(Return(mTestSignal));
    EXPECT_CALL(*UTSignalManager::mMockSm, Publish(mTestSignal, UintToByteVectorTrim<uint64_t>(expectedInitialFv, 8))).Times(1).WillOnce(Return(FvmErrorCode::kSuccess));

    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());
    EXPECT_EQ(FvmErrorCode::kSuccess, mFvm->MainFunction());
    EXPECT_EQ(mFvm->getFv(), expectedInitialFv);
    EXPECT_EQ(mFvm->getClockCount(), SOK_FM_MAIN_FUNCTION_PERIOD_MS);

    EXPECT_EQ(FvmErrorCode::kSuccess, mFvm->MainFunction());
    EXPECT_EQ(mFvm->getFv(), expectedInitialFv);
    EXPECT_EQ(mFvm->getClockCount(), 2 * SOK_FM_MAIN_FUNCTION_PERIOD_MS);
}

TEST_F(FreshnessValueManagerImplServerTest, challenge_response_success)
{   
    std::vector<uint8_t> retRandom(FVM_SERVER_NUM_OF_BYTES_INITIAL_FV, 0);
    retRandom[FVM_SERVER_NUM_OF_BYTES_INITIAL_FV - 1] = 1;
    uint64_t expectedInitialFv = 1;
    auto serializedFv = UintToByteVectorTrim<uint64_t>(expectedInitialFv,7);
    ISignalManager::SignalEventCallback challengeSignalCb;
    std::vector<uint8_t> testChallenge{1,2,3,4};
    std::vector<uint8_t> expectedMacData = testChallenge;
    expectedMacData.insert(expectedMacData.end(), serializedFv.begin(), serializedFv.end());
    std::vector<uint8_t> testMac{4,3,2,1,5,0};
    std::vector<uint8_t> macRes8Byte(testMac.begin(),testMac.begin() + AUTH_FV_SIGNATURE_SIZE_BYTES);

    EXPECT_CALL(*UTCsmAccessor::mMockCsm, GenerateRandomBytes(FVM_SERVER_NUM_OF_BYTES_INITIAL_FV)).Times(1).WillOnce(Return(CsmResult<std::vector<uint8_t>>(retRandom)));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(_)).Times(static_cast<int>(mTestClientConfig.size())).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(_, _)).Times(static_cast<int>(mTestClientConfig.size())).WillRepeatedly(DoAll(SaveArg<1>(&challengeSignalCb), Return(FvmErrorCode::kSuccess)));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, MacCreate(_, expectedMacData, _)).Times(1).WillOnce(Return(CsmResult<std::vector<uint8_t>>(testMac)));
    EXPECT_CALL(*UTSignalManager::mMockSm, Publish(_, serializedFv)).Times(1).WillOnce(Return(FvmErrorCode::kSuccess));
    EXPECT_CALL(*UTSignalManager::mMockSm, Publish(_, macRes8Byte)).Times(1).WillOnce(Return(FvmErrorCode::kSuccess));

    // init
    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());
    EXPECT_EQ(mFvm->getFv(), expectedInitialFv);

    // invoke challenge CB
    challengeSignalCb("SOK_Zeit_ECU1_Challenge", testChallenge);

    // auth FV distribution
    EXPECT_EQ(FvmErrorCode::kSuccess, mFvm->stub_sendAuthenticFvResponses());
}