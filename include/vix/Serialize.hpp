/**
 *
 *  @file Serialize.hpp
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

#ifndef VIX_SERIALIZE_HPP
#define VIX_SERIALIZE_HPP

#include <vix/Result.hpp>

#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace vix
{
  /**
   * @brief Identifies why a value could not be serialized.
   */
  enum class SerializeErrorCode
  {
    /**
     * @brief The destination representation could not hold the serialized
     *        value.
     */
    value_too_large,

    /**
     * @brief The underlying conversion failed for an unspecified reason.
     */
    conversion_failed
  };

  /**
   * @brief Describes a failure produced while serializing a value.
   *
   * SerializeError represents an operational failure of the textual
   * serialization process.
   *
   * The error does not own or retain the original value.
   */
  struct [[nodiscard]] SerializeError final
  {
    /**
     * @brief Reason the serialization operation failed.
     */
    SerializeErrorCode code{SerializeErrorCode::conversion_failed};

    /**
     * @brief Compares two serialization errors.
     */
    [[nodiscard]] friend constexpr bool operator==(
        const SerializeError &,
        const SerializeError &) noexcept = default;
  };

  namespace detail
  {
    template <typename T>
    inline constexpr bool is_serialize_integral_v =
        std::integral<T> &&
        !std::same_as<std::remove_cv_t<T>, bool>;

    template <typename T>
    inline constexpr bool is_serialize_floating_v =
        std::floating_point<T>;

    template <typename T>
    inline constexpr bool is_serialize_string_v =
        std::same_as<std::remove_cv_t<T>, std::string>;

    template <typename T>
    inline constexpr bool is_serialize_string_view_v =
        std::same_as<std::remove_cv_t<T>, std::string_view>;

    template <typename T>
    inline constexpr bool is_serialize_supported_v =
        is_serialize_integral_v<T> ||
        is_serialize_floating_v<T> ||
        is_serialize_string_v<T> ||
        is_serialize_string_view_v<T> ||
        std::same_as<std::remove_cv_t<T>, bool>;

    /**
     * @brief Buffer size used for built-in arithmetic serialization.
     *
     * The buffer is intentionally local and fixed-size so arithmetic
     * conversion itself requires no allocation.
     */
    inline constexpr std::size_t serialize_buffer_size = 128;

    [[nodiscard]] constexpr SerializeError make_serialize_error(
        SerializeErrorCode code) noexcept
    {
      return SerializeError{
          .code = code};
    }

    template <typename T>
    [[nodiscard]] Result<std::string, SerializeError>
    serialize_integral(T value)
    {
      std::array<char, serialize_buffer_size> buffer{};

      const auto result = std::to_chars(
          buffer.data(),
          buffer.data() + buffer.size(),
          value);

      if (result.ec == std::errc::value_too_large)
      {
        return Result<std::string, SerializeError>::failure(
            make_serialize_error(
                SerializeErrorCode::value_too_large));
      }

      if (result.ec != std::errc{})
      {
        return Result<std::string, SerializeError>::failure(
            make_serialize_error(
                SerializeErrorCode::conversion_failed));
      }

      return Result<std::string, SerializeError>::success(
          std::string(
              buffer.data(),
              static_cast<std::size_t>(
                  result.ptr - buffer.data())));
    }

    template <typename T>
    [[nodiscard]] Result<std::string, SerializeError>
    serialize_floating(T value)
    {
      std::array<char, serialize_buffer_size> buffer{};

      const auto result = std::to_chars(
          buffer.data(),
          buffer.data() + buffer.size(),
          value,
          std::chars_format::general);

      if (result.ec == std::errc::value_too_large)
      {
        return Result<std::string, SerializeError>::failure(
            make_serialize_error(
                SerializeErrorCode::value_too_large));
      }

      if (result.ec != std::errc{})
      {
        return Result<std::string, SerializeError>::failure(
            make_serialize_error(
                SerializeErrorCode::conversion_failed));
      }

      return Result<std::string, SerializeError>::success(
          std::string(
              buffer.data(),
              static_cast<std::size_t>(
                  result.ptr - buffer.data())));
    }

  } // namespace detail

  /**
   * @brief Identifies types supported by the built-in Standard serializer.
   *
   * The initial Standard serialization surface supports:
   *
   * - integral types other than bool
   * - floating-point types
   * - bool
   * - std::string
   * - std::string_view
   *
   * Specialized formats such as JSON, XML, protocol messages, database
   * records, or application-specific encodings should remain in their
   * respective domains.
   *
   * @tparam T Candidate source type.
   */
  template <typename T>
  concept Serializable =
      detail::is_serialize_supported_v<std::remove_cvref_t<T>>;

  /**
   * @brief Serializes a value into its canonical textual representation.
   *
   * serialize() provides the built-in Standard conversion from supported C++
   * values to text.
   *
   * Arithmetic conversion is locale-independent and uses std::to_chars.
   *
   * Boolean values are represented exactly as:
   *
   * @code
   * true
   * false
   * @endcode
   *
   * String values are copied unchanged.
   *
   * For example:
   *
   * @code
   * auto text = vix::serialize(8080);
   *
   * if (text)
   * {
   *   // text.value() == "8080"
   * }
   * @endcode
   *
   * @tparam T Source value type.
   * @param value Value to serialize.
   * @return Textual representation, or SerializeError when conversion fails.
   *
   * @note serialize() produces text. It does not write the result to an
   *       Output.
   * @note serialize() does not implement domain-specific formats.
   * @note Construction of the returned std::string may allocate.
   */
  template <Serializable T>
  [[nodiscard]] Result<std::string, SerializeError> serialize(
      T &&value)
  {
    using Value = std::remove_cvref_t<T>;

    if constexpr (detail::is_serialize_integral_v<Value>)
    {
      return detail::serialize_integral<Value>(
          static_cast<Value>(value));
    }
    else if constexpr (detail::is_serialize_floating_v<Value>)
    {
      return detail::serialize_floating<Value>(
          static_cast<Value>(value));
    }
    else if constexpr (std::same_as<Value, bool>)
    {
      return Result<std::string, SerializeError>::success(
          value ? std::string("true") : std::string("false"));
    }
    else if constexpr (std::same_as<Value, std::string>)
    {
      return Result<std::string, SerializeError>::success(
          std::string(std::forward<T>(value)));
    }
    else
    {
      static_assert(
          std::same_as<Value, std::string_view>,
          "Unsupported vix::serialize source type.");

      return Result<std::string, SerializeError>::success(
          std::string(value));
    }
  }

} // namespace vix

#endif // VIX_SERIALIZE_HPP
