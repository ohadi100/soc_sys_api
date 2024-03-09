/* Copyright (c) 2023 Volkswagen Group */

#ifndef MOCK_CSM_ACCESSOR_HPP
#define MOCK_CSM_ACCESSOR_HPP

#include <gmock/gmock.h>
#include "sok/common/ICsmAccessor.hpp"

namespace sok
{
namespace common    
{

class MockCsmAccessor : public ICsmAccessor
{
public:
    MOCK_METHOD(CsmResult<std::vector<uint8_t>>, MacCreate, (uint16_t, std::vector<uint8_t> const&, MacAlgorithm), (const, override));
    MOCK_METHOD(CsmErrorCode, MacVerify, (uint16_t, std::vector<uint8_t> const&, std::vector<uint8_t> const&, MacAlgorithm), (const, override));
    MOCK_METHOD(CsmErrorCode, IsKeyExists, (uint16_t), (const, override));
    MOCK_METHOD(CsmResult<std::vector<uint8_t>>, GenerateRandomBytes, (uint8_t), (const, override));
};

class UTCsmAccessor : public ICsmAccessor
{
public:
    CsmResult<std::vector<uint8_t>> 
    MacCreate(uint16_t keyId, std::vector<uint8_t> const& data, MacAlgorithm alg) const override
    {
        return mMockCsm->MacCreate(keyId, data, alg);
    }

    CsmErrorCode 
    MacVerify(uint16_t keyId, std::vector<uint8_t> const& data, std::vector<uint8_t> const& mac, MacAlgorithm alg) const override
    {
        return mMockCsm->MacVerify(keyId, data, mac, alg);
    }

    CsmErrorCode 
    IsKeyExists(uint16_t keyId) const override
    {
        return mMockCsm->IsKeyExists(keyId);
    }

    CsmResult<std::vector<uint8_t>> 
    GenerateRandomBytes(uint8_t size) const override
    {
        return mMockCsm->GenerateRandomBytes(size);
    }

    static MockCsmAccessor* mMockCsm;
};

}  // namespace common
}  // namespace sok

#endif  // MOCK_CSM_ACCESSOR_HPP