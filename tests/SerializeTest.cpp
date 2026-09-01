/**
 *
 *  @file SerializeTest.cpp
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

#include <vix/Serialize.hpp>

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

namespace
{
  static_assert(vix::Serializable<int>);
  static_assert(vix::Serializable<long>);
  static_assert(vix::Serializable<unsigned int>);
  static_assert(vix::Serializable<float>);
  static_assert(vix::Serializable<double>);
  static_assert(vix::Serializable<bool>);
  static_assert(vix::Serializable<std::string>);
  static_assert(vix::Serializable<std::string_view>);

  static_assert(!vix::Serializable<void *>);

  static_assert(std::same_as<
                decltype(vix::serialize(42)),
                vix::Result<std::string, vix::SerializeError>>);

  static_assert(std::same_as<
                decltype(vix::serialize(42.5)),
                vix::Result<std::string, vix::SerializeError>>);

  static_assert(std::same_as<
                decltype(vix::serialize(true)),
                vix::Result<std::string, vix::SerializeError>>);

  static_assert(std::same_as<
                decltype(vix::serialize(std::string{"Vix.cpp"})),
                vix::Result<std::string, vix::SerializeError>>);

  static_assert(std::same_as<
                decltype(vix::serialize(std::string_view{"Vix.cpp"})),
                vix::Result<std::string, vix::SerializeError>>);

  TEST(SerializeTest, SerializesPositiveInteger)
  {
    auto result =
        vix::serialize(42);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "42");
  }

  TEST(SerializeTest, SerializesNegativeInteger)
  {
    auto result =
        vix::serialize(-42);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "-42");
  }

  TEST(SerializeTest, SerializesZeroInteger)
  {
    auto result =
        vix::serialize(0);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "0");
  }

  TEST(SerializeTest, SerializesUnsignedInteger)
  {
    auto result =
        vix::serialize(42U);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "42");
  }

  TEST(SerializeTest, SerializesLongInteger)
  {
    auto result =
        vix::serialize(123456L);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "123456");
  }

  TEST(SerializeTest, SerializesIntegerMaximum)
  {
    const auto value =
        std::numeric_limits<int>::max();

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        std::to_string(value));
  }

  TEST(SerializeTest, SerializesIntegerMinimum)
  {
    const auto value =
        std::numeric_limits<int>::min();

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        std::to_string(value));
  }

  TEST(SerializeTest, SerializesUnsignedMaximum)
  {
    const auto value =
        std::numeric_limits<unsigned int>::max();

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        std::to_string(value));
  }

  TEST(SerializeTest, SerializesFloatingPointValue)
  {
    auto result =
        vix::serialize(42.5);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "42.5");
  }

  TEST(SerializeTest, SerializesNegativeFloatingPointValue)
  {
    auto result =
        vix::serialize(-0.125);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "-0.125");
  }

  TEST(SerializeTest, SerializesFloatingPointZero)
  {
    auto result =
        vix::serialize(0.0);

    ASSERT_TRUE(result);

    auto parsed =
        vix::serialize(0.0);

    ASSERT_TRUE(parsed);
    EXPECT_FALSE(parsed.value().empty());
  }

  TEST(SerializeTest, SerializesFloat)
  {
    auto result =
        vix::serialize(42.5F);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "42.5");
  }

  TEST(SerializeTest, SerializesDouble)
  {
    auto result =
        vix::serialize(42.5);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "42.5");
  }

  TEST(SerializeTest, SerializesFiniteFloatingPointMaximum)
  {
    const auto value =
        std::numeric_limits<double>::max();

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_FALSE(result.value().empty());
  }

  TEST(SerializeTest, SerializesFiniteFloatingPointLowest)
  {
    const auto value =
        std::numeric_limits<double>::lowest();

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_FALSE(result.value().empty());
  }

  TEST(SerializeTest, SerializesTrue)
  {
    auto result =
        vix::serialize(true);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "true");
  }

  TEST(SerializeTest, SerializesFalse)
  {
    auto result =
        vix::serialize(false);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "false");
  }

  TEST(SerializeTest, SerializesString)
  {
    const std::string value{
        "Vix.cpp Standard"};

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        "Vix.cpp Standard");
  }

  TEST(SerializeTest, SerializesEmptyString)
  {
    const std::string value;

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_TRUE(result.value().empty());
  }

  TEST(SerializeTest, SerializesStringWithoutModification)
  {
    const std::string value{
        "  Vix.cpp\nStandard\t42  "};

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), value);
  }

  TEST(SerializeTest, SerializesStringView)
  {
    constexpr std::string_view value{
        "Vix.cpp"};

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), value);
  }

  TEST(SerializeTest, SerializesEmptyStringView)
  {
    constexpr std::string_view value;

    auto result =
        vix::serialize(value);

    ASSERT_TRUE(result);
    EXPECT_TRUE(result.value().empty());
  }

  TEST(SerializeTest, StringViewSerializationCopiesContent)
  {
    std::string source{
        "Vix.cpp"};

    std::string_view view{
        source};

    auto result =
        vix::serialize(view);

    ASSERT_TRUE(result);

    source.assign(
        "changed");

    EXPECT_EQ(
        result.value(),
        "Vix.cpp");
  }

  TEST(SerializeTest, ArithmeticSerializationIsLocaleIndependent)
  {
    auto integer =
        vix::serialize(1234);

    auto floating =
        vix::serialize(12.5);

    ASSERT_TRUE(integer);
    ASSERT_TRUE(floating);

    EXPECT_EQ(integer.value(), "1234");
    EXPECT_EQ(floating.value(), "12.5");
  }

  TEST(SerializeTest, ProducesOwnedString)
  {
    std::string source{
        "Vix.cpp"};

    auto result =
        vix::serialize(source);

    ASSERT_TRUE(result);

    source.clear();

    EXPECT_EQ(
        result.value(),
        "Vix.cpp");
  }

  TEST(SerializeTest, ErrorEqualityComparesCode)
  {
    constexpr vix::SerializeError first{
        .code =
            vix::SerializeErrorCode::conversion_failed};

    constexpr vix::SerializeError second{
        .code =
            vix::SerializeErrorCode::conversion_failed};

    constexpr vix::SerializeError different{
        .code =
            vix::SerializeErrorCode::value_too_large};

    static_assert(first == second);
    static_assert(first != different);

    EXPECT_EQ(first, second);
    EXPECT_NE(first, different);
  }

} // namespace
