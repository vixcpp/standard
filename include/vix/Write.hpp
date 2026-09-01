/**
 *
 *  @file Write.hpp
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

#ifndef VIX_WRITE_HPP
#define VIX_WRITE_HPP

#include <vix/Output.hpp>

#include <cstddef>
#include <span>

namespace vix
{
  /**
   * @brief Writes bytes to an output destination.
   *
   * write() is the general Vix operation for transferring bytes from a
   * caller-owned buffer to an Output.
   *
   * The operation delegates to the destination's write() implementation while
   * preserving its error type and write semantics.
   *
   * A successful result contains the number of bytes consumed from @p data.
   * The returned count must not exceed the size of the supplied span.
   *
   * A successful write may consume fewer bytes than requested. Callers that
   * require complete delivery must continue writing the remaining bytes until
   * the entire span has been consumed or an error occurs.
   *
   * write() performs no allocation and introduces no intermediate buffer.
   *
   * @tparam O Output destination type.
   * @param output Destination to which bytes are written.
   * @param data Source bytes owned by the caller.
   * @return Number of bytes written, or the destination-specific error.
   *
   * @note A zero-byte successful write for a non-empty span does not indicate
   *       completion. Implementations should avoid reporting zero progress
   *       unless that behavior is explicitly meaningful for the destination.
   */
  template <Output O>
  [[nodiscard]] constexpr auto write(
      O &output,
      std::span<const std::byte> data) noexcept(noexcept(output.write(data)))
      -> Result<std::size_t, output_error_t<O>>
  {
    return output.write(data);
  }

  /**
   * @brief Writes bytes to a const-capable output destination.
   *
   * This overload is available only for destinations whose write operation is
   * intentionally callable through a const object.
   *
   * The operation preserves the destination's error type and performs no
   * allocation or intermediate buffering.
   *
   * @tparam O Const-capable output destination type.
   * @param output Destination to which bytes are written.
   * @param data Source bytes owned by the caller.
   * @return Number of bytes written, or the destination-specific error.
   *
   * @note Most resource-backed outputs are stateful and are expected to use
   *       the non-const overload instead.
   */
  template <ConstOutput O>
  [[nodiscard]] constexpr auto write(
      const O &output,
      std::span<const std::byte> data) noexcept(noexcept(output.write(data)))
      -> Result<std::size_t, output_error_t<O>>
  {
    return output.write(data);
  }

} // namespace vix

#endif // VIX_WRITE_HPP
