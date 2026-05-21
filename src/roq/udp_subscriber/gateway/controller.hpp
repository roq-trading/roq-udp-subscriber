/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/compat.hpp"

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/udp_subscriber/gateway/config.hpp"
#include "roq/udp_subscriber/gateway/incremental.hpp"
#include "roq/udp_subscriber/gateway/settings.hpp"
#include "roq/udp_subscriber/gateway/shared.hpp"
#include "roq/udp_subscriber/gateway/snapshot.hpp"

namespace roq {
namespace udp_subscriber {
namespace gateway {

struct Controller final : public server::Handler, public Snapshot::Handler, public Incremental::Handler {
  ROQ_PUBLIC static std::unique_ptr<server::Handler> create(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(Controller const &) = delete;

 protected:
  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Control> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  void operator()(Event<Subscribe> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  uint16_t operator()(Event<MassQuote> const &) override;

  uint16_t operator()(Event<CancelQuotes> const &) override;

  void operator()(metrics::Writer &) const override;

  // many
  void operator()(Trace<GatewaySettings> const &) override;
  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;
  void operator()(Trace<GatewayStatus> const &) override;
  void operator()(Trace<ReferenceData> const &, bool is_last) override;
  void operator()(Trace<MarketStatus> const &, bool is_last) override;
  void operator()(Trace<TopOfBook> const &, bool is_last) override;
  void operator()(Trace<TradeSummary> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate> const &, bool is_last) override;
  void operator()(Trace<CustomMetrics> const &, bool is_last, std::chrono::nanoseconds sending_time_utc) override;

 private:
  server::Dispatcher &dispatcher_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  Snapshot snapshot_;
  Incremental incremental_;
};

}  // namespace gateway
}  // namespace udp_subscriber
}  // namespace roq
