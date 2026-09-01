/**
 *
 *  @file library.cpp
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
  enum class PortError
  {
    out_of_range
  };

  struct PortValidator
  {
    [[nodiscard]] vix::Result<void, PortError> operator()(
        int port) const
    {
      if (port < 1 || port > 65535)
      {
        return vix::Result<
            void,
            PortError>::failure(PortError::out_of_range);
      }

      return vix::Result<
          void,
          PortError>::success();
    }
  };

  void inspect_port(
      std::string_view text)
  {
    auto parsed =
        vix::parse<int>(text);

    if (!parsed)
    {
      std::cout << "invalid port\n";
      return;
    }

    const int port =
        parsed.value();

    auto valid =
        vix::validate(
            port,
            PortValidator{});

    if (!valid)
    {
      std::cout << "port out of range\n";
      return;
    }

    auto serialized =
        vix::serialize(port);

    if (!serialized)
    {
      std::cout << "serialization failed\n";
      return;
    }

    std::cout
        << "port: "
        << serialized.value()
        << '\n';
  }

} // namespace

int main()
{
  inspect_port("8080");
  inspect_port("70000");
  inspect_port("invalid");

  return 0;
}
