/**
 *
 *  @file ValidateTest.cpp
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

#include <vix/Validate.hpp>

#include <gtest/gtest.h>

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
  enum class ValidationError
  {
    invalid,
    out_of_range
  };

  struct PositiveValidator
  {
    [[nodiscard]] vix::Result<void, ValidationError> operator()(
        int value) const
    {
      if (value <= 0)
      {
        return vix::Result<
            void,
            ValidationError>::failure(ValidationError::invalid);
      }

      return vix::Result<
          void,
          ValidationError>::success();
    }
  };

  struct RangeValidator
  {
    int minimum;
    int maximum;

    [[nodiscard]] vix::Result<void, ValidationError> operator()(
        int value) const
    {
      if (value < minimum || value > maximum)
      {
        return vix::Result<
            void,
            ValidationError>::failure(ValidationError::out_of_range);
      }

      return vix::Result<
          void,
          ValidationError>::success();
    }
  };

  struct MutableValidator
  {
    int calls{0};

    [[nodiscard]] vix::Result<void, ValidationError> operator()(
        int)
    {
      ++calls;

      return vix::Result<
          void,
          ValidationError>::success();
    }
  };

  struct StringValidator
  {
    enum class Error
    {
      empty
    };

    [[nodiscard]] vix::Result<void, Error> operator()(
        const std::string &value) const
    {
      if (value.empty())
      {
        return vix::Result<
            void,
            Error>::failure(Error::empty);
      }

      return vix::Result<
          void,
          Error>::success();
    }
  };

  struct DetailedError
  {
    std::string message;
    int position;

    [[nodiscard]] bool operator==(
        const DetailedError &) const = default;
  };

  struct DetailedValidator
  {
    [[nodiscard]] vix::Result<void, DetailedError> operator()(
        const std::string &value) const
    {
      if (value == "valid")
      {
        return vix::Result<
            void,
            DetailedError>::success();
      }

      return vix::Result<
          void,
          DetailedError>::failure(DetailedError{
          .message = "invalid value",
          .position = 0});
    }
  };

  struct MultipleErrorValidator
  {
    using Error = std::vector<std::string>;

    [[nodiscard]] vix::Result<void, Error> operator()(
        int value) const
    {
      Error errors;

      if (value < 0)
      {
        errors.emplace_back(
            "value must not be negative");
      }

      if (value % 2 != 0)
      {
        errors.emplace_back(
            "value must be even");
      }

      if (!errors.empty())
      {
        return vix::Result<
            void,
            Error>::failure(std::move(errors));
      }

      return vix::Result<
          void,
          Error>::success();
    }
  };

  struct InvalidBoolValidator
  {
    [[nodiscard]] bool operator()(int) const
    {
      return true;
    }
  };

  struct InvalidValueResultValidator
  {
    [[nodiscard]] vix::Result<int, ValidationError> operator()(
        int) const
    {
      return vix::Result<
          int,
          ValidationError>::success(42);
    }
  };

  struct MoveOnlyValidator
  {
    explicit MoveOnlyValidator(int expected)
        : expected_(
              std::make_unique<int>(expected))
    {
    }

    MoveOnlyValidator(
        const MoveOnlyValidator &) = delete;

    MoveOnlyValidator &operator=(
        const MoveOnlyValidator &) = delete;

    MoveOnlyValidator(
        MoveOnlyValidator &&) noexcept = default;

    MoveOnlyValidator &operator=(
        MoveOnlyValidator &&) noexcept = default;

    [[nodiscard]] vix::Result<void, ValidationError> operator()(
        int value) const
    {
      if (value != *expected_)
      {
        return vix::Result<
            void,
            ValidationError>::failure(ValidationError::invalid);
      }

      return vix::Result<
          void,
          ValidationError>::success();
    }

  private:
    std::unique_ptr<int> expected_;
  };

  static_assert(
      vix::ValidatorFor<PositiveValidator, int>);

  static_assert(
      vix::ValidatorFor<RangeValidator, int>);

  static_assert(
      vix::ValidatorFor<MutableValidator, int>);

  static_assert(
      vix::ValidatorFor<
          StringValidator,
          const std::string &>);

  static_assert(
      vix::ValidatorFor<
          DetailedValidator,
          const std::string &>);

  static_assert(
      vix::ValidatorFor<
          MultipleErrorValidator,
          int>);

  static_assert(
      !vix::ValidatorFor<
          InvalidBoolValidator,
          int>);

  static_assert(
      !vix::ValidatorFor<
          InvalidValueResultValidator,
          int>);

  static_assert(std::same_as<
                vix::validation_result_t<
                    PositiveValidator,
                    int>,
                vix::Result<void, ValidationError>>);

  static_assert(std::same_as<
                vix::validation_error_t<
                    PositiveValidator,
                    int>,
                ValidationError>);

  static_assert(std::same_as<
                vix::validation_error_t<
                    StringValidator,
                    const std::string &>,
                StringValidator::Error>);

  static_assert(std::same_as<
                decltype(vix::validate(
                    42,
                    PositiveValidator{})),
                vix::Result<void, ValidationError>>);

  TEST(ValidateTest, SuccessfulValidationReturnsSuccess)
  {
    auto result =
        vix::validate(
            42,
            PositiveValidator{});

    EXPECT_TRUE(result);
    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.has_error());
  }

  TEST(ValidateTest, FailedValidationReturnsValidatorError)
  {
    auto result =
        vix::validate(
            -1,
            PositiveValidator{});

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        ValidationError::invalid);
  }

  TEST(ValidateTest, PreservesSpecificErrorValue)
  {
    auto result =
        vix::validate(
            100,
            RangeValidator{
                .minimum = 1,
                .maximum = 10});

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        ValidationError::out_of_range);
  }

  TEST(ValidateTest, RangeValidatorAcceptsBoundaryValues)
  {
    RangeValidator validator{
        .minimum = 1,
        .maximum = 10};

    auto minimum =
        vix::validate(
            1,
            validator);

    auto maximum =
        vix::validate(
            10,
            validator);

    EXPECT_TRUE(minimum);
    EXPECT_TRUE(maximum);
  }

  TEST(ValidateTest, InvokesValidatorExactlyOnce)
  {
    MutableValidator validator;

    auto result =
        vix::validate(
            42,
            validator);

    ASSERT_TRUE(result);
    EXPECT_EQ(validator.calls, 1);
  }

  TEST(ValidateTest, SupportsMutableLvalueValidator)
  {
    MutableValidator validator;

    auto first =
        vix::validate(
            1,
            validator);

    auto second =
        vix::validate(
            2,
            validator);

    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    EXPECT_EQ(
        validator.calls,
        2);
  }

  TEST(ValidateTest, SupportsTemporaryValidator)
  {
    auto result =
        vix::validate(
            42,
            RangeValidator{
                .minimum = 0,
                .maximum = 100});

    EXPECT_TRUE(result);
  }

  TEST(ValidateTest, SupportsMoveOnlyValidator)
  {
    auto result =
        vix::validate(
            42,
            MoveOnlyValidator{42});

    EXPECT_TRUE(result);
  }

  TEST(ValidateTest, MoveOnlyValidatorPreservesFailure)
  {
    auto result =
        vix::validate(
            7,
            MoveOnlyValidator{42});

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        ValidationError::invalid);
  }

  TEST(ValidateTest, SupportsReferenceValue)
  {
    const std::string value{
        "Vix.cpp"};

    auto result =
        vix::validate(
            value,
            StringValidator{});

    EXPECT_TRUE(result);
  }

  TEST(ValidateTest, ReferenceValuePreservesValidatorError)
  {
    const std::string value;

    auto result =
        vix::validate(
            value,
            StringValidator{});

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        StringValidator::Error::empty);
  }

  TEST(ValidateTest, PreservesStructuredErrorType)
  {
    const std::string value{
        "invalid"};

    auto result =
        vix::validate(
            value,
            DetailedValidator{});

    static_assert(std::same_as<
                  decltype(result),
                  vix::Result<void, DetailedError>>);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        (DetailedError{
            .message = "invalid value",
            .position = 0}));
  }

  TEST(ValidateTest, StructuredValidatorCanSucceed)
  {
    const std::string value{
        "valid"};

    auto result =
        vix::validate(
            value,
            DetailedValidator{});

    EXPECT_TRUE(result);
  }

  TEST(ValidateTest, ErrorTypeMayRepresentMultipleErrors)
  {
    auto result =
        vix::validate(
            -3,
            MultipleErrorValidator{});

    static_assert(std::same_as<
                  decltype(result),
                  vix::Result<
                      void,
                      MultipleErrorValidator::Error>>);

    ASSERT_FALSE(result);
    ASSERT_EQ(result.error().size(), 2);

    EXPECT_EQ(
        result.error()[0],
        "value must not be negative");

    EXPECT_EQ(
        result.error()[1],
        "value must be even");
  }

  TEST(ValidateTest, MultipleErrorValidatorCanSucceed)
  {
    auto result =
        vix::validate(
            42,
            MultipleErrorValidator{});

    EXPECT_TRUE(result);
  }

  TEST(ValidateTest, DoesNotReplaceValidatorErrorType)
  {
    auto numeric =
        vix::validate(
            -1,
            PositiveValidator{});

    const std::string empty;

    auto text =
        vix::validate(
            empty,
            StringValidator{});

    static_assert(std::same_as<
                  decltype(numeric),
                  vix::Result<void, ValidationError>>);

    static_assert(std::same_as<
                  decltype(text),
                  vix::Result<
                      void,
                      StringValidator::Error>>);

    EXPECT_FALSE(numeric);
    EXPECT_FALSE(text);
  }

} // namespace
