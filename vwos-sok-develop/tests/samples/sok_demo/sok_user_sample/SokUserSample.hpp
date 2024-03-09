#include <iostream>
#include <thread>
#include <atomic>
#include "amsr/secoc/lifecycle.h"
#include "sok/fvm/FreshnessValueManager.hpp"
#include "sok/fvm/ISignalManager.hpp"

using namespace sok::fvm;
using namespace sok::common;

class SokUser {
public:

    SokUser();
    virtual ~SokUser() = default;
    
    bool Init();

    void Stop();

    void resendLastSokMsg(pdu_id pduId);

    std::unordered_multimap<pdu_id, std::vector<uint8_t>> GetIncomingPdusCache() const;
    void ClearIncomingPdusCache();

    void handleArrivingSignal(std::string const& signal, std::vector<uint8_t> const& value);

    bool StartFvmLoop();
    bool StopFvmLoop();
    bool SubscribeToAuthPduService(pdu_id pduId, SignalConfig const& signalConf);
    bool SendAuthPdu(pdu_id pduId, std::vector<uint8_t> data, SignalConfig const& signalConf);
    bool OfferCrService(pdu_id pduId, std::vector<uint8_t> const& responseData, SignalConfig const& signalConf);
    bool SubscribeToCrService(pdu_id pduId, SignalConfig const& signalConf);
    bool TriggerCr(pdu_id pduId);

private:

    std::shared_ptr<std::thread> mFvmMainLoopThread;
    std::shared_ptr<FreshnessValueManager> mFvm;
    std::unique_ptr<amsr::secoc::SecOCLibInterface> mSecoc;
    std::unique_ptr<amsr::secoc::GenAuthServiceProvider> mAuthGenerator;
    std::unique_ptr<amsr::secoc::VerifyServiceProvider> mAuthVerifier;
    std::atomic_bool mRunning;
    std::atomic_bool mFvmRunning;
    std::unordered_map<pdu_id, std::vector<uint8_t>> mLastSentMessages;
    std::unordered_multimap<pdu_id, std::vector<uint8_t>> mIncomingPduDataCache;
    std::shared_ptr<ISignalManager> mSignalManager;
};