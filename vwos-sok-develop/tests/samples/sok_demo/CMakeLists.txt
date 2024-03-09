cmake_minimum_required(VERSION 3.15...3.23)

set(PROJECT_NAME ${CONAN_PKG_NAME})

project(${PROJECT_NAME}
  LANGUAGES CXX
  VERSION ${CONAN_PKG_VERSION}
)

if (${SOK_FVM_SERVER})
    set(LOG_CTX_ID "SRC")
    set(LOG_DESCRIPTION "SOK source demo app")
    set(EXECUTABLE_NAME sok_source)
    set(FVM_LIB sok_fm::fm_server)
    set(PROCESS_ARG source)
    set(DIAG_APP_CONFIG_CPP ${CMAKE_CURRENT_SOURCE_DIR}/SysDiagApiIdsSource.cpp)
else ()
    set(LOG_CTX_ID "SINK")
    set(LOG_DESCRIPTION "SOK sink demo app")
    set(EXECUTABLE_NAME sok_sink)
    set(FVM_LIB sok_fm::sok_fm)
    set(PROCESS_ARG sink)
    set(DIAG_APP_CONFIG_CPP ${CMAKE_CURRENT_SOURCE_DIR}/SysDiagApiIdsSink.cpp)
endif()

include(vector-generator)

set(AMSR_COMPONENTS
    amsr::applicationbase
    amsr::socal
    amsr::com_r20_11
    amsr::em_application_client
    amsr::log_ara_logging
    amsr::someipprotocol
    amsr::ipcbinding
    amsr::secoc
)

find_package(AMSR CONFIG REQUIRED
    COMPONENTS
      ${AMSR_COMPONENTS}
)

find_package(vwos-sok-fm REQUIRED)
find_package(Threads REQUIRED)
find_package(protobuf REQUIRED)
find_package(system-diag-lib REQUIRED)
find_package(system_diag_common REQUIRED)

add_library(sokSampleProtoGeneratedFiles OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/sok_user_sample/gen/sok_demo_msg.pb.cc
)

# Ignore warnings for generated code.
target_compile_options(sokSampleProtoGeneratedFiles PRIVATE
    -Wno-useless-cast
    -Wno-suggest-override
    -Wno-sign-conversion
    -Wno-conversion
    -Wno-switch-default
    -Wno-missing-declarations
)

target_link_libraries(sokSampleProtoGeneratedFiles
    PUBLIC
    protobuf::libprotobuf
)

add_compile_definitions(AMSR_SECOC_FVM_LIB=${FVM_LIB})

add_executable(${EXECUTABLE_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/sok_user_sample/SokUserSample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SokDemo.cpp
    ${DIAG_APP_CONFIG_CPP}
)

target_include_directories(${EXECUTABLE_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/sok_user_sample/gen
    ${CMAKE_CURRENT_SOURCE_DIR}/sok_user_sample
)

target_link_libraries(${EXECUTABLE_NAME} 
    PUBLIC  
        ${AMSR_COMPONENTS}
        sokSampleProtoGeneratedFiles
        libsci::libsci
        vwos-mid-integration-interfaces::vwos-mid-integration-interfaces
        # vwos-crypto-libcrypto::vwos-crypto-libcrypto #due to conan packages conflicts
        vwos-sci-libutils::vwos-sci-libutils
        system-diag-lib::system-diag-lib
        ${FVM_LIB}
)

configure_file(${PROJECT_SOURCE_DIR}/model/design/Application.arxml
    ${CMAKE_CURRENT_BINARY_DIR}/model/design/Application.arxml @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/model/deployment/Default/MachineLogChannelMixin.arxml
    ${CMAKE_CURRENT_BINARY_DIR}/model/deployment/Default/MachineLogChannelMixin.arxml @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/model/deployment/Default/MachineSoftwareClusterMixin.arxml
    ${CMAKE_CURRENT_BINARY_DIR}/model/deployment/Default/MachineSoftwareClusterMixin.arxml @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/model/deployment/Default/Process.arxml
    ${CMAKE_CURRENT_BINARY_DIR}/model/deployment/Default/Process.arxml @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/model/deployment/Default/ProcessLoggingConfiguration.arxml
    ${CMAKE_CURRENT_BINARY_DIR}/model/deployment/Default/ProcessLoggingConfiguration.arxml @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/model/deployment/Default/ProcessToMachineMapping.arxml
    ${CMAKE_CURRENT_BINARY_DIR}/model/deployment/Default/ProcessToMachineMapping.arxml @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/model/deployment/Default/DiagnosticServiceDeployment.arxml
    ${CMAKE_CURRENT_BINARY_DIR}/model/deployment/Default/DiagnosticServiceDeployment.arxml @ONLY)

set(SRCGEN ${CMAKE_CURRENT_BINARY_DIR}/src-gen)
set(GENROOT ${SRCGEN}/${EXECUTABLE_NAME})
set(DVACFG_CMAKE_CONFIGURE ON)
dvacfg_file_gen(${EXECUTABLE_NAME}
  INPUT
    "${vwos-mid-integration-model_INCLUDE_DIR}"
    "${system_diag_common_INCLUDE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/model"
    
  OUTPUT
    "${GENROOT}/amsr_applicationbase.cmake"
    "${GENROOT}/amsr_logapi_config.cmake"
    "${GENROOT}/amsr_modelleddatatypes_api.cmake"
    "${GENROOT}/amsr_socal.cmake"
    "${GENROOT}/amsr_someipprotocol.cmake"
    "${GENROOT}/amsr_ipcbinding.cmake"
  GENERATOR
    DvACfg::amsr_applicationbase
    DvACfg::amsr_ipcbinding
    DvACfg::amsr_logapi_config
    DvACfg::amsr_modelleddatatypes_api
    DvACfg::amsr_socal
    DvACfg::amsr_someipprotocol
  DIRECTORY "${SRCGEN}"
)

install(TARGETS ${EXECUTABLE_NAME}
    RUNTIME DESTINATION sok_demo/${EXECUTABLE_NAME}/bin
)

install(
    DIRECTORY
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/model/design>
    DESTINATION model
)

install(
    DIRECTORY
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/model/deployment>
    DESTINATION model
)