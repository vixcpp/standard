/**
 *
 *  @file Command.cpp
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
#include "service_config/ServiceConfig.hpp"

#include <vix/Serialize.hpp>

#include <string>

namespace service_cli
{
  namespace
  {
    [[nodiscard]] Error invalid_arguments() noexcept
    {
      return Error{
          .code = ErrorCode::invalid_arguments};
    }

    [[nodiscard]] Error invalid_configuration(
        service_config::Error error)
    {
      return Error{
          .code = ErrorCode::invalid_configuration,
          .configuration_error = error};
    }

    [[nodiscard]] Error serialization_failed() noexcept
    {
      return Error{
          .code = ErrorCode::serialization_failed};
    }

  } // namespace

  vix::Result<std::string, Error> Command::run(
      std::span<const std::string_view> arguments) const
  {
    if (arguments.size() != 3)
    {
      return vix::Result<
          std::string,
          Error>::failure(invalid_arguments());
    }

    auto configuration =
        service_config::ServiceConfig::create(
            arguments[0],
            arguments[1],
            arguments[2]);

    if (!configuration)
    {
      return vix::Result<
          std::string,
          Error>::failure(invalid_configuration(configuration.error()));
    }

    const auto &config =
        configuration.value();

    auto port =
        vix::serialize(
            config.port());

    if (!port)
    {
      return vix::Result<
          std::string,
          Error>::failure(serialization_failed());
    }

    auto timeout =
        vix::serialize(
            config.timeout().count_milliseconds());

    if (!timeout)
    {
      return vix::Result<
          std::string,
          Error>::failure(serialization_failed());
    }

    std::string output;

    output.reserve(
        config.host().size() +
        port.value().size() +
        timeout.value().size() +
        32);

    output += "host: ";
    output += config.host();
    output += '\n';

    output += "port: ";
    output += port.value();
    output += '\n';

    output += "timeout: ";
    output += timeout.value();
    output += " ms\n";

    return vix::Result<
        std::string,
        Error>::success(std::move(output));
  }

} // namespace service_cli
