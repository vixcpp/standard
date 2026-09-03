cmake_minimum_required(VERSION 3.20)

project(
  service_cli_validation
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
# service_config
# ---------------------------------------------------------------------

set(
  SERVICE_CONFIG_BUILD_TESTS
  OFF
  CACHE BOOL
  "Build service_config validation tests"
  FORCE
)

if (NOT TARGET service_config::service_config)
  add_subdirectory(
    "${CMAKE_CURRENT_LIST_DIR}/../library"
    "${CMAKE_BINARY_DIR}/service-config"
  )
endif()

# ---------------------------------------------------------------------
# CLI library
# ---------------------------------------------------------------------

add_library(service_cli
  src/Command.cpp
)

add_library(
  service_cli::service_cli
  ALIAS
  service_cli
)

target_compile_features(service_cli
  PUBLIC
    cxx_std_20
)

target_include_directories(service_cli
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(service_cli
  PUBLIC
    vix::standard
    service_config::service_config
)

# ---------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------

add_executable(service-check
  main.cpp
)

target_link_libraries(service-check
  PRIVATE
    service_cli::service_cli
)

# ---------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------

option(
  SERVICE_CLI_BUILD_TESTS
  "Build service CLI validation tests"
  ON
)

if (SERVICE_CLI_BUILD_TESTS)
  include(CTest)
  enable_testing()

  find_package(GTest REQUIRED)

  add_executable(service_cli_tests
    tests/CommandTest.cpp
  )

  target_link_libraries(service_cli_tests
    PRIVATE
      service_cli::service_cli
      GTest::gtest_main
  )

  include(GoogleTest)

  gtest_discover_tests(service_cli_tests)
endif()
