/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/settings.hpp"

#include "roq/logging.hpp"

#include "roq/udp_subscriber/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

Settings::Settings(args::Parser const &args, server::Type type)
    : server::flags::Settings{args, type, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER}, exchange{flags::Flags::exchange()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace udp_subscriber
}  // namespace roq
