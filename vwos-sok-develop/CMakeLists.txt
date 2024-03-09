cmake_minimum_required(VERSION 3.15...3.23)

project(
    sok_fm
    LANGUAGES CXX
    VERSION ${CONAN_PKG_VERSION}
)

# C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOK_FM_LIB_NAME "sok_fm")
set(SOK_FM_SERVER_LIB_NAME "sok_fm_server")

set(UNIT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit)
set(SOK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SOK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(SOK_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(ENABLE_UNIT_TESTS OFF CACHE BOOL "Enable/disable Unit Tests target")
set(ENABLE_PARASOFT_SCA OFF CACHE BOOL "Enable/disable Parasoft SCA")
option(INTEGRATION_TESTS "Build for integration tests" OFF)

add_subdirectory(src)

if(ENABLE_UNIT_TESTS)
  add_subdirectory(tests)
endif()

include(vwos-clang-tidy)
vwos_clang_tidy_add_targets(CLANG_TIDY_HEADER_FILTER "${CMAKE_SOURCE_DIR}/(src|tests|include)/*" CLANG_TIDY_SOURCE_FILTER "${CMAKE_SOURCE_DIR}/(src|tests|include)/*")

file(GLOB_RECURSE SOURCES
  ${CMAKE_SOURCE_DIR}/src/**.cpp
  ${CMAKE_SOURCE_DIR}/src/**.h
  ${CMAKE_SOURCE_DIR}/src/**.hpp
  ${CMAKE_SOURCE_DIR}/tests/**.cpp
  ${CMAKE_SOURCE_DIR}/tests/**.h
  ${CMAKE_SOURCE_DIR}/tests/**.hpp
  ${CMAKE_SOURCE_DIR}/include/**.cpp
  ${CMAKE_SOURCE_DIR}/include/**.h
  ${CMAKE_SOURCE_DIR}/include/**.hpp  
)
include(vwos-clang-format)
vwos_clang_format_add_targets(SOURCE_FILES ${SOURCES})