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

#ifndef SERVICE_CONFIG_ERROR_HPP
#define SERVICE_CONFIG_ERROR_HPP

#include <cstddef>

namespace service_config
{
  /**
   * @brief Identifies an error produced while creating a service
   * configuration.
   */
  enum class ErrorCode
  {
    empty_host,
    invalid_port,
    port_out_of_range,
    invalid_timeout,
    timeout_out_of_range
  };

  /**
   * @brief Describes a service configuration error.
   *
   * The position identifies the byte at which textual parsing failed when
   * applicable. Errors that are not associated with textual parsing use
   * position zero.
   */
  struct Error
  {
    ErrorCode code;
    std::size_t position{0};

    [[nodiscard]] constexpr bool operator==(
        const Error &) const noexcept = default;
  };

} // namespace service_config

#endif // SERVICE_CONFIG_ERROR_HPP
