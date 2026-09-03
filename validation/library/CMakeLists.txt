cmake_minimum_required(VERSION 3.20)

project(
  service_config_validation
  LANGUAGES CXX
)

# ---------------------------------------------------------------------
# Vix Standard
# ---------------------------------------------------------------------

if (NOT TARGET vix::standard)
  add_subdirectory(
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    "${CMAKE_BINARY_DIR}/vix-standard"
  )
endif()

# ---------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------

add_library(service_config
  src/ServiceConfig.cpp
)

add_library(
  service_config::service_config
  ALIAS
  service_config
)

target_compile_features(service_config
  PUBLIC
    cxx_std_20
)

target_include_directories(service_config
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(service_config
  PUBLIC
    vix::standard
)

# ---------------------------------------------------------------------
# Example
# ---------------------------------------------------------------------

add_executable(service_config_example
  example.cpp
)

target_link_libraries(service_config_example
  PRIVATE
    service_config::service_config
)

# ---------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------

option(
  SERVICE_CONFIG_BUILD_TESTS
  "Build service_config validation tests"
  ON
)

if (SERVICE_CONFIG_BUILD_TESTS)
  include(CTest)
  enable_testing()

  find_package(GTest REQUIRED)

  add_executable(service_config_tests
    tests/ServiceConfigTest.cpp
  )

  target_link_libraries(service_config_tests
    PRIVATE
      service_config::service_config
      GTest::gtest_main
  )

  include(GoogleTest)

  gtest_discover_tests(service_config_tests)
endif()
