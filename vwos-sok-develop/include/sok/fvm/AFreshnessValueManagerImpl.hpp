/* Copyright (c) 2023 Volkswagen Group */

#ifndef A_FRESHNESS_VALUE_MANAGER_IMPL_HPP
#define A_FRESHNESS_VALUE_MANAGER_IMPL_HPP

#include <memory>
#include <atomic>
#include "FreshnessValueManagerError.hpp"
#include "FreshnessValueManagerDefinitions.hpp"
#include "FreshnessValueManagerConfigAccessor.hpp"
#include "IFvmRuntimeAttributesManager.hpp"
#include "ISignalManager.hpp"
#include "sok/common/ICsmAccessor.hpp"

namespace sok
{
namespace fvm
{

class AFreshnessValueManagerImpl
{
public:
    AFreshnessValueManagerImpl();
    virtual ~AFreshnessValueManagerImpl() = default;

    /**
     * @brief This function must be called before SOK-protected communication can begin.
     *        This function must be called every `SokFmMainFunctionPeriod` milliseconds after the first call.
     * 
     * 
     * @return FvmErrorCode 
     */
    virtual FvmErrorCode MainFunction() noexcept = 0;

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
     * @brief  Trigger the transmission of a challenge
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

protected:
    /**
     * @brief subscribes to signals of interest
     * 
     * @return bool result of subscription
     */
    bool registerToSignals() noexcept;

    /**
     * @brief increments internal timers, must be called by inheritors with each `MainFunction()` call
     * 
     */
    void incTimers() noexcept;

    /**
     * @brief does specific initialization actions. e.g.: registering for the FV distribution specific signals.
     * 
     * @return true on success
     * @return false on failure
     */
    virtual bool serverOrParticipantInit() noexcept = 0;

private:
    void incomingChallengeSignalCb(std::string const& signal, std::vector<uint8_t> const& value);
    FvmResult<FVContainer> getChallengeForIncomingResponse(SokFreshnessValueId SecOCFreshnessValueID, FVContainer const& SecOCTruncatedFreshnessValue);
    FvmResult<FVContainer> getChallengeForOutgoingResponse(SokFreshnessValueId SecOCFreshnessValueID);
    FvmResult<FVContainer> getRxFv(SokFreshnessValueId SecOCFreshnessValueID, SokFreshnessType type, FVContainer const& SecOCTruncatedFreshnessValue, uint16_t SecOCAuthVerifyAttempts);
    FvmResult<FVContainer> getTxFv(SokFreshnessValueId SecOCFreshnessValueID, SokFreshnessType type);
    void updateVerificationStatusChallenge(SokFreshnessValueId SecOCFreshnessValueID, bool verificationSucceeded);
    void updateVerificationStatusAuthBroadcast(SokFreshnessValueId SecOCFreshnessValueID, bool verificationSucceeded);

protected:
    std::atomic_bool mInitialized;
    std::atomic_bool mIsFvValid;
    uint64_t mTimeSinceInit;
    uint32_t mClockCount;
    std::atomic_uint64_t mFV;
    std::unordered_map<SokFreshnessValueId, std::pair<uint64_t, std::vector<uint8_t>>> mActiveOutgoingChallenges;
    std::unordered_map<SokFreshnessValueId, std::pair<uint64_t, std::vector<uint8_t>>> mActiveIncomingChallenges;
    std::unordered_map<std::string, SokFreshnessValueId> mChallengeSignalToFvId;
    std::unordered_map<SokFreshnessValueId, ChallengeReceivedIndicationCb> mFvIdToNotificationCallback;
    std::unordered_map<SokFreshnessValueId, std::vector<uint64_t>> mFvRxCandidates;
    std::shared_ptr<common::ICsmAccessor> mCsmAccessor;
    std::shared_ptr<ISignalManager> mSignalManager;
    std::shared_ptr<IFvmRuntimeAttributesManager> mAttrMgr;
    std::shared_ptr<IFreshnessValueManagerConfigAccessor> mFvmConfAccessor;
};

} // namespace fvm
} // namespace sok

#endif // A_FRESHNESS_VALUE_MANAGER_IMPL_HPP