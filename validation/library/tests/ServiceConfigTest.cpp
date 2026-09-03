/**
 *
 *  @file ServiceConfigTest.cpp
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

#include "service_config/Error.hpp"
#include "service_config/ServiceConfig.hpp"

#include <vix/Duration.hpp>
#include <vix/Result.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <type_traits>

namespace
{
  static_assert(std::same_as<
                decltype(service_config::ServiceConfig::create(
                    "127.0.0.1",
                    "8080",
                    "5000")),
                vix::Result<
                    service_config::ServiceConfig,
                    service_config::Error>>);

  TEST(ServiceConfigTest, CreatesValidConfiguration)
  {
    auto result =
        service_config::ServiceConfig::create(
            "127.0.0.1",
            "8080",
            "5000");

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value().host(),
        "127.0.0.1");

    EXPECT_EQ(
        result.value().port(),
        8080);

    EXPECT_EQ(
        result.value().timeout(),
        vix::Duration::milliseconds(5000));
  }

  TEST(ServiceConfigTest, AcceptsHostname)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "3000",
            "1000");

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value().host(),
        "localhost");

    EXPECT_EQ(
        result.value().port(),
        3000);
  }

  TEST(ServiceConfigTest, AcceptsMinimumPort)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "1",
            "1000");

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value().port(),
        1);
  }

  TEST(ServiceConfigTest, AcceptsMaximumPort)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "65535",
            "1000");

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value().port(),
        65535);
  }

  TEST(ServiceConfigTest, RejectsEmptyHost)
  {
    auto result =
        service_config::ServiceConfig::create(
            "",
            "8080",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::empty_host);
  }

  TEST(ServiceConfigTest, RejectsInvalidPort)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "hello",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::invalid_port);
  }

  TEST(ServiceConfigTest, RejectsEmptyPort)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::invalid_port);
  }

  TEST(ServiceConfigTest, RejectsPortWithTrailingCharacters)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080http",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::invalid_port);

    EXPECT_EQ(
        result.error().position,
        4U);
  }

  TEST(ServiceConfigTest, RejectsPortWithWhitespace)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            " 8080",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::invalid_port);
  }

  TEST(ServiceConfigTest, RejectsZeroPort)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "0",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::port_out_of_range);
  }

  TEST(ServiceConfigTest, RejectsNegativePort)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "-1",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::port_out_of_range);
  }

  TEST(ServiceConfigTest, RejectsPortAboveMaximum)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "65536",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::port_out_of_range);
  }

  TEST(ServiceConfigTest, RejectsPortOutsideIntegerRange)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "999999999999999999999999999",
            "5000");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::port_out_of_range);
  }

  TEST(ServiceConfigTest, RejectsInvalidTimeout)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080",
            "hello");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::invalid_timeout);
  }

  TEST(ServiceConfigTest, RejectsEmptyTimeout)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080",
            "");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::invalid_timeout);
  }

  TEST(ServiceConfigTest, RejectsTimeoutWithTrailingCharacters)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080",
            "5000ms");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::invalid_timeout);

    EXPECT_EQ(
        result.error().position,
        4U);
  }

  TEST(ServiceConfigTest, RejectsZeroTimeout)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080",
            "0");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::timeout_out_of_range);
  }

  TEST(ServiceConfigTest, RejectsNegativeTimeout)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080",
            "-1");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::timeout_out_of_range);
  }

  TEST(ServiceConfigTest, RejectsTimeoutOutsideIntegerRange)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080",
            "999999999999999999999999999");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        service_config::ErrorCode::timeout_out_of_range);
  }

  TEST(ServiceConfigTest, AcceptsOneMillisecondTimeout)
  {
    auto result =
        service_config::ServiceConfig::create(
            "localhost",
            "8080",
            "1");

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value().timeout(),
        vix::Duration::milliseconds(1));
  }

  TEST(ServiceConfigTest, ConfigurationOwnsHost)
  {
    std::string host{
        "localhost"};

    auto result =
        service_config::ServiceConfig::create(
            host,
            "8080",
            "5000");

    ASSERT_TRUE(result);

    host.assign(
        "changed");

    EXPECT_EQ(
        result.value().host(),
        "localhost");
  }

  TEST(ServiceConfigTest, ErrorEqualityPreservesCodeAndPosition)
  {
    constexpr service_config::Error first{
        .code =
            service_config::ErrorCode::invalid_port,
        .position = 4};

    constexpr service_config::Error second{
        .code =
            service_config::ErrorCode::invalid_port,
        .position = 4};

    constexpr service_config::Error different{
        .code =
            service_config::ErrorCode::invalid_timeout,
        .position = 4};

    static_assert(first == second);
    static_assert(first != different);

    EXPECT_EQ(first, second);
    EXPECT_NE(first, different);
  }

} // namespace
