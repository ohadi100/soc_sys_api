/* Copyright (c) 2023 Volkswagen Group */

#ifndef FVM_CONFIG_PARSER_JSON_SCHEMA_HPP
#define FVM_CONFIG_PARSER_JSON_SCHEMA_HPP

#include <string>

namespace sok
{
namespace fvm
{
namespace schema
{

constexpr uint8_t SCHEMA_VERSION = 1;

struct SchemaGeneralAttributes {
    const std::string SCHEMA_VERSION = "version";
    const std::string NETWORK_INTERFACE = "network_interface";
    const std::string ECU_NAME = "ecu_name";
    const std::string ECU_KEY_ID_AUTH_FV = "ecu_key_id_auth_fv";
};

struct SchemaAuthBroadcastConfig {
    const std::string OBJECT_NAME = "auth_br_config";
    const std::string FV_ID = "fv_id";
    const std::string SOK_FRESHNESS_TYPE = "sok_freshness_type";
    const std::string PDU_ID = "pdu_id";
    const std::string SESSION_COUNTER_LEN = "session_counter_length_bits";
};

struct SchemaChallengeResponseConfig {
    const std::string OBJECT_NAME = "challenge_response_config";
    const std::string FV_ID = "fv_id";
    const std::string CHALLENGE_TYPE = "challenge_type";
    const std::string SIGNAL = "signal";
};

constexpr char UNAUTH_FV_SIGNAL[] = "unauthenticated_fv_signal_config";
constexpr char AUTH_FV_VALUE_SIGNAL[] = "authenticated_fv_value_signal_config";
constexpr char AUTH_FV_SIGNATURE_SIGNAL[] = "authenticated_fv_signature_signal_config";
constexpr char AUTH_FV_CHALLENGE_SIGNAL[] = "authenticated_fv_value_challenge_config";

constexpr char ENUM_FRESHNESS_TYPE_FV[] = "FV";
constexpr char ENUM_FRESHNESS_TYPE_FV_SESSION_SENDER[] = "FV_SESSION_SENDER";
constexpr char ENUM_FRESHNESS_TYPE_FV_SESSION_RECEIVER[] = "FV_SESSION_RECEIVER";
constexpr char ENUM_FRESHNESS_TYPE_CHALLENGE[] = "CHALLENGE";
constexpr char ENUM_FRESHNESS_TYPE_RESPONSE[] = "RESPONSE";

struct SchemaKeyConfig {
    const std::string OBJECT_NAME = "key_config";
    const std::string FV_ID = "fv_id";
    const std::string KEY_ID = "key_id";
};

struct SchemaClientsConfig {
    const std::string OBJECT_NAME = "clients_signals_config";
    const std::string CLIENT_ECU_NAME = "client_ecu_name";
    const std::string KEY_ID = "key_id";
    const std::string VALUE_SIGNAL = "response_value_signal";
    const std::string SIGNATURE_SIGNAL = "response_signature_signal";
    const std::string CHALLENGE_SIGNAL = "challenge_signal";
};

struct SchemaFrameConfig {
    const std::string OBJECT_NAME = "frame_config";
    const std::string FRAME_NAME = "name";
    const std::string MAX_PAYLOAD = "frame_max_payload_size";
    const std::string SOURCE_IP = "source_ip";
    const std::string DESTINATION_IP = "destination_ip";
    const std::string SOURCE_PORT = "source_port";
    const std::string DESTINATION_PORT = "destination_port";
};

struct SchemaPduConfig {
    const std::string OBJECT_NAME = "pdu_config";
    const std::string PDU_NAME = "name";
    const std::string PDU_ID = "pdu_id";
    const std::string LENGTH_BYTES = "length_bytes";
};

struct SchemaSignalConfig {
    const std::string OBJECT_NAME = "signal_config";
    const std::string SIGNAL_NAME = "name";
    const std::string START_BYTE = "start_byte";
    const std::string LENGTH_BITS = "length_in_bits";
};

SchemaGeneralAttributes const GENERAL_ATTRIBUTES;
SchemaAuthBroadcastConfig const AUTH_BROADCAST_CONFIG_ATTRIBUTES;
SchemaChallengeResponseConfig const CHALLENGE_RESPONSE_CONFIG_ATTRIBUTES;
SchemaKeyConfig const KEY_CONFIG_ATTRIBUTES;
SchemaClientsConfig const CLIENTS_CONFIG_ATTRIBUTES;
SchemaFrameConfig const FRAME_CONFIG_ATTRIBUTES;
SchemaPduConfig const PDU_CONFIG_ATTRIBUTES;
SchemaSignalConfig const SIGNAL_CONFIG_ATTRIBUTES;

constexpr char FVM_CONFIG_SCHEMA[] = 
                "{"
                    "\"type\":\"object\","
                    "\"properties\": {"
                        "\"version\":{\"type\":\"integer\", \"minimum\": 0},"
                        "\"network_interface\":{\"type\":\"string\"},"
                        "\"ecu_name\":{\"type\":\"string\"},"
                        "\"ecu_key_id_auth_fv\":{\"type\":\"integer\", \"minimum\": 0, \"maximum\": 65535},"
                        "\"auth_br_config\":{\"type\":\"array\","
                            "\"items\":{"
                                "\"additionalProperties\": false,"
                                "\"properties\":{"
                                    "\"fv_id\":{\"type\":\"integer\", \"minimum\": 0},"
                                    "\"sok_freshness_type\":{\"enum\":[\"FV\", \"FV_SESSION_SENDER\", \"FV_SESSION_RECEIVER\"]},"
                                    "\"pdu_id\":{\"type\":\"integer\", \"minimum\": 0},"
                                    "\"session_counter_length_bits\":{\"type\":\"integer\", \"minimum\": 0}"
                                "},"
                                "\"required\": [\"fv_id\", \"sok_freshness_type\", \"pdu_id\", \"session_counter_length_bits\"]"
                            "}"
                        "},"
                        "\"challenge_response_config\":{\"type\":\"array\","
                            "\"items\":{"
                                "\"additionalProperties\": false,"
                                "\"properties\":{"
                                    "\"fv_id\":{\"type\":\"integer\", \"minimum\": 0},"
                                    "\"challenge_type\":{\"enum\":[\"CHALLENGE\", \"RESPONSE\"]},"
                                    "\"signal\":{ \"$ref\": \"#/$defs/complete_signal_config\" }"
                                "},"
                                "\"required\": [\"fv_id\", \"challenge_type\", \"signal\"]"
                            "}"
                        "},"
                        "\"unauthenticated_fv_signal_config\":{ \"$ref\": \"#/$defs/complete_signal_config\"},"
                        "\"authenticated_fv_value_signal_config\":{ \"$ref\": \"#/$defs/complete_signal_config\"},"
                        "\"authenticated_fv_signature_signal_config\":{ \"$ref\": \"#/$defs/complete_signal_config\"},"
                        "\"authenticated_fv_value_challenge_config\":{ \"$ref\": \"#/$defs/complete_signal_config\"},"
                        "\"key_config\":{\"type\":\"array\","
                            "\"items\":{"
                                "\"additionalProperties\": false,"
                                "\"properties\":{"
                                    "\"fv_id\":{\"type\":\"integer\", \"minimum\": 0},"
                                    "\"key_id\":{\"type\":\"integer\", \"minimum\": 0, \"maximum\": 65535}"
                                "},"
                                "\"required\": [\"fv_id\", \"key_id\"]"
                            "}"
                        "},"
                        "\"clients_signals_config\":{\"type\":\"array\","
                            "\"items\":{"
                                "\"additionalProperties\": false,"
                                "\"properties\":{"
                                    "\"client_ecu_name\":{\"type\":\"string\"},"
                                    "\"key_id\":{\"type\":\"integer\", \"minimum\": 0, \"maximum\": 65535},"
                                    "\"challenge_signal\":{ \"$ref\": \"#/$defs/complete_signal_config\"},"
                                    "\"response_value_signal\":{ \"$ref\": \"#/$defs/complete_signal_config\"},"
                                    "\"response_signature_signal\":{ \"$ref\": \"#/$defs/complete_signal_config\"}"
                                "},"
                                "\"required\": [\"client_ecu_name\", \"key_id\", \"challenge_signal\", \"response_value_signal\", \"response_signature_signal\"]"
                            "}"
                        "}"
                    "},"
                    "\"required\": ["
                        "\"version\","
                        "\"network_interface\","
                        "\"ecu_name\","
                        "\"ecu_key_id_auth_fv\","
                        "\"auth_br_config\","
                        "\"challenge_response_config\","
                        "\"unauthenticated_fv_signal_config\","
                        "\"authenticated_fv_value_signal_config\","
                        "\"authenticated_fv_signature_signal_config\","
                        "\"authenticated_fv_value_challenge_config\","
                        "\"key_config\","
                        "\"clients_signals_config\""
                    "],"
                    "\"$defs\": {"
                        "\"complete_signal_config\": { \"type\": \"object\"," 
                            "\"properties\": {"
                                "\"frame_config\": { \"type\": \"object\"," 
                                    "\"properties\": {"
                                        "\"name\":{\"type\":\"string\"},"
                                        "\"frame_max_payload_size\":{\"type\":\"integer\", \"minimum\": 0, \"maximum\": 65535},"
                                        "\"source_ip\":{\"type\":\"string\"},"
                                        "\"destination_ip\":{\"type\":\"string\"},"
                                        "\"source_port\":{\"type\":\"integer\", \"minimum\": 0, \"maximum\": 65535},"
                                        "\"destination_port\":{\"type\":\"integer\", \"minimum\": 0, \"maximum\": 65535}"
                                    "},"
                                    "\"required\": [\"name\", \"frame_max_payload_size\", \"source_ip\", \"destination_ip\", \"source_port\", \"destination_port\"]"
                                "},"
                                "\"pdu_config\": { \"type\": \"object\"," 
                                    "\"properties\": {"
                                        "\"name\":{\"type\":\"string\"},"
                                        "\"pdu_id\":{\"type\":\"integer\", \"minimum\": 0},"
                                        "\"length_bytes\":{\"type\":\"integer\", \"minimum\": 0}"
                                    "},"
                                    "\"required\": [\"name\", \"pdu_id\", \"length_bytes\"]"
                                "},"
                                "\"signal_config\": { \"type\": \"object\"," 
                                    "\"properties\": {"
                                        "\"name\":{\"type\":\"string\"},"
                                        "\"start_byte\":{\"type\":\"integer\", \"minimum\": 0},"
                                        "\"length_in_bits\":{\"type\":\"integer\", \"minimum\": 0}"
                                    "},"
                                    "\"required\": [\"name\", \"start_byte\", \"length_in_bits\"]"
                                "}"
                            "},"
                            "\"required\": [\"frame_config\", \"pdu_config\", \"signal_config\"]"
                        "}"
                    "}"
                "}";

} // namespace schema
} // namespace fvm
} // namespace sok

#endif // FVM_CONFIG_PARSER_JSON_SCHEMA_HPP