/**
 *
 *  @file DurationTest.cpp
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

#include <vix/Duration.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <type_traits>

namespace
{
  static_assert(std::is_trivially_copy_constructible_v<vix::Duration>);
  static_assert(std::is_trivially_copy_assignable_v<vix::Duration>);
  static_assert(std::is_nothrow_move_constructible_v<vix::Duration>);
  static_assert(std::is_nothrow_move_assignable_v<vix::Duration>);

  TEST(DurationTest, DefaultConstructionProducesZero)
  {
    constexpr vix::Duration duration;

    static_assert(duration.is_zero());
    static_assert(duration.count_nanoseconds() == 0);

    EXPECT_TRUE(duration.is_zero());
    EXPECT_FALSE(duration.is_positive());
    EXPECT_FALSE(duration.is_negative());
  }

  TEST(DurationTest, CreatesNanoseconds)
  {
    constexpr auto duration =
        vix::Duration::nanoseconds(42);

    static_assert(duration.count_nanoseconds() == 42);

    EXPECT_EQ(duration.count_nanoseconds(), 42);
  }

  TEST(DurationTest, CreatesMicroseconds)
  {
    constexpr auto duration =
        vix::Duration::microseconds(42);

    static_assert(duration.count_microseconds() == 42);
    static_assert(duration.count_nanoseconds() == 42'000);

    EXPECT_EQ(duration.count_microseconds(), 42);
    EXPECT_EQ(duration.count_nanoseconds(), 42'000);
  }

  TEST(DurationTest, CreatesMilliseconds)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(42);

    static_assert(duration.count_milliseconds() == 42);
    static_assert(duration.count_microseconds() == 42'000);

    EXPECT_EQ(duration.count_milliseconds(), 42);
    EXPECT_EQ(duration.count_microseconds(), 42'000);
  }

  TEST(DurationTest, CreatesSeconds)
  {
    constexpr auto duration =
        vix::Duration::seconds(42);

    static_assert(duration.count_seconds() == 42);
    static_assert(duration.count_milliseconds() == 42'000);

    EXPECT_EQ(duration.count_seconds(), 42);
    EXPECT_EQ(duration.count_milliseconds(), 42'000);
  }

  TEST(DurationTest, CreatesMinutes)
  {
    constexpr auto duration =
        vix::Duration::minutes(2);

    static_assert(duration.count_minutes() == 2);
    static_assert(duration.count_seconds() == 120);

    EXPECT_EQ(duration.count_minutes(), 2);
    EXPECT_EQ(duration.count_seconds(), 120);
  }

  TEST(DurationTest, CreatesHours)
  {
    constexpr auto duration =
        vix::Duration::hours(2);

    static_assert(duration.count_hours() == 2);
    static_assert(duration.count_minutes() == 120);

    EXPECT_EQ(duration.count_hours(), 2);
    EXPECT_EQ(duration.count_minutes(), 120);
  }

  TEST(DurationTest, ConstructsFromChronoDuration)
  {
    constexpr vix::Duration duration{
        std::chrono::milliseconds{250}};

    static_assert(duration.count_milliseconds() == 250);

    EXPECT_EQ(duration.count_milliseconds(), 250);
  }

  TEST(DurationTest, ConvertsToChronoDuration)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(1500);

    constexpr auto milliseconds =
        duration.as<std::chrono::milliseconds>();

    constexpr auto seconds =
        duration.as<std::chrono::seconds>();

    static_assert(milliseconds.count() == 1500);
    static_assert(seconds.count() == 1);

    EXPECT_EQ(milliseconds.count(), 1500);
    EXPECT_EQ(seconds.count(), 1);
  }

  TEST(DurationTest, ExposesNativeChronoRepresentation)
  {
    constexpr auto duration =
        vix::Duration::microseconds(5);

    constexpr auto native = duration.chrono();

    static_assert(
        native ==
        std::chrono::nanoseconds{5'000});

    EXPECT_EQ(
        native,
        std::chrono::nanoseconds{5'000});
  }

  TEST(DurationTest, CoarserConversionsTruncateRemainder)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(1500);

    static_assert(duration.count_seconds() == 1);

    EXPECT_EQ(duration.count_seconds(), 1);
    EXPECT_EQ(duration.count_milliseconds(), 1500);
  }

  TEST(DurationTest, ZeroFactoryProducesZero)
  {
    constexpr auto duration =
        vix::Duration::zero();

    static_assert(duration.is_zero());

    EXPECT_TRUE(duration.is_zero());
    EXPECT_EQ(duration.count_nanoseconds(), 0);
  }

  TEST(DurationTest, DetectsPositiveDuration)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(1);

    static_assert(duration.is_positive());

    EXPECT_TRUE(duration.is_positive());
    EXPECT_FALSE(duration.is_zero());
    EXPECT_FALSE(duration.is_negative());
  }

  TEST(DurationTest, DetectsNegativeDuration)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(-1);

    static_assert(duration.is_negative());

    EXPECT_TRUE(duration.is_negative());
    EXPECT_FALSE(duration.is_zero());
    EXPECT_FALSE(duration.is_positive());
  }

  TEST(DurationTest, SupportsUnaryPlus)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(42);

    constexpr auto result = +duration;

    static_assert(
        result ==
        vix::Duration::milliseconds(42));

    EXPECT_EQ(
        result,
        vix::Duration::milliseconds(42));
  }

  TEST(DurationTest, SupportsUnaryMinus)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(42);

    constexpr auto result = -duration;

    static_assert(
        result ==
        vix::Duration::milliseconds(-42));

    EXPECT_EQ(
        result,
        vix::Duration::milliseconds(-42));
  }

  TEST(DurationTest, AddsDurations)
  {
    constexpr auto lhs =
        vix::Duration::seconds(1);

    constexpr auto rhs =
        vix::Duration::milliseconds(500);

    constexpr auto result = lhs + rhs;

    static_assert(
        result ==
        vix::Duration::milliseconds(1500));

    EXPECT_EQ(
        result,
        vix::Duration::milliseconds(1500));
  }

  TEST(DurationTest, AddsDurationInPlace)
  {
    auto duration =
        vix::Duration::seconds(1);

    duration +=
        vix::Duration::milliseconds(500);

    EXPECT_EQ(
        duration,
        vix::Duration::milliseconds(1500));
  }

  TEST(DurationTest, SubtractsDurations)
  {
    constexpr auto lhs =
        vix::Duration::seconds(2);

    constexpr auto rhs =
        vix::Duration::milliseconds(500);

    constexpr auto result = lhs - rhs;

    static_assert(
        result ==
        vix::Duration::milliseconds(1500));

    EXPECT_EQ(
        result,
        vix::Duration::milliseconds(1500));
  }

  TEST(DurationTest, SubtractsDurationInPlace)
  {
    auto duration =
        vix::Duration::seconds(2);

    duration -=
        vix::Duration::milliseconds(500);

    EXPECT_EQ(
        duration,
        vix::Duration::milliseconds(1500));
  }

  TEST(DurationTest, MultipliesDuration)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(250);

    constexpr auto result = duration * 4;

    static_assert(
        result ==
        vix::Duration::seconds(1));

    EXPECT_EQ(
        result,
        vix::Duration::seconds(1));
  }

  TEST(DurationTest, MultipliesScalarByDuration)
  {
    constexpr auto duration =
        vix::Duration::milliseconds(250);

    constexpr auto result = 4 * duration;

    static_assert(
        result ==
        vix::Duration::seconds(1));

    EXPECT_EQ(
        result,
        vix::Duration::seconds(1));
  }

  TEST(DurationTest, MultipliesDurationInPlace)
  {
    auto duration =
        vix::Duration::milliseconds(250);

    duration *= 4;

    EXPECT_EQ(
        duration,
        vix::Duration::seconds(1));
  }

  TEST(DurationTest, DividesDurationByScalar)
  {
    constexpr auto duration =
        vix::Duration::seconds(2);

    constexpr auto result = duration / 4;

    static_assert(
        result ==
        vix::Duration::milliseconds(500));

    EXPECT_EQ(
        result,
        vix::Duration::milliseconds(500));
  }

  TEST(DurationTest, DividesDurationInPlace)
  {
    auto duration =
        vix::Duration::seconds(2);

    duration /= 4;

    EXPECT_EQ(
        duration,
        vix::Duration::milliseconds(500));
  }

  TEST(DurationTest, ComputesIntegralDurationRatio)
  {
    constexpr auto lhs =
        vix::Duration::seconds(2);

    constexpr auto rhs =
        vix::Duration::milliseconds(500);

    constexpr auto ratio = lhs / rhs;

    static_assert(ratio == 4);

    EXPECT_EQ(ratio, 4);
  }

  TEST(DurationTest, ComputesRemainder)
  {
    constexpr auto lhs =
        vix::Duration::milliseconds(1250);

    constexpr auto rhs =
        vix::Duration::milliseconds(500);

    constexpr auto remainder = lhs % rhs;

    static_assert(
        remainder ==
        vix::Duration::milliseconds(250));

    EXPECT_EQ(
        remainder,
        vix::Duration::milliseconds(250));
  }

  TEST(DurationTest, ComparesEqualDurationsAcrossUnits)
  {
    constexpr auto lhs =
        vix::Duration::seconds(1);

    constexpr auto rhs =
        vix::Duration::milliseconds(1000);

    static_assert(lhs == rhs);

    EXPECT_EQ(lhs, rhs);
  }

  TEST(DurationTest, OrdersDurations)
  {
    constexpr auto short_duration =
        vix::Duration::milliseconds(500);

    constexpr auto long_duration =
        vix::Duration::seconds(1);

    static_assert(short_duration < long_duration);
    static_assert(long_duration > short_duration);

    EXPECT_LT(short_duration, long_duration);
    EXPECT_GT(long_duration, short_duration);
  }

  TEST(DurationTest, OrdersNegativeDurations)
  {
    constexpr auto lhs =
        vix::Duration::seconds(-2);

    constexpr auto rhs =
        vix::Duration::seconds(-1);

    static_assert(lhs < rhs);

    EXPECT_LT(lhs, rhs);
  }

  TEST(DurationTest, ExposesMinimumValue)
  {
    constexpr auto minimum =
        vix::Duration::min();

    static_assert(
        minimum.count_nanoseconds() ==
        vix::Duration::chrono_type::min().count());

    EXPECT_EQ(
        minimum.count_nanoseconds(),
        vix::Duration::chrono_type::min().count());
  }

  TEST(DurationTest, ExposesMaximumValue)
  {
    constexpr auto maximum =
        vix::Duration::max();

    static_assert(
        maximum.count_nanoseconds() ==
        vix::Duration::chrono_type::max().count());

    EXPECT_EQ(
        maximum.count_nanoseconds(),
        vix::Duration::chrono_type::max().count());
  }

} // namespace
