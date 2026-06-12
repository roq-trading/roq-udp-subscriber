/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/udp_subscriber/application.hpp"

#include "roq/udp_subscriber/flags/settings.hpp"

#include "roq/udp_subscriber/gateway/config.hpp"
#include "roq/udp_subscriber/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Router<gateway::Controller>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace udp_subscriber
}  // namespace roq
