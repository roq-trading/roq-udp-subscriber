/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/listener.hpp"

#include "roq/utils/update.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/udp_subscriber/flags.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

namespace {
const Mask SUPPORTS{
    SupportType::TOP_OF_BOOK,
};

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

void Listener::operator()(Event<Timer> const &event) {
  // log::debug("{} {}"sv, last_update_time_, event.value.now);
  if (!last_update_time_.count())
    return;
  if ((last_update_time_ + flags::Flags::heartbeat_timeout()) < event.value.now) {
    last_update_time_ = {};
    auto trace_info = server::create_trace_info();
    publish_stream_status(trace_info, ConnectionStatus::DISCONNECTED);
  }
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

void Listener::operator()(Trace<Parser::Heartbeat const> const &event, core::udp::Frame const &frame) {
  update(event, frame);
}

void Listener::operator()(Trace<TopOfBook const> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event, true);
}

void Listener::operator()(Trace<CustomMetricsUpdate const> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame)) {
    auto &[trace_info, value] = event;
    CustomMetrics const custom_metrics{
        .label = value.label,
        .account = value.account,
        .exchange = value.exchange,
        .symbol = value.symbol,
        .measurements = value.measurements,
        .update_type = value.update_type,
    };
    create_trace_and_dispatch(handler_, trace_info, custom_metrics, true);
  }
}

template <typename T>
bool Listener::update(Trace<T const> const &event, core::udp::Frame const &frame) {
  // heartbeat
  auto &trace_info = event.trace_info;
  if (!last_update_time_.count())
    publish_stream_status(trace_info, ConnectionStatus::READY);
  last_update_time_ = trace_info.source_receive_time;
  // sequence number
  if (frame.source_session_id != source_session_id_) {
    log::warn<3>("+++ RESET SEQNO={} +++"sv, frame.source_seqno);
    source_session_id_ = frame.source_session_id;
    source_seqno_ = frame.source_seqno;
  } else {
    const constexpr auto HALF = uint32_t{1} >> 31;
    if (source_seqno_ > HALF && frame.source_seqno < HALF) {
      log::info("+++ WRAP-AROUND SEQNO={} +++"sv, frame.source_seqno);
      source_seqno_ = frame.source_seqno;
    } else if (source_seqno_ < frame.source_seqno) {
      source_seqno_ = frame.source_seqno;
    } else {
      log::warn<3>("*** DROP SEQNO={} ***"sv, frame.source_seqno);
      return false;  // drop
    }
  }
  return true;
}

void Listener::publish_stream_status(TraceInfo const &trace_info, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  StreamStatus const stream_status{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
      .transport = Transport::UDP,
      .protocol = {},  // note! can only be discovered
      .encoding = {},  // note! can only be discovered
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
  };
  log::debug("{}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

}  // namespace udp_subscriber
}  // namespace roq
