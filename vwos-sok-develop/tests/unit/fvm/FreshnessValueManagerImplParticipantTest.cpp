/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include <thread>
#include "sok/fvm/FreshnessValueManagerImplParticipant.hpp"
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

class FreshnessValueManagerImplParticipantStub : public FreshnessValueManagerImplParticipant {
public:

    bool 
    stub_serverOrParticipantInit()
    {
        return FreshnessValueManagerImplParticipant::serverOrParticipantInit();
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

    uint32_t getClockCount() {
        return mClockCount;
    }

    void setInitialized(bool status) {
        mInitialized = status;
    }
};

class FreshnessValueManagerImplParticipantTest : public ::testing::Test
{
public:
    FreshnessValueManagerImplParticipantTest()
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
        mTestSignal1.pduConfig = testPdu;
        mTestSignal1.name = "TEST_SIGNAL_NAME1";
        mTestSignal1.startByte = 0;
        mTestSignal1.lengthInBits = 64;
        mTestSignal2.pduConfig = testPdu;
        mTestSignal2.name = "TEST_SIGNAL_NAME2";
        mTestSignal2.startByte = 0;
        mTestSignal2.lengthInBits = 64;

        mTestKeyId = 123;
        mFvm = std::make_shared<FreshnessValueManagerImplParticipantStub>();
    }

    ~FreshnessValueManagerImplParticipantTest()
    {
        delete UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor;
        delete UTSignalManager::mMockSm;
        delete UTCsmAccessor::mMockCsm;
        delete UTFvmRuntimeAttributesManager::mMockFvmAttrMgr;
    }

    void authTimeReqSuccessCalls(std::vector<uint8_t> const& challenge)
    {
        EXPECT_CALL(*UTCsmAccessor::mMockCsm, GenerateRandomBytes(CHALLENGE_LENGTH_BYTES)).Times(1).WillOnce(Return(CsmResult<std::vector<uint8_t>>(challenge)));
        EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvChallengeSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
        EXPECT_CALL(*UTSignalManager::mMockSm, Publish(mTestSignal1, challenge)).Times(1).WillOnce(Return(FvmErrorCode::kSuccess));
    }

    std::shared_ptr<FreshnessValueManagerImplParticipantStub> mFvm;
    SignalConfig mTestSignal1;
    SignalConfig mTestSignal2;
    uint16_t mTestKeyId;
};

TEST_F(FreshnessValueManagerImplParticipantTest, participant_init_success)
{   
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEcuKeyIdForFvDistribution()).Times(1).WillOnce(Return(mTestKeyId));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(mTestKeyId)).Times(1).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvValueSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvSignatureSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetUnauthenticatedFvSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(mTestSignal1, _)).Times(3).WillRepeatedly(Return(FvmErrorCode::kSuccess));

    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());
}

TEST_F(FreshnessValueManagerImplParticipantTest, main_function_first_iter_success)
{   
    std::vector<uint8_t> bytes{0x1,0x2};
    ISignalManager::SignalEventCallback unAuthSignalCb;
    mFvm->setInitialized(true);

    authTimeReqSuccessCalls(bytes);
    
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEcuKeyIdForFvDistribution()).Times(1).WillOnce(Return(mTestKeyId));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(mTestKeyId)).Times(1).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvValueSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvSignatureSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetUnauthenticatedFvSignalConfig()).Times(1).WillRepeatedly(Return(mTestSignal1));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(mTestSignal1, _)).Times(3).WillOnce(Return(FvmErrorCode::kSuccess)).WillOnce(Return(FvmErrorCode::kSuccess)).WillOnce(DoAll(SaveArg<1>(&unAuthSignalCb), Return(FvmErrorCode::kSuccess)));
    
    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());

    EXPECT_EQ(FvmErrorCode::kSuccess ,mFvm->MainFunction());
}

