/**
 *
 *  @file Task.hpp
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

#ifndef VIX_TASK_HPP
#define VIX_TASK_HPP

#include <concepts>
#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace vix
{
  /**
   * @brief Represents deferred work that eventually produces a value.
   *
   * Task is a small coroutine-based execution primitive. A Task owns a
   * suspended coroutine and can either be started explicitly or composed
   * naturally with another coroutine through co_await.
   *
   * Tasks are lazy. Creating a Task does not immediately execute its coroutine.
   * Execution begins when start() is called or when the Task is awaited.
   *
   * Task does not provide a scheduler, event loop, thread pool, cancellation
   * policy, or thread-affinity model. Those concerns belong to higher-level
   * execution facilities and may be composed with Task when needed.
   *
   * Task itself represents eventual completion, not operational success or
   * failure. Operations that report explicit domain errors can use a value
   * such as:
   *
   * @code
   * vix::Task<vix::Result<Value, Error>>
   * @endcode
   *
   * Unhandled C++ exceptions thrown by the coroutine are captured and
   * rethrown when the result is observed.
   *
   * A Task has unique ownership of its coroutine and is therefore movable but
   * not copyable.
   *
   * @tparam T Value produced when the coroutine completes.
   *
   * @note T must be a non-void, non-reference, non-array object type.
   * @note Use Task<void> for work that completes without producing a value.
   * @note Coroutine-frame allocation is controlled by the C++ coroutine
   *       implementation and may require dynamic allocation.
   */
  template <typename T>
  class [[nodiscard]] Task
  {
    static_assert(!std::is_void_v<T>,
                  "Use Task<void> for work without a result value.");

    static_assert(!std::is_reference_v<T>,
                  "Task value type must not be a reference.");

    static_assert(!std::is_array_v<T>,
                  "Task value type must not be an array.");

  public:
    class promise_type;

    /**
     * @brief Native coroutine handle type used by Task.
     */
    using handle_type = std::coroutine_handle<promise_type>;

    /**
     * @brief Value produced by the Task.
     */
    using value_type = T;

    /**
     * @brief Creates an empty Task.
     *
     * An empty Task owns no coroutine and cannot be started or observed for a
     * result.
     */
    constexpr Task() noexcept = default;

    /**
     * @brief Transfers ownership from another Task.
     *
     * @param other Task whose coroutine is transferred.
     */
    constexpr Task(Task &&other) noexcept
        : handle_(std::exchange(other.handle_, {}))
    {
    }

    /**
     * @brief Transfers ownership from another Task.
     *
     * Any coroutine currently owned by this Task is destroyed before taking
     * ownership of @p other.
     *
     * @param other Task whose coroutine is transferred.
     * @return Reference to this Task.
     */
    constexpr Task &operator=(Task &&other) noexcept
    {
      if (this != &other)
      {
        reset();
        handle_ = std::exchange(other.handle_, {});
      }

      return *this;
    }

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    /**
     * @brief Destroys the owned coroutine, if any.
     *
     * Destroying a suspended Task cancels its remaining coroutine execution by
     * destroying the coroutine frame. This is an ownership action, not a
     * cooperative cancellation mechanism for external operations.
     */
    ~Task()
    {
      reset();
    }

    /**
     * @brief Checks whether this Task owns a coroutine.
     *
     * @return true when a coroutine is owned, otherwise false.
     */
    [[nodiscard]] constexpr bool valid() const noexcept
    {
      return static_cast<bool>(handle_);
    }

    /**
     * @brief Checks whether this Task owns a coroutine.
     *
     * @return true when the Task is valid.
     */
    [[nodiscard]] explicit constexpr operator bool() const noexcept
    {
      return valid();
    }

    /**
     * @brief Checks whether the Task has completed.
     *
     * @return true when a valid Task reached its final suspension point.
     *
     * @note An empty Task is not considered completed.
     */
    [[nodiscard]] bool done() const noexcept
    {
      return handle_ && handle_.done();
    }

    /**
     * @brief Starts or resumes execution of the Task.
     *
     * The coroutine is resumed once. If it suspends again, further progress
     * depends on the awaitable that caused the suspension or another explicit
     * resume through start().
     *
     * Calling start() on a completed Task has no effect.
     *
     * @throws std::logic_error if this Task owns no coroutine.
     */
    void start()
    {
      ensure_valid();

      if (!handle_.done())
      {
        handle_.resume();
      }
    }

    /**
     * @brief Returns the completed result.
     *
     * @return Reference to the stored result.
     *
     * @throws std::logic_error if the Task is empty or has not completed.
     * @throws Any exception captured from the coroutine body.
     */
    [[nodiscard]] T &result() &
    {
      ensure_completed();

      auto &promise = handle_.promise();
      promise.rethrow_if_exception();

      return *promise.value_;
    }

    /**
     * @brief Returns the completed result.
     *
     * @return Const reference to the stored result.
     *
     * @throws std::logic_error if the Task is empty or has not completed.
     * @throws Any exception captured from the coroutine body.
     */
    [[nodiscard]] const T &result() const &
    {
      ensure_completed();

      const auto &promise = handle_.promise();
      promise.rethrow_if_exception();

      return *promise.value_;
    }

    /**
     * @brief Returns the completed result by move.
     *
     * @return Rvalue reference to the stored result.
     *
     * @throws std::logic_error if the Task is empty or has not completed.
     * @throws Any exception captured from the coroutine body.
     */
    [[nodiscard]] T &&result() &&
    {
      ensure_completed();

      auto &promise = handle_.promise();
      promise.rethrow_if_exception();

      return std::move(*promise.value_);
    }

    /**
     * @brief Awaiter that consumes ownership of a Task coroutine.
     *
     * Awaiting a Task transfers coroutine ownership into the awaiter. The
     * coroutine frame remains alive until the await operation finishes or the
     * awaiting coroutine itself is destroyed.
     */
    class Awaiter
    {
    public:
      explicit constexpr Awaiter(handle_type handle) noexcept
          : handle_(handle)
      {
      }

      Awaiter(const Awaiter &) = delete;
      Awaiter &operator=(const Awaiter &) = delete;

      constexpr Awaiter(Awaiter &&other) noexcept
          : handle_(std::exchange(other.handle_, {}))
      {
      }

      constexpr Awaiter &operator=(Awaiter &&other) noexcept
      {
        if (this != &other)
        {
          reset();
          handle_ = std::exchange(other.handle_, {});
        }

        return *this;
      }

      ~Awaiter()
      {
        reset();
      }

      /**
       * @brief Checks whether suspension can be skipped.
       *
       * @return true when the Task has already completed or is empty.
       */
      [[nodiscard]] bool await_ready() const noexcept
      {
        return !handle_ || handle_.done();
      }

      /**
       * @brief Connects the awaiting coroutine to this Task.
       *
       * The Task coroutine is returned to the runtime so execution transfers
       * directly into it without an additional scheduler requirement.
       *
       * @param continuation Coroutine waiting for this Task.
       * @return Coroutine that should execute next.
       */
      [[nodiscard]] std::coroutine_handle<> await_suspend(
          std::coroutine_handle<> continuation) noexcept
      {
        handle_.promise().continuation_ = continuation;
        return handle_;
      }

      /**
       * @brief Returns the value produced by the completed Task.
       *
       * @return Produced value.
       *
       * @throws std::logic_error if the awaited Task is empty.
       * @throws Any exception captured from the Task coroutine.
       */
      [[nodiscard]] T await_resume()
      {
        if (!handle_)
        {
          throw std::logic_error("Cannot await an empty vix::Task.");
        }

        auto &promise = handle_.promise();
        promise.rethrow_if_exception();

        return std::move(*promise.value_);
      }

    private:
      void reset() noexcept
      {
        if (handle_)
        {
          handle_.destroy();
          handle_ = {};
        }
      }

      handle_type handle_{};
    };

    /**
     * @brief Awaits this Task and transfers ownership to the await operation.
     *
     * A Task is a single-owner, single-consumer object. Awaiting requires an
     * rvalue so the transfer of ownership is explicit.
     *
     * @return Awaiter owning the Task coroutine.
     */
    [[nodiscard]] Awaiter operator co_await() && noexcept
    {
      return Awaiter(std::exchange(handle_, {}));
    }

    /**
     * @brief Coroutine promise used to produce Task values.
     */
    class promise_type
    {
      friend class Task;
      friend class Awaiter;

    public:
      /**
       * @brief Creates the Task associated with this coroutine.
       *
       * @return Task owning this promise's coroutine frame.
       */
      [[nodiscard]] Task get_return_object() noexcept
      {
        return Task(handle_type::from_promise(*this), PrivateTag{});
      }

      /**
       * @brief Keeps newly created Tasks lazy.
       *
       * @return Suspension object that prevents immediate execution.
       */
      [[nodiscard]] constexpr std::suspend_always
      initial_suspend() const noexcept
      {
        return {};
      }

      /**
       * @brief Final suspension policy used to resume a waiting coroutine.
       */
      struct FinalAwaiter
      {
        /**
         * @brief Final suspension is always entered.
         *
         * @return false.
         */
        [[nodiscard]] constexpr bool await_ready() const noexcept
        {
          return false;
        }

        /**
         * @brief Resumes the coroutine waiting for this Task.
         *
         * @param handle Completed Task coroutine.
         * @return Waiting continuation, or a no-op coroutine when none exists.
         */
        [[nodiscard]] std::coroutine_handle<> await_suspend(
            handle_type handle) const noexcept
        {
          const auto continuation =
              handle.promise().continuation_;

          return continuation
                     ? continuation
                     : std::noop_coroutine();
        }

        /**
         * @brief No operation is required after final suspension.
         */
        constexpr void await_resume() const noexcept
        {
        }
      };

      /**
       * @brief Suspends the completed Task and resumes its continuation.
       *
       * @return Final awaiter.
       */
      [[nodiscard]] constexpr FinalAwaiter
      final_suspend() const noexcept
      {
        return {};
      }

      /**
       * @brief Stores the value returned by the coroutine.
       *
       * @tparam U Type used to construct T.
       * @param value Value produced by co_return.
       */
      template <typename U>
        requires std::constructible_from<T, U &&>
      void return_value(U &&value) noexcept(std::is_nothrow_constructible_v<T, U &&>)
      {
        value_.emplace(std::forward<U>(value));
      }

      /**
       * @brief Captures an exception escaping the coroutine body.
       */
      void unhandled_exception() noexcept
      {
        exception_ = std::current_exception();
      }

    private:
      /**
       * @brief Rethrows an exception captured during execution.
       */
      void rethrow_if_exception() const
      {
        if (exception_)
        {
          std::rethrow_exception(exception_);
        }
      }

      std::optional<T> value_;
      std::exception_ptr exception_;
      std::coroutine_handle<> continuation_{};
    };

  private:
    struct PrivateTag
    {
    };

    explicit constexpr Task(
        handle_type handle,
        PrivateTag) noexcept
        : handle_(handle)
    {
    }

    void ensure_valid() const
    {
      if (!handle_)
      {
        throw std::logic_error("vix::Task does not own a coroutine.");
      }
    }

    void ensure_completed() const
    {
      ensure_valid();

      if (!handle_.done())
      {
        throw std::logic_error(
            "vix::Task result requested before completion.");
      }
    }

    void reset() noexcept
    {
      if (handle_)
      {
        handle_.destroy();
        handle_ = {};
      }
    }

    handle_type handle_{};
  };

  /**
   * @brief Represents deferred work that completes without producing a value.
   *
   * Task<void> has the same ownership and lazy execution semantics as
   * Task<T>, but successful completion carries no value.
   *
   * Unhandled C++ exceptions are captured and rethrown when completion is
   * observed through result() or co_await.
   */
  template <>
  class [[nodiscard]] Task<void>
  {
  public:
    class promise_type;

    /**
     * @brief Native coroutine handle type used by Task<void>.
     */
    using handle_type = std::coroutine_handle<promise_type>;

    /**
     * @brief Value type produced by this Task specialization.
     */
    using value_type = void;

    constexpr Task() noexcept = default;

    constexpr Task(Task &&other) noexcept
        : handle_(std::exchange(other.handle_, {}))
    {
    }

    constexpr Task &operator=(Task &&other) noexcept
    {
      if (this != &other)
      {
        reset();
        handle_ = std::exchange(other.handle_, {});
      }

      return *this;
    }

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    ~Task()
    {
      reset();
    }

    /**
     * @brief Checks whether this Task owns a coroutine.
     *
     * @return true when a coroutine is owned.
     */
    [[nodiscard]] constexpr bool valid() const noexcept
    {
      return static_cast<bool>(handle_);
    }

    /**
     * @brief Checks whether this Task owns a coroutine.
     *
     * @return true when the Task is valid.
     */
    [[nodiscard]] explicit constexpr operator bool() const noexcept
    {
      return valid();
    }

    /**
     * @brief Checks whether the Task has completed.
     *
     * @return true when a valid Task reached final suspension.
     */
    [[nodiscard]] bool done() const noexcept
    {
      return handle_ && handle_.done();
    }

    /**
     * @brief Starts or resumes the Task once.
     *
     * @throws std::logic_error if the Task is empty.
     */
    void start()
    {
      ensure_valid();

      if (!handle_.done())
      {
        handle_.resume();
      }
    }

    /**
     * @brief Observes successful completion.
     *
     * @throws std::logic_error if the Task is empty or incomplete.
     * @throws Any exception captured from the coroutine body.
     */
    void result() const
    {
      ensure_completed();
      handle_.promise().rethrow_if_exception();
    }

    /**
     * @brief Awaiter that consumes ownership of a Task<void>.
     */
    class Awaiter
    {
    public:
      explicit constexpr Awaiter(handle_type handle) noexcept
          : handle_(handle)
      {
      }

      Awaiter(const Awaiter &) = delete;
      Awaiter &operator=(const Awaiter &) = delete;

      constexpr Awaiter(Awaiter &&other) noexcept
          : handle_(std::exchange(other.handle_, {}))
      {
      }

      constexpr Awaiter &operator=(Awaiter &&other) noexcept
      {
        if (this != &other)
        {
          reset();
          handle_ = std::exchange(other.handle_, {});
        }

        return *this;
      }

      ~Awaiter()
      {
        reset();
      }

      [[nodiscard]] bool await_ready() const noexcept
      {
        return !handle_ || handle_.done();
      }

      [[nodiscard]] std::coroutine_handle<> await_suspend(
          std::coroutine_handle<> continuation) noexcept
      {
        handle_.promise().continuation_ = continuation;
        return handle_;
      }

      /**
       * @brief Observes completion of the awaited Task.
       *
       * @throws std::logic_error if the Task is empty.
       * @throws Any exception captured from the coroutine body.
       */
      void await_resume()
      {
        if (!handle_)
        {
          throw std::logic_error("Cannot await an empty vix::Task.");
        }

        handle_.promise().rethrow_if_exception();
      }

    private:
      void reset() noexcept
      {
        if (handle_)
        {
          handle_.destroy();
          handle_ = {};
        }
      }

      handle_type handle_{};
    };

    /**
     * @brief Awaits this Task and transfers ownership to the await operation.
     *
     * A Task is a single-owner, single-consumer object. Awaiting requires an
     * rvalue so the transfer of ownership is explicit.
     *
     * @return Awaiter owning the Task coroutine.
     */
    [[nodiscard]] Awaiter operator co_await() && noexcept
    {
      return Awaiter(std::exchange(handle_, {}));
    }

    /**
     * @brief Coroutine promise used by Task<void>.
     */
    class promise_type
    {
      friend class Task;
      friend class Awaiter;

    public:
      [[nodiscard]] Task get_return_object() noexcept
      {
        return Task(handle_type::from_promise(*this), PrivateTag{});
      }

      [[nodiscard]] constexpr std::suspend_always
      initial_suspend() const noexcept
      {
        return {};
      }

      struct FinalAwaiter
      {
        [[nodiscard]] constexpr bool await_ready() const noexcept
        {
          return false;
        }

        [[nodiscard]] std::coroutine_handle<> await_suspend(
            handle_type handle) const noexcept
        {
          const auto continuation =
              handle.promise().continuation_;

          return continuation
                     ? continuation
                     : std::noop_coroutine();
        }

        constexpr void await_resume() const noexcept
        {
        }
      };

      [[nodiscard]] constexpr FinalAwaiter
      final_suspend() const noexcept
      {
        return {};
      }

      /**
       * @brief Marks successful completion without a value.
       */
      constexpr void return_void() const noexcept
      {
      }

      /**
       * @brief Captures an exception escaping the coroutine body.
       */
      void unhandled_exception() noexcept
      {
        exception_ = std::current_exception();
      }

    private:
      void rethrow_if_exception() const
      {
        if (exception_)
        {
          std::rethrow_exception(exception_);
        }
      }

      std::exception_ptr exception_;
      std::coroutine_handle<> continuation_{};
    };

  private:
    struct PrivateTag
    {
    };

    explicit constexpr Task(
        handle_type handle,
        PrivateTag) noexcept
        : handle_(handle)
    {
    }

    void ensure_valid() const
    {
      if (!handle_)
      {
        throw std::logic_error("vix::Task does not own a coroutine.");
      }
    }

    void ensure_completed() const
    {
      ensure_valid();

      if (!handle_.done())
      {
        throw std::logic_error(
            "vix::Task result requested before completion.");
      }
    }

    void reset() noexcept
    {
      if (handle_)
      {
        handle_.destroy();
        handle_ = {};
      }
    }

    handle_type handle_{};
  };

} // namespace vix

#endif // VIX_TASK_HPP
