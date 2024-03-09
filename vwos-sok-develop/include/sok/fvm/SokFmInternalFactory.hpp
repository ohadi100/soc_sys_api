/* Copyright (c) 2023 Volkswagen Group */

#ifndef SOK_FM_INTERNAL_FACTORY_HPP
#define SOK_FM_INTERNAL_FACTORY_HPP

#include <memory>
#include "sok/fvm/ISignalManager.hpp"
#include "sok/fvm/IFreshnessValueManagerConfigAccessor.hpp"
#include "sok/fvm/IFvmRuntimeAttributesManager.hpp"

namespace sok
{
namespace fvm
{

class SokFmInternalFactory
{
public:
    static std::shared_ptr<fvm::ISignalManager>      CreateSignalManager();
    static std::shared_ptr<fvm::IFreshnessValueManagerConfigAccessor>      CreateFreshnessValueManagerConfigAccessor();
    static std::shared_ptr<fvm::IFvmRuntimeAttributesManager>      CreateFvmRuntimeAttributesManager();
};

}  // namespace fvm
}  // namespace sok

#endif  // SOK_FM_INTERNAL_FACTORY_HPP