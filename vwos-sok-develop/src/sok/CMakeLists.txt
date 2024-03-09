cmake_minimum_required(VERSION 3.15...3.23)

find_package(Threads REQUIRED)
find_package(vwos-sci-libbackend REQUIRED)
# find_package(vwos-crypto-libcrypto REQUIRED) #due to conan packages conflicts
find_package(vwos-sci-libutils REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(vwos-mid-integration-interfaces REQUIRED)
find_package(system-diag-lib REQUIRED)
find_package(system_diag_common REQUIRED)

add_definitions("-DRAPIDJSON_IMPL -DRAPIDJSON_HAS_STDSTRING")

set(AMSR_COMPONENTS
    amsr::log_ara_logging
)

find_package(AMSR CONFIG REQUIRED
    COMPONENTS
      ${AMSR_COMPONENTS}
)

include_directories(
    ${SOK_INCLUDE_DIR}
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/fvm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/CsmAccessorAraCrypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/SokCommonInternalFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/AFreshnessValueManagerImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManagerImplServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManagerImplParticipant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueStateManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/SignalManagerSci.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FvmRuntimeAttributesManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/SokFmInternalFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FvmConfigParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/DiagnosticsHandler.cpp
)

set(LINK_LIBS
    ${AMSR_COMPONENTS}
    libsci::libsci
    # vwos-crypto-libcrypto::vwos-crypto-libcrypto #due to conan packages conflicts
    libsciutils::libscilogger
    rapidjson::rapidjson
    vwos-mid-integration-interfaces::vwos-mid-integration-interfaces
    vwos-sci-libutils::vwos-sci-libutils
    system-diag-lib::system-diag-lib
)

add_library(fvm_OBJECT OBJECT
    ${SOURCES}
)

target_link_libraries(fvm_OBJECT
    PRIVATE
        ${LINK_LIBS}
)

add_library(
    ${SOK_FM_LIB_NAME}
    SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManagerConfigAccessor.cpp
    )

set_property(TARGET ${SOK_FM_LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${SOK_FM_LIB_NAME}
    PUBLIC
        fvm_OBJECT
    PRIVATE
        ${LINK_LIBS}
)

## fvm server lib ##

add_library(
    ${SOK_FM_SERVER_LIB_NAME}
    SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManagerConfigAccessor.cpp
    )
    
target_compile_definitions(${SOK_FM_SERVER_LIB_NAME} PRIVATE SOK_FVM_SERVER)

set_property(TARGET ${SOK_FM_SERVER_LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)


target_link_libraries(${SOK_FM_SERVER_LIB_NAME}
    PUBLIC
        fvm_OBJECT
    PRIVATE
        ${LINK_LIBS}
)


install(TARGETS ${SOK_FM_LIB_NAME} ${SOK_FM_SERVER_LIB_NAME}
    LIBRARY DESTINATION lib
    )

file(GLOB config_FILES ${SOK_CONFIG_DIR}/*.json)
install(
    FILES
        ${config_FILES}
    DESTINATION
        config
)

# parasoft tools adds executable target thus it also adds entries to compile_commands.json
# To avoid unnecessary clang-tidy or other tool checks on these entries parasoft option should be
# enabled only when running parasoft targets
if(ENABLE_PARASOFT_SCA)
    include(vwos-parasoft-cpptest)

    # Define Parasoft target
    vwos_parasoft_add_target(sca
      TARGETS
        ${SOK_FM_LIB_NAME}
    )

    # Define what is to be done by Parasoft
    vwos_parasoft_add_job(sca
      JOB RUN_SCA
      EXCLUDE
        **/src-gen/**
        **/tests/**
    )
endif()