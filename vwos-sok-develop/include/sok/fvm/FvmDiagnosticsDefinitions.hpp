/* Copyright (c) 2023 Volkswagen Group */

#ifndef FVM_DIAGNOSTICS_DEFINITIONS_HPP
#define FVM_DIAGNOSTICS_DEFINITIONS_HPP

#include <cstdint>

namespace sok
{
namespace fvm
{

enum class FvmDiagnosticsParameter : uint16_t {
    kVerificationFailedList = 0x018BU,
    kSignatureFailedList = 0x018FU,
    kGeneralInfo = 0x0190U,
    kTimeInfo = 0x0191U,
    kFreshnessInfo = 0x0192U,
    kMissingKeyList = 0x0194U
};

enum class FvmFunctionActivateDeactivateCommand : uint8_t {
    kFvmFunctionDeactivate = 0U,
    kFvmFunctionActivate = 1U
};

enum class FvmSokEcuFunction : uint8_t {
    kFvmVwSokTimeServer = 0U,
    kFvmVwSokParticipant = 1U
};

} // namespace fvm
} // namespace sok

#endif // FVM_DIAGNOSTICS_DEFINITIONS_HPP