/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"
#include "roq/trace.hpp"

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/shared.hpp"

#include "roq/udp_subscriber/tools/header.hpp"

namespace roq {
namespace udp_subscriber {

struct Parser {
  struct Heartbeat final {};
  struct Handler {
    virtual void operator()(Trace<Heartbeat> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<GatewaySettings> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<StreamStatus> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<GatewayStatus> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<MarketStatus> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<TopOfBook> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<TradeSummary> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, tools::Header const &) = 0;
    virtual void operator()(Trace<CustomMetricsUpdate> const &, tools::Header const &) = 0;
  };

  static size_t dispatch(
      Handler &, tools::Header const &, std::span<std::byte const> const &payload, TraceInfo const &, Shared &);
};

}  // namespace udp_subscriber
}  // namespace roq
