/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/parser.hpp"
#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

struct FBSParser final : public Parser {
  static void dispatch_helper(
      Handler &, std::span<std::byte const> const &payload, TraceInfo const &, Shared &, core::udp::Frame const &);

 private:
  template <typename T>
  static void dispatch(Handler &, Event<T> const &, core::udp::Frame const &);
};

}  // namespace udp_subscriber
}  // namespace roq
