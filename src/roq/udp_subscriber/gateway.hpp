/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/udp_subscriber/config.hpp"
#include "roq/udp_subscriber/incremental.hpp"
#include "roq/udp_subscriber/shared.hpp"
#include "roq/udp_subscriber/snapshot.hpp"

namespace roq {
namespace udp_subscriber {

class Gateway final : public server::Handler, public Snapshot::Handler, public Incremental::Handler {
 public:
  Gateway(server::Dispatcher &, Config const &, io::Context &);

 protected:
  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  void operator()(metrics::Writer &) override;

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
  void operator()(Trace<CustomMetrics> const &, bool is_last) override;

 private:
  server::Dispatcher &dispatcher_;
  // io
  io::Context &context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  Snapshot snapshot_;
  Incremental incremental_;
};

}  // namespace udp_subscriber
}  // namespace roq
