/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/DiagnosticsHandler.hpp"
#include "e3/diag/ExtendedDiagnosticController.hpp"

namespace sok
{
namespace fvm
{

DiagnosticsHandler::DiagnosticsHandler()
: mProtection(e3::diag::ExtendedDiagnosticController::make()->getDiagnosticDataProtection())
{
}


} // namespace fvm
} // namespace sok
