/**
 *
 *  @file PrintLogTest.cpp
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

#include <vix/Log.hpp>
#include <vix/Print.hpp>
#include <vix/Result.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
  enum class OutputError
  {
    failed
  };

  class TextOutput
  {
  public:
    using error_type = OutputError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data)
    {
      if (failed_)
      {
        return vix::Result<
            std::size_t,
            error_type>::failure(OutputError::failed);
      }

      bytes_.insert(
          bytes_.end(),
          data.begin(),
          data.end());

      return vix::Result<
          std::size_t,
          error_type>::success(data.size());
    }

    [[nodiscard]] std::string text() const
    {
      return std::string(
          reinterpret_cast<const char *>(
              bytes_.data()),
          bytes_.size());
    }

    [[nodiscard]] const std::vector<std::byte> &bytes() const noexcept
    {
      return bytes_;
    }

    void fail() noexcept
    {
      failed_ = true;
    }

  private:
    std::vector<std::byte> bytes_;
    bool failed_{false};
  };

  class PartialTextOutput
  {
  public:
    using error_type = OutputError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data)
    {
      if (data.empty())
      {
        return vix::Result<
            std::size_t,
            error_type>::success(0);
      }

      return vix::Result<
          std::size_t,
          error_type>::success(1);
    }
  };

  class ConstTextOutput
  {
  public:
    using error_type = OutputError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data) const
    {
      return vix::Result<
          std::size_t,
          error_type>::success(data.size());
    }
  };

  class VoidLogTarget
  {
  public:
    void log(
        vix::LogLevel level,
        std::string_view message)
    {
      ++calls_;
      level_ = level;
      message_.assign(message);
    }

    [[nodiscard]] int calls() const noexcept
    {
      return calls_;
    }

    [[nodiscard]] vix::LogLevel level() const noexcept
    {
      return level_;
    }

    [[nodiscard]] const std::string &message() const noexcept
    {
      return message_;
    }

  private:
    int calls_{0};
    vix::LogLevel level_{vix::LogLevel::info};
    std::string message_;
  };

  class ReturningLogTarget
  {
  public:
    [[nodiscard]] int log(
        vix::LogLevel level,
        std::string_view message)
    {
      level_ = level;
      message_.assign(message);

      return 42;
    }

    [[nodiscard]] vix::LogLevel level() const noexcept
    {
      return level_;
    }

    [[nodiscard]] const std::string &message() const noexcept
    {
      return message_;
    }

  private:
    vix::LogLevel level_{vix::LogLevel::info};
    std::string message_;
  };

  class ConstLogTarget
  {
  public:
    [[nodiscard]] std::size_t log(
        vix::LogLevel,
        std::string_view message) const
    {
      return message.size();
    }
  };

  class NoexceptLogTarget
  {
  public:
    void log(
        vix::LogLevel,
        std::string_view) noexcept
    {
    }
  };

  class ThrowingLogTarget
  {
  public:
    void log(
        vix::LogLevel,
        std::string_view)
    {
    }
  };

  enum class LogError
  {
    rejected
  };

  class ResultLogTarget
  {
  public:
    [[nodiscard]] vix::Result<void, LogError> log(
        vix::LogLevel level,
        std::string_view)
    {
      if (level == vix::LogLevel::critical)
      {
        return vix::Result<
            void,
            LogError>::failure(LogError::rejected);
      }

      return vix::Result<
          void,
          LogError>::success();
    }
  };

  class InvalidLogTarget
  {
  public:
    void write(
        vix::LogLevel,
        std::string_view)
    {
    }
  };

  static_assert(vix::Output<TextOutput>);
  static_assert(vix::Output<PartialTextOutput>);
  static_assert(vix::ConstOutput<ConstTextOutput>);

  static_assert(vix::LogTarget<VoidLogTarget>);
  static_assert(vix::LogTarget<ReturningLogTarget>);
  static_assert(vix::LogTarget<ConstLogTarget>);
  static_assert(vix::LogTarget<ResultLogTarget>);

  static_assert(!vix::LogTarget<InvalidLogTarget>);

  static_assert(std::same_as<
                decltype(vix::print(
                    std::declval<TextOutput &>(),
                    std::declval<std::string_view>())),
                vix::Result<std::size_t, OutputError>>);

  static_assert(std::same_as<
                decltype(vix::log(
                    std::declval<VoidLogTarget &>(),
                    vix::LogLevel::info,
                    std::declval<std::string_view>())),
                void>);

  static_assert(std::same_as<
                decltype(vix::log(
                    std::declval<ReturningLogTarget &>(),
                    vix::LogLevel::info,
                    std::declval<std::string_view>())),
                int>);

  static_assert(std::same_as<
                vix::log_result_t<ReturningLogTarget>,
                int>);

  static_assert(std::same_as<
                vix::log_result_t<ResultLogTarget>,
                vix::Result<void, LogError>>);

  static_assert(
      noexcept(
          vix::log(
              std::declval<NoexceptLogTarget &>(),
              vix::LogLevel::info,
              std::declval<std::string_view>())));

  static_assert(
      !noexcept(
          vix::log(
              std::declval<ThrowingLogTarget &>(),
              vix::LogLevel::info,
              std::declval<std::string_view>())));

  TEST(PrintLogTest, PrintWritesTextToOutput)
  {
    TextOutput output;

    auto result =
        vix::print(
            output,
            "Vix.cpp Standard");

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        16U);

    EXPECT_EQ(
        output.text(),
        "Vix.cpp Standard");
  }

  TEST(PrintLogTest, PrintDoesNotAppendNewline)
  {
    TextOutput output;

    auto result =
        vix::print(
            output,
            "Vix.cpp");

    ASSERT_TRUE(result);

    EXPECT_EQ(
        output.text(),
        "Vix.cpp");

    EXPECT_EQ(
        output.bytes().size(),
        7U);
  }

  TEST(PrintLogTest, PrintPreservesWhitespace)
  {
    TextOutput output;

    constexpr std::string_view text{
        "  Vix.cpp\nStandard\t42  "};

    auto result =
        vix::print(
            output,
            text);

    ASSERT_TRUE(result);

    EXPECT_EQ(
        result.value(),
        text.size());

    EXPECT_EQ(
        output.text(),
        text);
  }

  TEST(PrintLogTest, PrintSupportsEmptyText)
  {
    TextOutput output;

    auto result =
        vix::print(
            output,
            "");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 0U);
    EXPECT_TRUE(output.bytes().empty());
  }

  TEST(PrintLogTest, PrintPreservesOutputError)
  {
    TextOutput output;
    output.fail();

    auto result =
        vix::print(
            output,
            "Vix.cpp");

    static_assert(std::same_as<
                  decltype(result),
                  vix::Result<
                      std::size_t,
                      OutputError>>);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        OutputError::failed);

    EXPECT_TRUE(output.bytes().empty());
  }

  TEST(PrintLogTest, PrintPreservesPartialWrite)
  {
    PartialTextOutput output;

    auto result =
        vix::print(
            output,
            "Vix.cpp");

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 1U);
  }

  TEST(PrintLogTest, PrintSupportsConstOutput)
  {
    const ConstTextOutput output;

    constexpr std::string_view text{
        "Vix.cpp"};

    auto result =
        vix::print(
            output,
            text);

    ASSERT_TRUE(result);
    EXPECT_EQ(
        result.value(),
        text.size());
  }

  TEST(PrintLogTest, PrintMayBeCalledSeveralTimes)
  {
    TextOutput output;

    auto first =
        vix::print(
            output,
            "Vix");

    auto second =
        vix::print(
            output,
            ".cpp");

    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    EXPECT_EQ(
        output.text(),
        "Vix.cpp");
  }

  TEST(PrintLogTest, LogForwardsLevelAndMessage)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::warning,
        "operation is slow");

    EXPECT_EQ(target.calls(), 1);

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::warning);

    EXPECT_EQ(
        target.message(),
        "operation is slow");
  }

  TEST(PrintLogTest, LogInvokesTargetExactlyOnce)
  {
    VoidLogTarget target;

    EXPECT_EQ(target.calls(), 0);

    vix::log(
        target,
        vix::LogLevel::info,
        "started");

    EXPECT_EQ(target.calls(), 1);
  }

  TEST(PrintLogTest, LogPreservesReturnType)
  {
    ReturningLogTarget target;

    auto result =
        vix::log(
            target,
            vix::LogLevel::debug,
            "message");

    static_assert(std::same_as<
                  decltype(result),
                  int>);

    EXPECT_EQ(result, 42);

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::debug);

    EXPECT_EQ(
        target.message(),
        "message");
  }

  TEST(PrintLogTest, LogSupportsConstTarget)
  {
    const ConstLogTarget target;

    constexpr std::string_view message{
        "Vix.cpp"};

    auto result =
        vix::log(
            target,
            vix::LogLevel::info,
            message);

    EXPECT_EQ(
        result,
        message.size());
  }

  TEST(PrintLogTest, LogCanPreserveResultReturn)
  {
    ResultLogTarget target;

    auto result =
        vix::log(
            target,
            vix::LogLevel::info,
            "started");

    static_assert(std::same_as<
                  decltype(result),
                  vix::Result<void, LogError>>);

    EXPECT_TRUE(result);
  }

  TEST(PrintLogTest, LogDoesNotHideTargetFailure)
  {
    ResultLogTarget target;

    auto result =
        vix::log(
            target,
            vix::LogLevel::critical,
            "rejected");

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        LogError::rejected);
  }

  TEST(PrintLogTest, SupportsTraceLevel)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::trace,
        "trace");

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::trace);
  }

  TEST(PrintLogTest, SupportsDebugLevel)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::debug,
        "debug");

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::debug);
  }

  TEST(PrintLogTest, SupportsInfoLevel)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::info,
        "info");

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::info);
  }

  TEST(PrintLogTest, SupportsWarningLevel)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::warning,
        "warning");

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::warning);
  }

  TEST(PrintLogTest, SupportsErrorLevel)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::error,
        "error");

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::error);
  }

  TEST(PrintLogTest, SupportsCriticalLevel)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::critical,
        "critical");

    EXPECT_EQ(
        target.level(),
        vix::LogLevel::critical);
  }

  TEST(PrintLogTest, LogPreservesEmptyMessage)
  {
    VoidLogTarget target;

    vix::log(
        target,
        vix::LogLevel::info,
        "");

    EXPECT_EQ(target.calls(), 1);
    EXPECT_TRUE(target.message().empty());
  }

  TEST(PrintLogTest, PrintAndLogHaveDistinctSemantics)
  {
    TextOutput output;
    VoidLogTarget log_target;

    auto printed =
        vix::print(
            output,
            "visible text");

    vix::log(
        log_target,
        vix::LogLevel::info,
        "program event");

    ASSERT_TRUE(printed);

    EXPECT_EQ(
        output.text(),
        "visible text");

    EXPECT_EQ(
        log_target.message(),
        "program event");
  }

} // namespace
