/* Copyright (c) 2023 Volkswagen Group */

#ifndef FVM_CONFIG_PARSER_HPP
#define FVM_CONFIG_PARSER_HPP

#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include "rapidjson/schema.h"
#include "sok/fvm/FreshnessValueManagerDefinitions.hpp"

/**
 * By default, rapidjson uses C assert() for internal assertions.
 * We can override it by defining RAPIDJSON_ASSERT(x) macro.
 */
#undef RAPIDJSON_ASSERT
#define RAPIDJSON_ASSERT(x) \
    if (!(x))               \
    throw std::logic_error("RapidJSON assertion error")

namespace sok
{
namespace fvm
{

class FvmConfigParser
{
public:
    FvmConfigParser();

    bool Parse(std::string const& json, SokFmConfig& config) noexcept;

private:
    bool fetchGeneralConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const;
    bool fetchAuthenticBroadcastConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const;
    bool fetchChallengeResponseConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const;
    bool fetchFvDistributionSignalConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const;
    bool fetchKeyConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const;
    bool fetchClientsConfigurations(rapidjson::Document const& doc, SokFmConfig& config) const;
    SokFreshnessType convertStringFreshnessTypeToEnum(std::string const& freshnessType) const;
    bool fetchSignalConfig(rapidjson::GenericObject<true, rapidjson::Value> const& object, SignalConfig& signalConfig) const;
private:
    std::shared_ptr<rapidjson::SchemaDocument> mSchema;
};

} // namespace fvm
} // namespace sok

#endif // FVM_CONFIG_PARSER_HPP