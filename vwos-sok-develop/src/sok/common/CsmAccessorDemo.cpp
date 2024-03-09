/* Copyright (c) 2023 Volkswagen Group */

#include "sok/common/CsmAccessorDemo.hpp"
#include <array>
#include <random>
#include <ctime>
#include <climits>
#include <algorithm>
#include <functional>
#include "sok/common/SokUtilities.hpp"

using random_bytes_engine = std::independent_bits_engine<
    std::default_random_engine, CHAR_BIT, unsigned char>;

namespace sok {
namespace common {

CsmAccessorDemo::CsmAccessorDemo() 
{
    std::srand(static_cast<uint>(std::time(nullptr)));
}

CsmResult<std::vector<uint8_t>> 
CsmAccessorDemo::MacCreate(uint16_t keyId, std::vector<uint8_t> const& data, MacAlgorithm alg) const
{
    (void)keyId;
    (void)alg;
    std::vector<uint8_t> mac = hash(data);

    return CsmResult<std::vector<uint8_t>>(mac);
}

CsmErrorCode 
CsmAccessorDemo::MacVerify(uint16_t keyId, std::vector<uint8_t> const& data, std::vector<uint8_t> const& mac, MacAlgorithm alg) const
{
    (void)keyId;
    (void)alg;
    if(hash(data) != mac){
        return CsmErrorCode::kErrorRng;
    }
    return CsmErrorCode::kSuccess;
}

CsmErrorCode 
CsmAccessorDemo::IsKeyExists(uint16_t keyId) const
{
    (void)keyId;
    return CsmErrorCode::kSuccess;
}

CsmResult<std::vector<uint8_t>> 
CsmAccessorDemo::GenerateRandomBytes(uint8_t size) const
{
    random_bytes_engine rbe(static_cast<uint8_t>(rand() % UINT8_MAX));
    std::vector<uint8_t> randomVec(size);

    std::generate(begin(randomVec), end(randomVec), std::ref(rbe));

    return CsmResult<std::vector<uint8_t>>(randomVec);
}

std::vector<uint8_t>
CsmAccessorDemo::hash(const std::vector<std::uint8_t>& data) const
{
    size_t hash = std::hash<std::string>{}(std::string(data.begin(), data.end()));

    return UintToByteVector<size_t>(hash);
}

} // namespace common
} // namespace sok