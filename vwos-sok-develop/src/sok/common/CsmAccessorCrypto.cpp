/* Copyright (c) 2023 Volkswagen Group */

#include "sok/common/CsmAccessorCrypto.hpp"
#include <e3crypto/RandomNumberGenerator.hpp>
#include "sok/common/Logger.hpp"

using vwg::e3p::sec::libe3crypto::CryptoResultCode;

namespace sok {
namespace common {

CsmAccessorCrypto::CsmAccessorCrypto() 
{
}

CsmResult<std::vector<uint8_t>> 
CsmAccessorCrypto::MacCreate(uint16_t keyId, std::vector<uint8_t> const& data, MacAlgorithm alg) const
{
    (void)keyId;
    (void)alg;
    (void)data;

    return CsmResult<std::vector<uint8_t>>({});
}

CsmErrorCode 
CsmAccessorCrypto::MacVerify(uint16_t keyId, std::vector<uint8_t> const& data, std::vector<uint8_t> const& mac, MacAlgorithm alg) const
{
    (void)keyId;
    (void)alg;
    (void)data;
    (void)mac;
    return CsmErrorCode::kSuccess;
}

CsmErrorCode 
CsmAccessorCrypto::IsKeyExists(uint16_t keyId) const
{
    (void)keyId;
    return CsmErrorCode::kSuccess;
}

CsmResult<std::vector<uint8_t>> 
CsmAccessorCrypto::GenerateRandomBytes(uint8_t size) const
{
    std::vector<uint8_t> buffer(size);
    auto result = vwg::e3p::sec::libe3crypto::RandomNumberGenerator::getRandom(&buffer.data()[0], size);
    if (CryptoResultCode::kSuccess != result) {
        LOGE("Failed generating random number of size: " << size);
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kErrorRng);
    }
    return CsmResult<std::vector<uint8_t>>(buffer);
}

} // namespace common
} // namespace sok