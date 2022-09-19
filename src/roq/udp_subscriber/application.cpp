/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/application.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/udp_subscriber/config.hpp"
#include "roq/udp_subscriber/flags.hpp"
#include "roq/udp_subscriber/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"sv, Flags::config_file());
  Config config(Flags::config_file());
  log::info<1>("config={}"sv, config);
  auto context = io::engine::ContextFactory::create(server::Flags::io_backend());
  log::info("Starting the gateway"sv);
  server::Settings settings{
      .package_name = ROQ_PACKAGE_NAME,
      .build_number = ROQ_BUILD_NUMBER,
      .api = {},
      .type = server::Type::ORDER_MANAGEMENT,
  };
  server::Router<Gateway>(settings, config, *context).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace udp_subscriber
}  // namespace roq
