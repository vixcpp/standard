/**
 *
 *  @file Run.hpp
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

#ifndef VIX_RUN_HPP
#define VIX_RUN_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace vix
{
  /**
   * @brief Describes an object that exposes an explicit run operation.
   *
   * RunnableWith is satisfied when an object can be invoked through a member
   * function named run() with the supplied arguments.
   *
   * The concept deliberately does not prescribe what run() returns. Different
   * domains may represent execution results differently while still sharing
   * the same basic operation.
   *
   * Examples of potentially runnable objects include command executors,
   * application components, engines, jobs, or other objects whose public
   * semantics explicitly define a run operation.
   *
   * RunnableWith does not imply:
   *
   * - asynchronous execution
   * - background execution
   * - process creation
   * - thread creation
   * - ownership transfer
   * - scheduling
   * - cancellation
   *
   * Those semantics remain the responsibility of the object being run.
   *
   * @tparam T Candidate runnable object type.
   * @tparam Args Argument types supplied to run().
   *
   * @note This concept does not require inheritance, virtual dispatch,
   *       allocation, or a Vix runtime.
   */
  template <typename T, typename... Args>
  concept RunnableWith =
      requires(T &&value, Args &&...args) {
        std::forward<T>(value).run(
            std::forward<Args>(args)...);
      };

  /**
   * @brief Describes an object that can be run without arguments.
   *
   * @tparam T Candidate runnable object type.
   */
  template <typename T>
  concept Runnable = RunnableWith<T>;

  /**
   * @brief Extracts the type returned by a run operation.
   *
   * @tparam T Runnable object type.
   * @tparam Args Argument types supplied to run().
   */
  template <typename T, typename... Args>
    requires RunnableWith<T, Args...>
  using run_result_t =
      decltype(std::declval<T>().run(
          std::declval<Args>()...));

  /**
   * @brief Executes the explicit run operation of an object.
   *
   * run() provides the general Vix expression for objects whose public
   * semantics already define a run operation.
   *
   * The function forwards directly to the object's run() member without
   * introducing scheduling, allocation, buffering, ownership changes, or
   * result adaptation.
   *
   * The exact return type is preserved.
   *
   * For example:
   *
   * @code
   * class Tool
   * {
   * public:
   *   int run()
   *   {
   *     return 0;
   *   }
   * };
   *
   * Tool tool;
   * auto status = vix::run(tool);
   * @endcode
   *
   * A domain may return a richer type:
   *
   * @code
   * vix::Result<Execution, ProcessError> run();
   * @endcode
   *
   * and vix::run() preserves that type unchanged.
   *
   * @tparam T Runnable object type.
   * @tparam Args Argument types supplied to the run operation.
   * @param value Object whose run operation is invoked.
   * @param args Arguments forwarded to the object's run operation.
   * @return Exactly what the object's run() member returns.
   *
   * @note run() does not turn arbitrary callables into Runnable objects.
   *       Use std::invoke for general callable invocation.
   * @note The existence of vix::run() does not make domain-specific execution
   *       semantics part of Standard.
   */
  template <typename T, typename... Args>
    requires RunnableWith<T &&, Args &&...>
  constexpr decltype(auto) run(
      T &&value,
      Args &&...args) noexcept(noexcept(std::forward<T>(value).run(std::forward<Args>(args)...)))
  {
    return std::forward<T>(value).run(
        std::forward<Args>(args)...);
  }

} // namespace vix

#endif // VIX_RUN_HPP
