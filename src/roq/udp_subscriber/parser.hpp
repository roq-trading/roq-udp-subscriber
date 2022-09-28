/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"
#include "roq/trace.hpp"

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/header.hpp"
#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

struct Parser {
  struct Heartbeat final {};
  struct Handler {
    virtual void operator()(Trace<Heartbeat> const &, Header const &) = 0;
    virtual void operator()(Trace<GatewaySettings> const &, Header const &) = 0;
    virtual void operator()(Trace<StreamStatus> const &, Header const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &, Header const &) = 0;
    virtual void operator()(Trace<GatewayStatus> const &, Header const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, Header const &) = 0;
    virtual void operator()(Trace<MarketStatus> const &, Header const &) = 0;
    virtual void operator()(Trace<TopOfBook> const &, Header const &) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, Header const &) = 0;
    virtual void operator()(Trace<TradeSummary> const &, Header const &) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, Header const &) = 0;
    virtual void operator()(Trace<CustomMetricsUpdate> const &, Header const &) = 0;
  };

  static size_t dispatch(
      Handler &, Header const &, std::span<std::byte const> const &payload, TraceInfo const &, Shared &);
};

}  // namespace udp_subscriber
}  // namespace roq
