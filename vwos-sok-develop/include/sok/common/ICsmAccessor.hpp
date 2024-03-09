/* Copyright (c) 2023 Volkswagen Group */

#ifndef I_CSM_ACCESSOR_HPP
#define I_CSM_ACCESSOR_HPP

#include <vector>
#include "sok/common/CommonError.hpp"
#include "sok/common/CommonDefinitions.hpp"

namespace sok {
namespace common {

class ICsmAccessor {
public:
    virtual ~ICsmAccessor() = default;

    /**
     * @brief Creates MAC
     * 
     * @param keyId symmetric key identifier to create the MAC with
     * @param data data to calculate the MAC over
     * @param alg algorithm of the MAC
     * @return CsmResult<std::vector<uint8_t>> Result object containing the generated MAC on success, error code otherwise
     */
    virtual CsmResult<std::vector<uint8_t>> MacCreate(uint16_t keyId, std::vector<uint8_t> const& data, MacAlgorithm alg) const = 0;

    /**
     * @brief Verifies MAC
     * 
     * @param keyId symmetric key identifier to verify the MAC with
     * @param data the data to verify
     * @param mac the authenticator
     * @param alg algorithm of the MAC
     * @return CsmErrorCode kSuccess upon success, error code otherwise
     */
    virtual CsmErrorCode MacVerify(uint16_t keyId, std::vector<uint8_t> const& data, std::vector<uint8_t> const& mac, MacAlgorithm alg) const = 0;

    /**
     * @brief Checks if the provided key identifier exists
     * 
     * @param keyId key identifier
     * @return CsmErrorCode kSuccess if key exists, error code otherwise
     */
    virtual CsmErrorCode IsKeyExists(uint16_t keyId) const = 0;

    /**
     * @brief Generates random vector of bytes at the provided size
     * 
     * @param size amount of bytes to generate
     * @return CsmResult<std::vector<uint8_t>> Result object containing the generated byte vector, error code otherwise 
     */
    virtual CsmResult<std::vector<uint8_t>> GenerateRandomBytes(uint8_t size) const = 0;
};

} // namespace common
} // namespace sok

#endif // I_CSM_ACCESSOR_HPP