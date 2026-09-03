/**
 *
 *  @file Command.hpp
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

#ifndef SERVICE_CLI_COMMAND_HPP
#define SERVICE_CLI_COMMAND_HPP

#include "service_cli/Error.hpp"

#include <vix/Result.hpp>

#include <span>
#include <string>
#include <string_view>

namespace service_cli
{
  /**
   * @brief Processes the service-check command.
   *
   * The command expects three arguments:
   *
   * host, port, and timeout in milliseconds.
   *
   * It delegates configuration parsing and validation to service_config and
   * returns the textual result that the executable can present to the user.
   */
  class Command final
  {
  public:
    /**
     * @brief Executes the command with the supplied arguments.
     *
     * @param arguments Command arguments excluding the executable name.
     *
     * @return Formatted command output, or an error describing why the
     * command could not be completed.
     */
    [[nodiscard]] vix::Result<std::string, Error> run(
        std::span<const std::string_view> arguments) const;
  };

} // namespace service_cli

#endif // SERVICE_CLI_COMMAND_HPP
