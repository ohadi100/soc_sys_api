/* Copyright (c) 2023 Volkswagen Group */

#ifndef I_SIGNAL_MANAGER_HPP
#define I_SIGNAL_MANAGER_HPP

#include <vector>
#include <string>
#include <functional>
#include "FreshnessValueManagerError.hpp"
#include "FreshnessValueManagerDefinitions.hpp"

namespace sok
{
namespace fvm
{

class ISignalManager
{
public:
    /**
     * @brief callback function that will be called when signal event arrive
     * @param[in] signal the name of the signal
     * @param[in] value the value of the signal
     */
    using SignalEventCallback = std::function<void(std::string const& signal, std::vector<uint8_t> const& value)>;

    virtual ~ISignalManager() = default;

    /**
     * @brief Subscribe for incoming signal
     * 
     * @param signal the signal name to listen to
     * @param cb a callback to be called when the signal event occurs
     * @return FvmErrorCode kSuccess upon success, error code on failure
     */
    virtual FvmErrorCode Subscribe(SignalConfig const& signalConfig, SignalEventCallback const& cb) = 0;

    /**
     * @brief Publish a signal
     * 
     * @param signal the name of the signal to publish
     * @param value the value for the outgoing signal
     * @return FvmErrorCode kSuccess upon success, error code on failure
     */
    virtual FvmErrorCode Publish(SignalConfig const& signalConfig, std::vector<uint8_t> const& value) = 0;
};

} // namespace fvm
} // namespace sok

#endif // I_SIGNAL_MANAGER_HPP