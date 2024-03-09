/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/FreshnessValueManagerConfigAccessor.hpp"
#include <numeric>
#include <fstream>
#include <sstream>
#include "integration/storagepathprovider.hpp"
#include "sok/common/Logger.hpp"

#ifdef SOK_FVM_SERVER
constexpr char FVM_CONFIG_FILE[] = "/fvm/config_server_source.json";
#else // PARTICIPANT
constexpr char FVM_CONFIG_FILE[] = "/fvm/config_participant_sink.json";
#endif

namespace sok
{
namespace fvm
{

FreshnessValueManagerConfigAccessor::FreshnessValueManagerConfigAccessor()
: mParser()
, mConfig()
, mInitialized(false)
{
}

std::shared_ptr<FreshnessValueManagerConfigAccessor> 
FreshnessValueManagerConfigAccessor::GetInstance()
{
    static FreshnessValueManagerConfigAccessor                         confAccessor;
    static std::shared_ptr<FreshnessValueManagerConfigAccessor> const confAccessorSP(&confAccessor, [](FreshnessValueManagerConfigAccessor*) {
    });
    return confAccessorSP;
}

bool
FreshnessValueManagerConfigAccessor::Init()
{
    if (mInitialized) {
        LOGW("Already initialized");
        return false;
    }
    vwg::e3p::integration::StoragePathProvider pathProvider;
    auto configPath = pathProvider.getStaticDataBasePath();

    std::ifstream configFile(configPath + FVM_CONFIG_FILE, std::ios::in);
    if (!configFile.is_open()) {
        LOGE("Failed to open FVM configuration file: " << FVM_CONFIG_FILE);
        return false;
    }
    std::string jsonConfig;
    std::stringstream strStream;
    strStream << configFile.rdbuf();
    jsonConfig = strStream.str();
    configFile.close();

    if(!mParser.Parse(jsonConfig, mConfig)) {
        return false;
    }
    LOGD("FreshnessValueManagerConfigAccessor::Init() DONE");
    mInitialized = true;
    return true;
}

FvmResult<SokFvConfigInstance> 
FreshnessValueManagerConfigAccessor::GetSokFvConfigInstanceByFvId(SokFreshnessValueId id) const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return FvmResult<SokFvConfigInstance>(FvmErrorCode::kNotInitialized);
    }
    auto findRes = mConfig.mAuthBroadcastConfig.find(id);
    if (mConfig.mAuthBroadcastConfig.end() == findRes) {
        return FvmResult<SokFvConfigInstance>(FvmErrorCode::kFVNotAvailable);
    }
    return FvmResult<SokFvConfigInstance>(findRes->second);
}

FvmResult<ChallengeConfigInstance> 
FreshnessValueManagerConfigAccessor::GetChallengeConfigInstanceByFvId(SokFreshnessValueId id) const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return FvmResult<ChallengeConfigInstance>(FvmErrorCode::kNotInitialized);
    }
    auto findRes = mConfig.mChallengesConfig.find(id);
    if (mConfig.mChallengesConfig.end() == findRes) {
        return FvmResult<ChallengeConfigInstance>(FvmErrorCode::kFVNotAvailable);
    }
    return FvmResult<ChallengeConfigInstance>(findRes->second);
}

FvmResult<SokFreshnessType>
FreshnessValueManagerConfigAccessor::GetEntryTypeByFvId(SokFreshnessValueId id) const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return FvmResult<SokFreshnessType>(FvmErrorCode::kNotInitialized);
    }
    auto fvFindRes = mConfig.mAuthBroadcastConfig.find(id);
    if (mConfig.mAuthBroadcastConfig.end() != fvFindRes) {
        return FvmResult<SokFreshnessType>(fvFindRes->second.type);
    }
    auto challengeFindRes = mConfig.mChallengesConfig.find(id);
    if (mConfig.mChallengesConfig.end() != challengeFindRes) {
        return FvmResult<SokFreshnessType>(challengeFindRes->second.type);
    }
    else {
        return FvmResult<SokFreshnessType>(FvmErrorCode::kFVNotAvailable);
    }
}   

std::vector<SokFreshnessValueId>
FreshnessValueManagerConfigAccessor::GetAllFreshnessValueIds() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    std::vector<SokFreshnessValueId> ret(mConfig.mAuthBroadcastConfig.size() + mConfig.mChallengesConfig.size());
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

std::vector<SokFreshnessValueId> 
FreshnessValueManagerConfigAccessor::GetAllAuthBroadcastFreshnessValueIds() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    std::vector<SokFreshnessValueId> ret(mConfig.mAuthBroadcastConfig.size());
    std::iota(ret.begin(), ret.end(), mConfig.mAuthBroadcastConfig.begin()->first);
    return ret;
}

std::vector<SokFreshnessValueId> 
FreshnessValueManagerConfigAccessor::GetAllChallengeFreshnessValueIds() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    std::vector<SokFreshnessValueId> ret(mConfig.mChallengesConfig.size());
    std::iota(ret.begin(), ret.end(), mConfig.mChallengesConfig.begin()->first);
    return ret;
}

FmServerClientsConfigMap
FreshnessValueManagerConfigAccessor::GetClientsConfigMap() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mFmServerClientsConfig;
}

std::string 
FreshnessValueManagerConfigAccessor::GetEcuName() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mEcuName;
}

std::string 
FreshnessValueManagerConfigAccessor::GetNetworkInterface() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mNetworkInterface;
}

SignalConfig 
FreshnessValueManagerConfigAccessor::GetUnauthenticatedFvSignalConfig() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mUnAuthFvDistributionSignal;
}

SignalConfig 
FreshnessValueManagerConfigAccessor::GetAuthenticatedFvValueSignalConfig() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mAuthFvValueSignal;
}

SignalConfig 
FreshnessValueManagerConfigAccessor::GetAuthenticatedFvSignatureSignalConfig() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mAuthFvSignatureSignal;
}

SignalConfig 
FreshnessValueManagerConfigAccessor::GetAuthenticatedFvChallengeSignalConfig() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mAuthFvChallengeSignal;
}

SokKeyConfig 
FreshnessValueManagerConfigAccessor::GetSokKeyConfig() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mKeyConfig;
}

uint16_t 
FreshnessValueManagerConfigAccessor::GetEcuKeyIdForFvDistribution() const
{
    if (!mInitialized) {
        LOGE("Config accessor was not initialized");
        return {};
    }
    return mConfig.mKeyIdForAuthFvDistribution;
}

} // namespace fvm
} // namespace sok