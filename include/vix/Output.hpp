/**
 *
 *  @file Output.hpp
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

#ifndef VIX_OUTPUT_HPP
#define VIX_OUTPUT_HPP

#include <vix/Result.hpp>

#include <concepts>
#include <cstddef>
#include <span>
#include <type_traits>

namespace vix
{
  /**
   * @brief Extracts the error type exposed by an output destination.
   *
   * @tparam T Output destination type.
   */
  template <typename T>
  using output_error_t =
      typename std::remove_cvref_t<T>::error_type;

  /**
   * @brief Describes a synchronous destination for bytes.
   *
   * Output is the common Vix contract for objects that can consume bytes
   * supplied by a caller.
   *
   * An output destination exposes its own error type and implements:
   *
   * @code
   * vix::Result<std::size_t, error_type>
   * write(std::span<const std::byte> data);
   * @endcode
   *
   * The caller owns the source buffer. Output does not require allocation,
   * buffering, flushing, seeking, ownership of the underlying resource, or
   * a particular transport.
   *
   * This allows independent domains such as files, memory buffers, process
   * streams, sockets, encoders, and adapters to participate in the same
   * byte-output model without sharing a domain-specific base class.
   *
   * A successful write returns the number of bytes consumed from the supplied
   * span. The returned count must never exceed the span size.
   *
   * A successful write may consume fewer bytes than requested. Callers that
   * require complete delivery must continue writing the remaining bytes until
   * the input span has been fully consumed or an error occurs.
   *
   * A zero-byte successful write for a non-empty span does not indicate
   * completion. Implementations should avoid reporting zero progress unless
   * that behavior is explicitly meaningful for the underlying synchronous
   * destination.
   *
   * Errors remain owned by the destination domain through its error_type.
   *
   * @tparam T Candidate output destination type.
   *
   * @note Output describes behavior only. It does not require inheritance,
   *       virtual dispatch, dynamic allocation, or shared ownership.
   * @note Output is intentionally synchronous. Asynchronous destinations
   *       should compose with the asynchronous execution model rather than
   *       extend this contract with additional states.
   */
  template <typename T>
  concept Output =
      requires(
          std::remove_reference_t<T> &output,
          std::span<const std::byte> data) {
        typename std::remove_cvref_t<T>::error_type;

        {
          output.write(data)
        } -> std::same_as<
            Result<
                std::size_t,
                typename std::remove_cvref_t<T>::error_type>>;
      };

  /**
   * @brief Checks whether an Output can be written through a const object.
   *
   * Most resource-backed outputs are stateful and therefore do not satisfy
   * this concept. It is provided for destinations whose mutation is external
   * to the object or whose write operation is intentionally const.
   *
   * @tparam T Candidate output destination type.
   */
  template <typename T>
  concept ConstOutput =
      requires(
          const std::remove_reference_t<T> &output,
          std::span<const std::byte> data) {
        typename std::remove_cvref_t<T>::error_type;

        {
          output.write(data)
        } -> std::same_as<
            Result<
                std::size_t,
                typename std::remove_cvref_t<T>::error_type>>;
      };

} // namespace vix

#endif // VIX_OUTPUT_HPP
