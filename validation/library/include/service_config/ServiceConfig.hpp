/**
 *
 *  @file ServiceConfig.hpp
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

#ifndef SERVICE_CONFIG_SERVICE_CONFIG_HPP
#define SERVICE_CONFIG_SERVICE_CONFIG_HPP

#include "service_config/Error.hpp"

#include <vix/Duration.hpp>
#include <vix/Result.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace service_config
{
  /**
   * @brief Stores the validated configuration of a network service.
   *
   * A ServiceConfig is created from textual configuration values. Creation
   * parses and validates those values before constructing the configuration,
   * so every successfully created instance contains a non-empty host, a valid
   * TCP port, and a positive timeout.
   */
  class ServiceConfig final
  {
  public:
    /**
     * @brief Creates a validated service configuration.
     *
     * @param host Service host name or address.
     * @param port Decimal TCP port in the range 1 to 65535.
     * @param timeout_ms Positive timeout expressed in milliseconds.
     *
     * @return The validated configuration, or an error describing why the
     * supplied values could not be accepted.
     */
    [[nodiscard]] static vix::Result<ServiceConfig, Error> create(
        std::string_view host,
        std::string_view port,
        std::string_view timeout_ms);

    /**
     * @brief Returns the configured host.
     */
    [[nodiscard]] const std::string &host() const noexcept;

    /**
     * @brief Returns the configured TCP port.
     */
    [[nodiscard]] std::uint16_t port() const noexcept;

    /**
     * @brief Returns the configured timeout.
     */
    [[nodiscard]] vix::Duration timeout() const noexcept;

  private:
    ServiceConfig(
        std::string host,
        std::uint16_t port,
        vix::Duration timeout) noexcept;

    std::string host_;
    std::uint16_t port_;
    vix::Duration timeout_;
  };

} // namespace service_config

#endif // SERVICE_CONFIG_SERVICE_CONFIG_HPP
