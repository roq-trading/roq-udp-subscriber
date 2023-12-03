/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/udp_subscriber/gateway.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

Gateway::Gateway(server::Dispatcher &dispatcher, Settings const &settings, Config const &, io::Context &context)
    : dispatcher_{dispatcher}, shared_{dispatcher, settings}, snapshot_{*this, context, ++stream_id_, shared_},
      incremental_{*this, context, ++stream_id_, shared_} {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  snapshot_(event);
  incremental_(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  incremental_(event);
  snapshot_(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  snapshot_(event);
  incremental_(event);
}

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &) {
}

uint16_t Gateway::operator()(
    Event<CreateOrder> const &, oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  throw oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw oms::NotSupported{"not supported"sv};
}

void Gateway::operator()(metrics::Writer &) {
}

void Gateway::operator()(Trace<GatewaySettings> const &) {
  // dispatcher_(event); // XXX FIXME no API
}

void Gateway::operator()(Trace<StreamStatus> const &event) {
  // note! order management is not supported (reason: string_map not populated)
  if (std::empty(event.value.account))
    dispatcher_(event);
}

void Gateway::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<GatewayStatus> const &) {
  // dispatcher_(event); // XXX FIXME no API
}

void Gateway::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<CustomMetrics> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

}  // namespace udp_subscriber
}  // namespace roq
