/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/udp_subscriber/fbs_parser.hpp"

#include "roq/logging.hpp"

#include "roq/clock.hpp"

#include "roq/utils/patterns.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === IMPLEMENTATION ===

void FBSParser::dispatch_helper(
    Handler &handler, std::span<std::byte const> const &payload, [[maybe_unused]] TraceInfo const &, Shared &shared, tools::Header const &header) {
  auto event = core::fbs::Decoder::create_event(payload);
  auto message_info = core::fbs::Decoder::create_message_info(event, 0, {}, {}, true);
  shared.decoder.dispatch(
      utils::overloaded{
          [](Event<DownloadBegin> const &) {},  // drop: client specific
          [](Event<DownloadEnd> const &) {},    // drop: client specific
          [](Event<Ready> const &) {},          // drop: client specific
          [&](Event<GatewaySettings> const &event) { dispatch(handler, event, header); },
          [&](Event<StreamStatus> const &event) { dispatch(handler, event, header); },
          [&](Event<ExternalLatency> const &event) { dispatch(handler, event, header); },
          [](Event<RateLimitsUpdate> const &) {},  // drop: order management
          [](Event<RateLimitTrigger> const &) {},  // drop: order management
          [&](Event<GatewayStatus> const &event) { dispatch(handler, event, header); },
          [&](Event<ReferenceData> const &event) { dispatch(handler, event, header); },
          [&](Event<MarketStatus> const &event) { dispatch(handler, event, header); },
          [&](Event<TopOfBook> const &event) { dispatch(handler, event, header); },
          [&](Event<MarketByPriceUpdate> const &event) { dispatch(handler, event, header); },
          [](Event<MarketByOrderUpdate> const &) {},  // ???
          [&](Event<TradeSummary> const &event) { dispatch(handler, event, header); },
          [&](Event<StatisticsUpdate> const &event) { dispatch(handler, event, header); },
          [](Event<TimeSeriesUpdate> const &) {},    // drop: server specific
          [](Event<CreateOrder> const &) {},         // drop: server specific
          [](Event<ModifyOrder> const &) {},         // drop: server specific
          [](Event<CancelOrder> const &) {},         // drop: server specific
          [](Event<CancelAllOrders> const &) {},     // drop: server specific
          [](Event<CancelAllOrdersAck> const &) {},  // drop: server specific
          [](Event<OrderAck> const &) {},            // drop: client specific
          [](Event<OrderUpdate> const &) {},         // drop: client specific
          [](Event<TradeUpdate> const &) {},         // drop: client specific
          [](Event<PositionUpdate> const &) {},      // ???
          [](Event<FundsUpdate> const &) {},         // ???
          [](Event<CustomMetrics> const &) {},       // drop: internal
          [&](Event<CustomMetricsUpdate> const &event) { dispatch(handler, event, header); },
          [](Event<CustomMatrix> const &) {},        // drop: internal
          [](Event<CustomMatrixUpdate> const &) {},  // drop: not supported
          [](Event<ParametersUpdate> const &) {},    // drop: client specific
          [](Event<Portfolio> const &) {},           // drop: client specific
          [](Event<PortfolioUpdate> const &) {},     // drop: client specific
          [](Event<RiskLimits> const &) {},          // drop: client specific
          [](Event<RiskLimitsUpdate> const &) {},    // drop: client specific
      },
      event,
      message_info);
}

template <typename T>
void FBSParser::dispatch(Handler &handler, Event<T> const &event, tools::Header const &header) {
  auto &[message_info, value] = event;
  auto now = clock::get_system();
  TraceInfo trace_info{
      now,
      now,
      message_info.origin_create_time_utc,
  };
  Trace trace{trace_info, value};
  handler(trace, header);
}

}  // namespace udp_subscriber
}  // namespace roq
