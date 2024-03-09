/* Copyright (c) 2023 Volkswagen Group */
#include "e3/diag/AppIdentification.hpp"

namespace e3
{
namespace diag
{

std::string
getApplicationId()
{
    return "sok_source";
}

std::string
getRegistrationRequiredId()
{
    return "sok_source/sok_sourceRootSWC/RDiagnosticRegistration";
}

std::string
getDataManagementProvidedId()
{
    return "sok_source/sok_sourceRootSWC/PDiagnosticDataManagement";
}

std::string
getDataManagementRequiredId()
{
    return "sok_source/sok_sourceRootSWC/RDiagnosticDataManagement";
}

}  // namespace diag
}  // namespace e3