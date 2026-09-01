/**
 *
 *  @file RunTest.cpp
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

#include <vix/Run.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace
{
  class SimpleRunnable
  {
  public:
    [[nodiscard]] int run()
    {
      ++calls_;
      return 42;
    }

    [[nodiscard]] int calls() const noexcept
    {
      return calls_;
    }

  private:
    int calls_{0};
  };

  class ConstRunnable
  {
  public:
    [[nodiscard]] int run() const
    {
      return 42;
    }
  };

  class ArgumentRunnable
  {
  public:
    [[nodiscard]] int run(
        int lhs,
        int rhs)
    {
      return lhs + rhs;
    }
  };

  class StringRunnable
  {
  public:
    [[nodiscard]] std::string run(
        std::string value) const
    {
      return value + " Standard";
    }
  };

  class VoidRunnable
  {
  public:
    void run()
    {
      executed_ = true;
    }

    [[nodiscard]] bool executed() const noexcept
    {
      return executed_;
    }

  private:
    bool executed_{false};
  };

  class ReferenceRunnable
  {
  public:
    explicit ReferenceRunnable(int value) noexcept
        : value_(value)
    {
    }

    [[nodiscard]] int &run() noexcept
    {
      return value_;
    }

    [[nodiscard]] const int &run() const noexcept
    {
      return value_;
    }

  private:
    int value_;
  };

  class MoveOnlyResultRunnable
  {
  public:
    [[nodiscard]] std::unique_ptr<int> run()
    {
      return std::make_unique<int>(42);
    }
  };

  class MoveOnlyArgumentRunnable
  {
  public:
    [[nodiscard]] int run(
        std::unique_ptr<int> value)
    {
      return *value;
    }
  };

  class NoexceptRunnable
  {
  public:
    [[nodiscard]] int run() noexcept
    {
      return 42;
    }
  };

  class ThrowingRunnable
  {
  public:
    [[nodiscard]] int run()
    {
      return 42;
    }
  };

  class OverloadedRunnable
  {
  public:
    [[nodiscard]] int run()
    {
      return 1;
    }

    [[nodiscard]] int run(int value)
    {
      return value;
    }

    [[nodiscard]] std::string run(
        std::string value)
    {
      return value;
    }
  };

  class RefQualifiedRunnable
  {
  public:
    [[nodiscard]] int run() &
    {
      return 1;
    }

    [[nodiscard]] int run() &&
    {
      return 2;
    }
  };

  class InvalidRunnable
  {
  public:
    [[nodiscard]] int execute()
    {
      return 42;
    }
  };

  struct CallableOnly
  {
    [[nodiscard]] int operator()() const
    {
      return 42;
    }
  };

  static_assert(
      vix::Runnable<SimpleRunnable &>);

  static_assert(
      vix::Runnable<const ConstRunnable &>);

  static_assert(
      vix::RunnableWith<
          ArgumentRunnable &,
          int,
          int>);

  static_assert(
      vix::RunnableWith<
          StringRunnable &,
          std::string>);

  static_assert(
      vix::Runnable<VoidRunnable &>);

  static_assert(
      vix::Runnable<ReferenceRunnable &>);

  static_assert(
      vix::Runnable<MoveOnlyResultRunnable &>);

  static_assert(
      vix::RunnableWith<
          MoveOnlyArgumentRunnable &,
          std::unique_ptr<int>>);

  static_assert(
      !vix::Runnable<InvalidRunnable &>);

  static_assert(
      !vix::Runnable<CallableOnly &>);

  static_assert(std::same_as<
                vix::run_result_t<SimpleRunnable &>,
                int>);

  static_assert(std::same_as<
                vix::run_result_t<VoidRunnable &>,
                void>);

  static_assert(std::same_as<
                vix::run_result_t<ReferenceRunnable &>,
                int &>);

  static_assert(std::same_as<
                vix::run_result_t<const ReferenceRunnable &>,
                const int &>);

  static_assert(std::same_as<
                decltype(vix::run(
                    std::declval<SimpleRunnable &>())),
                int>);

  static_assert(std::same_as<
                decltype(vix::run(
                    std::declval<ArgumentRunnable &>(),
                    20,
                    22)),
                int>);

  static_assert(std::same_as<
                decltype(vix::run(
                    std::declval<VoidRunnable &>())),
                void>);

  static_assert(std::same_as<
                decltype(vix::run(
                    std::declval<ReferenceRunnable &>())),
                int &>);

  static_assert(std::same_as<
                decltype(vix::run(
                    std::declval<const ReferenceRunnable &>())),
                const int &>);

  static_assert(
      noexcept(
          vix::run(
              std::declval<NoexceptRunnable &>())));

  static_assert(
      !noexcept(
          vix::run(
              std::declval<ThrowingRunnable &>())));

  TEST(RunTest, RunsObject)
  {
    SimpleRunnable runnable;

    auto result =
        vix::run(runnable);

    EXPECT_EQ(result, 42);
    EXPECT_EQ(runnable.calls(), 1);
  }

  TEST(RunTest, InvokesRunExactlyOnce)
  {
    SimpleRunnable runnable;

    EXPECT_EQ(runnable.calls(), 0);

    static_cast<void>(
        vix::run(runnable));

    EXPECT_EQ(runnable.calls(), 1);
  }

  TEST(RunTest, SupportsConstRunnable)
  {
    const ConstRunnable runnable;

    auto result =
        vix::run(runnable);

    EXPECT_EQ(result, 42);
  }

  TEST(RunTest, ForwardsArguments)
  {
    ArgumentRunnable runnable;

    auto result =
        vix::run(
            runnable,
            20,
            22);

    EXPECT_EQ(result, 42);
  }

  TEST(RunTest, ForwardsStringArgument)
  {
    StringRunnable runnable;

    auto result =
        vix::run(
            runnable,
            std::string{"Vix.cpp"});

    EXPECT_EQ(
        result,
        "Vix.cpp Standard");
  }

  TEST(RunTest, SupportsVoidReturn)
  {
    VoidRunnable runnable;

    EXPECT_FALSE(runnable.executed());

    vix::run(runnable);

    EXPECT_TRUE(runnable.executed());
  }

  TEST(RunTest, PreservesReferenceReturnType)
  {
    ReferenceRunnable runnable{42};

    int &result =
        vix::run(runnable);

    result = 84;

    EXPECT_EQ(
        vix::run(runnable),
        84);
  }

  TEST(RunTest, PreservesConstReferenceReturnType)
  {
    const ReferenceRunnable runnable{42};

    const int &result =
        vix::run(runnable);

    EXPECT_EQ(result, 42);
  }

  TEST(RunTest, PreservesMoveOnlyReturnValue)
  {
    MoveOnlyResultRunnable runnable;

    auto result =
        vix::run(runnable);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 42);
  }

  TEST(RunTest, ForwardsMoveOnlyArgument)
  {
    MoveOnlyArgumentRunnable runnable;

    auto value =
        std::make_unique<int>(42);

    auto result =
        vix::run(
            runnable,
            std::move(value));

    EXPECT_EQ(result, 42);
    EXPECT_EQ(value, nullptr);
  }

  TEST(RunTest, SelectsRunOverloadFromArguments)
  {
    OverloadedRunnable runnable;

    EXPECT_EQ(
        vix::run(runnable),
        1);

    EXPECT_EQ(
        vix::run(runnable, 42),
        42);

    EXPECT_EQ(
        vix::run(
            runnable,
            std::string{"Vix.cpp"}),
        "Vix.cpp");
  }

  TEST(RunTest, PreservesLvalueQualification)
  {
    RefQualifiedRunnable runnable;

    auto result =
        vix::run(runnable);

    EXPECT_EQ(result, 1);
  }

  TEST(RunTest, PreservesRvalueQualification)
  {
    auto result =
        vix::run(
            RefQualifiedRunnable{});

    EXPECT_EQ(result, 2);
  }

  TEST(RunTest, DoesNotIntroduceAdditionalRunSemantics)
  {
    class Counter
    {
    public:
      [[nodiscard]] int run(int amount)
      {
        value_ += amount;
        return value_;
      }

    private:
      int value_{0};
    };

    Counter counter;

    EXPECT_EQ(
        vix::run(counter, 10),
        10);

    EXPECT_EQ(
        vix::run(counter, 32),
        42);
  }

} // namespace
