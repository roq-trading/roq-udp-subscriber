/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>

#include "roq/udp_subscriber/parser.hpp"
#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

struct JSONParser final : public Parser {
  size_t dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &, Shared &) override;
};

}  // namespace udp_subscriber
}  // namespace roq
