/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <chrono>
#include <deque>
#include <string>
#include <utility>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/memory.hpp"

namespace roq {
namespace udp_subscriber {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }
  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

 private:
  server::Dispatcher &dispatcher_;
};

}  // namespace udp_subscriber
}  // namespace roq
