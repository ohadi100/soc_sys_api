/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include "sok/fvm/FvmConfigParser.hpp"

using namespace sok::fvm;

TEST(FvmConfigParserTest, parseConfigJsonSuccess)
{
    std::string json("{\"version\":1,\"network_interface\":\"sw4\",\"ecu_name\":\"ECU1\",\"ecu_key_id_auth_fv\":123,\"auth_br_config\":[{\"fv_id\":1,\"sok_freshness_type\":\"FV\",\"pdu_id\":123,\"session_counter_length_bits\":0}],\"challenge_response_config\":[{\"fv_id\":2,\"challenge_type\":\"CHALLENGE\",\"signal\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}}}],\"unauthenticated_fv_signal_config\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}},\"authenticated_fv_value_signal_config\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}},\"authenticated_fv_signature_signal_config\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}},\"authenticated_fv_value_challenge_config\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}},\"key_config\":[{\"fv_id\":1,\"key_id\":321}],\"clients_signals_config\":[{\"client_ecu_name\":\"ECU1\",\"key_id\":132,\"challenge_signal\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}},\"response_value_signal\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}},\"response_signature_signal\":{\"frame_config\":{\"name\":\"TETS_FRAME_NAME\",\"frame_max_payload_size\":16,\"source_ip\":\"fd53:7cb8:383:2::1\",\"destination_ip\":\"::1\",\"source_port\":1234,\"destination_port\":4321},\"pdu_config\":{\"name\":\"TEST_PDU_NAME\",\"pdu_id\":34,\"length_bytes\":8},\"signal_config\":{\"name\":\"TEST_SIGNAL_NAME\",\"start_byte\":0,\"length_in_bits\":64}}}]}");
    SokFmConfig outConfig;
    FvmConfigParser parser;
    ASSERT_TRUE(parser.Parse(json, outConfig));

    // check values
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
    SignalConfig testSignal;
    testSignal.pduConfig = testPdu;
    testSignal.name = "TEST_SIGNAL_NAME";
    testSignal.startByte = 0;
    testSignal.lengthInBits = 64;

    // general attributes
    EXPECT_EQ(outConfig.mNetworkInterface, "sw4");
    EXPECT_EQ(outConfig.mEcuName, "ECU1");
    EXPECT_EQ(outConfig.mKeyIdForAuthFvDistribution, 123);
    // auth broadcast config
    EXPECT_EQ(outConfig.mAuthBroadcastConfig.size(), 1);
    ASSERT_TRUE(outConfig.mAuthBroadcastConfig.end() != outConfig.mAuthBroadcastConfig.find(1));
    EXPECT_EQ(outConfig.mAuthBroadcastConfig[1].type, SokFreshnessType::kVwSokFreshnessValue);
    EXPECT_EQ(outConfig.mAuthBroadcastConfig[1].pduId, 123);
    EXPECT_EQ(outConfig.mAuthBroadcastConfig[1].sessionCounterLength, 0);
    // challenge response config
    EXPECT_EQ(outConfig.mChallengesConfig.size(), 1);
    ASSERT_TRUE(outConfig.mChallengesConfig.end() != outConfig.mChallengesConfig.find(2));
    EXPECT_EQ(outConfig.mChallengesConfig[2].challengeSignalConfig, testSignal);
    // unauth fv signal config
    EXPECT_EQ(outConfig.mUnAuthFvDistributionSignal, testSignal);
    // auth fv value signal config
    EXPECT_EQ(outConfig.mAuthFvValueSignal, testSignal);
    // auth fv signature signal config
    EXPECT_EQ(outConfig.mAuthFvSignatureSignal, testSignal);
    // auth fv challenge signal config
    EXPECT_EQ(outConfig.mAuthFvChallengeSignal, testSignal);
    // key config
    EXPECT_EQ(outConfig.mKeyConfig.size(), 1);
    ASSERT_TRUE(outConfig.mKeyConfig.end() != outConfig.mKeyConfig.find(1));
    EXPECT_EQ(outConfig.mKeyConfig[1], 321);
    // clients config
    EXPECT_EQ(outConfig.mFmServerClientsConfig.size(), 1);
    ASSERT_TRUE(outConfig.mFmServerClientsConfig.end() != outConfig.mFmServerClientsConfig.find("ECU1"));
    EXPECT_EQ(outConfig.mFmServerClientsConfig["ECU1"].keyId, 132);
    EXPECT_EQ(outConfig.mFmServerClientsConfig["ECU1"].clientChallengeSignal, testSignal);
    EXPECT_EQ(outConfig.mFmServerClientsConfig["ECU1"].clientResponseSignatureSignal, testSignal);
    EXPECT_EQ(outConfig.mFmServerClientsConfig["ECU1"].clientResponseValueSignal, testSignal);
}
