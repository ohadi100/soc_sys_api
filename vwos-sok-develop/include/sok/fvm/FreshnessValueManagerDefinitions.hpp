/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_MANAGER_DEFINITIONS_HPP
#define FRESHNESS_VALUE_MANAGER_DEFINITIONS_HPP

#include <functional>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace sok
{
namespace fvm
{

using SokFreshnessValueId = uint32_t;
using pdu_id = uint32_t;
using FVContainer = std::vector<uint8_t>;
using ChallengeReceivedIndicationCb = std::function<void(SokFreshnessValueId)>;


/**
 * @brief enum representing the type of the FV config array entry
 * 
 */
enum class SokFreshnessType : uint8_t {
    kVwSokFreshnessValue = 0,
    kVwSokFreshnessValueSessionSender,
    kVwSokFreshnessValueSessionReceiver,
    kVwSokFreshnessCrChallenge,
    kVwSokFreshnessCrResponse,

    kEndEnum
};

/**
 * @brief struct representing a verification status
 * 
 * @param fvId Holds the identifier of the freshness value.
 * @param verificationSucceeded boolean indication whether the verification succeeded of failed 
 * 
 */
struct SecOC_VerificationStatusType
{
    SokFreshnessValueId fvId;
    bool verificationSucceeded;
};

/**
 * @brief struct representing an instance of a runtime attributes for a single Freshness value ID
 * 
 * @param valueIsActive indicated whether this ID was already used before
 * @param sessionCounterLength the length of the session counter - if no session counter is used leave with zero
 * @param firstUsed settable time stamp for the first usage
 * @param latestVerifiedTime last time this ID was verified
 * @param lastRequestedForVerification last time this ID was requested for verification
 * @param latestSignedTime last time this ID was signed
 * @param lastRequestedForSigning last time this ID was requested for signing
 * @param sessionCounterLastUsed the cached session counter
 */
struct SokFmRuntimeAttributes
{
    bool     valueIsActive                = false;
    uint8_t  sessionCounterLength = 0;
    uint64_t firstUsed           = 0;
    uint64_t latestVerifiedTime           = 0;
    uint64_t lastRequestedForVerification = 0;
    uint64_t latestSignedTime             = 0;
    uint64_t lastRequestedForSigning      = 0;
    std::vector<uint8_t> sessionCounterLastUsed;
};

/**
 * @brief struct representing the configuration parameters of a Frame
 * 
 */
struct FrameConfig {
    std::string name;
    uint16_t     maxPayloadSizeBytes;  
    std::string sourceIp;
    std::string destinationIp;
    uint16_t sourcePort;
    uint16_t destinationPort;
};

/**
 * @brief struct representing the configuration parameters of a PDU
 * 
 */
struct PduConfig {
    FrameConfig frameConfig;
    std::string name;
    pdu_id id;
    uint32_t    lengthBytes;
};

/**
 * @brief struct representing the configuration parameters of a Signal
 * 
 */
struct SignalConfig {
    PduConfig pduConfig;
    std::string name;
    uint32_t startByte;
    uint32_t lengthInBits;
};

/**
 * @brief Structs to hold the needed parameters of a Challenge
 * 
 */
struct ChallengeConfigInstance {
    SokFreshnessType type;
    SignalConfig challengeSignalConfig;
};

/**
 * @brief Structs to hold the needed parameters of an Authentic broadcast PDU
 * 
 */
struct SokFvConfigInstance {
    SokFreshnessType type;
    pdu_id pduId;
    uint8_t sessionCounterLength;
};

/**
 * @brief for FVM server only - struct representing a FM participant parameters
 * 
 */
struct SokFvClientConfigArrayInstance {
    SignalConfig clientChallengeSignal;
    SignalConfig clientResponseValueSignal;
    SignalConfig clientResponseSignatureSignal;
    uint16_t keyId;
};

using SokFvConfig = std::unordered_map<SokFreshnessValueId, SokFvConfigInstance>;
using ChallengeConfig = std::unordered_map<SokFreshnessValueId, ChallengeConfigInstance>;
using FmServerClientsConfigMap = std::unordered_map<std::string, SokFvClientConfigArrayInstance>;
using SokKeyConfig = std::unordered_map<SokFreshnessValueId, uint16_t>;

struct SokFmConfig {
    std::string mNetworkInterface;
    std::string mEcuName;
    uint16_t mKeyIdForAuthFvDistribution;
    SokFvConfig mAuthBroadcastConfig;
    ChallengeConfig mChallengesConfig;
    SokKeyConfig mKeyConfig;
    FmServerClientsConfigMap mFmServerClientsConfig;
    SignalConfig mUnAuthFvDistributionSignal;
    SignalConfig mAuthFvChallengeSignal;
    SignalConfig mAuthFvValueSignal;
    SignalConfig mAuthFvSignatureSignal;
};

inline bool operator==(FrameConfig const& A, FrameConfig const& B)
{
    return (A.name == B.name) && (A.destinationIp == B.destinationIp) && (A.destinationPort == B.destinationPort) && (A.sourceIp == B.sourceIp)\
    && (A.sourcePort == B.sourcePort) && (A.maxPayloadSizeBytes == B.maxPayloadSizeBytes);
}

inline bool operator==(PduConfig const& A, PduConfig const& B)
{
    return (A.frameConfig == B.frameConfig) && (A.id == B.id) && (A.lengthBytes == B.lengthBytes) && (A.name == B.name);
}

inline bool operator==(SignalConfig const& A, SignalConfig const& B)
{
    return (A.pduConfig == B.pduConfig) && (A.startByte == B.startByte) && (A.lengthInBits == B.lengthInBits) && (A.name == B.name);
}

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_MANAGER_DEFINITIONS_HPP