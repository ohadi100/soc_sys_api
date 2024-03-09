/* Copyright (c) 2023 Volkswagen Group */

#include "sok/common/SokCommonInternalFactory.hpp"
#ifdef UNIT_TESTS
#include "MockCsmAccessor.hpp"
#else
#include "sok/common/CsmAccessorAraCrypto.hpp"
#endif
namespace sok
{
namespace common
{

std::shared_ptr<common::ICsmAccessor>    
SokCommonInternalFactory::CreateCsmAccessor()
{
#ifdef UNIT_TESTS
    return std::make_shared<common::UTCsmAccessor>();
#else
    return std::make_shared<common::CsmAccessorAraCrypto>();
#endif  // UNIT_TESTS
}

}  // namespace common
}  // namespace sok
