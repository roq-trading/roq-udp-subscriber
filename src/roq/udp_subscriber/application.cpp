/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/application.hpp"

#include "roq/udp_subscriber/config.hpp"
#include "roq/udp_subscriber/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === CONSTANTS ===

namespace {
auto const SETTINGS = server::Settings{
    .package_name = ROQ_PACKAGE_NAME,
    .build_number = ROQ_BUILD_NUMBER,
    .api = {},
    .type = server::Type::ORDER_MANAGEMENT,
};
}  // namespace

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  Config config;
  auto context = server::create_io_context();
  server::Router<Gateway>{SETTINGS, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace udp_subscriber
}  // namespace roq
