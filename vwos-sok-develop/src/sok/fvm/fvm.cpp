/* Copyright (c) 2023 Volkswagen Group */

#include <ara/com/secoc/fvm.h>
#include <algorithm>
#include <climits>
#include "sok/fvm/FreshnessValueManager.hpp"
#include "sok/common/Logger.hpp"

namespace ara {
namespace com {
namespace secoc {

FVM::FVM() noexcept
{
}

FVM::~FVM() noexcept
{
    try {
        if (sok::fvm::FvmErrorCode::kSuccess != sok::fvm::FreshnessValueManager::GetInstance()->Deinit()) {
            LOGE("Deinitialization of FVM has failed");
        }
    }
    catch(...) {
        LOGE("Caught unexpected exception");
    }
}

auto 
FVM::GetRxFreshness(std::uint16_t SecOCFreshnessValueID, FVContainer const& SecOCTruncatedFreshnessValue,
                      std::uint16_t SecOCAuthVerifyAttempts) noexcept -> ara::core::Result<FVContainer, SecOcFvmErrc>
{
    try {
        sok::fvm::FVContainer fvCont;
        fvCont.reserve(SecOCTruncatedFreshnessValue.length);
        std::copy_n(std::make_move_iterator(SecOCTruncatedFreshnessValue.value.begin()), SecOCTruncatedFreshnessValue.length, fvCont.begin());

        auto result = sok::fvm::FreshnessValueManager::GetInstance()->GetRxFreshness(SecOCFreshnessValueID, fvCont, SecOCAuthVerifyAttempts);
        if (result.isFailed()) {
            LOGE("Failed get rx freshness value");
            return ara::core::Result<FVContainer, SecOcFvmErrc>(ara::com::secoc::SecOcFvmErrc::kFVNotAvailable);
        }

        auto fv = result.getObject();
        FVContainer retFv;
        retFv.length = fv.size() * CHAR_BIT;
        std::copy_n(std::make_move_iterator(fv.begin()), retFv.length, retFv.value.begin());
                return ara::core::Result<FVContainer, SecOcFvmErrc>(retFv);
    }
    catch(...) {
        LOGE("Caught unexpected exception");
        return ara::core::Result<FVContainer, SecOcFvmErrc>(ara::com::secoc::SecOcFvmErrc::kFVNotAvailable);
    }
}

auto 
FVM::GetTxFreshness(std::uint16_t SecOCFreshnessValueID) noexcept -> ara::core::Result<FVContainer, SecOcFvmErrc>
{   
    try {
        auto result = sok::fvm::FreshnessValueManager::GetInstance()->GetTxFreshness(SecOCFreshnessValueID);
        if (result.isFailed()) {
            LOGE("Failed get tx freshness value");
            return ara::core::Result<FVContainer, SecOcFvmErrc>(ara::com::secoc::SecOcFvmErrc::kFVNotAvailable);
        }

        auto fv = result.getObject();
        FVContainer retFv;
        retFv.length = fv.size() * CHAR_BIT;
        std::copy_n(std::make_move_iterator(fv.begin()), retFv.length, retFv.value.begin());
                return ara::core::Result<FVContainer, SecOcFvmErrc>(retFv);
    }
    catch(...) {
        LOGE("Caught unexpected exception");
        return ara::core::Result<FVContainer, SecOcFvmErrc>(ara::com::secoc::SecOcFvmErrc::kFVNotAvailable);
    }
}

auto 
FVM::Initialize() noexcept -> ara::core::Result<void>
{
    try {
        if (sok::fvm::FvmErrorCode::kSuccess != sok::fvm::FreshnessValueManager::GetInstance()->Init()) {
            LOGE("Initialization of FVM has failed");
            return ara::core::Result<void>(MakeErrorCode(ara::com::secoc::SecOcFvmErrc::kFVInitializeFailed));
        }
        return {};
    }
    catch(...) {
        LOGE("Caught unexpected exception");
        return ara::core::Result<void>(MakeErrorCode(ara::com::secoc::SecOcFvmErrc::kFVInitializeFailed));
    }
}


} // secoc
} // com
} // ara