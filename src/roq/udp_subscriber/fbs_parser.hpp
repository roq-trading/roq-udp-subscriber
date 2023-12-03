/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/parser.hpp"
#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

struct FBSParser final : public Parser {
  static void dispatch_helper(
      Handler &, std::span<std::byte const> const &payload, TraceInfo const &, Shared &, tools::Header const &);

 private:
  template <typename T>
  static void dispatch(Handler &, Event<T> const &, tools::Header const &);
};

}  // namespace udp_subscriber
}  // namespace roq
