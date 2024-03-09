/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_MANAGER_CONSTANTS_HPP
#define FRESHNESS_VALUE_MANAGER_CONSTANTS_HPP

#include "FreshnessValueManagerDefinitions.hpp"
#include <string>

namespace sok
{
namespace fvm
{

/**
 * @brief Default FV for startup when there is no auth time up to `SOK_FM_TIME_VALID_TIMEOUT_MS`
 * 
 */
constexpr uint64_t SOK_UPSTART_TIME = 0xFFFFFFFFFFFFFF;

constexpr int64_t SOK_INVALID_TIME = 0xF00000000000;

/**
 * @brief The period in milliseconds of the calling frequency with which the function FreshnessValueManager::MainFunction() should be called
 * 
 */
constexpr uint8_t SOK_FM_MAIN_FUNCTION_PERIOD_MS = 5; 

/**
 * @brief The periodic increment of the SOK time in milliseconds
 * 
 */
constexpr uint8_t SOK_FM_TIME_INCREMENT_PERIOD_MS = 100; 

/**
 * @brief The maximum time interval in milliseconds during which an SOK time participant uses or accepts a standard time value for signature creation or verification, respectively
 * 
 */
constexpr uint16_t SOK_FM_TIME_VALID_TIMEOUT_MS = 500;

/**
 * @brief This value represents the maximum deviation in milliseconds of the internal authentic SOK time from the reference value given by the SOK time server.
 * 
 */
constexpr uint8_t SOK_FM_TIME_JITTER_MAX_MS = 30;

/**
 * @brief This value represents the maximum time in milliseconds that an SOK time participant waits for an answer from the the SOK time server after asking for the authentic SOK time
 * 
 */
constexpr uint8_t SOK_FM_TIME_REQUEST_TIMEOUT_MS = 250;

/**
 * @brief This value represents the maximum time in milliseconds that a challenge can remain hanging
 * 
 */
constexpr uint8_t SOK_FM_CHALLENGE_TIMEOUT_MS = 250;

/**
 * @brief This value represents the time interval in milliseconds after which the SOK timeserver sends an unprotected time message
 * 
 */
constexpr uint16_t SOK_FM_TIME_SEND_MS = 1000;

/**
 * @brief number of allowed verification attempts for freshness value IDs of Challenge/Response type
 * 
 */
constexpr uint8_t MAX_VERIFY_ATTEMPTS_CR_TYPE = 1;

/**
 * @brief number of allowed verification attempts for plain freshness value IDs
 * 
 */
constexpr uint8_t MAX_VERIFY_ATTEMPTS_FV_TYPE = 4;

/**
 * @brief number of allowed mac bytes
 * 
 */
constexpr int8_t AUTH_FV_SIGNATURE_SIZE_BYTES = 8;


constexpr uint8_t CHALLENGE_LENGTH_BYTES = 8;

constexpr uint8_t FVM_SERVER_NUM_OF_BYTES_INITIAL_FV = 7;

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_MANAGER_CONSTANTS_HPP