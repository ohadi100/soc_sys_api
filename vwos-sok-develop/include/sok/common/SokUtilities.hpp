/* Copyright (c) 2023 Volkswagen Group */

#ifndef SOK_UTILITIES_HPP
#define SOK_UTILITIES_HPP

#include <cstdint>
#include <vector>

namespace sok
{
namespace common
{

template <typename T>
std::vector<uint8_t>
UintToByteVector(T const num)
{
    std::vector<uint8_t> ret(sizeof(T));
    for (size_t i = 0; i < sizeof(T); i++) {
        ret[sizeof(T) - 1 - i] = (static_cast<uint8_t>(num >> (i * 8)));
    }
    return ret;
}

template <typename T>
std::vector<uint8_t>
UintToByteVectorLE(T const num)
{
    std::vector<uint8_t> ret(sizeof(T));
    for (size_t i = 0; i < sizeof(T); i++) {
        ret[i] = (static_cast<uint8_t>(num >> (i * 8)));
    }
    return ret;
}

template <typename T>
std::vector<uint8_t>
UintToByteVectorTrim(T const num, size_t numBytes)
{
    if (numBytes > sizeof(T)) {
        return {};
    }
    std::vector<uint8_t> ret(numBytes);
    for (size_t i = 0; i < numBytes; i++) {
        ret[numBytes - 1 - i] = (static_cast<uint8_t>(num >> (i * 8)));
    }
    return ret;
}

template <typename T>
T
ByteVectorToUint(std::vector<uint8_t> const& bytes)
{
    if (bytes.size() > sizeof(T)) {
        return 0;
    }
    T result = 0;    
    for (size_t i = 0; i < bytes.size(); i++) {
        result = static_cast<T>((result << 8) + bytes[i]);
    }
    return result;
}

} // namespace common
} // namespace sok

#endif  // SOK_UTILITIES_HPP