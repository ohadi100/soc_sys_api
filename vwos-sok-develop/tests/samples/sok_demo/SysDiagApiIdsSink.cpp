/* Copyright (c) 2023 Volkswagen Group */
#include "e3/diag/AppIdentification.hpp"

namespace e3
{
namespace diag
{

std::string
getApplicationId()
{
    return "sok_sink";
}

std::string
getRegistrationRequiredId()
{
    return "sok_sink/sok_sinkRootSWC/RDiagnosticRegistration";
}

std::string
getDataManagementProvidedId()
{
    return "sok_sink/sok_sinkRootSWC/PDiagnosticDataManagement";
}

std::string
getDataManagementRequiredId()
{
    return "sok_sink/sok_sinkRootSWC/RDiagnosticDataManagement";
}

}  // namespace diag
}  // namespace e3