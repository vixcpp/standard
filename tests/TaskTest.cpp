/**
 *
 *  @file TaskTest.cpp
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
#include <vix/Task.hpp>

#include <gtest/gtest.h>

#include <coroutine>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace
{
  static_assert(!std::is_copy_constructible_v<vix::Task<int>>);
  static_assert(!std::is_copy_assignable_v<vix::Task<int>>);
  static_assert(std::is_move_constructible_v<vix::Task<int>>);
  static_assert(std::is_move_assignable_v<vix::Task<int>>);

  static_assert(!std::is_copy_constructible_v<vix::Task<void>>);
  static_assert(!std::is_copy_assignable_v<vix::Task<void>>);
  static_assert(std::is_move_constructible_v<vix::Task<void>>);
  static_assert(std::is_move_assignable_v<vix::Task<void>>);

  static_assert(std::same_as<
                vix::Task<int>::value_type,
                int>);

  static_assert(std::same_as<
                vix::Task<void>::value_type,
                void>);

  static_assert(std::same_as<
                decltype(std::declval<vix::Task<int> &>().result()),
                int &>);

  static_assert(std::same_as<
                decltype(std::declval<const vix::Task<int> &>().result()),
                const int &>);

  static_assert(std::same_as<
                decltype(std::declval<vix::Task<int> &&>().result()),
                int &&>);

  vix::Task<int> make_value(int value)
  {
    co_return value;
  }

  vix::Task<void> complete()
  {
    co_return;
  }

  vix::Task<int> increment_counter(
      int &counter,
      int value)
  {
    ++counter;
    co_return value;
  }

  vix::Task<void> increment_counter(
      int &counter)
  {
    ++counter;
    co_return;
  }

  vix::Task<int> suspend_once(
      int &counter)
  {
    ++counter;

    co_await std::suspend_always{};

    ++counter;

    co_return counter;
  }

  vix::Task<void> suspend_once_void(
      int &counter)
  {
    ++counter;

    co_await std::suspend_always{};

    ++counter;

    co_return;
  }

  vix::Task<int> throw_value_error()
  {
    throw std::runtime_error("task failure");
    co_return 0;
  }

  vix::Task<void> throw_void_error()
  {
    throw std::runtime_error("task failure");
    co_return;
  }

  vix::Task<std::unique_ptr<int>> make_unique_value()
  {
    co_return std::make_unique<int>(42);
  }

  vix::Task<int> await_value()
  {
    const int value =
        co_await make_value(21);

    co_return value * 2;
  }

  vix::Task<void> await_void(
      bool &completed)
  {
    co_await complete();

    completed = true;
  }

  vix::Task<int> await_nested_tasks()
  {
    const int first =
        co_await make_value(20);

    const int second =
        co_await make_value(22);

    co_return first + second;
  }

  vix::Task<int> propagate_awaited_exception()
  {
    const int value =
        co_await throw_value_error();

    co_return value;
  }

  enum class OperationError
  {
    failed
  };

  using OperationResult =
      vix::Result<int, OperationError>;

  vix::Task<OperationResult> make_successful_operation()
  {
    co_return OperationResult::success(42);
  }

  vix::Task<OperationResult> make_failed_operation()
  {
    co_return OperationResult::failure(
        OperationError::failed);
  }

  TEST(TaskTest, DefaultConstructedTaskIsEmpty)
  {
    vix::Task<int> task;

    EXPECT_FALSE(task.valid());
    EXPECT_FALSE(static_cast<bool>(task));
    EXPECT_FALSE(task.done());
  }

  TEST(TaskTest, DefaultConstructedVoidTaskIsEmpty)
  {
    vix::Task<void> task;

    EXPECT_FALSE(task.valid());
    EXPECT_FALSE(static_cast<bool>(task));
    EXPECT_FALSE(task.done());
  }

  TEST(TaskTest, CreatedTaskOwnsCoroutine)
  {
    auto task = make_value(42);

    EXPECT_TRUE(task.valid());
    EXPECT_TRUE(static_cast<bool>(task));
    EXPECT_FALSE(task.done());
  }

  TEST(TaskTest, TaskIsLazy)
  {
    int counter = 0;

    auto task =
        increment_counter(counter, 42);

    EXPECT_EQ(counter, 0);
    EXPECT_FALSE(task.done());

    task.start();

    EXPECT_EQ(counter, 1);
    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.result(), 42);
  }

  TEST(TaskTest, VoidTaskIsLazy)
  {
    int counter = 0;

    auto task =
        increment_counter(counter);

    EXPECT_EQ(counter, 0);
    EXPECT_FALSE(task.done());

    task.start();

    EXPECT_EQ(counter, 1);
    EXPECT_TRUE(task.done());
    EXPECT_NO_THROW(task.result());
  }

  TEST(TaskTest, StartCompletesSimpleTask)
  {
    auto task = make_value(42);

    task.start();

    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.result(), 42);
  }

  TEST(TaskTest, StartCompletesSimpleVoidTask)
  {
    auto task = complete();

    task.start();

    EXPECT_TRUE(task.done());
    EXPECT_NO_THROW(task.result());
  }

  TEST(TaskTest, StartResumesSuspendedTaskOnce)
  {
    int counter = 0;

    auto task =
        suspend_once(counter);

    task.start();

    EXPECT_EQ(counter, 1);
    EXPECT_FALSE(task.done());

    task.start();

    EXPECT_EQ(counter, 2);
    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.result(), 2);
  }

  TEST(TaskTest, VoidTaskCanBeResumedAfterSuspension)
  {
    int counter = 0;

    auto task =
        suspend_once_void(counter);

    task.start();

    EXPECT_EQ(counter, 1);
    EXPECT_FALSE(task.done());

    task.start();

    EXPECT_EQ(counter, 2);
    EXPECT_TRUE(task.done());
    EXPECT_NO_THROW(task.result());
  }

  TEST(TaskTest, StartingCompletedTaskHasNoEffect)
  {
    int counter = 0;

    auto task =
        increment_counter(counter, 42);

    task.start();

    ASSERT_TRUE(task.done());
    ASSERT_EQ(counter, 1);

    task.start();
    task.start();

    EXPECT_TRUE(task.done());
    EXPECT_EQ(counter, 1);
    EXPECT_EQ(task.result(), 42);
  }

  TEST(TaskTest, ResultBeforeStartThrows)
  {
    auto task = make_value(42);

    EXPECT_THROW(
        static_cast<void>(task.result()),
        std::logic_error);
  }

  TEST(TaskTest, ResultBeforeCompletionThrows)
  {
    int counter = 0;

    auto task =
        suspend_once(counter);

    task.start();

    ASSERT_FALSE(task.done());

    EXPECT_THROW(
        static_cast<void>(task.result()),
        std::logic_error);
  }

  TEST(TaskTest, VoidResultBeforeStartThrows)
  {
    auto task = complete();

    EXPECT_THROW(
        task.result(),
        std::logic_error);
  }

  TEST(TaskTest, StartOnEmptyTaskThrows)
  {
    vix::Task<int> task;

    EXPECT_THROW(
        task.start(),
        std::logic_error);
  }

  TEST(TaskTest, StartOnEmptyVoidTaskThrows)
  {
    vix::Task<void> task;

    EXPECT_THROW(
        task.start(),
        std::logic_error);
  }

  TEST(TaskTest, ResultOnEmptyTaskThrows)
  {
    vix::Task<int> task;

    EXPECT_THROW(
        static_cast<void>(task.result()),
        std::logic_error);
  }

  TEST(TaskTest, ResultOnEmptyVoidTaskThrows)
  {
    vix::Task<void> task;

    EXPECT_THROW(
        task.result(),
        std::logic_error);
  }

  TEST(TaskTest, ResultReturnsStoredValue)
  {
    auto task = make_value(42);

    task.start();

    ASSERT_TRUE(task.done());
    EXPECT_EQ(task.result(), 42);
  }

  TEST(TaskTest, ResultReturnsMutableReference)
  {
    auto task = make_value(42);

    task.start();

    int &value = task.result();
    value = 84;

    EXPECT_EQ(task.result(), 84);
  }

  TEST(TaskTest, ConstResultReturnsConstReference)
  {
    auto task = make_value(42);

    task.start();

    const auto &const_task = task;
    const int &value = const_task.result();

    EXPECT_EQ(value, 42);
  }

  TEST(TaskTest, RvalueResultSupportsMoveOnlyValue)
  {
    auto task = make_unique_value();

    task.start();

    auto value =
        std::move(task).result();

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 42);
  }

  TEST(TaskTest, MoveConstructionTransfersOwnership)
  {
    auto source = make_value(42);

    auto destination =
        std::move(source);

    EXPECT_FALSE(source.valid());
    EXPECT_TRUE(destination.valid());

    destination.start();

    ASSERT_TRUE(destination.done());
    EXPECT_EQ(destination.result(), 42);
  }

  TEST(TaskTest, MoveAssignmentTransfersOwnership)
  {
    auto source = make_value(42);
    auto destination = make_value(7);

    destination = std::move(source);

    EXPECT_FALSE(source.valid());
    EXPECT_TRUE(destination.valid());

    destination.start();

    ASSERT_TRUE(destination.done());
    EXPECT_EQ(destination.result(), 42);
  }

  TEST(TaskTest, MovedFromTaskCanBeAssignedAgain)
  {
    auto first = make_value(42);
    auto second = std::move(first);

    ASSERT_FALSE(first.valid());

    first = make_value(7);

    EXPECT_TRUE(first.valid());
    EXPECT_TRUE(second.valid());

    first.start();
    second.start();

    EXPECT_EQ(first.result(), 7);
    EXPECT_EQ(second.result(), 42);
  }

  TEST(TaskTest, CapturesExceptionFromCoroutine)
  {
    auto task = throw_value_error();

    EXPECT_NO_THROW(task.start());
    EXPECT_TRUE(task.done());

    EXPECT_THROW(
        static_cast<void>(task.result()),
        std::runtime_error);
  }

  TEST(TaskTest, VoidTaskCapturesExceptionFromCoroutine)
  {
    auto task = throw_void_error();

    EXPECT_NO_THROW(task.start());
    EXPECT_TRUE(task.done());

    EXPECT_THROW(
        task.result(),
        std::runtime_error);
  }

  TEST(TaskTest, PreservesExceptionMessage)
  {
    auto task = throw_value_error();

    task.start();

    try
    {
      static_cast<void>(task.result());
      FAIL() << "Expected task result to throw.";
    }
    catch (const std::runtime_error &error)
    {
      EXPECT_STREQ(
          error.what(),
          "task failure");
    }
  }

  TEST(TaskTest, CanAwaitAnotherTask)
  {
    auto task = await_value();

    EXPECT_FALSE(task.done());

    task.start();

    ASSERT_TRUE(task.done());
    EXPECT_EQ(task.result(), 42);
  }

  TEST(TaskTest, CanAwaitVoidTask)
  {
    bool completed = false;

    auto task =
        await_void(completed);

    EXPECT_FALSE(completed);

    task.start();

    EXPECT_TRUE(task.done());
    EXPECT_TRUE(completed);
    EXPECT_NO_THROW(task.result());
  }

  TEST(TaskTest, CanAwaitSeveralTasksSequentially)
  {
    auto task = await_nested_tasks();

    task.start();

    ASSERT_TRUE(task.done());
    EXPECT_EQ(task.result(), 42);
  }

  TEST(TaskTest, AwaitedExceptionPropagatesToParentTask)
  {
    auto task =
        propagate_awaited_exception();

    EXPECT_NO_THROW(task.start());
    EXPECT_TRUE(task.done());

    EXPECT_THROW(
        static_cast<void>(task.result()),
        std::runtime_error);
  }

  TEST(TaskTest, CanProduceResultValue)
  {
    auto task =
        make_successful_operation();

    task.start();

    ASSERT_TRUE(task.done());

    const auto &result = task.result();

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
  }

  TEST(TaskTest, CanProduceFailedResultValue)
  {
    auto task =
        make_failed_operation();

    task.start();

    ASSERT_TRUE(task.done());

    const auto &result = task.result();

    ASSERT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        OperationError::failed);
  }

  TEST(TaskTest, TaskAndResultKeepDifferentResponsibilities)
  {
    auto task =
        make_failed_operation();

    task.start();

    ASSERT_TRUE(task.done());

    const auto &result = task.result();

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        OperationError::failed);
  }

  TEST(TaskTest, DestroyingUnstartedTaskDoesNotExecuteCoroutine)
  {
    int counter = 0;

    {
      auto task =
          increment_counter(counter, 42);

      EXPECT_TRUE(task.valid());
      EXPECT_EQ(counter, 0);
    }

    EXPECT_EQ(counter, 0);
  }

  TEST(TaskTest, DestroyingSuspendedTaskStopsRemainingCoroutineExecution)
  {
    int counter = 0;

    {
      auto task =
          suspend_once(counter);

      task.start();

      ASSERT_EQ(counter, 1);
      ASSERT_FALSE(task.done());
    }

    EXPECT_EQ(counter, 1);
  }

} // namespace
