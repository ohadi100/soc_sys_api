#include "SokUserSample.hpp"
#include <limits>
#include "gen/sok_demo_msg.pb.h"
#include "sok/fvm/FreshnessValueManagerConstants.hpp"
#include "sok/fvm/FreshnessValueManager.hpp"
#include "sok/fvm/SokFmInternalFactory.hpp"
#include "sok/common/SokUtilities.hpp"
#include "sok/common/Logger.hpp"

using namespace sok;
using namespace sok::fvm;


SokUser::SokUser()
: mFvmMainLoopThread()
, mFvm(FreshnessValueManager::GetInstance())
, mSecoc()
, mAuthGenerator()
, mAuthVerifier()
, mRunning(false)
, mFvmRunning(false)
, mLastSentMessages()
, mIncomingPduDataCache()
, mSignalManager(SokFmInternalFactory::CreateSignalManager())
{

}

bool 
SokUser::Init() 
{
    if (mRunning) {
        return false;
    }

    mRunning = true;
    auto secocInitRes = amsr::secoc::InitializeComponent("secoc_config.json");
    if (!secocInitRes) {
        LOGE("Failed to create secoc object" << secocInitRes.Error().Message());
        return false;
    }
    mSecoc = std::move(secocInitRes.Value());
    mAuthGenerator = mSecoc->CreateGenAuthServiceProvider();
    mAuthVerifier = mSecoc->CreateVerifyServiceProvider();

    LOGD("SokUser::Init()");
    return true;
}

void 
SokUser::Stop()
{
    if (!mRunning) {
        return;
    }

    if (amsr::secoc::IsComponentInitialized()) {
        std::ignore = amsr::secoc::DeinitializeComponent();
    }

    mRunning = false;
}

void 
SokUser::resendLastSokMsg(pdu_id pduId)
{
    auto lastMsg = mLastSentMessages.find(pduId);
    if (mLastSentMessages.end() != lastMsg) {
        // mEndPoints[pduId].sendBytes(lastMsg->second.data(), static_cast<int>(lastMsg->second.size()));
    }
}

void 
SokUser::handleArrivingSignal(std::string const& signal, std::vector<uint8_t> const& value)
{
    SokDemoMessage msg;
    if (!msg.ParseFromArray(&value[0], static_cast<int>(value.size()))) {
        LOGE("Failed parsing sok demo msg, signal: " << signal);
        return;
    }
    LOGD("Received sok demo msg via signal: " << signal);
    pdu_id pduId = static_cast<pdu_id>(msg.pdu_id());
    auto secureMsg = std::vector<uint8_t>(msg.protection_msg().begin(), msg.protection_msg().end());

    auto verifyRes = mAuthVerifier->Verify(pduId, secureMsg);
    if (!verifyRes) {
        LOGE("Failed verifying incoming SOK msg with PDU ID: " << pduId << ", error: " << verifyRes.Error().Message());
    }
    else {
        LOGD("Verifying incoming SOK msg successfully, PDU ID: " << pduId << ", received data: " << std::string(verifyRes.Value().verified_msg_buffer.begin(), verifyRes.Value().verified_msg_buffer.end()));
        mIncomingPduDataCache.insert({pduId, verifyRes.Value().verified_msg_buffer});
    }
    // workaround for SecOC unsupported feature for verification status / retry 
    SecOC_VerificationStatusType status;
    status.fvId = pduId;
    status.verificationSucceeded = true;
    mFvm->VerificationStatusCallout(status);
}

bool 
SokUser::StartFvmLoop()
{
    if (mFvmRunning) {
        LOGE("FVM loop already running");
        return false;
    }

    mFvmRunning = true;
    mFvmMainLoopThread = std::make_shared<std::thread>([&](){
        while (mFvmRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SOK_FM_MAIN_FUNCTION_PERIOD_MS));
            if (FvmErrorCode::kSuccess != mFvm->MainFunction()) {
                LOGE("MainFunction error");
                break;
            }
        }
    });

    return true;
}

bool 
SokUser::StopFvmLoop()
{
    if (!mFvmRunning) {
        LOGE("FVM loop is not running");
        return false;
    }

    mFvmRunning = false;
    if (mFvmMainLoopThread->joinable()) {
        mFvmMainLoopThread->join();
    }
    mFvm->Deinit();
    return true;
}

bool 
SokUser::SubscribeToAuthPduService(pdu_id pduId, SignalConfig const& signalConf)
{
    if (!mRunning) {
        return false;
    }
    auto subscribeRes = mSignalManager->Subscribe(signalConf, [&](std::string const& signal, std::vector<uint8_t> const& value) {
        std::thread t([=](){
            this->handleArrivingSignal(signal, value);
        });
        t.detach();
    });
    if (FvmErrorCode::kSuccess != subscribeRes) {
        LOGE("Failed subscribing to signal with pdu ID: " << pduId);
        return false;
    }
    return true;
}

bool 
SokUser::SendAuthPdu(pdu_id pduId, std::vector<uint8_t> data, SignalConfig const& signalConf)
{        
    auto genRes = mAuthGenerator->Generate(pduId, data);
    if (!genRes) {
        LOGE("Failed generating secure PDU, PDU ID: " << pduId << ", error: " << genRes.Error().Message());
        return false;
    }
    // workaround for SecOC unsupported feature for Tx confirmation 
    mFvm->SPduTxConfirmation(pduId);
    LOGD("Generated secure PDU successfully");

    SokDemoMessage outMsg;
    outMsg.set_pdu_id(static_cast<int32_t>(pduId));
    outMsg.set_protection_msg(std::string(genRes.Value().secured_msg_buffer.begin(), genRes.Value().secured_msg_buffer.end()));

    std::vector<uint8_t> outData(outMsg.ByteSizeLong());
    outMsg.SerializeToArray(&outData[0], static_cast<int32_t>(outMsg.ByteSizeLong()));
    auto publishRes = mSignalManager->Publish(signalConf, outData);
    mLastSentMessages[pduId] = outData;
    return (FvmErrorCode::kSuccess == publishRes);
}

bool 
SokUser::OfferCrService(pdu_id pduId, std::vector<uint8_t> const& responseData, SignalConfig const& signalConf)
{
    if (!mRunning) {
        return false;
    }

    auto cb = [=](SokFreshnessValueId fvID) {
        static std::string responseString = std::string(responseData.begin(), responseData.end());
        LOGD("App response to challenge CB was triggered with FV ID: " << fvID << ", Responding with data: " << responseString);
        std::thread t([&](){SendAuthPdu(pduId, responseData, signalConf);});
        t.detach();
    };

    if (FvmErrorCode::kSuccess != mFvm->OfferCrRequest(pduId, cb)) {
        LOGE("Failed offering response for challenge with FV ID: " << pduId);
        return false;
    }
    
    return true;
}

bool 
SokUser::SubscribeToCrService(pdu_id pduId, SignalConfig const& signalConf)
{
    return SubscribeToAuthPduService(pduId, signalConf);
}

bool 
SokUser::TriggerCr(pdu_id pduId)
{
    mFvm->TriggerCrRequest(pduId);
    return true;
}

std::unordered_multimap<pdu_id, std::vector<uint8_t>> 
SokUser::GetIncomingPdusCache() const
{
    return mIncomingPduDataCache;
}

void 
SokUser::ClearIncomingPdusCache()
{
    mIncomingPduDataCache.clear();
}