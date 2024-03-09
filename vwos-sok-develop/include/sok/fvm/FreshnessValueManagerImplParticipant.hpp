/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_MANAGER_IMPL_PARTICIPANT_HPP
#define FRESHNESS_VALUE_MANAGER_IMPL_PARTICIPANT_HPP

#include "AFreshnessValueManagerImpl.hpp"
#include "FreshnessValueStateManager.hpp"
#include <mutex>

namespace sok
{
namespace fvm
{

class FreshnessValueManagerImplParticipant : public AFreshnessValueManagerImpl
{  
public:
    FreshnessValueManagerImplParticipant();
    virtual ~FreshnessValueManagerImplParticipant() = default;

    /**
     * @brief This function must be called before SOK-protected communication can begin.
     *        This function must be called every `SokFmMainFunctionPeriod` milliseconds after the first call.
     * 
     * 
     * @return FvmErrorCode 
     */
    FvmErrorCode MainFunction() noexcept override;

    /**
     * @brief register for the FV distribution specific signals
     * 
     * @return true on success
     * @return false on failure
     */
    bool serverOrParticipantInit() noexcept override;

private:
    void incomingAuthFvSignalsCb(std::string const& signal, std::vector<uint8_t> const& value);
    void incomingUnAuthFvSignalsCb(std::string const& signal, std::vector<uint8_t> const& value);  
    FvmErrorCode requestAnAuthenticFv();
    FvmErrorCode waitForAnAuthenticFv();
    FvmErrorCode processAnAuthenticFv();
    FvmErrorCode processAnUnauthenticFv();

    std::shared_ptr<FreshnessValueStateManager> mFvStateManager;
    uint32_t mTimeSinceAuthFvReq;
    FVContainer mActiveFvChallenge;
    FVContainer mUnAuthFv;
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> mCrAuthFvAndMac;
    uint16_t mEcuKeyIdForFvDistribution;
    std::mutex mRecFVMutex;
    std::mutex mRecUnauthFvMutex;
};

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_MANAGER_IMPL_PARTICIPANT_HPP