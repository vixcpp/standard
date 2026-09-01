/**
 *
 *  @file Print.hpp
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

#ifndef VIX_PRINT_HPP
#define VIX_PRINT_HPP

#include <vix/Output.hpp>
#include <vix/Write.hpp>

#include <cstddef>
#include <span>
#include <string_view>

namespace vix
{
  /**
   * @brief Presents text to an output destination.
   *
   * print() is the general Vix operation for sending textual content to an
   * Output.
   *
   * Unlike write(), which operates directly on arbitrary bytes, print()
   * expresses that the supplied data is text intended for presentation.
   *
   * The text is viewed as bytes and forwarded directly to the destination.
   * No intermediate string or buffer is created.
   *
   * print() does not:
   *
   * - append a newline
   * - format arbitrary values
   * - serialize structured data
   * - select a global output destination
   * - flush the destination
   * - guarantee that the complete text is consumed in one operation
   *
   * A successful result contains the number of bytes consumed by the output.
   * The value may be smaller than text.size() when the underlying destination
   * performs a partial write.
   *
   * For example:
   *
   * @code
   * auto result = vix::print(output, "Hello");
   *
   * if (!result)
   * {
   *   // inspect result.error()
   * }
   * @endcode
   *
   * Text conversion and presentation remain separate concerns. A value that
   * is not already text should first be converted through an appropriate
   * representation operation before being passed to print().
   *
   * @tparam O Output destination type.
   * @param output Destination that receives the text.
   * @param text Text to present.
   * @return Number of bytes consumed, or the destination-specific error.
   *
   * @note print() performs no dynamic allocation.
   * @note print() preserves the error type of the destination.
   * @note print() does not own or retain the supplied text.
   */
  template <Output O>
  [[nodiscard]] auto print(
      O &output,
      std::string_view text) noexcept(noexcept(vix::write(output,
                                                          std::as_bytes(
                                                              std::span<const char>(
                                                                  text.data(),
                                                                  text.size())))))
      -> Result<std::size_t, output_error_t<O>>
  {
    const auto bytes = std::as_bytes(
        std::span<const char>(
            text.data(),
            text.size()));

    return vix::write(output, bytes);
  }

  /**
   * @brief Presents text to a const-capable output destination.
   *
   * This overload is available only for destinations whose write operation is
   * intentionally callable through a const object.
   *
   * The operation has the same semantics as the mutable-output overload and
   * performs no allocation or intermediate buffering.
   *
   * @tparam O Const-capable output destination type.
   * @param output Destination that receives the text.
   * @param text Text to present.
   * @return Number of bytes consumed, or the destination-specific error.
   *
   * @note Most resource-backed outputs are stateful and are expected to use
   *       the non-const overload instead.
   */
  template <ConstOutput O>
  [[nodiscard]] auto print(
      const O &output,
      std::string_view text) noexcept(noexcept(vix::write(output,
                                                          std::as_bytes(
                                                              std::span<const char>(
                                                                  text.data(),
                                                                  text.size())))))
      -> Result<std::size_t, output_error_t<O>>
  {
    const auto bytes = std::as_bytes(
        std::span<const char>(
            text.data(),
            text.size()));

    return vix::write(output, bytes);
  }

} // namespace vix

#endif // VIX_PRINT_HPP
