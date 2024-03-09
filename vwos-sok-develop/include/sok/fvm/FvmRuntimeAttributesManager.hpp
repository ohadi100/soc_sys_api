/* Copyright (c) 2023 Volkswagen Group */

#ifndef FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP
#define FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP

#include <unordered_map>
#include "IFvmRuntimeAttributesManager.hpp"
#include "IFreshnessValueManagerConfigAccessor.hpp"

namespace sok
{
namespace fvm
{

class FvmRuntimeAttributesManager : public IFvmRuntimeAttributesManager
{
public:
    FvmRuntimeAttributesManager();
    bool Init() override;
    void Reset() override;
    bool IsActive(SokFreshnessValueId fvId) const override;
    void SetActive(SokFreshnessValueId fvId, uint64_t activationTime) override;
    std::vector<uint8_t> IncSessionCounter(SokFreshnessValueId fvId) override;
    std::vector<uint8_t> GetSessionCounter(SokFreshnessValueId fvId) const override;
    std::vector<uint8_t> GetNextSessionCounter(SokFreshnessValueId fvId) const override;
    void UpdateEvent(EventType type, SokFreshnessValueId fvId, uint64_t value) override;
    uint64_t GetEvent(EventType type, SokFreshnessValueId fvId) const override;

private:
    void incrementByteArray(std::vector<uint8_t>& vec) const;

    std::shared_ptr<IFreshnessValueManagerConfigAccessor> mConfigAccessor;
    std::unordered_map<SokFreshnessValueId, SokFmRuntimeAttributes> mAttributesMap;
};

} // namespace fvm
} // namespace sok

#endif // FVM_RUNTIME_ATTRIBUTES_MANAGER_HPP