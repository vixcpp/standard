/**
 *
 *  @file ResultTest.cpp
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

#include <vix/Result.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace
{
  enum class TestError
  {
    invalid,
    unavailable
  };

  using IntResult = vix::Result<int, TestError>;
  using VoidResult = vix::Result<void, TestError>;

  static_assert(std::same_as<IntResult::value_type, int>);
  static_assert(std::same_as<IntResult::error_type, TestError>);
  static_assert(std::same_as<VoidResult::value_type, void>);
  static_assert(std::same_as<VoidResult::error_type, TestError>);

  static_assert(!std::is_copy_constructible_v<
                vix::Result<std::unique_ptr<int>, TestError>>);

  static_assert(std::is_move_constructible_v<
                vix::Result<std::unique_ptr<int>, TestError>>);

  TEST(ResultTest, CreatesSuccessfulResult)
  {
    auto result = IntResult::success(42);

    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.has_error());
    EXPECT_TRUE(static_cast<bool>(result));
    EXPECT_EQ(result.value(), 42);
  }

  TEST(ResultTest, CreatesFailedResult)
  {
    auto result = IntResult::failure(TestError::invalid);

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(result.has_error());
    EXPECT_FALSE(static_cast<bool>(result));
    EXPECT_EQ(result.error(), TestError::invalid);
  }

  TEST(ResultTest, ConstructsValueFromCompatibleType)
  {
    auto result =
        vix::Result<std::string, TestError>::success("Vix.cpp");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), "Vix.cpp");
  }

  TEST(ResultTest, ConstructsErrorFromCompatibleType)
  {
    using StringErrorResult = vix::Result<int, std::string>;

    auto result =
        StringErrorResult::failure("operation failed");

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), "operation failed");
  }

  TEST(ResultTest, ValueThrowsWhenResultContainsError)
  {
    auto result = IntResult::failure(TestError::invalid);

    EXPECT_THROW(
        static_cast<void>(result.value()),
        std::bad_variant_access);
  }

  TEST(ResultTest, ErrorThrowsWhenResultContainsValue)
  {
    auto result = IntResult::success(42);

    EXPECT_THROW(
        static_cast<void>(result.error()),
        std::bad_variant_access);
  }

  TEST(ResultTest, ValueIfReturnsPointerForSuccess)
  {
    auto result = IntResult::success(42);

    int *value = result.value_if();

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 42);
    EXPECT_EQ(result.error_if(), nullptr);
  }

  TEST(ResultTest, ErrorIfReturnsPointerForFailure)
  {
    auto result = IntResult::failure(TestError::unavailable);

    TestError *error = result.error_if();

    ASSERT_NE(error, nullptr);
    EXPECT_EQ(*error, TestError::unavailable);
    EXPECT_EQ(result.value_if(), nullptr);
  }

  TEST(ResultTest, ConstValueIfPreservesConstness)
  {
    const auto result = IntResult::success(42);

    static_assert(std::same_as<
                  decltype(result.value_if()),
                  const int *>);

    ASSERT_NE(result.value_if(), nullptr);
    EXPECT_EQ(*result.value_if(), 42);
  }

  TEST(ResultTest, ConstErrorIfPreservesConstness)
  {
    const auto result =
        IntResult::failure(TestError::invalid);

    static_assert(std::same_as<
                  decltype(result.error_if()),
                  const TestError *>);

    ASSERT_NE(result.error_if(), nullptr);
    EXPECT_EQ(*result.error_if(), TestError::invalid);
  }

  TEST(ResultTest, ValueOrReturnsStoredValue)
  {
    const auto result = IntResult::success(42);

    EXPECT_EQ(result.value_or(7), 42);
  }

  TEST(ResultTest, ValueOrReturnsFallbackForFailure)
  {
    const auto result =
        IntResult::failure(TestError::invalid);

    EXPECT_EQ(result.value_or(7), 7);
  }

  TEST(ResultTest, ValueOrMovesStoredValue)
  {
    using PointerResult =
        vix::Result<std::unique_ptr<int>, TestError>;

    auto result =
        PointerResult::success(std::make_unique<int>(42));

    auto value = std::move(result).value_or(
        std::make_unique<int>(7));

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 42);
  }

  TEST(ResultTest, ValueOrMovesFallback)
  {
    using PointerResult =
        vix::Result<std::unique_ptr<int>, TestError>;

    auto result =
        PointerResult::failure(TestError::invalid);

    auto value = std::move(result).value_or(
        std::make_unique<int>(7));

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 7);
  }

  TEST(ResultTest, MapTransformsSuccessfulValue)
  {
    auto result = IntResult::success(21);

    auto mapped = result.map(
        [](int value)
        {
          return value * 2;
        });

    static_assert(std::same_as<
                  decltype(mapped),
                  vix::Result<int, TestError>>);

    ASSERT_TRUE(mapped);
    EXPECT_EQ(mapped.value(), 42);
  }

  TEST(ResultTest, MapPreservesFailure)
  {
    auto result =
        IntResult::failure(TestError::unavailable);

    bool invoked = false;

    auto mapped = result.map(
        [&invoked](int)
        {
          invoked = true;
          return 42;
        });

    EXPECT_FALSE(invoked);
    ASSERT_FALSE(mapped);
    EXPECT_EQ(mapped.error(), TestError::unavailable);
  }

  TEST(ResultTest, MapCanChangeValueType)
  {
    auto result = IntResult::success(42);

    auto mapped = result.map(
        [](int value)
        {
          return std::to_string(value);
        });

    static_assert(std::same_as<
                  decltype(mapped),
                  vix::Result<std::string, TestError>>);

    ASSERT_TRUE(mapped);
    EXPECT_EQ(mapped.value(), "42");
  }

  TEST(ResultTest, MapCanProduceVoidResult)
  {
    auto result = IntResult::success(42);

    int observed = 0;

    auto mapped = result.map(
        [&observed](int value)
        {
          observed = value;
        });

    static_assert(std::same_as<
                  decltype(mapped),
                  vix::Result<void, TestError>>);

    EXPECT_TRUE(mapped);
    EXPECT_EQ(observed, 42);
  }

  TEST(ResultTest, RvalueMapSupportsMoveOnlyValue)
  {
    using PointerResult =
        vix::Result<std::unique_ptr<int>, TestError>;

    auto result =
        PointerResult::success(std::make_unique<int>(42));

    auto mapped = std::move(result).map(
        [](std::unique_ptr<int> value)
        {
          return *value;
        });

    ASSERT_TRUE(mapped);
    EXPECT_EQ(mapped.value(), 42);
  }

  TEST(ResultTest, AndThenChainsSuccessfulOperation)
  {
    auto result = IntResult::success(21);

    auto chained = result.and_then(
        [](int value) -> IntResult
        {
          return IntResult::success(value * 2);
        });

    ASSERT_TRUE(chained);
    EXPECT_EQ(chained.value(), 42);
  }

  TEST(ResultTest, AndThenPreservesExistingFailure)
  {
    auto result =
        IntResult::failure(TestError::invalid);

    bool invoked = false;

    auto chained = result.and_then(
        [&invoked](int) -> IntResult
        {
          invoked = true;
          return IntResult::success(42);
        });

    EXPECT_FALSE(invoked);
    ASSERT_FALSE(chained);
    EXPECT_EQ(chained.error(), TestError::invalid);
  }

  TEST(ResultTest, AndThenPropagatesNextFailure)
  {
    auto result = IntResult::success(42);

    auto chained = result.and_then(
        [](int) -> IntResult
        {
          return IntResult::failure(
              TestError::unavailable);
        });

    ASSERT_FALSE(chained);
    EXPECT_EQ(
        chained.error(),
        TestError::unavailable);
  }

  TEST(ResultTest, AndThenCanChangeValueType)
  {
    auto result = IntResult::success(42);

    auto chained = result.and_then(
        [](int value)
            -> vix::Result<std::string, TestError>
        {
          return vix::Result<
              std::string,
              TestError>::success(std::to_string(value));
        });

    ASSERT_TRUE(chained);
    EXPECT_EQ(chained.value(), "42");
  }

  TEST(ResultTest, RvalueAndThenSupportsMoveOnlyValue)
  {
    using PointerResult =
        vix::Result<std::unique_ptr<int>, TestError>;

    auto result =
        PointerResult::success(std::make_unique<int>(42));

    auto chained = std::move(result).and_then(
        [](std::unique_ptr<int> value) -> IntResult
        {
          return IntResult::success(*value);
        });

    ASSERT_TRUE(chained);
    EXPECT_EQ(chained.value(), 42);
  }

  TEST(ResultTest, VoidResultRepresentsSuccess)
  {
    auto result = VoidResult::success();

    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.has_error());
    EXPECT_TRUE(static_cast<bool>(result));
    EXPECT_NO_THROW(result.value());
  }

  TEST(ResultTest, VoidResultRepresentsFailure)
  {
    auto result =
        VoidResult::failure(TestError::invalid);

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(result.has_error());
    EXPECT_FALSE(static_cast<bool>(result));
    EXPECT_EQ(result.error(), TestError::invalid);
  }

  TEST(ResultTest, VoidValueThrowsWhenFailed)
  {
    auto result =
        VoidResult::failure(TestError::invalid);

    EXPECT_THROW(
        result.value(),
        std::bad_variant_access);
  }

  TEST(ResultTest, VoidErrorThrowsWhenSuccessful)
  {
    auto result = VoidResult::success();

    EXPECT_THROW(
        static_cast<void>(result.error()),
        std::bad_variant_access);
  }

  TEST(ResultTest, VoidErrorIfReflectsState)
  {
    auto success = VoidResult::success();
    auto failure =
        VoidResult::failure(TestError::unavailable);

    EXPECT_EQ(success.error_if(), nullptr);

    ASSERT_NE(failure.error_if(), nullptr);
    EXPECT_EQ(
        *failure.error_if(),
        TestError::unavailable);
  }

  TEST(ResultTest, VoidMapInvokesFunctionOnSuccess)
  {
    auto result = VoidResult::success();

    bool invoked = false;

    auto mapped = result.map(
        [&invoked]()
        {
          invoked = true;
          return 42;
        });

    ASSERT_TRUE(mapped);
    EXPECT_TRUE(invoked);
    EXPECT_EQ(mapped.value(), 42);
  }

  TEST(ResultTest, VoidMapPreservesFailure)
  {
    auto result =
        VoidResult::failure(TestError::invalid);

    bool invoked = false;

    auto mapped = result.map(
        [&invoked]()
        {
          invoked = true;
          return 42;
        });

    EXPECT_FALSE(invoked);
    ASSERT_FALSE(mapped);
    EXPECT_EQ(mapped.error(), TestError::invalid);
  }

  TEST(ResultTest, VoidMapCanRemainVoid)
  {
    auto result = VoidResult::success();

    bool invoked = false;

    auto mapped = result.map(
        [&invoked]()
        {
          invoked = true;
        });

    static_assert(std::same_as<
                  decltype(mapped),
                  VoidResult>);

    EXPECT_TRUE(mapped);
    EXPECT_TRUE(invoked);
  }

  TEST(ResultTest, VoidAndThenChainsSuccessfulOperation)
  {
    auto result = VoidResult::success();

    auto chained = result.and_then(
        []() -> IntResult
        {
          return IntResult::success(42);
        });

    ASSERT_TRUE(chained);
    EXPECT_EQ(chained.value(), 42);
  }

  TEST(ResultTest, VoidAndThenPreservesFailure)
  {
    auto result =
        VoidResult::failure(TestError::invalid);

    bool invoked = false;

    auto chained = result.and_then(
        [&invoked]() -> IntResult
        {
          invoked = true;
          return IntResult::success(42);
        });

    EXPECT_FALSE(invoked);
    ASSERT_FALSE(chained);
    EXPECT_EQ(chained.error(), TestError::invalid);
  }

  TEST(ResultTest, SameValueAndErrorTypesRemainUnambiguous)
  {
    using SameTypeResult =
        vix::Result<std::string, std::string>;

    auto success =
        SameTypeResult::success("value");

    auto failure =
        SameTypeResult::failure("error");

    ASSERT_TRUE(success);
    EXPECT_EQ(success.value(), "value");

    ASSERT_FALSE(failure);
    EXPECT_EQ(failure.error(), "error");
  }

} // namespace
