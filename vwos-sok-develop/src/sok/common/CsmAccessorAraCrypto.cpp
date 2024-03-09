/* Copyright (c) 2023 Volkswagen Group */

#include "sok/common/CsmAccessorAraCrypto.hpp"
#include "sok/common/Logger.hpp"
#include <ara/crypto/keys/entry_point.h>
#include <ara/crypto/cryp/symmetric_key_wrapper_ctx.h>
#include <ara/crypto/cryp/symmetric_key_context.h>
#include "sok/common/SokUtilities.hpp"


using ara::crypto::cryp::CryptoProvider;

namespace sok {
namespace common {

CsmAccessorAraCrypto::CsmAccessorAraCrypto() 
{
    auto cryptoProvider = ara::crypto::cryp::LoadCryptoProvider(nullptr);
    auto keyStorageProvider = ara::crypto::keys::LoadKeyStorageProvider();

    if(cryptoProvider.HasValue() && keyStorageProvider.HasValue())
    {
        mCryptoProvider  = cryptoProvider.Value(); 
        mKeyStorageProvider = keyStorageProvider.Value();
    }
    else{
        LOGE("cannot load Key Storage provider or crypto Provider");
    }
}

CsmResult<std::vector<uint8_t>> 
CsmAccessorAraCrypto::MacCreate(uint16_t keyId, std::vector<uint8_t> const& data, MacAlgorithm alg) const
{
    if(MacAlgorithm::kAes128Cmac != alg){
        LOGE("the requested mac algorithm is not supported, currently only AES128-CMAC is supported");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kError);
    }

     if(!mCryptoProvider || !mKeyStorageProvider){
        LOGE("cryptoProvider or keyStorageProvider are not initialized");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kError);
    }

    std::string strMacAlgorithm = EnumToStringMacAlgorithm(alg);
    std::string keyStr = "b2adc146-eefa-11e8-8eb2-f2801f1b9fd1"; //TO remove and use keyId
    ara::crypto::Uuid slot_uid;
    bool getKeyUuidRes= GetKeyUuid(keyStr, slot_uid);
    if(!getKeyUuidRes){
        LOGE("there is no slots, key isn't exist");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kKeyNotFound);
    }

    auto slot_number = mKeyStorageProvider->FindSlot(slot_uid);

    /* Get Algortihm Id and create MessageAuthenticationCodeCtx object */
    auto alg_id_mac = mCryptoProvider->ConvertToAlgId(strMacAlgorithm); 
    auto createMessageAuthnCodeCtx = mCryptoProvider->CreateMessageAuthnCodeCtx(alg_id_mac);
    
    if(!createMessageAuthnCodeCtx.HasValue())
    {
        LOGE("cannot create MAC");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kError);
    }
    auto mac_ctx = std::move(createMessageAuthnCodeCtx.Value());
    
    /* Initialize input and output data */
    std::vector<std::uint8_t> digest_data{};
    digest_data.resize(mac_ctx->GetDigestSize());

    ara::crypto::ReadOnlyMemRegion message{data};
    ara::crypto::WritableMemRegion digest{digest_data};

    /* Load key from crypto provider with trusted container from key storage provider */
    auto openAsUser = mKeyStorageProvider->OpenAsUser(slot_number);

    if(!openAsUser.HasValue()) // if there is no value in openAsUser, return error
    {
        LOGE("cannot load key from crypto provider");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kError);
    }
    auto key_trusted_container = std::move(openAsUser.Value());

    auto loadConcreteObject = mCryptoProvider->LoadConcreteObject<ara::crypto::cryp::SymmetricKey>(*key_trusted_container);

   if(!loadConcreteObject.HasValue()) // if there is no value in loadConcreteObject, return error
    {
        LOGE("cannot load key from crypto provider");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kError);
    }

    auto symmetric_key = std::move(loadConcreteObject.Value());

    /* Set the loaded key and compute tag */
    mac_ctx->SetKey(*symmetric_key);
    mac_ctx->Start();
    mac_ctx->Update(message.subspan(0,  message.size()));
    mac_ctx->Finish();
    mac_ctx->GetDigest(digest);

    return CsmResult<std::vector<uint8_t>>(digest_data);
}

