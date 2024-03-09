cmake_minimum_required(VERSION 3.15...3.23)

find_package(GTest REQUIRED)
find_package(RapidJSON REQUIRED)

add_definitions("-DRAPIDJSON_IMPL -DRAPIDJSON_HAS_STDSTRING")

set(GTEST_NAME ${PROJECT_NAME}_gtest)

include_directories(
        ${SOK_INCLUDE_DIR}
        ${UNIT_TEST_DIR}/mock
)

set(SOURCES
        ${SOK_SOURCE_DIR}/sok/common/CsmAccessorDemo.cpp
        ${SOK_SOURCE_DIR}/sok/common/SokCommonInternalFactory.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/SokFmInternalFactory.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/FreshnessValueManager.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/AFreshnessValueManagerImpl.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/FreshnessValueManagerImplServer.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/FreshnessValueManagerImplParticipant.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/FreshnessValueStateManager.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/FvmRuntimeAttributesManager.cpp
        ${SOK_SOURCE_DIR}/sok/fvm/FvmConfigParser.cpp
    )

set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/fvm/AFreshnessValueManagerImplTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManagerImplServerTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FvmRuntimeAttributesManagerTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/CsmAccessorDemoTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueManagerImplParticipantTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FvmConfigParserTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fvm/FreshnessValueStateManagerTest.cpp
        )

add_executable(${GTEST_NAME}
        ${SOURCES}
        ${TEST_SOURCES}
        )

target_compile_definitions(${GTEST_NAME} PRIVATE UNIT_TESTS)

target_link_libraries(${GTEST_NAME}
        PUBLIC
        gtest::gtest
        rapidjson::rapidjson
        )

add_custom_target(tests DEPENDS ${GTEST_NAME})
add_test(NAME ${GTEST_NAME} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gtest2pdf)

install(TARGETS ${GTEST_NAME} DESTINATION bin)