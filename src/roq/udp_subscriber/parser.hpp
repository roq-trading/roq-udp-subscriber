/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"
#include "roq/trace.hpp"

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

struct Parser {
  struct Heartbeat final {};
  struct Handler {
    virtual void operator()(Trace<Heartbeat> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<GatewaySettings> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<StreamStatus> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<GatewayStatus> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<MarketStatus> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<TopOfBook> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<TradeSummary> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, core::udp::Frame const &) = 0;
    virtual void operator()(Trace<CustomMetricsUpdate> const &, core::udp::Frame const &) = 0;
  };

  static size_t dispatch(
      Handler &, core::udp::Frame const &, std::span<std::byte const> const &payload, TraceInfo const &, Shared &);
};

}  // namespace udp_subscriber
}  // namespace roq
