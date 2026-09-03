/**
 *
 *  @file Error.hpp
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

#ifndef SERVICE_CLI_ERROR_HPP
#define SERVICE_CLI_ERROR_HPP

#include "service_config/Error.hpp"

#include <optional>

namespace service_cli
{
  /**
   * @brief Identifies an error produced while processing a CLI command.
   */
  enum class ErrorCode
  {
    invalid_arguments,
    invalid_configuration,
    serialization_failed
  };

  /**
   * @brief Describes a service CLI error.
   *
   * Configuration errors preserve the original service_config error instead
   * of replacing domain-specific information with a generic CLI failure.
   */
  struct Error
  {
    ErrorCode code;

    std::optional<service_config::Error>
        configuration_error{};

    [[nodiscard]] bool operator==(
        const Error &) const = default;
  };

} // namespace service_cli

#endif // SERVICE_CLI_ERROR_HPP
