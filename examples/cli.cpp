/**
 *
 *  @file cli.cpp
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

#include <vix/Parse.hpp>
#include <vix/Result.hpp>
#include <vix/Serialize.hpp>
#include <vix/Validate.hpp>

#include <iostream>
#include <string_view>

namespace
{
  enum class CountError
  {
    out_of_range
  };

  struct CountValidator
  {
    [[nodiscard]] vix::Result<void, CountError> operator()(
        int count) const
    {
      if (count < 1 || count > 100)
      {
        return vix::Result<
            void,
            CountError>::failure(CountError::out_of_range);
      }

      return vix::Result<
          void,
          CountError>::success();
    }
  };

  int execute(
      std::string_view argument)
  {
    auto parsed =
        vix::parse<int>(argument);

    if (!parsed)
    {
      std::cerr
          << "error: expected an integer\n";

      return 1;
    }

    const int count =
        parsed.value();

    auto valid =
        vix::validate(
            count,
            CountValidator{});

    if (!valid)
    {
      std::cerr
          << "error: value must be between 1 and 100\n";

      return 1;
    }

    auto serialized =
        vix::serialize(count);

    if (!serialized)
    {
      std::cerr
          << "error: could not serialize value\n";

      return 1;
    }

    std::cout
        << "count: "
        << serialized.value()
        << '\n';

    return 0;
  }

} // namespace

int main(
    int argc,
    char **argv)
{
  if (argc != 2)
  {
    std::cerr
        << "usage: vix_standard_cli <count>\n";

    return 1;
  }

  return execute(argv[1]);
}
