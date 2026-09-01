/**
 *
 *  @file service.cpp
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

#include <vix/Duration.hpp>
#include <vix/Result.hpp>
#include <vix/Run.hpp>

#include <iostream>

namespace
{
  enum class ServiceError
  {
    invalid_interval
  };

  class Service
  {
  public:
    explicit Service(
        vix::Duration interval) noexcept
        : interval_(interval)
    {
    }

    [[nodiscard]] vix::Result<void, ServiceError> run()
    {
      if (!interval_.is_positive())
      {
        return vix::Result<
            void,
            ServiceError>::failure(ServiceError::invalid_interval);
      }

      std::cout
          << "service started with interval "
          << interval_.count_milliseconds()
          << " ms\n";

      return vix::Result<
          void,
          ServiceError>::success();
    }

  private:
    vix::Duration interval_;
  };

} // namespace

int main()
{
  Service service{
      vix::Duration::seconds(5)};

  auto result =
      vix::run(service);

  if (!result)
  {
    std::cerr
        << "service failed to start\n";

    return 1;
  }

  return 0;
}
