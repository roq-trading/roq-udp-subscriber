/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/udp_subscriber/flags/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {
namespace flags {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, ROQ_GIT_DESCRIBE_HASH, {}}, flags::Flags{flags::Flags::create()},
      misc{flags::Misc::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace flags
}  // namespace udp_subscriber
}  // namespace roq
