/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/SokFmInternalFactory.hpp"
#ifdef UNIT_TESTS
#include "MockSignalManager.hpp"
#include "MockFreshnessValueManagerConfigAccessor.hpp"
#include "MockFvmRuntimeAttributesManager.hpp"
#else
#include "sok/fvm/SignalManagerSci.hpp"
#include "sok/fvm/FreshnessValueManagerConfigAccessor.hpp"
#include "sok/fvm/FvmRuntimeAttributesManager.hpp"
#endif  // UNIT_TESTS

namespace sok
{
namespace fvm
{

std::shared_ptr<ISignalManager>
SokFmInternalFactory::CreateSignalManager()
{
#ifdef UNIT_TESTS
    return std::make_shared<UTSignalManager>();
#else
    return std::make_shared<SignalManagerSci>();
#endif  // UNIT_TESTS
}

std::shared_ptr<IFreshnessValueManagerConfigAccessor>
SokFmInternalFactory::CreateFreshnessValueManagerConfigAccessor()
{
#ifdef UNIT_TESTS
    return std::make_shared<UTFreshnessValueManagerConfigAccessor>();
#else
    return FreshnessValueManagerConfigAccessor::GetInstance();
#endif  // UNIT_TESTS
}

std::shared_ptr<IFvmRuntimeAttributesManager>
SokFmInternalFactory::CreateFvmRuntimeAttributesManager()
{
#ifdef UNIT_TESTS
    return std::make_shared<UTFvmRuntimeAttributesManager>();
#else
    return std::make_shared<FvmRuntimeAttributesManager>();
#endif  // UNIT_TESTS
}

}  // namespace fvm
}  // namespace sok
