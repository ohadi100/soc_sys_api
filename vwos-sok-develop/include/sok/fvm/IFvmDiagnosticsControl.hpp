/* Copyright (c) 2023 Volkswagen Group */

#ifndef I_FVM_DIAGNOSTICS_CONTROL_HPP
#define I_FVM_DIAGNOSTICS_CONTROL_HPP

#include "sok/fvm/FreshnessValueManagerDefinitions.hpp"
#include "sok/fvm/FvmDiagnosticsDefinitions.hpp"
#include "sok/fvm/FvmDiagnosticsError.hpp"

namespace sok
{
namespace fvm
{

class IFvmDiagnosticsControl
{
public:
    virtual ~IFvmDiagnosticsControl() = default;

    /**
     * @brief The SOK-function start deactivation routine.
     *
     * @param[in] command - the activate or deactivate command.
     *
     * @return true/false depending on the operation status.
     */
    virtual bool SokFunctionDeactivationStartRoutine(FvmFunctionActivateDeactivateCommand const& command) const = 0;

    /**
     * @brief The SOK-function (de)activation result request routine.
     *
     * The function is returning the previously requested activate/deactivate operation status.
     *
     * @return true/false, depending on the operation status.
     */
    virtual bool SokFunctionDeactivationRequestResultsRoutine() const = 0;
};


}  // namespace fvm
}  // namespace sok

#endif  // I_FVM_DIAGNOSTICS_CONTROL_HPP