cmake_minimum_required(VERSION 3.20)

# ---------------------------------------------------------------------
# Project
# ---------------------------------------------------------------------

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(vix_standard LANGUAGES CXX)
  set(VIX_STANDARD_STANDALONE ON)
else()
  set(VIX_STANDARD_STANDALONE OFF)
endif()

# ---------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------

option(
  VIX_STANDARD_BUILD_TESTS
  "Build Vix Standard tests"
  OFF
)

option(
  VIX_STANDARD_BUILD_EXAMPLES
  "Build Vix Standard examples"
  OFF
)

option(
  VIX_STANDARD_BUILD_BENCHMARKS
  "Build Vix Standard benchmarks"
  OFF
)

option(
  VIX_STANDARD_ENABLE_WARNINGS
  "Enable compiler warnings for Vix Standard"
  ON
)

# ---------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------

add_library(vix_standard
  src/Read.cpp
  src/Write.cpp
  src/Run.cpp
  src/Print.cpp
  src/Log.cpp
)

add_library(vix::standard ALIAS vix_standard)

set_target_properties(vix_standard PROPERTIES
  EXPORT_NAME standard
)

target_compile_features(vix_standard
  PUBLIC
    cxx_std_20
)

target_include_directories(vix_standard
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Public headers are listed explicitly so the build graph remains visible.
target_sources(vix_standard
  PRIVATE
    include/vix/Result.hpp
    include/vix/Duration.hpp
    include/vix/Instant.hpp
    include/vix/Task.hpp
    include/vix/Input.hpp
    include/vix/Output.hpp
    include/vix/Read.hpp
    include/vix/Write.hpp
    include/vix/Parse.hpp
    include/vix/Serialize.hpp
    include/vix/Validate.hpp
    include/vix/Run.hpp
    include/vix/Print.hpp
    include/vix/Log.hpp
)

# ---------------------------------------------------------------------
# Warnings
# ---------------------------------------------------------------------

if (VIX_STANDARD_ENABLE_WARNINGS)
  if (MSVC)
    target_compile_options(vix_standard
      PRIVATE
        /W4
        /permissive-
    )
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(vix_standard
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
  endif()
endif()

# ---------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------

if (VIX_STANDARD_BUILD_TESTS)
  include(CTest)
  enable_testing()

  find_package(GTest REQUIRED)

  add_executable(vix_standard_tests
    tests/ResultTest.cpp
    tests/DurationTest.cpp
    tests/InstantTest.cpp
    tests/TaskTest.cpp
    tests/InputOutputTest.cpp
    tests/ReadWriteTest.cpp
    tests/ParseTest.cpp
    tests/SerializeTest.cpp
    tests/ValidateTest.cpp
    tests/RunTest.cpp
    tests/PrintLogTest.cpp
  )

  target_link_libraries(vix_standard_tests
    PRIVATE
      vix::standard
      GTest::gtest_main
  )

  target_compile_features(vix_standard_tests
    PRIVATE
      cxx_std_20
  )

  include(GoogleTest)

  gtest_discover_tests(vix_standard_tests)
endif()

# ---------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------

if (VIX_STANDARD_BUILD_EXAMPLES)
  add_executable(vix_standard_library_example
    examples/library.cpp
  )

  target_link_libraries(vix_standard_library_example
    PRIVATE
      vix::standard
  )

  add_executable(vix_standard_cli_example
    examples/cli.cpp
  )

  target_link_libraries(vix_standard_cli_example
    PRIVATE
      vix::standard
  )

  add_executable(vix_standard_service_example
    examples/service.cpp
  )

  target_link_libraries(vix_standard_service_example
    PRIVATE
      vix::standard
  )
endif()

# ---------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------

if (VIX_STANDARD_BUILD_BENCHMARKS)
  find_package(benchmark REQUIRED)

  add_executable(vix_standard_compile_benchmark
    benchmarks/CompileBenchmark.cpp
  )

  target_link_libraries(vix_standard_compile_benchmark
    PRIVATE
      vix::standard
      benchmark::benchmark_main
  )

  add_executable(vix_standard_runtime_benchmark
    benchmarks/RuntimeBenchmark.cpp
  )

  target_link_libraries(vix_standard_runtime_benchmark
    PRIVATE
      vix::standard
      benchmark::benchmark_main
  )
endif()
