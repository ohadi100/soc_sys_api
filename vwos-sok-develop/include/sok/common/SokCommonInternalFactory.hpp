/* Copyright (c) 2023 Volkswagen Group */

#ifndef SOK_COMMON_INTERNAL_FACTORY_HPP
#define SOK_COMMON_INTERNAL_FACTORY_HPP

#include <memory>
#include "sok/common/ICsmAccessor.hpp"

namespace sok
{
namespace common
{

class SokCommonInternalFactory
{
public:
    static std::shared_ptr<ICsmAccessor>      CreateCsmAccessor();
};

}  // namespace common
}  // namespace sok

#endif  // SOK_COMMON_INTERNAL_FACTORY_HPP