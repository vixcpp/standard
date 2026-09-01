/**
 *
 *  @file Duration.hpp
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

#ifndef VIX_DURATION_HPP
#define VIX_DURATION_HPP

#include <chrono>
#include <compare>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace vix
{
  /**
   * @brief Represents an amount of elapsed or requested time.
   *
   * Duration is the general Vix representation of a span of time. It can be
   * used by independent domains for concepts such as timeouts, retry delays,
   * cache lifetimes, timers, process limits, and scheduling intervals.
   *
   * The value is stored with nanosecond resolution and does not require
   * dynamic allocation.
   *
   * Duration represents an amount of time, not a point on a clock. Use
   * Instant for a point in time.
   *
   * Negative durations are valid. Their interpretation depends on the
   * operation consuming the value.
   *
   * @note Arithmetic follows the range of the underlying signed 64-bit
   *       nanosecond representation.
   */
  class [[nodiscard]] Duration final
  {
  public:
    /**
     * @brief Integral representation used by Duration.
     */
    using rep = std::int64_t;

    /**
     * @brief Native chrono representation used internally.
     */
    using chrono_type = std::chrono::nanoseconds;

    /**
     * @brief Creates a zero duration.
     */
    constexpr Duration() noexcept = default;

    /**
     * @brief Creates a Duration from an integral std::chrono duration.
     *
     * The supplied duration is converted to nanosecond resolution.
     *
     * @tparam Rep Chrono duration representation type.
     * @tparam Period Chrono duration period.
     * @param value Duration to convert.
     */
    template <std::integral Rep, typename Period>
    explicit constexpr Duration(
        std::chrono::duration<Rep, Period> value) noexcept
        : value_(std::chrono::duration_cast<chrono_type>(value))
    {
    }

    /**
     * @brief Creates a Duration expressed in nanoseconds.
     *
     * @param value Number of nanoseconds.
     * @return Corresponding Duration.
     */
    [[nodiscard]] static constexpr Duration nanoseconds(rep value) noexcept
    {
      return Duration(chrono_type(value), PrivateTag{});
    }

    /**
     * @brief Creates a Duration expressed in microseconds.
     *
     * @param value Number of microseconds.
     * @return Corresponding Duration.
     */
    [[nodiscard]] static constexpr Duration microseconds(rep value) noexcept
    {
      return Duration(
          std::chrono::duration_cast<chrono_type>(
              std::chrono::microseconds(value)),
          PrivateTag{});
    }

    /**
     * @brief Creates a Duration expressed in milliseconds.
     *
     * @param value Number of milliseconds.
     * @return Corresponding Duration.
     */
    [[nodiscard]] static constexpr Duration milliseconds(rep value) noexcept
    {
      return Duration(
          std::chrono::duration_cast<chrono_type>(
              std::chrono::milliseconds(value)),
          PrivateTag{});
    }

    /**
     * @brief Creates a Duration expressed in seconds.
     *
     * @param value Number of seconds.
     * @return Corresponding Duration.
     */
    [[nodiscard]] static constexpr Duration seconds(rep value) noexcept
    {
      return Duration(
          std::chrono::duration_cast<chrono_type>(
              std::chrono::seconds(value)),
          PrivateTag{});
    }

    /**
     * @brief Creates a Duration expressed in minutes.
     *
     * @param value Number of minutes.
     * @return Corresponding Duration.
     */
    [[nodiscard]] static constexpr Duration minutes(rep value) noexcept
    {
      return Duration(
          std::chrono::duration_cast<chrono_type>(
              std::chrono::minutes(value)),
          PrivateTag{});
    }

    /**
     * @brief Creates a Duration expressed in hours.
     *
     * @param value Number of hours.
     * @return Corresponding Duration.
     */
    [[nodiscard]] static constexpr Duration hours(rep value) noexcept
    {
      return Duration(
          std::chrono::duration_cast<chrono_type>(
              std::chrono::hours(value)),
          PrivateTag{});
    }

    /**
     * @brief Returns a zero duration.
     *
     * @return Duration whose value is zero.
     */
    [[nodiscard]] static constexpr Duration zero() noexcept
    {
      return Duration{};
    }

    /**
     * @brief Returns the minimum representable Duration.
     *
     * @return Minimum Duration value.
     */
    [[nodiscard]] static constexpr Duration min() noexcept
    {
      return Duration(chrono_type::min(), PrivateTag{});
    }

    /**
     * @brief Returns the maximum representable Duration.
     *
     * @return Maximum Duration value.
     */
    [[nodiscard]] static constexpr Duration max() noexcept
    {
      return Duration(chrono_type::max(), PrivateTag{});
    }

    /**
     * @brief Returns the duration in nanoseconds.
     *
     * @return Number of nanoseconds.
     */
    [[nodiscard]] constexpr rep count_nanoseconds() const noexcept
    {
      return value_.count();
    }

    /**
     * @brief Converts the duration to microseconds.
     *
     * Conversion toward a coarser unit truncates any remainder.
     *
     * @return Number of complete microseconds.
     */
    [[nodiscard]] constexpr rep count_microseconds() const noexcept
    {
      return std::chrono::duration_cast<std::chrono::microseconds>(value_)
          .count();
    }

    /**
     * @brief Converts the duration to milliseconds.
     *
     * Conversion toward a coarser unit truncates any remainder.
     *
     * @return Number of complete milliseconds.
     */
    [[nodiscard]] constexpr rep count_milliseconds() const noexcept
    {
      return std::chrono::duration_cast<std::chrono::milliseconds>(value_)
          .count();
    }

    /**
     * @brief Converts the duration to seconds.
     *
     * Conversion toward a coarser unit truncates any remainder.
     *
     * @return Number of complete seconds.
     */
    [[nodiscard]] constexpr rep count_seconds() const noexcept
    {
      return std::chrono::duration_cast<std::chrono::seconds>(value_)
          .count();
    }

    /**
     * @brief Converts the duration to minutes.
     *
     * Conversion toward a coarser unit truncates any remainder.
     *
     * @return Number of complete minutes.
     */
    [[nodiscard]] constexpr rep count_minutes() const noexcept
    {
      return std::chrono::duration_cast<std::chrono::minutes>(value_)
          .count();
    }

    /**
     * @brief Converts the duration to hours.
     *
     * Conversion toward a coarser unit truncates any remainder.
     *
     * @return Number of complete hours.
     */
    [[nodiscard]] constexpr rep count_hours() const noexcept
    {
      return std::chrono::duration_cast<std::chrono::hours>(value_)
          .count();
    }

    /**
     * @brief Converts this Duration to a std::chrono duration.
     *
     * @tparam ChronoDuration Destination std::chrono duration type.
     * @return Converted duration.
     */
    template <typename ChronoDuration>
      requires requires {
        typename ChronoDuration::rep;
        typename ChronoDuration::period;
      }
    [[nodiscard]] constexpr ChronoDuration as() const noexcept
    {
      return std::chrono::duration_cast<ChronoDuration>(value_);
    }

    /**
     * @brief Returns the native chrono representation.
     *
     * @return Duration represented as std::chrono::nanoseconds.
     */
    [[nodiscard]] constexpr chrono_type chrono() const noexcept
    {
      return value_;
    }

    /**
     * @brief Checks whether this Duration is zero.
     *
     * @return true when the duration is exactly zero.
     */
    [[nodiscard]] constexpr bool is_zero() const noexcept
    {
      return value_.count() == 0;
    }

    /**
     * @brief Checks whether this Duration is positive.
     *
     * @return true when the duration is greater than zero.
     */
    [[nodiscard]] constexpr bool is_positive() const noexcept
    {
      return value_.count() > 0;
    }

    /**
     * @brief Checks whether this Duration is negative.
     *
     * @return true when the duration is less than zero.
     */
    [[nodiscard]] constexpr bool is_negative() const noexcept
    {
      return value_.count() < 0;
    }

    /**
     * @brief Adds another duration to this value.
     *
     * @param other Duration to add.
     * @return Reference to this Duration.
     */
    constexpr Duration &operator+=(Duration other) noexcept
    {
      value_ += other.value_;
      return *this;
    }

    /**
     * @brief Subtracts another duration from this value.
     *
     * @param other Duration to subtract.
     * @return Reference to this Duration.
     */
    constexpr Duration &operator-=(Duration other) noexcept
    {
      value_ -= other.value_;
      return *this;
    }

    /**
     * @brief Multiplies this duration by an integral scalar.
     *
     * @param scalar Multiplication factor.
     * @return Reference to this Duration.
     */
    constexpr Duration &operator*=(rep scalar) noexcept
    {
      value_ *= scalar;
      return *this;
    }

    /**
     * @brief Divides this duration by an integral scalar.
     *
     * @param scalar Divisor.
     * @return Reference to this Duration.
     *
     * @pre scalar must not be zero.
     */
    constexpr Duration &operator/=(rep scalar) noexcept
    {
      value_ /= scalar;
      return *this;
    }

    /**
     * @brief Returns the positive form of this duration.
     *
     * @return Unchanged Duration.
     */
    [[nodiscard]] constexpr Duration operator+() const noexcept
    {
      return *this;
    }

    /**
     * @brief Returns the negated duration.
     *
     * @return Duration with the opposite sign.
     */
    [[nodiscard]] constexpr Duration operator-() const noexcept
    {
      return Duration(-value_, PrivateTag{});
    }

    /**
     * @brief Compares two durations.
     */
    [[nodiscard]] friend constexpr auto operator<=>(
        Duration lhs,
        Duration rhs) noexcept = default;

    /**
     * @brief Adds two durations.
     *
     * @param lhs Left duration.
     * @param rhs Right duration.
     * @return Sum of both durations.
     */
    [[nodiscard]] friend constexpr Duration operator+(
        Duration lhs,
        Duration rhs) noexcept
    {
      lhs += rhs;
      return lhs;
    }

    /**
     * @brief Subtracts two durations.
     *
     * @param lhs Left duration.
     * @param rhs Right duration.
     * @return Difference between both durations.
     */
    [[nodiscard]] friend constexpr Duration operator-(
        Duration lhs,
        Duration rhs) noexcept
    {
      lhs -= rhs;
      return lhs;
    }

    /**
     * @brief Multiplies a duration by an integral scalar.
     *
     * @param duration Duration to multiply.
     * @param scalar Multiplication factor.
     * @return Scaled Duration.
     */
    [[nodiscard]] friend constexpr Duration operator*(
        Duration duration,
        rep scalar) noexcept
    {
      duration *= scalar;
      return duration;
    }

    /**
     * @brief Multiplies an integral scalar by a duration.
     *
     * @param scalar Multiplication factor.
     * @param duration Duration to multiply.
     * @return Scaled Duration.
     */
    [[nodiscard]] friend constexpr Duration operator*(
        rep scalar,
        Duration duration) noexcept
    {
      duration *= scalar;
      return duration;
    }

    /**
     * @brief Divides a duration by an integral scalar.
     *
     * @param duration Duration to divide.
     * @param scalar Divisor.
     * @return Scaled Duration.
     *
     * @pre scalar must not be zero.
     */
    [[nodiscard]] friend constexpr Duration operator/(
        Duration duration,
        rep scalar) noexcept
    {
      duration /= scalar;
      return duration;
    }

    /**
     * @brief Computes the ratio between two durations.
     *
     * @param lhs Dividend duration.
     * @param rhs Divisor duration.
     * @return Integral ratio between both durations.
     *
     * @pre rhs must not be zero.
     */
    [[nodiscard]] friend constexpr rep operator/(
        Duration lhs,
        Duration rhs) noexcept
    {
      return lhs.value_ / rhs.value_;
    }

    /**
     * @brief Computes the remainder after dividing two durations.
     *
     * @param lhs Dividend duration.
     * @param rhs Divisor duration.
     * @return Remaining Duration.
     *
     * @pre rhs must not be zero.
     */
    [[nodiscard]] friend constexpr Duration operator%(
        Duration lhs,
        Duration rhs) noexcept
    {
      return Duration(lhs.value_ % rhs.value_, PrivateTag{});
    }

  private:
    struct PrivateTag
    {
    };

    constexpr Duration(
        chrono_type value,
        PrivateTag) noexcept
        : value_(value)
    {
    }

    chrono_type value_{};
  };

} // namespace vix

#endif // VIX_DURATION_HPP
