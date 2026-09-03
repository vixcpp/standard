/**
 *
 *  @file main.cpp
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

#include <vix/Run.hpp>

#include <array>
#include <iostream>
#include <string_view>

namespace
{
  void print_error(
      const service_cli::Error &error)
  {
    switch (error.code)
    {
    case service_cli::ErrorCode::invalid_arguments:
      std::cerr
          << "usage: service-check "
          << "<host> <port> <timeout-ms>\n";
      break;

    case service_cli::ErrorCode::invalid_configuration:
      std::cerr
          << "error: invalid service configuration\n";
      break;

    case service_cli::ErrorCode::serialization_failed:
      std::cerr
          << "error: failed to format command output\n";
      break;
    }
  }

} // namespace

int main(
    int argc,
    char **argv)
{
  if (argc != 4)
  {
    std::cerr
        << "usage: service-check "
        << "<host> <port> <timeout-ms>\n";

    return 1;
  }

  const std::array<std::string_view, 3> arguments{
      argv[1],
      argv[2],
      argv[3]};

  service_cli::Command command;

  auto result =
      vix::run(
          command,
          std::span<const std::string_view>{
              arguments});

  if (!result)
  {
    print_error(
        result.error());

    return 1;
  }

  std::cout
      << result.value();

  return 0;
}
