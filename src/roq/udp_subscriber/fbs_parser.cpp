/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/udp_subscriber/fbs_parser.hpp"

#include "roq/clock.hpp"

#include "roq/codec/flatbuffers/decoder.hpp"

#include "roq/core/fbs/decoder.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === HELPERS ===

namespace {
struct Bridge final : public roq::codec::flatbuffers::Decoder::Handler {
  Bridge(Parser::Handler &handler, tools::Header const &header) : handler_{handler}, header_{header} {}

 protected:
  void operator()(Event<Control> const &) {}         // drop: XXX
  void operator()(Event<ControlAck> const &) {}      // drop: XXX
  void operator()(Event<ServiceUpdate> const &) {}   // drop: XXX
  void operator()(Event<StrategyUpdate> const &) {}  // drop: XXX
  void operator()(Event<LegsUpdate> const &) {}      // drop: XXX
  void operator()(Event<DownloadBegin> const &) {}   // drop: XXX
  void operator()(Event<DownloadEnd> const &) {}     // drop: XXX
  void operator()(Event<Ready> const &) {}           // drop: XXX
  void operator()(Event<GatewaySettings> const &event) { dispatch(event); }
  void operator()(Event<StreamStatus> const &event) { dispatch(event); }
  void operator()(Event<ExternalLatency> const &event) { dispatch(event); }
  void operator()(Event<RateLimitsUpdate> const &) {}  // drop: order management
  void operator()(Event<RateLimitTrigger> const &) {}  // drop: order management
  void operator()(Event<GatewayStatus> const &event) { dispatch(event); }
  void operator()(Event<ReferenceData> const &event) { dispatch(event); }
  void operator()(Event<MarketStatus> const &event) { dispatch(event); }
  void operator()(Event<TopOfBook> const &event) { dispatch(event); }
  void operator()(Event<MarketByPriceUpdate> const &event) { dispatch(event); }
  void operator()(Event<MarketByOrderUpdate> const &) {}  // drop: ???
  void operator()(Event<TradeSummary> const &event) { dispatch(event); }
  void operator()(Event<StatisticsUpdate> const &event) { dispatch(event); }
  void operator()(Event<TimeSeriesUpdate> const &) {}    // drop: server specific
  void operator()(Event<Subscribe> const &) {}           // drop: server specific
  void operator()(Event<CreateOrder> const &) {}         // drop: server specific
  void operator()(Event<ModifyOrder> const &) {}         // drop: server specific
  void operator()(Event<CancelOrder> const &) {}         // drop: server specific
  void operator()(Event<CancelAllOrders> const &) {}     // drop: server specific
  void operator()(Event<CancelAllOrdersAck> const &) {}  // drop: server specific
  void operator()(Event<OrderAck> const &) {}            // drop: server specific
  void operator()(Event<OrderUpdate> const &) {}         // drop: server specific
  void operator()(Event<TradeUpdate> const &) {}         // drop: server specific
  void operator()(Event<PositionUpdate> const &) {}      // drop: ???
  void operator()(Event<FundsUpdate> const &) {}         // drop: ???
  void operator()(Event<CustomMetrics> const &) {}
  void operator()(Event<CustomMetricsUpdate> const &event) { dispatch(event); }
  void operator()(Event<CustomMatrix> const &) {}        // drop: XXX
  void operator()(Event<CustomMatrixUpdate> const &) {}  // drop: XXX
  void operator()(Event<ParametersUpdate> const &) {}    // drop: XXX
  void operator()(Event<Portfolio> const &) {}           // drop: XXX
  void operator()(Event<PortfolioUpdate> const &) {}     // drop: XXX
  void operator()(Event<RiskLimits> const &) {}          // drop: XXX
  void operator()(Event<RiskLimitsUpdate> const &) {}    // drop: XXX
  void operator()(Event<MassQuote> const &) {}           // drop: XXX
  void operator()(Event<MassQuoteAck> const &) {}        // drop: XXX
  void operator()(Event<CancelQuotes> const &) {}        // drop: XXX
  void operator()(Event<CancelQuotesAck> const &) {}     // drop: XXX

  void dispatch(auto &event) {
    auto &[message_info, value] = event;
    auto now = clock::get_system();
    TraceInfo trace_info{
        now,
        now,
        message_info.origin_create_time_utc,
    };
    Trace trace{trace_info, value};
    handler_(trace, header_);
  }

 private:
  Parser::Handler &handler_;
  tools::Header const &header_;
};
}  // namespace

// === IMPLEMENTATION ===

void FBSParser::dispatch_helper(Handler &handler, std::span<std::byte const> const &payload, TraceInfo const &, Shared &shared, tools::Header const &header) {
  auto event = core::fbs::Decoder::create_event(payload);
  auto message_info = core::fbs::Decoder::create_message_info(event, 0, {}, {}, true);
  Bridge bridge{handler, header};
  (*shared.decoder)(bridge, payload, message_info);
}

}  // namespace udp_subscriber
}  // namespace roq
