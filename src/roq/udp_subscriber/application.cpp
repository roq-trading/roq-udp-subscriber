/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/application.hpp"

#include "roq/udp_subscriber/config.hpp"
#include "roq/udp_subscriber/gateway.hpp"
#include "roq/udp_subscriber/settings.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === CONSTANTS ===

namespace {
auto const TYPE = server::Type::ORDER_MANAGEMENT;
}  // namespace

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  Settings settings{TYPE};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Router<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace udp_subscriber
}  // namespace roq
