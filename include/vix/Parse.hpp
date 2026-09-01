/**
 *
 *  @file Parse.hpp
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

#ifndef VIX_PARSE_HPP
#define VIX_PARSE_HPP

#include <vix/Result.hpp>

#include <charconv>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace vix
{
  /**
   * @brief Identifies why a textual value could not be parsed.
   */
  enum class ParseErrorCode
  {
    /**
     * @brief The supplied text is empty.
     */
    empty_input,

    /**
     * @brief The text does not represent a valid value of the requested type.
     */
    invalid_value,

    /**
     * @brief The parsed value cannot be represented by the requested type.
     */
    out_of_range,

    /**
     * @brief Valid input was followed by unexpected characters.
     */
    trailing_characters
  };

  /**
   * @brief Describes a failure produced while parsing text.
   *
   * ParseError contains the reason for the failure and the byte position at
   * which parsing stopped or detected invalid input.
   *
   * The error does not own the source text.
   */
  struct [[nodiscard]] ParseError final
  {
    /**
     * @brief Reason the parse operation failed.
     */
    ParseErrorCode code{ParseErrorCode::invalid_value};

    /**
     * @brief Byte offset associated with the failure.
     *
     * For trailing input, this identifies the first unconsumed byte.
     * For errors that cannot identify a more precise location, the position
     * is zero.
     */
    std::size_t position{0};

    /**
     * @brief Compares two parse errors.
     */
    [[nodiscard]] friend constexpr bool operator==(
        const ParseError &,
        const ParseError &) noexcept = default;
  };

  namespace detail
  {
    template <typename T>
    inline constexpr bool is_parse_integral_v =
        std::integral<T> &&
        !std::same_as<std::remove_cv_t<T>, bool>;

    template <typename T>
    inline constexpr bool is_parse_floating_v =
        std::floating_point<T>;

    template <typename T>
    inline constexpr bool is_parse_string_v =
        std::same_as<std::remove_cv_t<T>, std::string>;

    template <typename T>
    inline constexpr bool is_parse_supported_v =
        is_parse_integral_v<T> ||
        is_parse_floating_v<T> ||
        is_parse_string_v<T> ||
        std::same_as<std::remove_cv_t<T>, bool>;

    [[nodiscard]] constexpr ParseError make_parse_error(
        ParseErrorCode code,
        std::size_t position = 0) noexcept
    {
      return ParseError{
          .code = code,
          .position = position};
    }

    template <typename T>
    [[nodiscard]] Result<T, ParseError> parse_integral(
        std::string_view text)
    {
      if (text.empty())
      {
        return Result<T, ParseError>::failure(
            make_parse_error(ParseErrorCode::empty_input));
      }

      T value{};

      const char *first = text.data();
      const char *last = text.data() + text.size();

      const auto result = std::from_chars(
          first,
          last,
          value,
          10);

      if (result.ec == std::errc::invalid_argument)
      {
        return Result<T, ParseError>::failure(
            make_parse_error(ParseErrorCode::invalid_value));
      }

      if (result.ec == std::errc::result_out_of_range)
      {
        return Result<T, ParseError>::failure(
            make_parse_error(ParseErrorCode::out_of_range));
      }

      if (result.ptr != last)
      {
        return Result<T, ParseError>::failure(
            make_parse_error(
                ParseErrorCode::trailing_characters,
                static_cast<std::size_t>(result.ptr - first)));
      }

      return Result<T, ParseError>::success(value);
    }

    template <typename T>
    [[nodiscard]] Result<T, ParseError> parse_floating(
        std::string_view text)
    {
      if (text.empty())
      {
        return Result<T, ParseError>::failure(
            make_parse_error(ParseErrorCode::empty_input));
      }

      T value{};

      const char *first = text.data();
      const char *last = text.data() + text.size();

      const auto result = std::from_chars(
          first,
          last,
          value,
          std::chars_format::general);

      if (result.ec == std::errc::invalid_argument)
      {
        return Result<T, ParseError>::failure(
            make_parse_error(ParseErrorCode::invalid_value));
      }

      if (result.ec == std::errc::result_out_of_range)
      {
        return Result<T, ParseError>::failure(
            make_parse_error(ParseErrorCode::out_of_range));
      }

      if (result.ptr != last)
      {
        return Result<T, ParseError>::failure(
            make_parse_error(
                ParseErrorCode::trailing_characters,
                static_cast<std::size_t>(result.ptr - first)));
      }

      return Result<T, ParseError>::success(value);
    }

    [[nodiscard]] inline Result<bool, ParseError> parse_bool(
        std::string_view text)
    {
      if (text.empty())
      {
        return Result<bool, ParseError>::failure(
            make_parse_error(ParseErrorCode::empty_input));
      }

      if (text == "true")
      {
        return Result<bool, ParseError>::success(true);
      }

      if (text == "false")
      {
        return Result<bool, ParseError>::success(false);
      }

      return Result<bool, ParseError>::failure(
          make_parse_error(ParseErrorCode::invalid_value));
    }

  } // namespace detail

  /**
   * @brief Identifies types supported by the built-in Standard parser.
   *
   * The initial Standard parsing surface supports:
   *
   * - integral types other than bool
   * - floating-point types
   * - bool
   * - std::string
   *
   * Specialized domains should provide their own parsing APIs when parsing
   * requires domain-specific rules or richer error information.
   *
   * @tparam T Candidate destination type.
   */
  template <typename T>
  concept Parsable =
      detail::is_parse_supported_v<std::remove_cv_t<T>>;

  /**
   * @brief Parses a textual value into a requested C++ type.
   *
   * parse() converts text into a value without throwing for ordinary parse
   * failures. Invalid syntax, range errors, and trailing characters are
   * represented through ParseError.
   *
   * Parsing is strict. The complete input must represent the requested value.
   * Leading or trailing whitespace is not removed implicitly.
   *
   * For example:
   *
   * @code
   * auto port = vix::parse<int>("8080");
   *
   * if (!port)
   * {
   *   // inspect port.error()
   * }
   * @endcode
   *
   * Boolean parsing accepts exactly:
   *
   * @code
   * true
   * false
   * @endcode
   *
   * std::string parsing copies the supplied text unchanged.
   *
   * @tparam T Destination type.
   * @param text Text to parse.
   * @return Parsed value, or ParseError when conversion fails.
   *
   * @note parse() does not trim whitespace.
   * @note Numeric parsing uses locale-independent std::from_chars semantics.
   * @note Domain-specific formats should normally remain in their domain
   *       rather than extending the root parser with unrelated rules.
   */
  template <Parsable T>
  [[nodiscard]] Result<T, ParseError> parse(
      std::string_view text)
  {
    using Value = std::remove_cv_t<T>;

    if constexpr (detail::is_parse_integral_v<Value>)
    {
      return detail::parse_integral<Value>(text);
    }
    else if constexpr (detail::is_parse_floating_v<Value>)
    {
      return detail::parse_floating<Value>(text);
    }
    else if constexpr (std::same_as<Value, bool>)
    {
      return detail::parse_bool(text);
    }
    else
    {
      static_assert(
          std::same_as<Value, std::string>,
          "Unsupported vix::parse destination type.");

      return Result<Value, ParseError>::success(
          std::string(text));
    }
  }

} // namespace vix

#endif // VIX_PARSE_HPP
