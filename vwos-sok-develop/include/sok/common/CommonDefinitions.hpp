/* Copyright (c) 2023 Volkswagen Group */

#ifndef COMMON_DEFINITIONS_HPP
#define COMMON_DEFINITIONS_HPP

#include <cstdint>

namespace sok
{
namespace common
{

enum class MacAlgorithm : uint8_t {
    kSipHash24 = 0U,
    kAes128Cmac,
    kEndEnum
};

} // namespace common
} // namespace sok

#endif // COMMON_DEFINITIONS_HPP