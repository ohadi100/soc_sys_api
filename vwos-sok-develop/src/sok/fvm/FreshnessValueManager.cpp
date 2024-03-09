/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/FreshnessValueManager.hpp"
#include "sok/fvm/AFreshnessValueManagerImpl.hpp"
#include "sok/fvm/FreshnessValueManagerImplParticipant.hpp"
#include "sok/fvm/FreshnessValueManagerImplServer.hpp"

namespace sok
{
namespace fvm
{

std::shared_ptr<FreshnessValueManager> 
FreshnessValueManager::GetInstance()
{
    static FreshnessValueManager                         fvm;
    static std::shared_ptr<FreshnessValueManager> const fvmSP(&fvm, [](FreshnessValueManager*) {
    });
    return fvmSP;
}

FreshnessValueManager::FreshnessValueManager()
{
#ifdef SOK_FVM_SERVER
    pImpl = std::make_unique<FreshnessValueManagerImplServer>();
#else
    pImpl = std::make_unique<FreshnessValueManagerImplParticipant>();
#endif
}

FreshnessValueManager::~FreshnessValueManager() = default;

FvmResult<FVContainer> 
FreshnessValueManager::GetRxFreshness(SokFreshnessValueId SecOCFreshnessValueID, const FVContainer &SecOCTruncatedFreshnessValue, uint16_t SecOCAuthVerifyAttempts) noexcept
{
    return pImpl->GetRxFreshness(SecOCFreshnessValueID, SecOCTruncatedFreshnessValue, SecOCAuthVerifyAttempts);
}

FvmResult<FVContainer>
FreshnessValueManager::GetTxFreshness(SokFreshnessValueId SecOCFreshnessValueID) noexcept
{
    return pImpl->GetTxFreshness(SecOCFreshnessValueID);
}

void 
FreshnessValueManager::VerificationStatusCallout(SecOC_VerificationStatusType verificationStatus) noexcept
{
    pImpl->VerificationStatusCallout(verificationStatus);
}

void 
FreshnessValueManager::SPduTxConfirmation(SokFreshnessValueId SecOCFreshnessValueID) noexcept
{
    pImpl->SPduTxConfirmation(SecOCFreshnessValueID);
}

FvmErrorCode 
FreshnessValueManager::MainFunction() noexcept
{
    return pImpl->MainFunction();
}

FvmErrorCode 
FreshnessValueManager::Init() noexcept
{
    return pImpl->Init();
}

FvmErrorCode 
FreshnessValueManager::Deinit() noexcept
{
    return pImpl->Deinit();
}

FvmErrorCode 
FreshnessValueManager::TriggerCrRequest(SokFreshnessValueId SecOCFreshnessValueID)
{
    return pImpl->TriggerCrRequest(SecOCFreshnessValueID);
}

FvmErrorCode 
FreshnessValueManager::OfferCrRequest(SokFreshnessValueId SecOCFreshnessValueID, ChallengeReceivedIndicationCb const& cb)
{
    return pImpl->OfferCrRequest(SecOCFreshnessValueID, cb);
}

} // namespace fvm
} // namespace sok
