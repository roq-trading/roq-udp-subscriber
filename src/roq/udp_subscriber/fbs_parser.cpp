/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/fbs_parser.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/logging.hpp"

#include "roq/core/clock.hpp"
#include "roq/core/patterns.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

void FBSParser::dispatch_helper(
    Handler &handler,
    std::span<std::byte const> const &payload,
    [[maybe_unused]] TraceInfo const &,
    Shared &shared,
    [[maybe_unused]] core::udp::Frame const &) {
  // log::debug("{}"sv, debug::hex::Message{payload});
  auto event = core::fbs::Decoder::create_event(payload);
  auto message_info = core::fbs::Decoder::create_message_info(event, 0, {}, {}, true);
  shared.decoder.dispatch(
      overloaded{
          [](Event<DownloadBegin> const &) {},
          [](Event<DownloadEnd> const &) {},
          [](Event<GatewaySettings> const &) {},
          [](Event<StreamStatus> const &) {},
          [](Event<ExternalLatency> const &) {},
          [](Event<RateLimitTrigger> const &) {},
          [](Event<GatewayStatus> const &) {},
          [](Event<ReferenceData> const &) {},
          [](Event<MarketStatus> const &) {},
          [&](Event<TopOfBook> const &event) { dispatch(handler, event); },
          [](Event<MarketByPriceUpdate> const &) {},
          [](Event<MarketByOrderUpdate> const &) {},
          [](Event<TradeSummary> const &) {},
          [](Event<StatisticsUpdate> const &) {},
          [](Event<CreateOrder> const &) {},
          [](Event<ModifyOrder> const &) {},
          [](Event<CancelOrder> const &) {},
          [](Event<CancelAllOrders> const &) {},
          [](Event<OrderAck> const &) {},
          [](Event<OrderUpdate> const &) {},
          [](Event<TradeUpdate> const &) {},
          [](Event<PositionUpdate> const &) {},
          [](Event<FundsUpdate> const &) {},
          [](Event<CustomMetrics> const &) {},
          [&](Event<CustomMetricsUpdate> const &event) { dispatch(handler, event); },
          [](Event<ParameterUpdate> const &) {},
      },
      event,
      message_info);
}

template <typename T>
void FBSParser::dispatch(Handler &handler, Event<T> const &event) {
  auto &[message_info, value] = event;
  auto now = core::clock::GetSystem();
  TraceInfo trace_info{
      .source_receive_time = now,
      .origin_create_time = now,
      .origin_create_time_utc = message_info.origin_create_time_utc,
  };
  Trace trace{trace_info, value};
  handler(trace);
}

}  // namespace udp_subscriber
}  // namespace roq
