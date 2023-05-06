/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/settings.hpp"

#include "roq/udp_subscriber/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

Settings Settings::create(server::Type type) {
  auto settings = server::create_settings(type, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER);
  return {settings};
}

}  // namespace udp_subscriber
}  // namespace roq
