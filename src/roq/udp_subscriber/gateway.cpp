/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/gateway.hpp"

#include "roq/logging.hpp"

#include "roq/udp_subscriber/flags.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

Gateway::Gateway(server::Dispatcher &dispatcher, Config const &, io::Context &context)
    : dispatcher_(dispatcher), context_(context), shared_(dispatcher),
      listener_(*this, context, ++stream_id_, shared_) {
}

void Gateway::operator()(Event<Start> const &) {
  log::info("Starting the gateway..."sv);
}

void Gateway::operator()(Event<Stop> const &) {
  log::info("Stopping the gateway..."sv);
}

void Gateway::operator()(Event<Timer> const &) {
  context_.drain();
}

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &) {
}

uint16_t Gateway::operator()(
    Event<CreateOrder> const &, oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &,
    oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw oms::NotSupported("not supported"sv);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw oms::NotSupported("not supported"sv);
}

void Gateway::operator()(metrics::Writer &) {
}

void Gateway::operator()(Trace<TopOfBook const> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<CustomMetrics const> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

}  // namespace udp_subscriber
}  // namespace roq
