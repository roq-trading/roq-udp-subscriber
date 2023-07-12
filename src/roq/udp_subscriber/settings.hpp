/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/udp_subscriber/flags/common.hpp"

namespace roq {
namespace udp_subscriber {

struct Settings final : public server::flags::Settings {
  explicit Settings(args::Parser const &, server::Type);

  std::string_view exchange;

  flags::Common__flags common;
};

}  // namespace udp_subscriber
}  // namespace roq

template <>
struct fmt::formatter<roq::udp_subscriber::Settings> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::udp_subscriber::Settings const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange="{}", )"
        R"(common={}, )"
        R"(server={})"
        R"(}})"_cf,
        value.exchange,
        value.common,
        static_cast<roq::server::Settings const &>(value));
  }
};
