/* Copyright (c) 2023 Volkswagen Group */

#ifndef MOCK_SIGNAL_MANAGER_HPP
#define MOCK_SIGNAL_MANAGER_HPP

#include <gmock/gmock.h>
#include "sok/fvm/ISignalManager.hpp"

namespace sok
{
namespace fvm    
{

class MockSignalManager : public ISignalManager
{
public:
    MOCK_METHOD(FvmErrorCode, Subscribe, (SignalConfig const&, SignalEventCallback const&), (override));
    MOCK_METHOD(FvmErrorCode, Publish, (SignalConfig const&, std::vector<uint8_t> const&), (override));
};

class UTSignalManager : public ISignalManager
{
public:
    FvmErrorCode 
    Subscribe(SignalConfig const& signalConfig, SignalEventCallback const& cb) override
    {
        return mMockSm->Subscribe(signalConfig, cb);
    }

    FvmErrorCode 
    Publish(SignalConfig const& signalConfig, std::vector<uint8_t> const& value) override
    {
        return mMockSm->Publish(signalConfig, value);
    }

    static MockSignalManager* mMockSm;
};

}  // namespace fvm
}  // namespace sok

#endif  // MOCK_SIGNAL_MANAGER_HPP