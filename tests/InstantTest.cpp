/**
 *
 *  @file InstantTest.cpp
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
#include <vix/Instant.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <type_traits>

namespace
{
  static_assert(std::is_trivially_copy_constructible_v<vix::Instant>);
  static_assert(std::is_trivially_copy_assignable_v<vix::Instant>);
  static_assert(std::is_nothrow_move_constructible_v<vix::Instant>);
  static_assert(std::is_nothrow_move_assignable_v<vix::Instant>);

  TEST(InstantTest, DefaultConstructionUsesClockEpoch)
  {
    constexpr vix::Instant instant;

    constexpr auto native = instant.chrono();

    static_assert(
        native ==
        vix::Instant::chrono_type{});

    EXPECT_EQ(
        native,
        vix::Instant::chrono_type{});
  }

  TEST(InstantTest, CreatesFromNativeChronoTimePoint)
  {
    constexpr auto native =
        vix::Instant::chrono_type{
            std::chrono::seconds{42}};

    constexpr auto instant =
        vix::Instant::from_chrono(native);

    static_assert(instant.chrono() == native);

    EXPECT_EQ(instant.chrono(), native);
  }

  TEST(InstantTest, ExposesNativeChronoRepresentation)
  {
    constexpr auto native =
        vix::Instant::chrono_type{
            std::chrono::milliseconds{250}};

    constexpr auto instant =
        vix::Instant::from_chrono(native);

    constexpr auto result = instant.chrono();

    static_assert(result == native);

    EXPECT_EQ(result, native);
  }

  TEST(InstantTest, NowProducesCurrentMonotonicTime)
  {
    const auto before =
        vix::Instant::clock_type::now();

    const auto instant =
        vix::Instant::now();

    const auto after =
        vix::Instant::clock_type::now();

    EXPECT_GE(instant.chrono(), before);
    EXPECT_LE(instant.chrono(), after);
  }

  TEST(InstantTest, ComputesDurationSinceEarlierInstant)
  {
    constexpr auto earlier =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{10}});

    constexpr auto later =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{12}});

    constexpr auto elapsed =
        later.since(earlier);

    static_assert(
        elapsed ==
        vix::Duration::seconds(2));

    EXPECT_EQ(
        elapsed,
        vix::Duration::seconds(2));
  }

  TEST(InstantTest, SinceCanProduceNegativeDuration)
  {
    constexpr auto earlier =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{10}});

    constexpr auto later =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{12}});

    constexpr auto duration =
        earlier.since(later);

    static_assert(
        duration ==
        vix::Duration::seconds(-2));

    EXPECT_EQ(
        duration,
        vix::Duration::seconds(-2));
  }

  TEST(InstantTest, ComputesDurationUntilLaterInstant)
  {
    constexpr auto earlier =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{5}});

    constexpr auto later =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{8}});

    constexpr auto duration =
        earlier.until(later);

    static_assert(
        duration ==
        vix::Duration::seconds(3));

    EXPECT_EQ(
        duration,
        vix::Duration::seconds(3));
  }

  TEST(InstantTest, UntilCanProduceNegativeDuration)
  {
    constexpr auto earlier =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{5}});

    constexpr auto later =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{8}});

    constexpr auto duration =
        later.until(earlier);

    static_assert(
        duration ==
        vix::Duration::seconds(-3));

    EXPECT_EQ(
        duration,
        vix::Duration::seconds(-3));
  }

  TEST(InstantTest, EqualInstantsHaveZeroDistance)
  {
    constexpr auto instant =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{42}});

    constexpr auto duration =
        instant.since(instant);

    static_assert(duration.is_zero());

    EXPECT_TRUE(duration.is_zero());
  }

  TEST(InstantTest, AddsDuration)
  {
    constexpr auto instant =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{10}});

    constexpr auto result =
        instant + vix::Duration::seconds(5);

    constexpr auto expected =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{15}});

    static_assert(result == expected);

    EXPECT_EQ(result, expected);
  }

  TEST(InstantTest, AddsDurationWithDurationOnLeft)
  {
    constexpr auto instant =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{10}});

    constexpr auto result =
        vix::Duration::seconds(5) + instant;

    constexpr auto expected =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{15}});

    static_assert(result == expected);

    EXPECT_EQ(result, expected);
  }

  TEST(InstantTest, AddsDurationInPlace)
  {
    auto instant =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{10}});

    instant += vix::Duration::seconds(5);

    const auto expected =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{15}});

    EXPECT_EQ(instant, expected);
  }

  TEST(InstantTest, SubtractsDuration)
  {
    constexpr auto instant =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{10}});

    constexpr auto result =
        instant - vix::Duration::seconds(4);

    constexpr auto expected =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{6}});

    static_assert(result == expected);

    EXPECT_EQ(result, expected);
  }

  TEST(InstantTest, SubtractsDurationInPlace)
  {
    auto instant =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{10}});

    instant -= vix::Duration::seconds(4);

    const auto expected =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{6}});

    EXPECT_EQ(instant, expected);
  }

  TEST(InstantTest, SubtractingInstantsProducesDuration)
  {
    constexpr auto lhs =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{20}});

    constexpr auto rhs =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{12}});

    constexpr auto duration =
        lhs - rhs;

    static_assert(
        duration ==
        vix::Duration::seconds(8));

    EXPECT_EQ(
        duration,
        vix::Duration::seconds(8));
  }

  TEST(InstantTest, SubtractingInstantsCanProduceNegativeDuration)
  {
    constexpr auto lhs =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{12}});

    constexpr auto rhs =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{20}});

    constexpr auto duration =
        lhs - rhs;

    static_assert(
        duration ==
        vix::Duration::seconds(-8));

    EXPECT_EQ(
        duration,
        vix::Duration::seconds(-8));
  }

  TEST(InstantTest, ComparesEqualInstants)
  {
    constexpr auto lhs =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{42}});

    constexpr auto rhs =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{42}});

    static_assert(lhs == rhs);

    EXPECT_EQ(lhs, rhs);
  }

  TEST(InstantTest, OrdersInstants)
  {
    constexpr auto earlier =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{1}});

    constexpr auto later =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::seconds{2}});

    static_assert(earlier < later);
    static_assert(later > earlier);

    EXPECT_LT(earlier, later);
    EXPECT_GT(later, earlier);
  }

  TEST(InstantTest, ElapsedMeasuresTimeSinceInstant)
  {
    const auto start =
        vix::Instant::now();

    const auto elapsed =
        start.elapsed();

    EXPECT_GE(
        elapsed,
        vix::Duration::zero());
  }

  TEST(InstantTest, DetectsPastInstant)
  {
    const auto instant =
        vix::Instant::now() -
        vix::Duration::seconds(1);

    EXPECT_TRUE(instant.is_past());
    EXPECT_FALSE(instant.is_future());
  }

  TEST(InstantTest, DetectsFutureInstant)
  {
    const auto instant =
        vix::Instant::now() +
        vix::Duration::seconds(1);

    EXPECT_TRUE(instant.is_future());
    EXPECT_FALSE(instant.is_past());
  }

  TEST(InstantTest, PreservesSubsecondPrecision)
  {
    constexpr auto first =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::nanoseconds{100}});

    constexpr auto second =
        vix::Instant::from_chrono(
            vix::Instant::chrono_type{
                std::chrono::nanoseconds{142}});

    constexpr auto duration =
        second - first;

    static_assert(
        duration ==
        vix::Duration::nanoseconds(42));

    EXPECT_EQ(
        duration,
        vix::Duration::nanoseconds(42));
  }

} // namespace
