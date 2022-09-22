/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/incremental.hpp"

#include <arpa/inet.h>

#include <string>

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
  auto address = server::Flags::udp_incremental_address();
  auto port = server::Flags::udp_incremental_port();
  std::string tmp{std::empty(address) ? "127.0.0.1"sv : address};  // note! default is localhost
  struct in_addr localhost {
    .s_addr = inet_addr(tmp.c_str()),
  };
  auto network_address = io::NetworkAddress{port, localhost};
  auto socket_options = Mask{
      io::SocketOption::REUSE_ADDRESS,
  };
  auto receiver = context.create_udp_receiver(handler, network_address, socket_options);
  return receiver;
}
}  // namespace

Incremental::Incremental(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), shared_(shared), receiver_(create_receiver(*this, context)) {
}

void Incremental::operator()(Event<Start> const &) {
}

void Incremental::operator()(Event<Stop> const &) {
}

void Incremental::operator()(Event<Timer> const &event) {
  if (!last_update_time_.count())
    return;
  if ((last_update_time_ + flags::Flags::udp_heartbeat_timeout()) < event.value.now) {
    last_update_time_ = {};
    auto trace_info = server::create_trace_info();
    publish_stream_status(trace_info, ConnectionStatus::DISCONNECTED);
  }
}

void Incremental::operator()(metrics::Writer &) {
}

void Incremental::operator()(io::net::udp::Receiver::Read const &) {
  auto trace_info = server::create_trace_info();
  auto parse = [&](auto &frame, auto &payload) {
    auto bytes = Parser::dispatch(*this, frame, payload, trace_info, shared_);
    if (bytes != std::size(payload))
      log::warn("Unexpected: bytes={}, len(payload)={}"sv, bytes, std::size(payload));
  };
  if (reader_.recv(*receiver_, [&](auto &frame, auto &payload) { buffer_(frame, payload, parse); })) {
  } else {
    log::warn("Unexpected: invalid datagram"sv);
  }
}

void Incremental::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

void Incremental::operator()(Trace<Parser::Heartbeat> const &event, core::udp::Frame const &frame) {
  update(event, frame);
}

void Incremental::operator()(Trace<GatewaySettings> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event);
}

void Incremental::operator()(Trace<StreamStatus> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event);
}

void Incremental::operator()(Trace<ExternalLatency> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event);
}

void Incremental::operator()(Trace<GatewayStatus> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event);
}

void Incremental::operator()(Trace<ReferenceData> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event, true);
}

void Incremental::operator()(Trace<MarketStatus> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event, true);
}

void Incremental::operator()(Trace<TopOfBook> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event, true);
}

void Incremental::operator()(Trace<MarketByPriceUpdate> const &event, core::udp::Frame const &frame) {
  log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event, true);
}

void Incremental::operator()(Trace<TradeSummary> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event, true);
}

void Incremental::operator()(Trace<StatisticsUpdate> const &event, core::udp::Frame const &frame) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, frame))
    handler_(event, true);
}

void Incremental::operator()(Trace<CustomMetricsUpdate> const &event, core::udp::Frame const &frame) {
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
bool Incremental::update(Trace<T> const &event, core::udp::Frame const &frame) {
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

void Incremental::publish_stream_status(TraceInfo const &, ConnectionStatus connection_status) {
  if (!utils::update(connection_status_, connection_status))
    return;
  /* XXX FIXME competing with StreamStatus from origin...???
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
  create_trace_and_dispatch(handler_, trace_info, stream_status);
  */
}

}  // namespace udp_subscriber
}  // namespace roq
