/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/FvmConfigParser.hpp"
#include "rapidjson/error/en.h"
#include "sok/fvm/FvmConfigParserJsonSchema.hpp"
#include "sok/common/Logger.hpp"

namespace sok
{
namespace fvm
{

FvmConfigParser::FvmConfigParser()
: mSchema()
{
}

bool 
FvmConfigParser::Parse(std::string const& json, SokFmConfig& config) noexcept
{
    try {
        if (json.empty()) {
            LOGE("Empty json string provided");
            return false;
        }
        rapidjson::Document sd;

        if (!mSchema && !sd.Parse(schema::FVM_CONFIG_SCHEMA).HasParseError()) {
            mSchema = std::make_shared<rapidjson::SchemaDocument>(sd);
        }
        else {
            LOGE(rapidjson::GetParseError_En(sd.GetParseError()));
            return false;
        }

        rapidjson::Document doc;
        doc.Parse(json);
        if (doc.HasParseError() || !doc.IsObject()) {
            LOGE("Failed parsing the fvm json config: " << rapidjson::GetParseError_En(doc.GetParseError()));
            return false;
        }

        rapidjson::SchemaValidator sv(*mSchema);
        if (!doc.Accept(sv)) {
            LOGE("Failed validating json store with schema. Invalid keyword: "<< sv.GetInvalidSchemaKeyword());
            return false;
        }

        if (doc[schema::GENERAL_ATTRIBUTES.SCHEMA_VERSION].GetUint() != schema::SCHEMA_VERSION) {
            LOGE("json version does not match current json schema version");
            return false;
        }

        if (!fetchGeneralConfigurations(doc, config)) {
            return false;
        }

        if (!fetchAuthenticBroadcastConfigurations(doc, config)) {
            return false;
        }

        if (!fetchChallengeResponseConfigurations(doc, config)) {
            return false;
        }

        if (!fetchFvDistributionSignalConfigurations(doc, config)) {
            return false;
        }

        if (!fetchKeyConfigurations(doc, config)) {
            return false;
        }

        if (!fetchClientsConfigurations(doc, config)) {
            return false;
        }

        return true;

    } 
    catch (std::exception const& ex)
    {
        LOGE(ex.what());
        return false;
    }
}

bool 
FvmConfigParser::fetchGeneralConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const
{
    config.mEcuName = doc[schema::GENERAL_ATTRIBUTES.ECU_NAME].GetString();
    config.mNetworkInterface = doc[schema::GENERAL_ATTRIBUTES.NETWORK_INTERFACE].GetString();
    config.mKeyIdForAuthFvDistribution = static_cast<uint16_t>(doc[schema::GENERAL_ATTRIBUTES.ECU_KEY_ID_AUTH_FV].GetUint());
    return true;
}

bool 
FvmConfigParser::fetchAuthenticBroadcastConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const
{
    auto authBrArray = doc[schema::AUTH_BROADCAST_CONFIG_ATTRIBUTES.OBJECT_NAME].GetArray();
    for (auto const& brObject : authBrArray) {
        auto fvId = brObject[schema::AUTH_BROADCAST_CONFIG_ATTRIBUTES.FV_ID].GetUint();
        SokFvConfigInstance brConfig;
        brConfig.type = convertStringFreshnessTypeToEnum(brObject[schema::AUTH_BROADCAST_CONFIG_ATTRIBUTES.SOK_FRESHNESS_TYPE].GetString());
        brConfig.pduId = brObject[schema::AUTH_BROADCAST_CONFIG_ATTRIBUTES.PDU_ID].GetUint();
        brConfig.sessionCounterLength = static_cast<uint8_t>(brObject[schema::AUTH_BROADCAST_CONFIG_ATTRIBUTES.SESSION_COUNTER_LEN].GetUint());

        auto emplaceRes = config.mAuthBroadcastConfig.emplace(std::make_pair(fvId, brConfig));
        if (!emplaceRes.second) {
            LOGE("Failed to insert auth broadcast config with ID: " << fvId << ", possibly duplicated?");
            return false;
        }
    }
    return true;
}

bool 
FvmConfigParser::fetchChallengeResponseConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const
{
    auto challengeResponseArray = doc[schema::CHALLENGE_RESPONSE_CONFIG_ATTRIBUTES.OBJECT_NAME].GetArray();
    for (auto const& crObject : challengeResponseArray) {
        auto fvId = crObject[schema::CHALLENGE_RESPONSE_CONFIG_ATTRIBUTES.FV_ID].GetUint();
        ChallengeConfigInstance challengeConfig;
        challengeConfig.type = convertStringFreshnessTypeToEnum(crObject[schema::CHALLENGE_RESPONSE_CONFIG_ATTRIBUTES.CHALLENGE_TYPE].GetString());
        auto completeSignalObject = crObject[schema::CHALLENGE_RESPONSE_CONFIG_ATTRIBUTES.SIGNAL].GetObject();
        if (!fetchSignalConfig(completeSignalObject, challengeConfig.challengeSignalConfig)) {
            return false;
        }
        auto emplaceRes = config.mChallengesConfig.emplace(std::make_pair(fvId, challengeConfig));
        if (!emplaceRes.second) {
            LOGE("Failed to insert challenge config with ID: " << fvId << ", possibly duplicated?");
            return false;
        }
    }
    return true;
}

bool 
FvmConfigParser::fetchFvDistributionSignalConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const
{
    // unauth fv broadcast
    auto unAuthFvSignalObj = doc[schema::UNAUTH_FV_SIGNAL].GetObject();
    if (!fetchSignalConfig(unAuthFvSignalObj, config.mUnAuthFvDistributionSignal)) {
        return false;
    }

    // auth fv challenge
    auto authFvChallengeSignalObj = doc[schema::AUTH_FV_CHALLENGE_SIGNAL].GetObject();
    if (!fetchSignalConfig(authFvChallengeSignalObj, config.mAuthFvChallengeSignal)) {
        return false;
    }

    // auth fv value
    auto authFvValueSignalObj = doc[schema::AUTH_FV_VALUE_SIGNAL].GetObject();
    if (!fetchSignalConfig(authFvValueSignalObj, config.mAuthFvValueSignal)) {
        return false;
    }

    // auth fv signature
    auto authFvSignatureSignalObj = doc[schema::AUTH_FV_SIGNATURE_SIGNAL].GetObject();
    if (!fetchSignalConfig(authFvSignatureSignalObj, config.mAuthFvSignatureSignal)) {
        return false;
    }

    return true;
}

bool 
FvmConfigParser::fetchKeyConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const
{
    auto keyConfArray = doc[schema::KEY_CONFIG_ATTRIBUTES.OBJECT_NAME].GetArray();
    for (auto const& keyObject : keyConfArray) {
        auto fvId = keyObject[schema::KEY_CONFIG_ATTRIBUTES.FV_ID].GetUint();
        auto keyId = keyObject[schema::KEY_CONFIG_ATTRIBUTES.KEY_ID].GetUint();

        auto emplaceRes = config.mKeyConfig.emplace(std::make_pair(fvId, keyId));
        if (!emplaceRes.second) {
            LOGE("Failed to insert key ID with FV ID: " << fvId << ", possibly duplicated?");
            return false;
        }
    }
    return true;
}

bool 
FvmConfigParser::fetchClientsConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const
{
    auto clientsArray = doc[schema::CLIENTS_CONFIG_ATTRIBUTES.OBJECT_NAME].GetArray();
    for (auto const& clientObject : clientsArray) {
        // fetch client's ECU name
        auto clientEcuName = clientObject[schema::CLIENTS_CONFIG_ATTRIBUTES.CLIENT_ECU_NAME].GetString();
        SokFvClientConfigArrayInstance clientConfigInstance;

        // fetch key ID
        clientConfigInstance.keyId = static_cast<uint16_t>(clientObject[schema::CLIENTS_CONFIG_ATTRIBUTES.KEY_ID].GetUint());

        // fetch challenge signal
        auto authFvChallengeSignalObj = clientObject[schema::CLIENTS_CONFIG_ATTRIBUTES.CHALLENGE_SIGNAL].GetObject();
        if (!fetchSignalConfig(authFvChallengeSignalObj, clientConfigInstance.clientChallengeSignal)) {
            LOGE("Failed to fetch FV challenge signal for client with ECU name: " << clientEcuName);
            return false;
        }

        // fetch fv value signal
        auto authFvValueSignalObj = clientObject[schema::CLIENTS_CONFIG_ATTRIBUTES.VALUE_SIGNAL].GetObject();
        if (!fetchSignalConfig(authFvValueSignalObj, clientConfigInstance.clientResponseValueSignal)) {
            LOGE("Failed to fetch FV value signal for client with ECU name: " << clientEcuName);
            return false;
        }

        // fetch fv signature signal
        auto authFvSignatureSignalObj = clientObject[schema::CLIENTS_CONFIG_ATTRIBUTES.SIGNATURE_SIGNAL].GetObject();
        if (!fetchSignalConfig(authFvSignatureSignalObj, clientConfigInstance.clientResponseSignatureSignal)) {
            LOGE("Failed to fetch FV signature signal for client with ECU name: " << clientEcuName);
            return false;
        }

        auto emplaceRes = config.mFmServerClientsConfig.emplace(std::make_pair(clientEcuName, clientConfigInstance));
        if (!emplaceRes.second) {
            LOGE("Failed to insert client config with name: " << clientEcuName << ", possibly duplicated?");
            return false;
        }
    }
    return true;

}

SokFreshnessType 
FvmConfigParser::convertStringFreshnessTypeToEnum(std::string const& freshnessType) const
{
    if (schema::ENUM_FRESHNESS_TYPE_CHALLENGE == freshnessType) {
        return SokFreshnessType::kVwSokFreshnessCrChallenge;
    }
    else if (schema::ENUM_FRESHNESS_TYPE_RESPONSE == freshnessType) {
        return SokFreshnessType::kVwSokFreshnessCrResponse;
    }
    else if (schema::ENUM_FRESHNESS_TYPE_FV == freshnessType) {
        return SokFreshnessType::kVwSokFreshnessValue;
    }
    else if (schema::ENUM_FRESHNESS_TYPE_FV_SESSION_RECEIVER == freshnessType) {
        return SokFreshnessType::kVwSokFreshnessValueSessionReceiver;
    }
    else if (schema::ENUM_FRESHNESS_TYPE_FV_SESSION_SENDER == freshnessType) {
        return SokFreshnessType::kVwSokFreshnessValueSessionSender;
    }
    else {
        LOGE("Invalid freshness type!");
        return SokFreshnessType::kEndEnum;
    }
}   

bool 
FvmConfigParser::fetchSignalConfig(rapidjson::GenericObject<true, rapidjson::Value> const& object, SignalConfig& signalConfig) const
{
    // build frame config
    auto frameObject = object[schema::FRAME_CONFIG_ATTRIBUTES.OBJECT_NAME].GetObject();
    FrameConfig fr;
    fr.maxPayloadSizeBytes = static_cast<uint16_t>(frameObject[schema::FRAME_CONFIG_ATTRIBUTES.MAX_PAYLOAD].GetUint());
    fr.name = frameObject[schema::FRAME_CONFIG_ATTRIBUTES.FRAME_NAME].GetString();
    fr.destinationIp = frameObject[schema::FRAME_CONFIG_ATTRIBUTES.DESTINATION_IP].GetString();
    fr.destinationPort = static_cast<uint16_t>(frameObject[schema::FRAME_CONFIG_ATTRIBUTES.DESTINATION_PORT].GetUint());
    fr.sourceIp = frameObject[schema::FRAME_CONFIG_ATTRIBUTES.SOURCE_IP].GetString();
    fr.sourcePort = static_cast<uint16_t>(frameObject[schema::FRAME_CONFIG_ATTRIBUTES.SOURCE_PORT].GetUint());

    // build PDU config
    auto pduObject = object[schema::PDU_CONFIG_ATTRIBUTES.OBJECT_NAME].GetObject();
    PduConfig pdu;
    pdu.frameConfig = fr;
    pdu.name = pduObject[schema::PDU_CONFIG_ATTRIBUTES.PDU_NAME].GetString();
    pdu.id = pduObject[schema::PDU_CONFIG_ATTRIBUTES.PDU_ID].GetUint();
    pdu.lengthBytes = pduObject[schema::PDU_CONFIG_ATTRIBUTES.LENGTH_BYTES].GetUint();

    // build signal config
    auto signalObject = object[schema::SIGNAL_CONFIG_ATTRIBUTES.OBJECT_NAME].GetObject();
    signalConfig.pduConfig = pdu;
    signalConfig.name = signalObject[schema::SIGNAL_CONFIG_ATTRIBUTES.SIGNAL_NAME].GetString();
    signalConfig.lengthInBits = signalObject[schema::SIGNAL_CONFIG_ATTRIBUTES.LENGTH_BITS].GetUint();
    signalConfig.startByte = signalObject[schema::SIGNAL_CONFIG_ATTRIBUTES.START_BYTE].GetUint();

    return true;
}

} // namespace fvm
} // namespace sok
