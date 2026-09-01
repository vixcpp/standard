/**
 *
 *  @file Input.hpp
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

#ifndef VIX_INPUT_HPP
#define VIX_INPUT_HPP

#include <vix/Result.hpp>

#include <concepts>
#include <cstddef>
#include <span>
#include <type_traits>

namespace vix
{
  /**
   * @brief Extracts the error type exposed by an input source.
   *
   * @tparam T Input source type.
   */
  template <typename T>
  using input_error_t =
      typename std::remove_cvref_t<T>::error_type;

  /**
   * @brief Describes a synchronous source of bytes.
   *
   * Input is the common Vix contract for objects that can provide bytes to a
   * caller-owned buffer.
   *
   * An input source exposes its own error type and implements:
   *
   * @code
   * vix::Result<std::size_t, error_type>
   * read(std::span<std::byte> buffer);
   * @endcode
   *
   * The caller owns the destination buffer. Input does not require allocation,
   * buffering, seeking, ownership of the underlying resource, or a particular
   * transport.
   *
   * This allows independent domains such as files, memory buffers, processes,
   * sockets, decompression streams, and adapters to participate in the same
   * byte-input model without sharing a domain-specific base class.
   *
   * A successful read returns the number of bytes written into the supplied
   * buffer. The returned count must never exceed the buffer size.
   *
   * A successful result containing zero indicates that no more bytes are
   * available from a completed synchronous input source. Sources with
   * non-blocking or asynchronous behavior should expose those semantics
   * through a more appropriate domain-specific abstraction rather than
   * redefining zero-byte reads.
   *
   * Errors remain owned by the source domain through its error_type.
   *
   * @tparam T Candidate input source type.
   *
   * @note Input describes behavior only. It does not require inheritance,
   *       virtual dispatch, dynamic allocation, or shared ownership.
   * @note Input is intentionally synchronous. Asynchronous byte sources should
   *       compose with the asynchronous execution model rather than overload
   *       this contract with additional states.
   */
  template <typename T>
  concept Input =
      requires(
          std::remove_reference_t<T> &input,
          std::span<std::byte> buffer) {
        typename std::remove_cvref_t<T>::error_type;

        {
          input.read(buffer)
        } -> std::same_as<
            Result<
                std::size_t,
                typename std::remove_cvref_t<T>::error_type>>;
      };

  /**
   * @brief Checks whether an Input can be read through a const object.
   *
   * Most resource-backed inputs are stateful and therefore do not satisfy this
   * concept. It is provided for immutable or externally state-managed sources
   * whose read operation is intentionally const.
   *
   * @tparam T Candidate input source type.
   */
  template <typename T>
  concept ConstInput =
      requires(
          const std::remove_reference_t<T> &input,
          std::span<std::byte> buffer) {
        typename std::remove_cvref_t<T>::error_type;

        {
          input.read(buffer)
        } -> std::same_as<
            Result<
                std::size_t,
                typename std::remove_cvref_t<T>::error_type>>;
      };

} // namespace vix

#endif // VIX_INPUT_HPP
