/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/udp_subscriber/gateway/controller.hpp"

#include "roq/logging.hpp"

#include "roq/server/oms/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {
namespace gateway {

// === IMPLEMENTATION ===

std::unique_ptr<server::Handler> Controller::create(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context) {
  return std::make_unique<Controller>(dispatcher, settings, config, context);
}

Controller::Controller(server::Dispatcher &dispatcher, Settings const &settings, Config const &, io::Context &context)
    : dispatcher_{dispatcher}, shared_{dispatcher, settings}, snapshot_{*this, context, ++stream_id_, shared_},
      incremental_{*this, context, ++stream_id_, shared_} {
}

void Controller::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  snapshot_(event);
  incremental_(event);
}

void Controller::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  incremental_(event);
  snapshot_(event);
}

void Controller::operator()(Event<Timer> const &event) {
  snapshot_(event);
  incremental_(event);
}

void Controller::operator()(Event<Control> const &event) {
  auto &[message_info, control] = event;
  switch (control.action) {
    using enum Action;
    case UNDEFINED:
      assert(false);
      break;
    case ENABLE:
      dispatcher_(State::ENABLED);
      break;
    case DISABLE:
      dispatcher_(State::DISABLED);
      break;
  }
}

void Controller::operator()(Event<Connected> const &) {
}

void Controller::operator()(Event<Disconnected> const &) {
}

void Controller::operator()(Event<Subscribe> const &) {
}

uint16_t Controller::operator()(
    Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(
    Event<ModifyOrder> const &,
    server::oms::Order const &,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(
    Event<CancelOrder> const &,
    server::oms::Order const &,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Controller::operator()(metrics::Writer &) const {
}

void Controller::operator()(Trace<GatewaySettings> const &) {
  // dispatcher_(event); // XXX FIXME no API
}

void Controller::operator()(Trace<StreamStatus> const &event) {
  // note! order management is not supported (reason: string_map not populated)
  if (std::empty(event.value.account)) {
    dispatcher_(event);
  }
}

void Controller::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Controller::operator()(Trace<GatewayStatus> const &) {
  // dispatcher_(event); // XXX FIXME no API
}

void Controller::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Controller::operator()(Trace<CustomMetrics> const &event, bool is_last, std::chrono::nanoseconds sending_time_utc) {
  dispatcher_(event, is_last, sending_time_utc);
}

}  // namespace gateway
}  // namespace udp_subscriber
}  // namespace roq
