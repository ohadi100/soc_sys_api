/* Copyright (c) 2023 Volkswagen Group */

#include "sok/fvm/SignalManagerSci.hpp"

#include <algorithm>
#include <sci/api/SciApiFactory.hpp>
#include <sci/api/ISciApi.hpp>
#include "sok/fvm/SokFmInternalFactory.hpp"
#include "sok/common/SokUtilities.hpp"
#include "sok/common/Logger.hpp"

namespace sok
{
namespace fvm
{

void SokSignalEventHandler::onEvent(EventType event, ReceivedSignal const& signal) noexcept
{
    try {
        if (EventType::kOnChange == event) {
            auto& signalName = signal.configuration().name;

            auto bufferRes = signal.getRawBuffer();
            if (bufferRes) {
                LOGD("Received signal: " << signalName << " with raw buffer, triggering CB");
                mCb(signalName, bufferRes.value());
                return;
            }
            auto valRes = signal.getRawValue();
            if (valRes) {
                LOGD("Received signal: " << signalName << " with raw value, triggering CB");
                auto byteVec = common::UintToByteVectorLE<uint64_t>(valRes.value());
                std::reverse(byteVec.begin(), byteVec.end());
                mCb(signalName, byteVec);
                return;
            }
            LOGE("SignalEventHandler received signal without a raw buffer/value: " << signalName);
            return;
        }
    } catch (std::exception const& ex) {
        LOGE("exception, what(): " << ex.what());
    } catch (...) {
        LOGE("exception");
    }
}

SignalManagerSci::SignalManagerSci()
: mInitialized(false)
, mSignalClient()
, mConfAccessor(SokFmInternalFactory::CreateFreshnessValueManagerConfigAccessor())
, mOutSignals()
, mInSignals()
, mSignalEventHandlers()
, mIncomingFramesCache()
, mIncomingPdusCache()
, mOutgoingFramesCache()
, mOutgoingPdusCache()
{
    auto sciApi = vwg::e3p::com::sci::api::SciApiFactory::initFactory("SOK-FM_Signals");
    if (nullptr == sciApi) {
        LOGE("Failed initializing sci factory");
        return;
    }
    auto clientResult = sciApi->createSignalClient("SOK-FM_Signals");   
    if (!clientResult)
    {
        LOGE("Failed to create Signal client");
        return;
    }
    mSignalClient = clientResult.value();
    mInitialized = true;
}

SignalManagerSci::~SignalManagerSci()
{

}

FvmErrorCode 
SignalManagerSci::Subscribe(SignalConfig const& signalConfig, SignalEventCallback const& cb)
{
    if (!mInitialized) {
        LOGE("SignalManagerSci wasn't initialized successfully");
        return FvmErrorCode::kNotInitialized;
    }

    auto sigFindRes = mInSignals.find(signalConfig.name);
    if (mInSignals.end() != sigFindRes) {
        LOGW("Already subscribed to signal with name: " << signalConfig.name);
        return FvmErrorCode::kAlreadyInitialized;
    }
    auto signal = createIncomingSignal(signalConfig);
    if (!signal) {
        return FvmErrorCode::kGeneralError;
    }

    auto eventHandler = std::make_shared<SokSignalEventHandler>(cb);
    if (Status::kOk != signal->registerEventHandler(EventType::kOnChange, eventHandler)) {
        LOGE("Failed to register signal event handler for signal: " << signalConfig.name);
        return FvmErrorCode::kGeneralError;
    }

    mSignalEventHandlers.push_back(eventHandler);

    return FvmErrorCode::kSuccess;
}

FvmErrorCode 
SignalManagerSci::Publish(SignalConfig const& signalConfig, std::vector<uint8_t> const& value)
{
    if (!mInitialized) {
        LOGE("SignalManagerSci wasn't initialized successfully");
        return FvmErrorCode::kNotInitialized;
    }

    std::shared_ptr<TransmittedSignal> signal;
    auto sigFindRes = mOutSignals.find(signalConfig.name);
    if (mOutSignals.end() != sigFindRes) {
        signal = sigFindRes->second;
    }
    else {
        signal = createOutgoingSignal(signalConfig);
        if (!signal) {
            return FvmErrorCode::kGeneralError;
        }
        mOutSignals[signalConfig.name] = signal;
    }

    if (value.size() > 8) {
        if (Status::kOk != signal->setValue(value)) {
            LOGE("Failed setting signal: " << signalConfig.name);
            return FvmErrorCode::kGeneralError;
        }
    }
    else {
        if (Status::kOk != signal->setValue(common::ByteVectorToUint<uint64_t>(value))) {
            LOGE("Failed setting signal: " << signalConfig.name);
            return FvmErrorCode::kGeneralError;
        }
    }

    LOGD("Sent successfully signal: " << signalConfig.name);
    return FvmErrorCode::kSuccess;
}

std::shared_ptr<ReceivedSignal> 
SignalManagerSci::createIncomingSignal(SignalConfig const& sokSignalConfig)
{
    Signal sciSignalConfig = createSciSignalConfig(sokSignalConfig);

    auto createSignal = [this, sciSignalConfig](std::shared_ptr<ReceivedPdu> const& pdu) -> std::shared_ptr<ReceivedSignal> {
        auto signalResult = mSignalClient->createReceivedSignal(pdu, sciSignalConfig);
        if (!signalResult) {
            LOGE("Failed to create received Signal: " << sciSignalConfig.name);
            return nullptr;
        }
        return signalResult.value();
    };

    auto createPdu = [this, sokSignalConfig](std::shared_ptr<ReceivedFrame> const& frame) -> std::shared_ptr<ReceivedPdu> {
        Pdu pduConfig = createSciPduConfig(sokSignalConfig.pduConfig);
        auto pduResult = mSignalClient->createReceivedPdu(frame, pduConfig);
        if (!pduResult) {
            LOGE("Failed to create received Pdu: " << pduConfig.name);
            return nullptr;
        }
        if(pduResult.value()->activate() != Status::kOk) {
            LOGE("Failed to activate pdu: " << pduConfig.name);
            return nullptr;
        }
        mIncomingPdusCache[sokSignalConfig.pduConfig.name] = pduResult.value();
        return pduResult.value();
    };

    auto frameCacheFindRes = mIncomingFramesCache.find(sokSignalConfig.pduConfig.frameConfig.name);
    // check if such frame was already created
    if (mIncomingFramesCache.end() != frameCacheFindRes) {
        // check if such pdu was already created
        auto pduCacheFindRes = mIncomingPdusCache.find(sokSignalConfig.pduConfig.name);
        if (mIncomingPdusCache.end() != pduCacheFindRes) {
            return createSignal(pduCacheFindRes->second);
        }
        else {
            auto pdu = createPdu(frameCacheFindRes->second);
            if (!pdu) {
                return nullptr;
            }
            return createSignal(pdu);
        }
    }
    else {
        Frame frameConfig = createSciFrameConfig(sokSignalConfig.pduConfig.frameConfig, true);

        auto frameResult = mSignalClient->createReceivedFrame(frameConfig);
        if (!frameResult) {
            LOGE("Failed to create received frame: " << frameConfig.name);
            return nullptr;
        }
        mIncomingFramesCache[sokSignalConfig.pduConfig.frameConfig.name] = frameResult.value();
        auto pdu = createPdu(frameResult.value());
        if (!pdu) {
            return nullptr;
        }
        return createSignal(pdu);
    }
}

std::shared_ptr<TransmittedSignal> 
SignalManagerSci::createOutgoingSignal(SignalConfig const& sokSignalConfig)
{
    Signal sciSignalConfig = createSciSignalConfig(sokSignalConfig);

    auto createSignal = [this, sciSignalConfig](std::shared_ptr<TransmittedPdu> const& pdu) -> std::shared_ptr<TransmittedSignal> {
        auto signalResult = mSignalClient->createTransmittedSignal(pdu, sciSignalConfig);
        if (!signalResult) {
            LOGE("Failed to create Transmitted Signal: " << sciSignalConfig.name);
            return nullptr;
        }
        return signalResult.value();
    };

    auto createPdu = [this, sokSignalConfig](std::shared_ptr<TransmittedFrame> const& frame) -> std::shared_ptr<TransmittedPdu> {
        Pdu pduConfig = createSciPduConfig(sokSignalConfig.pduConfig);

        auto pduResult = mSignalClient->createTransmittedPdu(frame, pduConfig);
        if (!pduResult) {
            LOGE("Failed to create Transmitted Pdu: " << pduConfig.name);
            return nullptr;
        }
        if(pduResult.value()->activate() != Status::kOk) {
                LOGE("Failed to activate pdu: " << pduConfig.name);
                return nullptr;
        }
        mOutgoingPdusCache[sokSignalConfig.pduConfig.name] = pduResult.value();
        return pduResult.value();
    };

    auto frameCacheFindRes = mOutgoingFramesCache.find(sokSignalConfig.pduConfig.frameConfig.name);
    // check if such frame was already created
    if (mOutgoingFramesCache.end() != frameCacheFindRes) {
        // check if such pdu was already created
        auto pduCacheFindRes = mOutgoingPdusCache.find(sokSignalConfig.pduConfig.name);
        if (mOutgoingPdusCache.end() != pduCacheFindRes) {
            return createSignal(pduCacheFindRes->second);
        }
        else {
            auto pdu = createPdu(frameCacheFindRes->second);
            if (!pdu) {
                return nullptr;
            }
            return createSignal(pdu);
        }
    }
    else {
        Frame frameConfig = createSciFrameConfig(sokSignalConfig.pduConfig.frameConfig, false);

        auto frameResult = mSignalClient->createTransmittedFrame(frameConfig);
        if (!frameResult) {
            LOGE("Failed to create Transmitted frame: " << frameConfig.name);
            return nullptr;
        }
        mOutgoingFramesCache[sokSignalConfig.pduConfig.frameConfig.name] = frameResult.value();
        auto pdu = createPdu(frameResult.value());
        if (!pdu) {
            return nullptr;
        }
        return createSignal(pdu);
    }
}

Frame
SignalManagerSci::createSciFrameConfig(FrameConfig const& frameConfig, bool isIncoming) const
{
    Socket sourceSocket {
        frameConfig.sourceIp,
        frameConfig.sourcePort,
        mConfAccessor->GetNetworkInterface()
    };

    Socket destinationSocket {
        frameConfig.destinationIp,
        frameConfig.destinationPort,
        mConfAccessor->GetNetworkInterface()
    };

    Frame sciFrameConfig {
        frameConfig.name,
        sourceSocket,
        destinationSocket,
        frameConfig.maxPayloadSizeBytes,
        Milliseconds(0),
        isIncoming ? Frame::Direction::kReceived : Frame::Direction::kTransmitted
    };

    return sciFrameConfig;
}

Pdu
SignalManagerSci::createSciPduConfig(PduConfig const& pduConfig) const
{
    Pdu sciPduConfig{
        pduConfig.name,
        pduConfig.id,
        pduConfig.lengthBytes,
        Pdu::SendType::kNoMsgSendType,
        true,
        Milliseconds(0),
        Milliseconds(0),
        Milliseconds(0),
        Milliseconds(0xFFFFFFFF),
        0xFFFF
    };
    return sciPduConfig;
}

Signal
SignalManagerSci::createSciSignalConfig(SignalConfig const& signalConfig) const
{
    Signal sciSignalConfig {
        signalConfig.name, 
        signalConfig.lengthInBits,
        signalConfig.startByte,
        0U,
        signalConfig.lengthInBits > 64 ? Signal::Endianness::kOpaque : Signal::Endianness::kLittleEndian,
        Signal::SendType::kOnChange
    };
    return sciSignalConfig;
}


} // namespace fvm
} // namespace sok
