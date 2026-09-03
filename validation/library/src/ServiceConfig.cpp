/**
 *
 *  @file ServiceConfig.cpp
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

#include "service_config/ServiceConfig.hpp"

#include <vix/Parse.hpp>

#include <cstdint>
#include <utility>

namespace service_config
{
  namespace
  {
    constexpr std::int64_t minimum_port = 1;
    constexpr std::int64_t maximum_port = 65535;

    constexpr std::int64_t maximum_timeout_ms =
        vix::Duration::max().count_nanoseconds() /
        1'000'000;

    [[nodiscard]] Error port_parse_error(
        const vix::ParseError &error) noexcept
    {
      if (error.code == vix::ParseErrorCode::out_of_range)
      {
        return Error{
            .code = ErrorCode::port_out_of_range,
            .position = error.position};
      }

      return Error{
          .code = ErrorCode::invalid_port,
          .position = error.position};
    }

    [[nodiscard]] Error timeout_parse_error(
        const vix::ParseError &error) noexcept
    {
      if (error.code == vix::ParseErrorCode::out_of_range)
      {
        return Error{
            .code = ErrorCode::timeout_out_of_range,
            .position = error.position};
      }

      return Error{
          .code = ErrorCode::invalid_timeout,
          .position = error.position};
    }

  } // namespace

  ServiceConfig::ServiceConfig(
      std::string host,
      std::uint16_t port,
      vix::Duration timeout) noexcept
      : host_(std::move(host)),
        port_(port),
        timeout_(timeout)
  {
  }

  vix::Result<ServiceConfig, Error> ServiceConfig::create(
      std::string_view host,
      std::string_view port,
      std::string_view timeout_ms)
  {
    if (host.empty())
    {
      return vix::Result<
          ServiceConfig,
          Error>::failure(Error{
          .code = ErrorCode::empty_host,
          .position = 0});
    }

    auto parsed_port =
        vix::parse<std::int64_t>(port);

    if (!parsed_port)
    {
      return vix::Result<
          ServiceConfig,
          Error>::failure(port_parse_error(parsed_port.error()));
    }

    if (
        parsed_port.value() < minimum_port ||
        parsed_port.value() > maximum_port)
    {
      return vix::Result<
          ServiceConfig,
          Error>::failure(Error{
          .code = ErrorCode::port_out_of_range,
          .position = 0});
    }

    auto parsed_timeout =
        vix::parse<std::int64_t>(timeout_ms);

    if (!parsed_timeout)
    {
      return vix::Result<
          ServiceConfig,
          Error>::failure(timeout_parse_error(parsed_timeout.error()));
    }

    if (
        parsed_timeout.value() <= 0 ||
        parsed_timeout.value() > maximum_timeout_ms)
    {
      return vix::Result<
          ServiceConfig,
          Error>::failure(Error{
          .code = ErrorCode::timeout_out_of_range,
          .position = 0});
    }

    return vix::Result<
        ServiceConfig,
        Error>::success(ServiceConfig{
        std::string{host},
        static_cast<std::uint16_t>(
            parsed_port.value()),
        vix::Duration::milliseconds(
            parsed_timeout.value())});
  }

  const std::string &ServiceConfig::host() const noexcept
  {
    return host_;
  }

  std::uint16_t ServiceConfig::port() const noexcept
  {
    return port_;
  }

  vix::Duration ServiceConfig::timeout() const noexcept
  {
    return timeout_;
  }

} // namespace service_config
