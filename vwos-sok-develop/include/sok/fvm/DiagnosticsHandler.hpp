/* Copyright (c) 2023 Volkswagen Group */

#ifndef DIAGNOSTICS_HANDLER_HPP
#define DIAGNOSTICS_HANDLER_HPP

#include <memory>


// forward declaration
namespace e3 {
namespace diag {
class DiagnosticDataProtection;
} //diag
} //e3

namespace sok
{
namespace fvm
{


class DiagnosticsHandler {
    DiagnosticsHandler();
private:
    std::shared_ptr<e3::diag::DiagnosticDataProtection> mProtection;
};

} // namespace fvm
} // namespace sok

#endif // DIAGNOSTICS_HANDLER_HPP