/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/trace.hpp"

#include "roq/custom_metrics_update.hpp"
#include "roq/top_of_book.hpp"

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

struct Parser {
  struct Heartbeat final {};
  struct Handler {
    virtual void operator()(Trace<Heartbeat const> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<TopOfBook const> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<CustomMetricsUpdate const> const &, core::udp::Frame const &) = 0;
  };

  static size_t dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &, Shared &);
};

}  // namespace udp_subscriber
}  // namespace roq
