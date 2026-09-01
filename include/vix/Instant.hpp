/**
 *
 *  @file Instant.hpp
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

#ifndef VIX_INSTANT_HPP
#define VIX_INSTANT_HPP

#include <vix/Duration.hpp>

#include <chrono>
#include <compare>

namespace vix
{
  /**
   * @brief Represents a monotonic point in time.
   *
   * Instant is the general Vix representation of a point on a monotonic
   * clock. It is intended for measuring elapsed time, computing deadlines,
   * and comparing points observed during program execution.
   *
   * Instant uses std::chrono::steady_clock so its progression is not affected
   * by changes to the system wall clock.
   *
   * An Instant has no calendar or civil-time meaning. It does not represent
   * a date, a Unix timestamp, or a time that should be persisted or exchanged
   * between processes.
   *
   * Use Duration to represent the distance between two Instants.
   *
   * @note The epoch of std::chrono::steady_clock is unspecified.
   * @note Instant values are meaningful only relative to other Instants
   *       produced by the same clock domain.
   */
  class [[nodiscard]] Instant final
  {
  public:
    /**
     * @brief Clock used by Instant.
     */
    using clock_type = std::chrono::steady_clock;

    /**
     * @brief Native clock duration type.
     */
    using clock_duration = clock_type::duration;

    /**
     * @brief Native clock time-point type.
     */
    using chrono_type = clock_type::time_point;

    /**
     * @brief Creates an Instant at the native clock epoch.
     *
     * The default value is primarily useful as a neutral value for storage
     * and initialization. It does not represent the current time.
     */
    constexpr Instant() noexcept = default;

    /**
     * @brief Returns the current monotonic time point.
     *
     * @return Current Instant from std::chrono::steady_clock.
     */
    [[nodiscard]] static Instant now() noexcept(noexcept(clock_type::now()))
    {
      return Instant(clock_type::now(), PrivateTag{});
    }

    /**
     * @brief Creates an Instant from a native steady-clock time point.
     *
     * This function provides explicit interoperability with std::chrono
     * without exposing the internal representation through constructors.
     *
     * @param value Native steady-clock time point.
     * @return Corresponding Instant.
     */
    [[nodiscard]] static constexpr Instant from_chrono(
        chrono_type value) noexcept
    {
      return Instant(value, PrivateTag{});
    }

    /**
     * @brief Returns the native std::chrono representation.
     *
     * @return Steady-clock time point represented by this Instant.
     */
    [[nodiscard]] constexpr chrono_type chrono() const noexcept
    {
      return value_;
    }

    /**
     * @brief Computes the duration elapsed since another Instant.
     *
     * The result is positive when this Instant occurs after @p earlier,
     * zero when both Instants are equal, and negative when this Instant
     * occurs before @p earlier.
     *
     * @param earlier Instant used as the reference point.
     * @return Duration between @p earlier and this Instant.
     */
    [[nodiscard]] constexpr Duration since(Instant earlier) const noexcept
    {
      return Duration(
          std::chrono::duration_cast<Duration::chrono_type>(
              value_ - earlier.value_));
    }

    /**
     * @brief Computes the duration until another Instant.
     *
     * The result is positive when @p later occurs after this Instant,
     * zero when both Instants are equal, and negative when @p later
     * occurs before this Instant.
     *
     * @param later Instant used as the destination point.
     * @return Duration between this Instant and @p later.
     */
    [[nodiscard]] constexpr Duration until(Instant later) const noexcept
    {
      return later.since(*this);
    }

    /**
     * @brief Computes the duration elapsed since this Instant.
     *
     * A positive result indicates that this Instant is in the past relative
     * to the current monotonic clock value.
     *
     * @return Duration from this Instant to Instant::now().
     */
    [[nodiscard]] Duration elapsed() const
        noexcept(noexcept(clock_type::now()))
    {
      return now().since(*this);
    }

    /**
     * @brief Checks whether this Instant is earlier than the current time.
     *
     * @return true when this Instant occurs before Instant::now().
     */
    [[nodiscard]] bool is_past() const
        noexcept(noexcept(clock_type::now()))
    {
      return *this < now();
    }

    /**
     * @brief Checks whether this Instant is later than the current time.
     *
     * @return true when this Instant occurs after Instant::now().
     */
    [[nodiscard]] bool is_future() const
        noexcept(noexcept(clock_type::now()))
    {
      return *this > now();
    }

    /**
     * @brief Advances this Instant by a Duration.
     *
     * @param duration Amount of time to add.
     * @return Reference to this Instant.
     */
    constexpr Instant &operator+=(Duration duration) noexcept
    {
      value_ += std::chrono::duration_cast<clock_duration>(
          duration.chrono());

      return *this;
    }

    /**
     * @brief Moves this Instant backward by a Duration.
     *
     * @param duration Amount of time to subtract.
     * @return Reference to this Instant.
     */
    constexpr Instant &operator-=(Duration duration) noexcept
    {
      value_ -= std::chrono::duration_cast<clock_duration>(
          duration.chrono());

      return *this;
    }

    /**
     * @brief Compares two Instants.
     *
     * Comparison is valid because both values belong to the same monotonic
     * clock domain.
     */
    [[nodiscard]] friend constexpr auto operator<=>(
        Instant lhs,
        Instant rhs) noexcept = default;

    /**
     * @brief Returns an Instant advanced by a Duration.
     *
     * @param instant Starting Instant.
     * @param duration Amount of time to add.
     * @return Advanced Instant.
     */
    [[nodiscard]] friend constexpr Instant operator+(
        Instant instant,
        Duration duration) noexcept
    {
      instant += duration;
      return instant;
    }

    /**
     * @brief Returns an Instant advanced by a Duration.
     *
     * @param duration Amount of time to add.
     * @param instant Starting Instant.
     * @return Advanced Instant.
     */
    [[nodiscard]] friend constexpr Instant operator+(
        Duration duration,
        Instant instant) noexcept
    {
      instant += duration;
      return instant;
    }

    /**
     * @brief Returns an Instant moved backward by a Duration.
     *
     * @param instant Starting Instant.
     * @param duration Amount of time to subtract.
     * @return Adjusted Instant.
     */
    [[nodiscard]] friend constexpr Instant operator-(
        Instant instant,
        Duration duration) noexcept
    {
      instant -= duration;
      return instant;
    }

    /**
     * @brief Computes the duration between two Instants.
     *
     * The result is positive when @p lhs occurs after @p rhs and negative
     * when @p lhs occurs before @p rhs.
     *
     * @param lhs Left Instant.
     * @param rhs Right Instant.
     * @return Duration from @p rhs to @p lhs.
     */
    [[nodiscard]] friend constexpr Duration operator-(
        Instant lhs,
        Instant rhs) noexcept
    {
      return lhs.since(rhs);
    }

  private:
    struct PrivateTag
    {
    };

    constexpr Instant(
        chrono_type value,
        PrivateTag) noexcept
        : value_(value)
    {
    }

    chrono_type value_{};
  };

} // namespace vix

#endif // VIX_INSTANT_HPP
