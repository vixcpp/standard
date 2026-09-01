/**
 *
 *  @file Result.hpp
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

#ifndef VIX_RESULT_HPP
#define VIX_RESULT_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>
#include <variant>

namespace vix
{
  template <typename T, typename E>
  class Result;

  namespace detail
  {
    template <typename T>
    struct IsResult : std::false_type
    {
    };

    template <typename T, typename E>
    struct IsResult<Result<T, E>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_result_v =
        IsResult<std::remove_cvref_t<T>>::value;
  }

  /**
   * @brief Represents either a value or an error.
   *
   * Result models the outcome of an operation that can either produce a value
   * of type T or fail with an error of type E.
   *
   * The value and error alternatives are stored directly inside the Result.
   * Constructing a Result does not require dynamic allocation.
   *
   * A Result is created explicitly with success() or failure(). This avoids
   * ambiguity when T and E are the same type and keeps success and failure
   * visible at construction sites.
   *
   * @tparam T Value type produced on success.
   * @tparam E Error type produced on failure.
   *
   * @note T and E must be non-reference, non-array object types.
   * @note Use Result<void, E> for operations that succeed without producing
   *       a value.
   */
  template <typename T, typename E>
  class [[nodiscard]] Result
  {
    static_assert(!std::is_void_v<T>,
                  "Use Result<void, E> for a result without a value.");

    static_assert(!std::is_void_v<E>,
                  "Result error type must not be void.");

    static_assert(!std::is_reference_v<T>,
                  "Result value type must not be a reference.");

    static_assert(!std::is_reference_v<E>,
                  "Result error type must not be a reference.");

    static_assert(!std::is_array_v<T>,
                  "Result value type must not be an array.");

    static_assert(!std::is_array_v<E>,
                  "Result error type must not be an array.");

  public:
    /**
     * @brief Type stored by a successful Result.
     */
    using value_type = T;

    /**
     * @brief Type stored by a failed Result.
     */
    using error_type = E;

    /**
     * @brief Creates a successful Result.
     *
     * @tparam U Type used to construct T.
     * @param value Value to store.
     * @return A Result containing the constructed value.
     */
    template <typename U>
      requires std::constructible_from<T, U &&>
    [[nodiscard]] static constexpr Result success(U &&value) noexcept(std::is_nothrow_constructible_v<T, U &&>)
    {
      return Result(ValueTag{}, std::forward<U>(value));
    }

    /**
     * @brief Creates a failed Result.
     *
     * @tparam G Type used to construct E.
     * @param error Error to store.
     * @return A Result containing the constructed error.
     */
    template <typename G>
      requires std::constructible_from<E, G &&>
    [[nodiscard]] static constexpr Result failure(G &&error) noexcept(std::is_nothrow_constructible_v<E, G &&>)
    {
      return Result(ErrorTag{}, std::forward<G>(error));
    }

    Result(const Result &) = default;
    Result(Result &&) noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_move_constructible_v<E>) = default;

    Result &operator=(const Result &) = default;
    Result &operator=(Result &&) noexcept(
        std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_move_assignable_v<E>) = default;

    ~Result() = default;

    /**
     * @brief Checks whether the Result contains a value.
     *
     * @return true when the operation succeeded, otherwise false.
     */
    [[nodiscard]] constexpr bool has_value() const noexcept
    {
      return storage_.index() == 0;
    }

    /**
     * @brief Checks whether the Result contains an error.
     *
     * @return true when the operation failed, otherwise false.
     */
    [[nodiscard]] constexpr bool has_error() const noexcept
    {
      return !has_value();
    }

    /**
     * @brief Tests whether the Result represents success.
     *
     * @return true when the Result contains a value.
     */
    [[nodiscard]] explicit constexpr operator bool() const noexcept
    {
      return has_value();
    }

    /**
     * @brief Returns the stored value.
     *
     * @return Reference to the stored value.
     *
     * @throws std::bad_variant_access if the Result contains an error.
     */
    [[nodiscard]] constexpr T &value() &
    {
      return std::get<0>(storage_);
    }

    /**
     * @brief Returns the stored value.
     *
     * @return Const reference to the stored value.
     *
     * @throws std::bad_variant_access if the Result contains an error.
     */
    [[nodiscard]] constexpr const T &value() const &
    {
      return std::get<0>(storage_);
    }

    /**
     * @brief Returns the stored value by move.
     *
     * @return Rvalue reference to the stored value.
     *
     * @throws std::bad_variant_access if the Result contains an error.
     */
    [[nodiscard]] constexpr T &&value() &&
    {
      return std::get<0>(std::move(storage_));
    }

    /**
     * @brief Returns the stored error.
     *
     * @return Reference to the stored error.
     *
     * @throws std::bad_variant_access if the Result contains a value.
     */
    [[nodiscard]] constexpr E &error() &
    {
      return std::get<1>(storage_);
    }

    /**
     * @brief Returns the stored error.
     *
     * @return Const reference to the stored error.
     *
     * @throws std::bad_variant_access if the Result contains a value.
     */
    [[nodiscard]] constexpr const E &error() const &
    {
      return std::get<1>(storage_);
    }

    /**
     * @brief Returns the stored error by move.
     *
     * @return Rvalue reference to the stored error.
     *
     * @throws std::bad_variant_access if the Result contains a value.
     */
    [[nodiscard]] constexpr E &&error() &&
    {
      return std::get<1>(std::move(storage_));
    }

    /**
     * @brief Returns a pointer to the stored value when present.
     *
     * @return Pointer to the value, or nullptr when the Result contains
     *         an error.
     */
    [[nodiscard]] constexpr T *value_if() noexcept
    {
      return std::get_if<0>(&storage_);
    }

    /**
     * @brief Returns a pointer to the stored value when present.
     *
     * @return Const pointer to the value, or nullptr when the Result contains
     *         an error.
     */
    [[nodiscard]] constexpr const T *value_if() const noexcept
    {
      return std::get_if<0>(&storage_);
    }

    /**
     * @brief Returns a pointer to the stored error when present.
     *
     * @return Pointer to the error, or nullptr when the Result contains
     *         a value.
     */
    [[nodiscard]] constexpr E *error_if() noexcept
    {
      return std::get_if<1>(&storage_);
    }

    /**
     * @brief Returns a pointer to the stored error when present.
     *
     * @return Const pointer to the error, or nullptr when the Result contains
     *         a value.
     */
    [[nodiscard]] constexpr const E *error_if() const noexcept
    {
      return std::get_if<1>(&storage_);
    }

    /**
     * @brief Returns the stored value or a fallback value.
     *
     * @tparam U Type used to construct the fallback T.
     * @param fallback Value used when this Result contains an error.
     * @return A copy of the stored value or the constructed fallback.
     */
    template <typename U>
      requires std::copy_constructible<T> &&
               std::constructible_from<T, U &&>
    [[nodiscard]] constexpr T value_or(U &&fallback) const &
    {
      if (has_value())
      {
        return value();
      }

      return T(std::forward<U>(fallback));
    }

    /**
     * @brief Returns the stored value by move or a fallback value.
     *
     * @tparam U Type used to construct the fallback T.
     * @param fallback Value used when this Result contains an error.
     * @return The stored value by move or the constructed fallback.
     */
    template <typename U>
      requires std::move_constructible<T> &&
               std::constructible_from<T, U &&>
    [[nodiscard]] constexpr T value_or(U &&fallback) &&
    {
      if (has_value())
      {
        return std::move(*this).value();
      }

      return T(std::forward<U>(fallback));
    }

    /**
     * @brief Transforms the stored value while preserving the error type.
     *
     * The callable is invoked only when this Result contains a value.
     *
     * @tparam F Callable type.
     * @param function Transformation applied to the value.
     * @return A Result containing the transformed value, or a copy of the
     *         existing error.
     */
    template <typename F>
      requires std::invocable<F, T &> &&
               std::copy_constructible<E>
    [[nodiscard]] constexpr auto map(F &&function) &
    {
      using InvokeResult = std::invoke_result_t<F, T &>;
      using U = std::remove_cvref_t<InvokeResult>;

      if constexpr (std::is_void_v<InvokeResult>)
      {
        if (has_value())
        {
          std::invoke(std::forward<F>(function), value());
          return Result<void, E>::success();
        }

        return Result<void, E>::failure(error());
      }
      else
      {
        if (has_value())
        {
          return Result<U, E>::success(
              std::invoke(std::forward<F>(function), value()));
        }

        return Result<U, E>::failure(error());
      }
    }

    /**
     * @brief Transforms the stored const value while preserving the error type.
     *
     * @tparam F Callable type.
     * @param function Transformation applied to the value.
     * @return A Result containing the transformed value, or a copy of the
     *         existing error.
     */
    template <typename F>
      requires std::invocable<F, const T &> &&
               std::copy_constructible<E>
    [[nodiscard]] constexpr auto map(F &&function) const &
    {
      using InvokeResult = std::invoke_result_t<F, const T &>;
      using U = std::remove_cvref_t<InvokeResult>;

      if constexpr (std::is_void_v<InvokeResult>)
      {
        if (has_value())
        {
          std::invoke(std::forward<F>(function), value());
          return Result<void, E>::success();
        }

        return Result<void, E>::failure(error());
      }
      else
      {
        if (has_value())
        {
          return Result<U, E>::success(
              std::invoke(std::forward<F>(function), value()));
        }

        return Result<U, E>::failure(error());
      }
    }

    /**
     * @brief Transforms the stored value by move while preserving the error
     *        type.
     *
     * @tparam F Callable type.
     * @param function Transformation applied to the value.
     * @return A Result containing the transformed value, or the existing
     *         error by move.
     */
    template <typename F>
      requires std::invocable<F, T &&> &&
               std::move_constructible<E>
    [[nodiscard]] constexpr auto map(F &&function) &&
    {
      using InvokeResult = std::invoke_result_t<F, T &&>;
      using U = std::remove_cvref_t<InvokeResult>;

      if constexpr (std::is_void_v<InvokeResult>)
      {
        if (has_value())
        {
          std::invoke(
              std::forward<F>(function),
              std::move(*this).value());

          return Result<void, E>::success();
        }

        return Result<void, E>::failure(
            std::move(*this).error());
      }
      else
      {
        if (has_value())
        {
          return Result<U, E>::success(
              std::invoke(
                  std::forward<F>(function),
                  std::move(*this).value()));
        }

        return Result<U, E>::failure(
            std::move(*this).error());
      }
    }

    /**
     * @brief Chains another Result-producing operation.
     *
     * The callable is invoked only when this Result contains a value.
     * The returned Result must use the same error type E.
     *
     * @tparam F Callable accepting T& and returning another Result.
     * @param function Operation to invoke on success.
     * @return Result returned by the callable, or the current error.
     */
    template <typename F>
      requires std::invocable<F, T &> &&
               std::copy_constructible<E>
    [[nodiscard]] constexpr auto and_then(F &&function) &
    {
      using Next =
          std::remove_cvref_t<std::invoke_result_t<F, T &>>;

      static_assert(
          detail::is_result_v<Next>,
          "Result::and_then requires a callable returning vix::Result.");

      if constexpr (detail::is_result_v<Next>)
      {
        static_assert(
            std::same_as<typename Next::error_type, E>,
            "Result::and_then requires the same error type.");

        if (has_value())
        {
          return std::invoke(
              std::forward<F>(function),
              value());
        }

        return Next::failure(error());
      }
    }

    /**
     * @brief Chains another Result-producing operation.
     *
     * The callable receives the stored value by move.
     *
     * @tparam F Callable accepting T&& and returning another Result.
     * @param function Operation to invoke on success.
     * @return Result returned by the callable, or the current error by move.
     */
    template <typename F>
      requires std::invocable<F, T &&> &&
               std::move_constructible<E>
    [[nodiscard]] constexpr auto and_then(F &&function) &&
    {
      using Next =
          std::remove_cvref_t<std::invoke_result_t<F, T &&>>;

      static_assert(
          detail::is_result_v<Next>,
          "Result::and_then requires a callable returning vix::Result.");

      if constexpr (detail::is_result_v<Next>)
      {
        static_assert(
            std::same_as<typename Next::error_type, E>,
            "Result::and_then requires the same error type.");

        if (has_value())
        {
          return std::invoke(
              std::forward<F>(function),
              std::move(*this).value());
        }

        return Next::failure(
            std::move(*this).error());
      }
    }

  private:
    struct ValueTag
    {
    };

    struct ErrorTag
    {
    };

    template <typename U>
      requires std::constructible_from<T, U &&>
    explicit constexpr Result(ValueTag, U &&value) noexcept(std::is_nothrow_constructible_v<T, U &&>)
        : storage_(
              std::in_place_index<0>,
              std::forward<U>(value))
    {
    }

    template <typename G>
      requires std::constructible_from<E, G &&>
    explicit constexpr Result(ErrorTag, G &&error) noexcept(std::is_nothrow_constructible_v<E, G &&>)
        : storage_(
              std::in_place_index<1>,
              std::forward<G>(error))
    {
    }

    std::variant<T, E> storage_;
  };

  /**
   * @brief Represents success without a value or failure with an error.
   *
   * Result<void, E> is intended for operations whose successful completion
   * is itself the result.
   *
   * No placeholder value is required on success.
   *
   * @tparam E Error type produced on failure.
   */
  template <typename E>
  class [[nodiscard]] Result<void, E>
  {
    static_assert(!std::is_void_v<E>,
                  "Result error type must not be void.");

    static_assert(!std::is_reference_v<E>,
                  "Result error type must not be a reference.");

    static_assert(!std::is_array_v<E>,
                  "Result error type must not be an array.");

  public:
    /**
     * @brief Successful value type.
     */
    using value_type = void;

    /**
     * @brief Type stored by a failed Result.
     */
    using error_type = E;

    /**
     * @brief Creates a successful Result.
     *
     * @return A successful Result.
     */
    [[nodiscard]] static constexpr Result success() noexcept
    {
      return Result(SuccessTag{});
    }

    /**
     * @brief Creates a failed Result.
     *
     * @tparam G Type used to construct E.
     * @param error Error to store.
     * @return A Result containing the constructed error.
     */
    template <typename G>
      requires std::constructible_from<E, G &&>
    [[nodiscard]] static constexpr Result failure(G &&error) noexcept(std::is_nothrow_constructible_v<E, G &&>)
    {
      return Result(ErrorTag{}, std::forward<G>(error));
    }

    Result(const Result &) = default;
    Result(Result &&) noexcept(
        std::is_nothrow_move_constructible_v<E>) = default;

    Result &operator=(const Result &) = default;
    Result &operator=(Result &&) noexcept(
        std::is_nothrow_move_assignable_v<E>) = default;

    ~Result() = default;

    /**
     * @brief Checks whether the operation succeeded.
     *
     * @return true when successful, otherwise false.
     */
    [[nodiscard]] constexpr bool has_value() const noexcept
    {
      return storage_.index() == 0;
    }

    /**
     * @brief Checks whether the operation failed.
     *
     * @return true when an error is stored.
     */
    [[nodiscard]] constexpr bool has_error() const noexcept
    {
      return !has_value();
    }

    /**
     * @brief Tests whether the Result represents success.
     *
     * @return true when successful.
     */
    [[nodiscard]] explicit constexpr operator bool() const noexcept
    {
      return has_value();
    }

    /**
     * @brief Verifies that the Result represents success.
     *
     * @throws std::bad_variant_access if the Result contains an error.
     */
    constexpr void value() const
    {
      static_cast<void>(std::get<0>(storage_));
    }

    /**
     * @brief Returns the stored error.
     *
     * @return Reference to the stored error.
     *
     * @throws std::bad_variant_access if the Result represents success.
     */
    [[nodiscard]] constexpr E &error() &
    {
      return std::get<1>(storage_);
    }

    /**
     * @brief Returns the stored error.
     *
     * @return Const reference to the stored error.
     *
     * @throws std::bad_variant_access if the Result represents success.
     */
    [[nodiscard]] constexpr const E &error() const &
    {
      return std::get<1>(storage_);
    }

    /**
     * @brief Returns the stored error by move.
     *
     * @return Rvalue reference to the stored error.
     *
     * @throws std::bad_variant_access if the Result represents success.
     */
    [[nodiscard]] constexpr E &&error() &&
    {
      return std::get<1>(std::move(storage_));
    }

    /**
     * @brief Returns a pointer to the stored error when present.
     *
     * @return Pointer to the error, or nullptr on success.
     */
    [[nodiscard]] constexpr E *error_if() noexcept
    {
      return std::get_if<1>(&storage_);
    }

    /**
     * @brief Returns a pointer to the stored error when present.
     *
     * @return Const pointer to the error, or nullptr on success.
     */
    [[nodiscard]] constexpr const E *error_if() const noexcept
    {
      return std::get_if<1>(&storage_);
    }

    /**
     * @brief Transforms successful completion into another value.
     *
     * The callable is invoked only when this Result represents success.
     *
     * @tparam F Callable type.
     * @param function Transformation to invoke.
     * @return A transformed Result or a copy of the existing error.
     */
    template <typename F>
      requires std::invocable<F> &&
               std::copy_constructible<E>
    [[nodiscard]] constexpr auto map(F &&function) const &
    {
      using InvokeResult = std::invoke_result_t<F>;
      using U = std::remove_cvref_t<InvokeResult>;

      if constexpr (std::is_void_v<InvokeResult>)
      {
        if (has_value())
        {
          std::invoke(std::forward<F>(function));
          return Result<void, E>::success();
        }

        return Result<void, E>::failure(error());
      }
      else
      {
        if (has_value())
        {
          return Result<U, E>::success(
              std::invoke(std::forward<F>(function)));
        }

        return Result<U, E>::failure(error());
      }
    }

    /**
     * @brief Chains another Result-producing operation.
     *
     * The callable is invoked only when this Result represents success.
     * The returned Result must use the same error type E.
     *
     * @tparam F Callable returning another Result.
     * @param function Operation to invoke on success.
     * @return Result returned by the callable, or the current error.
     */
    template <typename F>
      requires std::invocable<F> &&
               std::copy_constructible<E>
    [[nodiscard]] constexpr auto and_then(F &&function) const &
    {
      using Next =
          std::remove_cvref_t<std::invoke_result_t<F>>;

      static_assert(
          detail::is_result_v<Next>,
          "Result::and_then requires a callable returning vix::Result.");

      if constexpr (detail::is_result_v<Next>)
      {
        static_assert(
            std::same_as<typename Next::error_type, E>,
            "Result::and_then requires the same error type.");

        if (has_value())
        {
          return std::invoke(std::forward<F>(function));
        }

        return Next::failure(error());
      }
    }

    /**
     * @brief Chains another Result-producing operation by move.
     *
     * @tparam F Callable returning another Result.
     * @param function Operation to invoke on success.
     * @return Result returned by the callable, or the current error by move.
     */
    template <typename F>
      requires std::invocable<F> &&
               std::move_constructible<E>
    [[nodiscard]] constexpr auto and_then(F &&function) &&
    {
      using Next =
          std::remove_cvref_t<std::invoke_result_t<F>>;

      static_assert(
          detail::is_result_v<Next>,
          "Result::and_then requires a callable returning vix::Result.");

      if constexpr (detail::is_result_v<Next>)
      {
        static_assert(
            std::same_as<typename Next::error_type, E>,
            "Result::and_then requires the same error type.");

        if (has_value())
        {
          return std::invoke(std::forward<F>(function));
        }

        return Next::failure(
            std::move(*this).error());
      }
    }

  private:
    struct SuccessTag
    {
    };

    struct ErrorTag
    {
    };

    explicit constexpr Result(SuccessTag) noexcept
        : storage_(std::in_place_index<0>)
    {
    }

    template <typename G>
      requires std::constructible_from<E, G &&>
    explicit constexpr Result(ErrorTag, G &&error) noexcept(std::is_nothrow_constructible_v<E, G &&>)
        : storage_(
              std::in_place_index<1>,
              std::forward<G>(error))
    {
    }

    std::variant<std::monostate, E> storage_;
  };

} // namespace vix

#endif // VIX_RESULT_HPP
