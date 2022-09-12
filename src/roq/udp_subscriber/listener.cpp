/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/listener.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/udp_subscriber/flags.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

namespace {
auto create_receiver(auto &handler, auto &context) {
  auto port = Flags::udp_port();
  auto receiver = context.create_udp_receiver(handler, io::NetworkAddress{port});
  return receiver;
}
}  // namespace

Listener::Listener(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), shared_(shared), receiver_(create_receiver(*this, context)) {
}

void Listener::operator()(Event<Start> const &) {
}

void Listener::operator()(Event<Stop> const &) {
}

void Listener::operator()(Event<Timer> const &) {
}

void Listener::operator()(metrics::Writer &) {
}

void Listener::operator()(io::net::udp::Receiver::Read const &) {
  auto trace_info = server::create_trace_info();
  while (receive_buffer_.append(*receiver_)) {
    auto message = std::data(receive_buffer_);
    log::info<5>("received {} byte(s)"sv, std::size(message));
    auto bytes = Parser::dispatch(*this, message, trace_info, shared_);
    if (!bytes || bytes != std::size(message)) {
      log::warn("{}"sv, debug::hex::Message{message});
      log::fatal("Failed to parse message"sv);
    }
    receive_buffer_.clear();
  }
}

void Listener::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

void Listener::operator()(Trace<TopOfBook const> const &event) {
  log::info<3>("{}"sv, event.value);
  handler_(event, true);
}

void Listener::operator()(Trace<CustomMetrics const> const &event) {
  log::info<3>("{}"sv, event.value);
  handler_(event, true);
}

}  // namespace udp_subscriber
}  // namespace roq