TEST_F(FreshnessValueManagerImplParticipantTest, auth_time_req_unanswered_iter_success)
{   
    std::vector<uint8_t> bytes{0x1,0x2};
    ISignalManager::SignalEventCallback unAuthSignalCb;
    uint16_t iterations = 156;
    uint16_t authReqTimes = static_cast<uint16_t>(((iterations * SOK_FM_MAIN_FUNCTION_PERIOD_MS) / SOK_FM_TIME_REQUEST_TIMEOUT_MS));
    mFvm->setInitialized(true);

    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEcuKeyIdForFvDistribution()).Times(1).WillOnce(Return(mTestKeyId));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(mTestKeyId)).Times(1).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvValueSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvSignatureSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetUnauthenticatedFvSignalConfig()).Times(1).WillRepeatedly(Return(mTestSignal1));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(mTestSignal1, _)).Times(3).WillOnce(Return(FvmErrorCode::kSuccess)).WillOnce(Return(FvmErrorCode::kSuccess)).WillOnce(DoAll(SaveArg<1>(&unAuthSignalCb), Return(FvmErrorCode::kSuccess)));
    
    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());
    
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, GenerateRandomBytes(CHALLENGE_LENGTH_BYTES)).Times(authReqTimes).WillRepeatedly(Return(CsmResult<std::vector<uint8_t>>(bytes)));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvChallengeSignalConfig()).Times(authReqTimes).WillRepeatedly(Return(mTestSignal1));
    EXPECT_CALL(*UTSignalManager::mMockSm, Publish(mTestSignal1, bytes)).Times(authReqTimes).WillRepeatedly(Return(FvmErrorCode::kSuccess));
    
    for (int i = 0 ; i < iterations ; i++) {
        EXPECT_EQ(FvmErrorCode::kSuccess ,mFvm->MainFunction());
    }
}

TEST_F(FreshnessValueManagerImplParticipantTest, unauth_fv_event_success)
{   
    std::vector<uint8_t> bytes{0x1,0x2};
    ISignalManager::SignalEventCallback unAuthSignalCb;
    auto unauthTime = UintToByteVector<uint64_t>(SOK_FM_TIME_JITTER_MAX_MS + 1); // just enough to create jitter from start up FV (0)
    mFvm->setInitialized(true);

    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEcuKeyIdForFvDistribution()).Times(1).WillOnce(Return(mTestKeyId));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(mTestKeyId)).Times(1).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvValueSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvSignatureSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetUnauthenticatedFvSignalConfig()).Times(2).WillRepeatedly(Return(mTestSignal1));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(mTestSignal1, _)).Times(3).WillOnce(Return(FvmErrorCode::kSuccess)).WillOnce(Return(FvmErrorCode::kSuccess)).WillOnce(DoAll(SaveArg<1>(&unAuthSignalCb), Return(FvmErrorCode::kSuccess)));
    authTimeReqSuccessCalls(bytes);

    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());

    unAuthSignalCb(mTestSignal1.name, unauthTime);

    EXPECT_EQ(FvmErrorCode::kSuccess ,mFvm->MainFunction());
}

TEST_F(FreshnessValueManagerImplParticipantTest, auth_fv_receive_success)
{   
    std::vector<uint8_t> challenge{0,0,0,0,0,0,0x1,0x2};
    uint64_t authTime = 56454;
    auto serializedAuthTime = UintToByteVector<uint64_t>(authTime);
    std::vector<uint8_t> mac{0x1,0x2,0x3};
    auto verificationData = challenge;
    verificationData.insert(verificationData.end(), serializedAuthTime.begin() + 1, serializedAuthTime.end());
    ISignalManager::SignalEventCallback authSignalCb;
    mFvm->setInitialized(true);

    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetEcuKeyIdForFvDistribution()).Times(1).WillOnce(Return(mTestKeyId));
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, IsKeyExists(mTestKeyId)).Times(1).WillOnce(Return(CsmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvValueSignalConfig()).Times(2).WillRepeatedly(Return(mTestSignal1));
    EXPECT_CALL(*UTSignalManager::mMockSm, Subscribe(_, _)).Times(3).WillOnce(DoAll(SaveArg<1>(&authSignalCb), Return(FvmErrorCode::kSuccess))).WillOnce(Return(FvmErrorCode::kSuccess)).WillOnce(Return(FvmErrorCode::kSuccess));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetAuthenticatedFvSignatureSignalConfig()).Times(2).WillRepeatedly(Return(mTestSignal2));
    EXPECT_CALL(*UTFreshnessValueManagerConfigAccessor::mMockFvConfAccessor, GetUnauthenticatedFvSignalConfig()).Times(1).WillOnce(Return(mTestSignal1));
    authTimeReqSuccessCalls(challenge);
    EXPECT_CALL(*UTCsmAccessor::mMockCsm, MacVerify(_, verificationData, mac, _)).Times(1).WillOnce(Return(CsmErrorCode::kSuccess));

    EXPECT_TRUE(mFvm->stub_serverOrParticipantInit());
    EXPECT_EQ(FvmErrorCode::kSuccess ,mFvm->MainFunction()); // will trigger auth time req
    authSignalCb(mTestSignal1.name, serializedAuthTime);
    authSignalCb(mTestSignal2.name, mac);
    EXPECT_EQ(FvmErrorCode::kSuccess ,mFvm->MainFunction()); // will trigger handling of auth time
    EXPECT_EQ(mFvm->getFv(), authTime);
}