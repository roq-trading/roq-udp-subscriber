/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/udp_subscriber/application.hpp"

#include "roq/udp_subscriber/config.hpp"
#include "roq/udp_subscriber/gateway.hpp"
#include "roq/udp_subscriber/settings.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Router<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace udp_subscriber
}  // namespace roq
