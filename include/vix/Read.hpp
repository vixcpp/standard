/**
 *
 *  @file Read.hpp
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

#ifndef VIX_READ_HPP
#define VIX_READ_HPP

#include <vix/Input.hpp>

#include <cstddef>
#include <span>

namespace vix
{
  /**
   * @brief Reads bytes from an input source.
   *
   * read() is the general Vix operation for transferring bytes from an Input
   * into a caller-owned buffer.
   *
   * The operation delegates to the source's read() implementation while
   * preserving its error type and read semantics.
   *
   * A successful result contains the number of bytes written into @p buffer.
   * The returned count must not exceed the size of the supplied span.
   *
   * A successful result containing zero indicates that the synchronous input
   * source has no more bytes available.
   *
   * read() performs no allocation and introduces no intermediate buffer.
   *
   * @tparam I Input source type.
   * @param input Source from which bytes are read.
   * @param buffer Destination buffer owned by the caller.
   * @return Number of bytes read, or the source-specific error.
   *
   * @note A successful read may fill only part of the supplied buffer.
   *       Callers that require a complete buffer must continue reading until
   *       the requested amount has been obtained, the source reaches its end,
   *       or an error occurs.
   */
  template <Input I>
  [[nodiscard]] constexpr auto read(
      I &input,
      std::span<std::byte> buffer) noexcept(noexcept(input.read(buffer)))
      -> Result<std::size_t, input_error_t<I>>
  {
    return input.read(buffer);
  }

  /**
   * @brief Reads bytes from a const-capable input source.
   *
   * This overload is available only for sources whose read operation is
   * intentionally callable through a const object.
   *
   * The operation preserves the source's error type and performs no
   * allocation or intermediate buffering.
   *
   * @tparam I Const-capable input source type.
   * @param input Source from which bytes are read.
   * @param buffer Destination buffer owned by the caller.
   * @return Number of bytes read, or the source-specific error.
   *
   * @note Most resource-backed inputs are stateful and are expected to use the
   *       non-const overload instead.
   */
  template <ConstInput I>
  [[nodiscard]] constexpr auto read(
      const I &input,
      std::span<std::byte> buffer) noexcept(noexcept(input.read(buffer)))
      -> Result<std::size_t, input_error_t<I>>
  {
    return input.read(buffer);
  }

} // namespace vix

#endif // VIX_READ_HPP
