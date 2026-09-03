/**
 *
 *  @file CommandTest.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/vixcpp/standard
 *
 *  Licensed under the MIT License.
 *  See the LICENSE file in the project root for license information.
 *
 *  Vix.cpp Standard
 */

#include "service_cli/Command.hpp"
#include "service_cli/Error.hpp"

#include "service_config/Error.hpp"

#include <vix/Result.hpp>
#include <vix/Run.hpp>

#include <gtest/gtest.h>

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace
{
  static_assert(
      vix::RunnableWith<
          service_cli::Command &,
          std::span<const std::string_view>>);

  static_assert(std::same_as<
                decltype(std::declval<
                             const service_cli::Command &>()
                             .run(
                                 std::declval<
                                     std::span<const std::string_view>>())),
                vix::Result<
                    std::string,
                    service_cli::Error>>);

  static_assert(std::same_as<
                decltype(vix::run(
                    std::declval<
                        service_cli::Command &>(),
                    std::declval<
                        std::span<const std::string_view>>())),
                vix::Result<
                    std::string,
                    service_cli::Error>>);

  TEST(CommandTest, RunsWithValidArguments)
  {
    const std::array<std::string_view, 3> arguments{
        "127.0.0.1",
        "8080",
        "5000"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value(),
        "host: 127.0.0.1\n"
        "port: 8080\n"
        "timeout: 5000 ms\n");
  }

  TEST(CommandTest, SupportsHostname)
  {
    const std::array<std::string_view, 3> arguments{
        "localhost",
        "3000",
        "1000"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value(),
        "host: localhost\n"
        "port: 3000\n"
        "timeout: 1000 ms\n");
  }

  TEST(CommandTest, RejectsMissingArguments)
  {
    const std::array<std::string_view, 2> arguments{
        "localhost",
        "8080"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_cli::ErrorCode::invalid_arguments);

    EXPECT_FALSE(
        result.error().configuration_error.has_value());
  }

  TEST(CommandTest, RejectsExtraArguments)
  {
    const std::array<std::string_view, 4> arguments{
        "localhost",
        "8080",
        "5000",
        "extra"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_cli::ErrorCode::invalid_arguments);
  }

  TEST(CommandTest, PreservesEmptyHostError)
  {
    const std::array<std::string_view, 3> arguments{
        "",
        "8080",
        "5000"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_cli::ErrorCode::invalid_configuration);

    ASSERT_TRUE(
        result.error().configuration_error.has_value());

    EXPECT_EQ(
        result.error().configuration_error->code,
        service_config::ErrorCode::empty_host);
  }

  TEST(CommandTest, PreservesInvalidPortError)
  {
    const std::array<std::string_view, 3> arguments{
        "localhost",
        "hello",
        "5000"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_cli::ErrorCode::invalid_configuration);

    ASSERT_TRUE(
        result.error().configuration_error.has_value());

    EXPECT_EQ(
        result.error().configuration_error->code,
        service_config::ErrorCode::invalid_port);
  }

  TEST(CommandTest, PreservesPortRangeError)
  {
    const std::array<std::string_view, 3> arguments{
        "localhost",
        "70000",
        "5000"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    ASSERT_TRUE(
        result.error().configuration_error.has_value());

    EXPECT_EQ(
        result.error().configuration_error->code,
        service_config::ErrorCode::port_out_of_range);
  }

  TEST(CommandTest, PreservesPortErrorPosition)
  {
    const std::array<std::string_view, 3> arguments{
        "localhost",
        "8080http",
        "5000"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    ASSERT_TRUE(
        result.error().configuration_error.has_value());

    EXPECT_EQ(
        result.error().configuration_error->code,
        service_config::ErrorCode::invalid_port);

    EXPECT_EQ(
        result.error().configuration_error->position,
        4U);
  }

  TEST(CommandTest, PreservesInvalidTimeoutError)
  {
    const std::array<std::string_view, 3> arguments{
        "localhost",
        "8080",
        "hello"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_cli::ErrorCode::invalid_configuration);

    ASSERT_TRUE(
        result.error().configuration_error.has_value());

    EXPECT_EQ(
        result.error().configuration_error->code,
        service_config::ErrorCode::invalid_timeout);
  }

  TEST(CommandTest, PreservesTimeoutRangeError)
  {
    const std::array<std::string_view, 3> arguments{
        "localhost",
        "8080",
        "-1"};

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_FALSE(result);

    ASSERT_TRUE(
        result.error().configuration_error.has_value());

    EXPECT_EQ(
        result.error().configuration_error->code,
        service_config::ErrorCode::timeout_out_of_range);
  }

  TEST(CommandTest, CanBeExecutedThroughVixRun)
  {
    const std::array<std::string_view, 3> arguments{
        "localhost",
        "8080",
        "5000"};

    service_cli::Command command;

    auto result =
        vix::run(
            command,
            std::span<const std::string_view>{
                arguments});

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value(),
        "host: localhost\n"
        "port: 8080\n"
        "timeout: 5000 ms\n");
  }

  TEST(CommandTest, CommandDoesNotModifyArguments)
  {
    std::array<std::string_view, 3> arguments{
        "localhost",
        "8080",
        "5000"};

    const auto original =
        arguments;

    service_cli::Command command;

    auto result =
        command.run(arguments);

    ASSERT_TRUE(result);

    EXPECT_EQ(
        arguments,
        original);
  }

  TEST(CommandTest, ErrorEqualityPreservesConfigurationError)
  {
    const service_cli::Error first{
        .code =
            service_cli::ErrorCode::invalid_configuration,
        .configuration_error =
            service_config::Error{
                .code =
                    service_config::ErrorCode::invalid_port,
                .position = 4}};

    const service_cli::Error second{
        .code =
            service_cli::ErrorCode::invalid_configuration,
        .configuration_error =
            service_config::Error{
                .code =
                    service_config::ErrorCode::invalid_port,
                .position = 4}};

    const service_cli::Error different{
        .code =
            service_cli::ErrorCode::invalid_arguments};

    EXPECT_EQ(first, second);
    EXPECT_NE(first, different);
  }

} // namespace