CsmErrorCode 
CsmAccessorAraCrypto::MacVerify(uint16_t keyId, std::vector<uint8_t> const& data, std::vector<uint8_t> const& mac, MacAlgorithm alg) const
{
    CsmErrorCode retVal = CsmErrorCode::kError;
     
    CsmResult<std::vector<uint8_t>> macRes = MacCreate(keyId, data, alg);
   if(macRes.isSucceeded())
   {
        std::vector<uint8_t> macVec = macRes.getObject();
        if(std::equal(mac.begin(),mac.end(), macVec.begin()))
        {
           retVal = CsmErrorCode::kSuccess; 
        }
        else{
            LOGE("MacVerify failed, the mac are not equal")
        }
   }

    return retVal;
}

CsmErrorCode 
CsmAccessorAraCrypto::IsKeyExists(uint16_t keyId) const
{
    ara::crypto::Uuid slot_uid;
    bool getKeyUuidRes = GetKeyUuid("b2adc146-eefa-11e8-8eb2-f2801f1b9fd1", slot_uid);
    if(!getKeyUuidRes){ 
        LOGE("there is no slots, key isn't exist");
        return CsmErrorCode::kKeyNotFound;
    }
    return CsmErrorCode::kSuccess;
}

CsmResult<std::vector<uint8_t>> 
CsmAccessorAraCrypto::GenerateRandomBytes(uint8_t size) const
{
    if(!mCryptoProvider || !mKeyStorageProvider){
        LOGE("cryptoProvider or keyStorageProvider are not initialized");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kError);
    }
    
    auto alg_id_rng = mCryptoProvider->ConvertToAlgId(ara::core::StringView{"RNG,SYSTEM"});
    auto rng_result = mCryptoProvider->CreateRandomGeneratorCtx(alg_id_rng);

    if(!rng_result.HasValue()) 
    {
        // Could not create preseeded RNG (system might not have enough entropy for seedgeneration)
        LOGI("GenerateRandomBytes error: system might not have enough entropy for seedgeneration");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kErrorRng);
    }
    
    std::vector<uint8_t> random_vec(size);
    ara::crypto::WritableMemRegion random_data{random_vec};

    ara::core::Result<void> success = rng_result.Value()->Generate(random_data);
    
    if(!success.HasValue()) {
        LOGI("GenerateRandomBytes failed");
        return CsmResult<std::vector<uint8_t>>(CsmErrorCode::kError);
    }

    return CsmResult<std::vector<uint8_t>>(random_vec);
}

std::string
CsmAccessorAraCrypto::EnumToStringMacAlgorithm(MacAlgorithm const eMacAlgorithm) const
{
    std::string strMacAlgorithm = "";
    switch (eMacAlgorithm) {
    case MacAlgorithm::kAes128Cmac:
        strMacAlgorithm = CsmAccessorAraCrypto::kAes128CmacString;
        break;
    default:
        break;
    }
    return strMacAlgorithm;
}

bool 
CsmAccessorAraCrypto::GetKeyUuid(std::string const& keyId,ara::crypto::Uuid& slotOut) const
{
    bool retVal = false;
    auto it = mKeysMap.find(keyId);
    if(it == mKeysMap.end()){ // Key is not present in the map
        LOGI("inserting key to cache");
        /* Get slot number for slot UID and CryptoProvider responsible for the respective slot */
        ara::core::Result<ara::crypto::Uuid> slot_uid = KeyStorageProvider::SlotUid::From(keyId);
        if(slot_uid.HasValue())
        {
            retVal = true;
            slotOut = slot_uid.Value();
            mKeysMap.emplace(keyId,slot_uid.Value());
        }
    }
    else{
        LOGI("key was found in cache");
        slotOut = it->second;
        retVal = true;
    }
    return retVal;
}

} // namespace common
} // namespace sok