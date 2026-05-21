/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/gateway/parser.hpp"
#include "roq/udp_subscriber/gateway/shared.hpp"

namespace roq {
namespace udp_subscriber {
namespace gateway {

struct FBSParser final : public Parser {
  static void dispatch_helper(Handler &, std::span<std::byte const> const &payload, TraceInfo const &, Shared &, tools::Header const &);
};

}  // namespace gateway
}  // namespace udp_subscriber
}  // namespace roq
