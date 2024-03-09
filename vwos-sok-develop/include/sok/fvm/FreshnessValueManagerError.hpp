/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_MANAGER_ERROR_HPP
#define FRESHNESS_VALUE_MANAGER_ERROR_HPP

#include "sok/common/ErrorOrObjectResult.hpp"

namespace sok
{
namespace fvm
{

enum class FvmErrorCode : uint8_t {
    kSuccess = 0U,
    kFVNotAvailable = 1U,
    kFVInitializeFailed = 2U,
    kFvIdNotFound,
    kNotInitialized,
    kAlreadyInitialized,
    kGeneralError,
    kRngError,
    kKeyNotFound,

    kEndEnum
};

template <typename T>
using FvmResult     = common::ErrorOrObjectResult<FvmErrorCode, T>;

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_MANAGER_ERROR_HPP