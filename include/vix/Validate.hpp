/**
 *
 *  @file Validate.hpp
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

#ifndef VIX_VALIDATE_HPP
#define VIX_VALIDATE_HPP

#include <vix/Result.hpp>

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace vix
{
  namespace detail
  {
    template <typename T>
    struct IsValidationResult : std::false_type
    {
    };

    template <typename E>
    struct IsValidationResult<Result<void, E>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_validation_result_v =
        IsValidationResult<std::remove_cvref_t<T>>::value;
  }

  /**
   * @brief Describes a validation operation for a value.
   *
   * A validator is any callable that accepts a value and returns:
   *
   * @code
   * vix::Result<void, E>
   * @endcode
   *
   * where E is chosen by the validator or by the domain that owns the
   * validation rule.
   *
   * Successful validation carries no value because the original value remains
   * owned by the caller. Failure preserves the validator-specific error type.
   *
   * Validation does not define a universal Vix validation error. A parser,
   * configuration system, schema library, database layer, or application may
   * require different error information.
   *
   * @tparam V Validator callable type.
   * @tparam T Value type accepted by the validator.
   *
   * @note ValidatorFor does not require inheritance, allocation, or a
   *       particular validation framework.
   * @note Validators may be stateful.
   */
  template <typename V, typename T>
  concept ValidatorFor =
      std::invocable<V, T> &&
      detail::is_validation_result_v<std::invoke_result_t<V, T>>;

  /**
   * @brief Extracts the Result type produced by a validator.
   *
   * @tparam V Validator callable type.
   * @tparam T Value type passed to the validator.
   */
  template <typename V, typename T>
    requires ValidatorFor<V, T>
  using validation_result_t =
      std::remove_cvref_t<std::invoke_result_t<V, T>>;

  /**
   * @brief Extracts the error type produced by a validator.
   *
   * @tparam V Validator callable type.
   * @tparam T Value type passed to the validator.
   */
  template <typename V, typename T>
    requires ValidatorFor<V, T>
  using validation_error_t =
      typename validation_result_t<V, T>::error_type;

  /**
   * @brief Validates a value using a validator.
   *
   * validate() is the general Vix operation for applying a validation rule
   * without imposing a domain-specific schema, error type, or object model.
   *
   * The validator receives the value and returns:
   *
   * @code
   * vix::Result<void, Error>
   * @endcode
   *
   * A successful result means the value satisfies the rule. A failed result
   * contains the validator-specific error.
   *
   * For example:
   *
   * @code
   * enum class PortError
   * {
   *   out_of_range
   * };
   *
   * auto valid_port = [](int port) -> vix::Result<void, PortError>
   * {
   *   if (port < 1 || port > 65535)
   *   {
   *     return vix::Result<void, PortError>::failure(
   *       PortError::out_of_range);
   *   }
   *
   *   return vix::Result<void, PortError>::success();
   * };
   *
   * auto result = vix::validate(8080, valid_port);
   * @endcode
   *
   * validate() does not copy, transform, or take ownership of the value beyond
   * what is implied by the argument types of the supplied validator.
   *
   * @tparam T Value argument type.
   * @tparam V Validator callable type.
   * @param value Value to validate.
   * @param validator Validation operation to invoke.
   * @return Result produced by the validator.
   *
   * @note Validation failure is distinct from value absence.
   * @note validate() does not accumulate several errors by itself. Domains
   *       that require error accumulation may use an error type that contains
   *       multiple validation failures.
   */
  template <typename T, typename V>
    requires ValidatorFor<V &&, T &&>
  [[nodiscard]] constexpr auto validate(
      T &&value,
      V &&validator) noexcept(std::is_nothrow_invocable_v<V &&, T &&>)
      -> validation_result_t<V &&, T &&>
  {
    return std::invoke(
        std::forward<V>(validator),
        std::forward<T>(value));
  }

} // namespace vix

#endif // VIX_VALIDATE_HPP
