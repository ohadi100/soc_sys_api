/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_MANAGER_HPP
#define FRESHNESS_VALUE_MANAGER_HPP

#include <memory>
#include "FreshnessValueManagerError.hpp"
#include "FreshnessValueManagerDefinitions.hpp"

namespace sok
{
namespace fvm
{

// forward declaration for the abstract impl class
class AFreshnessValueManagerImpl;

class FreshnessValueManager 
{
public:
    static std::shared_ptr<FreshnessValueManager> GetInstance();
    ~FreshnessValueManager();
    /**
     * @brief This method is used by the SecOC to obtain the current freshness value for validation of incoming secured I-PDUs
     * 
     * @param SecOCFreshnessValueID the identifier of the freshness value
     * @param SecOCTruncatedFreshnessValue optional - A shortened part of the freshness value may be transmitted by the SecOC as "Truncated Freshness Value" in the secured I-PDU. In the context of SOK, the truncated freshness value is used to transmit the session counter if one is used. Otherwise, the length of the Truncated Freshness Value is zero.
     * @param SecOCAuthVerifyAttempts the number of authentication verify attempts of this I-PDU/message since the last reception. The value is 0 for the first attempt and incremented on every unsuccessful verification attempt up to a configured `SecOCAuthenticationVerifyAttempts`.
     * @return FvmResult<FVContainer> freshness value container that holds the freshness value to be used for the calculation of the the authenticator by the SecOC or recoverable error.
     */
    FvmResult<FVContainer> GetRxFreshness(SokFreshnessValueId SecOCFreshnessValueID, const FVContainer &SecOCTruncatedFreshnessValue, uint16_t SecOCAuthVerifyAttempts) noexcept;

    /**
     * @brief This method is used by the SecOC to obtain the current freshness value for creation of outgoing secure I-PDUs
     * 
     * @param SecOCFreshnessValueID the identifier of the freshness value
     * @return FvmResult<FVContainer> freshness value container that holds the freshness value to be used for the calculation of the the authenticator by the SecOC or recoverable error.
     */
    FvmResult<FVContainer> GetTxFreshness(SokFreshnessValueId SecOCFreshnessValueID) noexcept;

    /**
     * @brief This function receives the result of a signature verification from the SecOC module.
     * 
     * @param [in] SecOC_VerificationStatusType - Data structure to bundle the status of a verification attempt for a specific Freshness Value and Data ID
     */
    void VerificationStatusCallout(SecOC_VerificationStatusType verificationStatus) noexcept;

    /**
     * @brief This interface is used by the SecOC to indicate that the Secured I-PDU has been initiated for transmission.
     * 
     * @param [in] SecOCFreshnessValueID - Holds the identifier of the freshness value.
     */
    void SPduTxConfirmation(SokFreshnessValueId SecOCFreshnessValueID) noexcept;

    /**
     * @brief This function must be called before SOK-protected communication can begin.
     *        This function must be called every `SokFmMainFunctionPeriod` milliseconds after the first call.
     * 
     * 
     * @return FvmErrorCode 
     */
    FvmErrorCode MainFunction() noexcept;

    /**
     * @brief This function resets the internal state of SOK-FM to its initial value and checks the status of the VKMS keys used by SOK.
     * 
     * @return FvmErrorCode no return value in case of success, kFVInitializeFailed otherwise.
     */
    FvmErrorCode Init() noexcept;

    /**
     * @brief This function sets all global status variables to their initial values.
     * This function is to be called as soon as the ECU can no longer guarantee that the function 
     * MainFunction() is called at the specified intervals, e.g. when the ECU goes to sleep. 
     * Furthermore the status of all SOK protocols is reset. The function puts the SOK-FM in an inactive 
     * state with respect to SOK time until the next time the SokFm_MainFunction() is called. Challenge-response protocols can still be executed.
     * 
     * @return FvmErrorCode 
     */
    FvmErrorCode Deinit() noexcept;

    /**
     * @brief  Trigger the transmission of a challenge (consumer side)
     * 
     */
    FvmErrorCode TriggerCrRequest(SokFreshnessValueId SecOCFreshnessValueID);

    /**
     * @brief Offer a CR service (producer side)
     * 
     * @param SecOCFreshnessValueID freshness ID for the CR
     * @param cb a callback to be called to indicate that a challenge matching the provided SecOCFreshnessValueID has arrived
     * @return FvmErrorCode if offer registration has succeeded, error code otherwise
     */
    FvmErrorCode OfferCrRequest(SokFreshnessValueId SecOCFreshnessValueID, ChallengeReceivedIndicationCb const& cb);


    FreshnessValueManager(const FreshnessValueManager&)            = delete;
    FreshnessValueManager(FreshnessValueManager&&)                 = delete;
    FreshnessValueManager& operator=(const FreshnessValueManager&) = delete;
    FreshnessValueManager& operator=(FreshnessValueManager&&)      = delete;

private:
    /*
     * Singleton class private constructor
     */
    FreshnessValueManager();

private:
    std::unique_ptr<AFreshnessValueManagerImpl> pImpl;
};

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_MANAGER_HPP