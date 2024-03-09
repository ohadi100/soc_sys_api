/* Copyright (c) 2023 Volkswagen Group */

#ifndef FRESHNESS_VALUE_MANAGER_IMPL_SERVER_HPP
#define FRESHNESS_VALUE_MANAGER_IMPL_SERVER_HPP

#include "AFreshnessValueManagerImpl.hpp"
#include <unordered_map>
#include <regex>
#include <mutex>

namespace sok
{
namespace fvm
{

class FreshnessValueManagerImplServer : public AFreshnessValueManagerImpl
{
public:
    FreshnessValueManagerImplServer();
    ~FreshnessValueManagerImplServer() = default;

    /**
     * @brief This function must be called before SOK-protected communication can begin.
     *        This function must be called every `SokFmMainFunctionPeriod` milliseconds after the first call.
     * 
     * 
     * @return FvmErrorCode 
     */
    FvmErrorCode MainFunction() noexcept override;

    bool serverOrParticipantInit() noexcept override;

#ifndef UNIT_TESTS
private:
#endif // UNIT_TESTS

    void incomingAuthFvChallengeSignalsCb(std::string const& signal, std::vector<uint8_t> const& challenge);
    FvmErrorCode unauthenticatedBroadcast();
    FvmErrorCode sendAuthenticFvResponses();

private:
    std::mutex mChallengesMutex;
    std::atomic_bool mNeedToSendAuthFvResponses;
    std::atomic_bool mNeedToBroadcastFv;
    std::unordered_map<std::string, std::vector<uint8_t>> mResponsePendingChallenges;
    std::unordered_map<std::string, uint16_t> mClientNameToKeyId;
    FmServerClientsConfigMap mClientConfigMap;
};

} // namespace fvm
} // namespace sok

#endif // FRESHNESS_VALUE_MANAGER_IMPL_SERVER_HPP