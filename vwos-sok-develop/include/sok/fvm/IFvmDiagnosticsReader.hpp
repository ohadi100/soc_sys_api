/* Copyright (c) 2023 Volkswagen Group */

#ifndef I_FVM_DIAGNOSTICS_READER_HPP
#define I_FVM_DIAGNOSTICS_READER_HPP

#include <vector>
#include "sok/fvm/FreshnessValueManagerDefinitions.hpp"
#include "sok/fvm/FvmDiagnosticsDefinitions.hpp"
#include "sok/fvm/FvmDiagnosticsError.hpp"

namespace sok
{
namespace fvm
{

class IFvmDiagnosticsReader
{
public:
    virtual ~IFvmDiagnosticsReader() = default;

    /**
     * @brief Read the verification failed list status.
     *
     * @return the FvmDiagnosticsErrorCode for the verification failed list.
     */
    virtual FvmDiagnosticsErrorCode ReadVerificationFailedListStatus() const = 0;

    /**
     * @brief Read the verification failed list number.
     *
     * @return FvmDiagnosticResult - either the uint64 object representing the number of the verification failed entries
     * either the error code of the operation.
     */
    virtual FvmDiagnosticsResult<uint64_t> ReadVerificationFailedListNumber() const = 0;

    /**
     * @brief Read the verification failed list data.
     *
     * @return FvmDiagnosticResult - either the list of failed pdu_ids, either the error code of the read attempt.
     */
    virtual FvmDiagnosticsResult<std::vector<pdu_id>> ReadVerificationFailedListData() const = 0;

    /**
     * @brief Read the signature failed list status.
     *
     * @return the FvmDiagnosticsErrorCode for the signature failed list.
     */
    virtual FvmDiagnosticsErrorCode ReadSignatureFailedListStatus() const = 0;

    /**
     * @brief Read the signature failed list number.
     *
     * @return either the uint64 object representing the number of failed signatures, either the error code of the
     * operation.
     */
    virtual FvmDiagnosticsResult<uint64_t> ReadSignatureFailedListNumber() const = 0;

    /**
     * @brief Read the signature failed list data.
     *
     * @return either the list of failed signatures, either the error code of the operation.
     */
    virtual FvmDiagnosticsResult<std::vector<pdu_id>> ReadSignatureFailedListData() const = 0;

    /**
     * @brief Read the general info status.
     *
     * @return the FvmDiagnosticsErrorCode for the general info status.
     */
    virtual FvmDiagnosticsErrorCode ReadGeneralInfoStatus() const = 0;

    /**
     * @brief Read the general info ecu function.
     *
     * @return the general info ecu function.
     */
    virtual FvmSokEcuFunction ReadGeneralInfoEcuFunction() const = 0;

    /**
     * @brief Read the general info main period.
     *
     * @return either the general info main period (uint64 object), either the error code.
     */
    virtual FvmDiagnosticsResult<uint64_t> ReadGeneralInfoMainPeriod() const = 0;

    /**
     * @brief Read the time info main status.
     *
     * @return the FvmDiagnosticsErrorCode for the time info status
     */
    virtual FvmDiagnosticsErrorCode ReadTimeInfoStatus() const = 0;

    /**
     * @brief Read the time info validity.
     *
     * @return true the timeinfo is valid
     *         false otherwise
     */
    virtual bool ReadTimeInfoValidity() const = 0;

    /**
     * @brief Read the current SOK time.
     *
     * @return the current SOK time (uint64 object) or the operation error code.
     */
    virtual FvmDiagnosticsResult<uint64_t> ReadTimeInfoCurrentSokTime() const = 0;

    /**
     * @brief Read the jitter status (maxJitterExceeded - true, false otherwise).
     *
     * @return true - when the max jitter exceeded.
     *         false - otherwise
     */
    virtual bool ReadTimeInfoJitterStatus() const = 0;

    /**
     * @brief Read the freshness info status.
     *
     * @return the FvmDiagnosticsErrorCode for the freshness info status
     */
    virtual FvmDiagnosticsErrorCode ReadFreshnessInfoStatus() const = 0;

    /**
     * @brief Read the freshness info number.
     *
     * @return the value of the freshness info number or the operation error code
     */
    virtual FvmDiagnosticsResult<uint64_t> ReadFreshnessInfoNumber() const = 0;

    /**
     * @brief Read the freshness info data.
     *
     * @return the freshness info data (uint64 obj) or read operation status error code.
     */
    virtual FvmDiagnosticsResult<uint64_t> ReadFreshnessInfoData() const = 0;

    /**
     * @brief Read the missing key list status.
     *
     * @return the FvmDiagnosticsErrorCode for the missing key list.
     */
    virtual FvmDiagnosticsErrorCode ReadMissingKeyListStatus() const = 0;

    /**
     * @brief Read the missing key list number.
     *
     * @return the number of missing keys (uint64 object) or the read operation error code.
     */
    virtual FvmDiagnosticsResult<uint64_t> ReadMissingKeyListNumber() const = 0;

    /**
     * @brief Read the missing key list data.
     *
     * @return the list of the missing keys data, or the read operation status error code.
     */
    virtual FvmDiagnosticsResult<std::vector<uint16_t>> ReadMissingKeyListData() const = 0;
};

}  // namespace fvm
}  // namespace sok

#endif  // I_FVM_DIAGNOSTICS_READER_HPP