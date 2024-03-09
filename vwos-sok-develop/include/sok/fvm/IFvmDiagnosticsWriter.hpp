/* Copyright (c) 2023 Volkswagen Group */

#ifndef I_FVM_DIAGNOSTICS_WRITER_HPP
#define I_FVM_DIAGNOSTICS_WRITER_HPP

#include "sok/fvm/FreshnessValueManagerDefinitions.hpp"
#include "sok/fvm/FvmDiagnosticsDefinitions.hpp"
#include "sok/fvm/FvmDiagnosticsError.hpp"

namespace sok
{
namespace fvm
{

class IFvmDiagnosticsWriter
{
public:
    virtual ~IFvmDiagnosticsWriter() = default;

    /**
     * @brief Sets the status of the verification of a pdu_id.
     *
     * @param[in] pdu - the pdu_id for which the verification will be set.
     * @param[in] status - the status to be set.
     *
     * @return true/false depending on the state set operation result.
     */
    virtual bool SetPduVerificationFailedStatus(pdu_id const& pdu, FvmDiagnosticsErrorCode const& status) const = 0;

    /**
     * @brief Set the status of the signature of a pdu_id.
     *
     * @param[in] pdu - the pdu_id for which the status will be set.
     * @param[in] status - the status to be set.
     *
     * @return true/false depending on the status set operation result.
     */
    virtual bool SetPduSignatureFailedStatus(pdu_id const& pdu, FvmDiagnosticsErrorCode const& status) const = 0;

    /**
     * @brief Set the general info status of a pdu_id.
     *
     * @param[in] pdu - the pdu_id for which the general info status will be set.
     * @param[in] status - the status to be set.
     *
     * @return true/false based on the status set operation result.
     */
    virtual bool SetGeneralInfoStatus(pdu_id const& pdu, FvmDiagnosticsErrorCode const& status) const = 0;

    /**
     * @brief Set the general ecu function (time_server or participant).
     *
     * @param[in] ecuFunction - the function to be set (kFvmVwSokTimeServer or kFvmVwSokParticipant).
     *
     * @return true/false based on the function set operation result.
     */
    virtual bool SetGeneralInfoEcuFunction(FvmSokEcuFunction const& ecuFunction) const = 0;

    /**
     * @brief Set the main period.
     *
     * @param[in] periodInSeconds - the time period in seconds to be set.
     *
     * @return true/false depending on the set operation result.
     */
    virtual bool SetGeneralInfoMainPeriod(uint8_t periodInSeconds) const = 0;

    /**
     * @brief Set the timeinfo status.
     *
     * @param[in] status - the status to be set.
     *
     * @return true/false depending on the status set.
     */
    virtual bool SetTimeInfoStatus(FvmDiagnosticsErrorCode const& status) const = 0;

    /**
     * @brief Set the validity of the timeinfo.
     *
     * @param[in] valid - true if the timeinfo is valid, false otherwise.
     *
     * @return true/false depending on the status set operation.
     */
    virtual bool SetTimeInfoValidity(bool valid) const = 0;

    /**
     * @brief Set the current SOK time.
     *
     * @param[in] newSokTime - the new time to be set in SOK.
     *
     * @return true/false based on the set operation.
     */
    virtual bool SetTimeInfoCurrentSokTime(uint64_t newSokTime) const = 0;

    /**
     * @brief Set the maxJitterExceeded info.
     *
     * @param[in] maxJitterExceeded - true if the maxJitter is exceeded, false otherwise.
     *
     * @return true/false based on the operation result.
     */
    virtual bool SetTimeInfoJitterStatus(bool maxJitterExceeded) const = 0;

    /**
     * @brief Set the freshness info status.
     *
     * @param[in] freshnessId - the id of the freshness to be set.
     * @param[in] status - the status to be set.
     *
     * @return true/false based on the set operation result.
     */
    virtual bool SetFreshnessInfoStatus(SokFreshnessValueId const&     freshnessId,
                                        FvmDiagnosticsErrorCode const& status) const
        = 0;

    /**
     * @brief Set the missing key status.
     *
     * @param[in] keyId - the id of the key to be set.
     * @param[in] status - the status to be set.
     *
     * @return true/false - based on the set operation.
     */
    virtual bool SetMissingKeyStatus(uint16_t keyId, FvmDiagnosticsErrorCode const& status) const = 0;
};


}  // namespace fvm
}  // namespace sok

#endif  // I_FVM_DIAGNOSTICS_WRITER_HPP