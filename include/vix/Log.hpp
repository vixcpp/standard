/**
 *
 *  @file Log.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/vixcpp/standard
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Vix.cpp Standard
 */

#ifndef VIX_LOG_HPP
#define VIX_LOG_HPP

#include <concepts>
#include <string_view>
#include <utility>

namespace vix
{
  /**
   * @brief Identifies the severity of a log event.
   *
   * LogLevel provides a small, domain-independent severity vocabulary for
   * recording program events.
   *
   * The enumeration does not define filtering, formatting, storage,
   * timestamps, destinations, or transport behavior. Those concerns belong
   * to the logging implementation receiving the event.
   */
  enum class LogLevel
  {
    /**
     * @brief Fine-grained execution information.
     */
    trace,

    /**
     * @brief Diagnostic information useful during development.
     */
    debug,

    /**
     * @brief Normal program information.
     */
    info,

    /**
     * @brief Unexpected or potentially problematic behavior that does not
     *        prevent continued execution.
     */
    warning,

    /**
     * @brief An operation or part of the program failed.
     */
    error,

    /**
     * @brief A severe failure that may prevent continued operation.
     */
    critical
  };

  /**
   * @brief Describes an object that can record a textual log event.
   *
   * A LogTarget exposes:
   *
   * @code
   * target.log(level, message);
   * @endcode
   *
   * The concept deliberately does not prescribe the return type of the
   * operation.
   *
   * A logging implementation may return:
   *
   * @code
   * void
   * bool
   * vix::Result<void, Error>
   * @endcode
   *
   * or another domain-appropriate result when necessary.
   *
   * LogTarget describes only the ability to record an event. It does not
   * require:
   *
   * - global logger state
   * - dynamic allocation
   * - asynchronous execution
   * - a formatting library
   * - files
   * - terminal output
   * - network transport
   * - filtering
   *
   * @tparam T Candidate logging target type.
   *
   * @note LogTarget does not require inheritance or virtual dispatch.
   */
  template <typename T>
  concept LogTarget =
      requires(
          T &&target,
          LogLevel level,
          std::string_view message) {
        std::forward<T>(target).log(level, message);
      };

  /**
   * @brief Extracts the type returned by a logging operation.
   *
   * @tparam T Logging target type.
   */
  template <typename T>
    requires LogTarget<T>
  using log_result_t =
      decltype(std::declval<T>().log(
          std::declval<LogLevel>(),
          std::declval<std::string_view>()));

  /**
   * @brief Records a textual program event.
   *
   * log() is the general Vix expression for recording an event through an
   * explicit logging target.
   *
   * The operation forwards directly to the target's log() implementation.
   * Standard does not select a global logger, allocate an intermediate
   * message, format arbitrary values, or impose a storage backend.
   *
   * For example:
   *
   * @code
   * vix::log(logger, vix::LogLevel::info, "server started");
   * @endcode
   *
   * The exact return type of the target is preserved.
   *
   * This keeps logging policy outside Standard while allowing different
   * logging implementations to share a small common operation.
   *
   * @tparam T Logging target type.
   * @param target Destination that records the event.
   * @param level Severity associated with the event.
   * @param message Textual event description.
   * @return Exactly what the target's log() operation returns.
   *
   * @note log() records a program event. It is distinct from print(), which
   *       presents text to an output destination, and write(), which transfers
   *       bytes.
   * @note The supplied message is non-owning. A logging implementation that
   *       retains the message after the call must copy or otherwise own the
   *       required data.
   */
  template <typename T>
    requires LogTarget<T &&>
  constexpr decltype(auto) log(
      T &&target,
      LogLevel level,
      std::string_view message) noexcept(noexcept(std::forward<T>(target).log(level,
                                                                              message)))
  {
    return std::forward<T>(target).log(
        level,
        message);
  }

} // namespace vix

#endif // VIX_LOG_HPP
