/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <fmt/ranges.h>

#include <toml++/toml.h>

#include <string>
#include <string_view>
#include <vector>

#include "roq/logging.hpp"
#include "roq/server.hpp"

#include "roq/server/config/dispatcher.hpp"
#include "roq/server/config/reader.hpp"

namespace roq {
namespace udp_subscriber {

struct Config final : public server::config::Dispatcher, public server::config::Reader::Handler {
  Config();

  std::string get_master_account() const;

  std::string get_api_key(std::string_view const &account) const;

 protected:
  // server::config::Dispatcher
  void dispatch(server::config::Handler &) const override;

  // server::config::Reader::Handler
  void operator()(server::config::Symbols &&) override;
  void operator()(server::config::Account &&) override;
  void operator()(server::config::User &&) override;
  void operator()(server::config::RateLimit &&) override;
  void operator()(server::config::RequestTemplate, std::string_view const &label, toml::table &) override;
  void operator()(std::string_view const &key, toml::node &) override;

 public:
  server::config::Users users;
  server::config::Symbols symbols;
  server::config::Accounts accounts;
  std::string master_account_;
  server::config::RateLimits rate_limits;
};

/*
 * REST API
 * https://api-public.sandbox.pro.udp_subscriber.com
 *
 * Websocket Feed
 * wss://ws-feed-public.sandbox.pro.udp_subscriber.com
 *
 * FIX API
 * tcp+ssl://fix-public.sandbox.pro.udp_subscriber.com:4198
 */

}  // namespace udp_subscriber
}  // namespace roq

template <>
struct fmt::formatter<roq::udp_subscriber::Config> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::udp_subscriber::Config const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(symbols={}, )"
        R"(accounts=[{}], )"
        R"(master_account="{}", )"
        R"(users=[{}], )"
        R"(rate_limits=[{}])"
        R"(}})"_cf,
        value.symbols,
        fmt::join(value.accounts, ", "sv),
        value.master_account_,
        fmt::join(value.users, ", "sv),
        fmt::join(value.rate_limits, ", "sv));
  }
};
