/* Copyright (c) 2023 Volkswagen Group */

#ifndef SIGNAL_MANAGER_SCI_HPP
#define SIGNAL_MANAGER_SCI_HPP

#include <unordered_map>
#include <atomic>
#include <list>
#include "ISignalManager.hpp"
#include <sci/api/ISignalClient.hpp>
#include <sci/api/IEventHandlers.hpp>
#include "IFreshnessValueManagerConfigAccessor.hpp"

using vwg::e3p::com::sci::api::ISignalClient;
using vwg::e3p::com::sci::api::ISignalEventHandler;
using vwg::e3p::com::sci::ReceivedSignal;
using vwg::e3p::com::sci::TransmittedSignal;
using vwg::e3p::com::sci::ReceivedPdu;
using vwg::e3p::com::sci::ReceivedFrame;
using vwg::e3p::com::sci::TransmittedPdu;
using vwg::e3p::com::sci::TransmittedFrame;
using vwg::e3p::com::sci::EventType;
using vwg::e3p::com::sci::config::Signal;
using vwg::e3p::com::sci::config::Pdu;
using vwg::e3p::com::sci::config::Frame;
using vwg::e3p::com::sci::config::Socket;
using vwg::e3p::com::sci::Status;
using vwg::e3p::com::sci::Milliseconds;

namespace sok
{
namespace fvm
{
    
using IncomingSignalsMap = std::unordered_map<std::string, std::shared_ptr<ReceivedSignal>>;

class SokSignalEventHandler;

class SignalManagerSci : public ISignalManager
{
public:

    SignalManagerSci();
    ~SignalManagerSci();

    /**
     * @brief Subscribe for incoming signal
     * 
     * @param signal the signal name to listen to
     * @param cb a callback to be called when the signal event occurs
     * @return FvmErrorCode kSuccess upon success, error code on failure
     */
    FvmErrorCode Subscribe(SignalConfig const& signalConfig, SignalEventCallback const& cb) override;

    /**
     * @brief Publish a signal
     * 
     * @param signal the name of the signal to publish
     * @param value the value for the outgoing signal
     * @return FvmErrorCode kSuccess upon success, error code on failure
     */
    FvmErrorCode Publish(SignalConfig const& signalConfig, std::vector<uint8_t> const& value) override;

private:
    std::shared_ptr<ReceivedSignal> createIncomingSignal(SignalConfig const& signalConfig);
    std::shared_ptr<TransmittedSignal> createOutgoingSignal(SignalConfig const& signalConfig);
    Frame createSciFrameConfig(FrameConfig const& frameConfig, bool isIncoming) const;
    Pdu createSciPduConfig(PduConfig const& pduConfig) const;
    Signal createSciSignalConfig(SignalConfig const& signalConfig) const;

private:
    std::atomic_bool mInitialized;
    std::shared_ptr<ISignalClient> mSignalClient;
    std::shared_ptr<IFreshnessValueManagerConfigAccessor> mConfAccessor;
    std::unordered_map<std::string, std::shared_ptr<TransmittedSignal>> mOutSignals;
    IncomingSignalsMap mInSignals;
    std::list<std::shared_ptr<SokSignalEventHandler>> mSignalEventHandlers;
    std::unordered_map<std::string, std::shared_ptr<ReceivedFrame>> mIncomingFramesCache;
    std::unordered_map<std::string, std::shared_ptr<ReceivedPdu>> mIncomingPdusCache;
    std::unordered_map<std::string, std::shared_ptr<TransmittedFrame>> mOutgoingFramesCache;
    std::unordered_map<std::string, std::shared_ptr<TransmittedPdu>> mOutgoingPdusCache;
};

class SokSignalEventHandler : public ISignalEventHandler
{
public:
    SokSignalEventHandler(ISignalManager::SignalEventCallback const& cb) : mCb(cb) {}

    void onEvent(EventType event, ReceivedSignal const& signal) noexcept override;

private:
    ISignalManager::SignalEventCallback mCb;
};

} // namespace fvm
} // namespace sok

#endif // SIGNAL_MANAGER_SCI_HPP