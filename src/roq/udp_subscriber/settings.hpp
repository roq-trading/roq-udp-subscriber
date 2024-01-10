/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/udp_subscriber/flags/common.hpp"
#include "roq/udp_subscriber/flags/flags.hpp"

namespace roq {
namespace udp_subscriber {

struct Settings final : public server::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::Common common;
};

}  // namespace udp_subscriber
}  // namespace roq

template <>
struct fmt::formatter<roq::udp_subscriber::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::udp_subscriber::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange="{}", )"
        R"(common={}, )"
        R"(server={})"
        R"(}})"sv,
        value.exchange,
        value.common,
        static_cast<roq::server::Settings const &>(value));
  }
};
