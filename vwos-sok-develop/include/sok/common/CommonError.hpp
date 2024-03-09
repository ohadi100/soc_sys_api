/* Copyright (c) 2023 Volkswagen Group */

#ifndef COMMON_ERROR_HPP
#define COMMON_ERROR_HPP

#include "sok/common/ErrorOrObjectResult.hpp"

namespace sok
{
namespace common
{

enum class CsmErrorCode : uint8_t {
    kSuccess = 0U,
    kKeyNotFound = 1U,
    kErrorRng = 2U,
    kError = 3U
};

template <typename T>
using CsmResult     = ErrorOrObjectResult<CsmErrorCode, T>;

} // namespace common
} // namespace sok

#endif // COMMON_ERROR_HPP