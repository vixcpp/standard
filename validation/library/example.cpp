/**
 *
 *  @file example.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/vixcpp/standard
 *
 *  Licensed under the MIT License.
 *  See the LICENSE file in the project root for license information.
 *
 *  Vix.cpp Standard
 */

#include "service_config/ServiceConfig.hpp"
#include <iostream>

int main()
{
  auto config = service_config::ServiceConfig::create("127.0.0.1", "8080", "5000");

  if (!config)
  {
    std::cerr << "failed to create service configuration\n";

    return 1;
  }

  std::cout << "host: " << config.value().host() << '\n';
  std::cout << "port: " << config.value().port() << '\n';

  std::cout << "timeout: " << config.value().timeout().count_milliseconds() << " ms\n";

  return 0;
}
