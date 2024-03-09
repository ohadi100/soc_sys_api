/* Copyright (c) 2023 Volkswagen Group */

#ifndef FVM_DIAGNOSTICS_ERROR_HPP
#define FVM_DIAGNOSTICS_ERROR_HPP

#include "sok/common/ErrorOrObjectResult.hpp"

namespace sok
{
namespace fvm
{

enum class FvmDiagnosticsErrorCode : uint8_t
{
    kSuccess            = 0U,
    kVerificationFailed = 1U,
    kSignatureFailed    = 2U,
    kGeneralInformation = 3U,
    kMissingKeys        = 4U,
    kFailed             = 5U,
};

template <typename T>
using FvmDiagnosticsResult = common::ErrorOrObjectResult<FvmDiagnosticsErrorCode, T>;

enum class FvmFunctionActivateDeactivateErrorCode : uint8_t
{
    kSuccess = 0U,
    kError = 1U
};

}  // namespace fvm
}  // namespace sok

#endif  // FVM_DIAGNOSTICS_ERROR_HPP