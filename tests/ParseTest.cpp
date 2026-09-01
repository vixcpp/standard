/**
 *
 *  @file ParseTest.cpp
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

#include <vix/Parse.hpp>

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

namespace
{
  static_assert(vix::Parsable<int>);
  static_assert(vix::Parsable<long>);
  static_assert(vix::Parsable<unsigned int>);
  static_assert(vix::Parsable<float>);
  static_assert(vix::Parsable<double>);
  static_assert(vix::Parsable<bool>);
  static_assert(vix::Parsable<std::string>);

  static_assert(!vix::Parsable<std::string_view>);
  static_assert(!vix::Parsable<void *>);

  static_assert(std::same_as<
                decltype(vix::parse<int>("42")),
                vix::Result<int, vix::ParseError>>);

  static_assert(std::same_as<
                decltype(vix::parse<double>("42.5")),
                vix::Result<double, vix::ParseError>>);

  static_assert(std::same_as<
                decltype(vix::parse<bool>("true")),
                vix::Result<bool, vix::ParseError>>);

  static_assert(std::same_as<
                decltype(vix::parse<std::string>("Vix.cpp")),
                vix::Result<std::string, vix::ParseError>>);

  TEST(ParseTest, ParsesPositiveInteger)
  {
    auto result =
        vix::parse<int>("42");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
  }

  TEST(ParseTest, ParsesNegativeInteger)
  {
    auto result =
        vix::parse<int>("-42");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), -42);
  }

  TEST(ParseTest, ParsesZeroInteger)
  {
    auto result =
        vix::parse<int>("0");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 0);
  }

  TEST(ParseTest, ParsesUnsignedInteger)
  {
    auto result =
        vix::parse<unsigned int>("42");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 42U);
  }

  TEST(ParseTest, ParsesLongInteger)
  {
    auto result =
        vix::parse<long>("123456");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 123456L);
  }

  TEST(ParseTest, ParsesIntegerMaximum)
  {
    const auto text =
        std::to_string(std::numeric_limits<int>::max());

    auto result =
        vix::parse<int>(text);

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        std::numeric_limits<int>::max());
  }

  TEST(ParseTest, ParsesIntegerMinimum)
  {
    const auto text =
        std::to_string(std::numeric_limits<int>::min());

    auto result =
        vix::parse<int>(text);

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        std::numeric_limits<int>::min());
  }

  TEST(ParseTest, RejectsEmptyIntegerInput)
  {
    auto result =
        vix::parse<int>("");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::empty_input);

    EXPECT_EQ(
        result.error().position,
        0U);
  }

  TEST(ParseTest, RejectsInvalidInteger)
  {
    auto result =
        vix::parse<int>("abc");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);

    EXPECT_EQ(
        result.error().position,
        0U);
  }

  TEST(ParseTest, RejectsIntegerWithTrailingCharacters)
  {
    auto result =
        vix::parse<int>("42abc");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::trailing_characters);

    EXPECT_EQ(
        result.error().position,
        2U);
  }

  TEST(ParseTest, RejectsIntegerWithLeadingWhitespace)
  {
    auto result =
        vix::parse<int>(" 42");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);

    EXPECT_EQ(
        result.error().position,
        0U);
  }

  TEST(ParseTest, RejectsIntegerWithTrailingWhitespace)
  {
    auto result =
        vix::parse<int>("42 ");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::trailing_characters);

    EXPECT_EQ(
        result.error().position,
        2U);
  }

  TEST(ParseTest, RejectsIntegerOutsideRange)
  {
    auto result =
        vix::parse<int>(
            "999999999999999999999999999999999999");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::out_of_range);
  }

  TEST(ParseTest, RejectsNegativeUnsignedInteger)
  {
    auto result =
        vix::parse<unsigned int>("-1");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);
  }

  TEST(ParseTest, ParsesFloat)
  {
    auto result =
        vix::parse<float>("42.5");

    ASSERT_TRUE(result);
    EXPECT_FLOAT_EQ(result.value(), 42.5F);
  }

  TEST(ParseTest, ParsesDouble)
  {
    auto result =
        vix::parse<double>("42.5");

    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(result.value(), 42.5);
  }

  TEST(ParseTest, ParsesNegativeFloatingPointValue)
  {
    auto result =
        vix::parse<double>("-0.125");

    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(result.value(), -0.125);
  }

  TEST(ParseTest, ParsesFloatingPointExponent)
  {
    auto result =
        vix::parse<double>("1.25e3");

    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(result.value(), 1250.0);
  }

  TEST(ParseTest, RejectsEmptyFloatingPointInput)
  {
    auto result =
        vix::parse<double>("");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::empty_input);
  }

  TEST(ParseTest, RejectsInvalidFloatingPointValue)
  {
    auto result =
        vix::parse<double>("abc");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);

    EXPECT_EQ(
        result.error().position,
        0U);
  }

  TEST(ParseTest, RejectsFloatingPointTrailingCharacters)
  {
    auto result =
        vix::parse<double>("42.5ms");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::trailing_characters);

    EXPECT_EQ(
        result.error().position,
        4U);
  }

  TEST(ParseTest, RejectsFloatingPointLeadingWhitespace)
  {
    auto result =
        vix::parse<double>(" 42.5");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);
  }

  TEST(ParseTest, RejectsFloatingPointTrailingWhitespace)
  {
    auto result =
        vix::parse<double>("42.5 ");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::trailing_characters);

    EXPECT_EQ(
        result.error().position,
        4U);
  }

  TEST(ParseTest, RejectsFloatingPointOutsideRange)
  {
    auto result =
        vix::parse<double>("1e9999");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::out_of_range);
  }

  TEST(ParseTest, ParsesTrue)
  {
    auto result =
        vix::parse<bool>("true");

    ASSERT_TRUE(result);
    EXPECT_TRUE(result.value());
  }

  TEST(ParseTest, ParsesFalse)
  {
    auto result =
        vix::parse<bool>("false");

    ASSERT_TRUE(result);
    EXPECT_FALSE(result.value());
  }

  TEST(ParseTest, RejectsEmptyBooleanInput)
  {
    auto result =
        vix::parse<bool>("");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::empty_input);
  }

  TEST(ParseTest, BooleanParsingIsCaseSensitive)
  {
    auto result =
        vix::parse<bool>("True");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);
  }

  TEST(ParseTest, RejectsNumericBoolean)
  {
    auto result =
        vix::parse<bool>("1");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);
  }

  TEST(ParseTest, RejectsBooleanWithWhitespace)
  {
    auto result =
        vix::parse<bool>(" true ");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::invalid_value);
  }

  TEST(ParseTest, ParsesStringWithoutModification)
  {
    auto result =
        vix::parse<std::string>("Vix.cpp");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "Vix.cpp");
  }

  TEST(ParseTest, ParsesEmptyString)
  {
    auto result =
        vix::parse<std::string>("");

    ASSERT_TRUE(result);
    EXPECT_TRUE(result.value().empty());
  }

  TEST(ParseTest, StringParsingPreservesWhitespace)
  {
    auto result =
        vix::parse<std::string>("  Vix.cpp  ");

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        "  Vix.cpp  ");
  }

  TEST(ParseTest, StringParsingPreservesEmbeddedCharacters)
  {
    const std::string_view text{
        "Vix.cpp\nStandard\t42"};

    auto result =
        vix::parse<std::string>(text);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), text);
  }

  TEST(ParseTest, ErrorEqualityComparesCodeAndPosition)
  {
    constexpr vix::ParseError first{
        .code = vix::ParseErrorCode::trailing_characters,
        .position = 4};

    constexpr vix::ParseError second{
        .code = vix::ParseErrorCode::trailing_characters,
        .position = 4};

    constexpr vix::ParseError different{
        .code = vix::ParseErrorCode::invalid_value,
        .position = 0};

    static_assert(first == second);
    static_assert(first != different);

    EXPECT_EQ(first, second);
    EXPECT_NE(first, different);
  }

  TEST(ParseTest, TrailingPositionIdentifiesFirstUnconsumedByte)
  {
    auto result =
        vix::parse<int>("123xyz");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error().code,
        vix::ParseErrorCode::trailing_characters);

    EXPECT_EQ(
        result.error().position,
        3U);
  }

} // namespace
