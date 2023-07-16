/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER}, flags::Flags{flags::Flags::create()},
      common{flags::Common::create()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace udp_subscriber
}  // namespace roq
