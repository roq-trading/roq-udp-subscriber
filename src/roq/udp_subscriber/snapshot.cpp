/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/snapshot.hpp"

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
  auto address = server::Flags::udp_snapshot_address();
  auto port = server::Flags::udp_snapshot_port();
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

Snapshot::Snapshot(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), shared_(shared), receiver_(create_receiver(*this, context)) {
}

void Snapshot::operator()(Event<Start> const &) {
}

void Snapshot::operator()(Event<Stop> const &) {
}

void Snapshot::operator()(Event<Timer> const &event) {
  if (!last_update_time_.count())
    return;
  if ((last_update_time_ + flags::Flags::udp_heartbeat_timeout()) < event.value.now) {
    last_update_time_ = {};
    auto trace_info = server::create_trace_info();
    publish_stream_status(trace_info, ConnectionStatus::DISCONNECTED);
  }
}

void Snapshot::operator()(metrics::Writer &) {
}

void Snapshot::operator()(io::net::udp::Receiver::Read const &) {
  auto trace_info = server::create_trace_info();
  auto parse = [&](auto &header, auto &payload) {
    log::debug("header={}, len(payload)={}"sv, header, std::size(payload));
    auto bytes = Parser::dispatch(*this, header, payload, trace_info, shared_);
    if (bytes != std::size(payload))
      log::warn("Unexpected: bytes={}, len(payload)={}"sv, bytes, std::size(payload));
  };
  if (reader_.recv(*receiver_, [&](auto &frame, auto &payload) {
        log::debug("frame={}, len(payload)={}"sv, frame, std::size(payload));
        buffer_(frame, payload, parse);
      })) {
  } else {
    log::warn("Unexpected: invalid datagram"sv);
  }
}

void Snapshot::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

void Snapshot::operator()(Trace<Parser::Heartbeat> const &event, Header const &header) {
  update(event, header);
}

void Snapshot::operator()(Trace<GatewaySettings> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header))
    handler_(event);
}

void Snapshot::operator()(Trace<StreamStatus> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header))
    handler_(event);
}

void Snapshot::operator()(Trace<ExternalLatency> const &, Header const &) {
  // log::info<3>("{}"sv, event.value);
  log::fatal("Unexpected"sv);
}

void Snapshot::operator()(Trace<GatewayStatus> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header))
    handler_(event);
}

void Snapshot::operator()(Trace<ReferenceData> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header))
    handler_(event, true);
}

void Snapshot::operator()(Trace<MarketStatus> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header))
    handler_(event, true);
}

void Snapshot::operator()(Trace<TopOfBook> const &, Header const &) {
  // log::info<3>("{}"sv, event.value);
  log::fatal("Unexpected"sv);
}

void Snapshot::operator()(Trace<MarketByPriceUpdate> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header))
    handler_(event, true);
}

void Snapshot::operator()(Trace<TradeSummary> const &, Header const &) {
  // log::info<3>("{}"sv, event.value);
  log::fatal("Unexpected"sv);
}

void Snapshot::operator()(Trace<StatisticsUpdate> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header))
    handler_(event, true);
}

void Snapshot::operator()(Trace<CustomMetricsUpdate> const &event, Header const &header) {
  // log::info<3>("{}"sv, event.value);
  if (update(event, header)) {
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
bool Snapshot::update(Trace<T> const &event, Header const &) {
  // heartbeat
  auto &trace_info = event.trace_info;
  if (!last_update_time_.count())
    publish_stream_status(trace_info, ConnectionStatus::READY);
  last_update_time_ = trace_info.source_receive_time;
  return true;
}

void Snapshot::publish_stream_status(TraceInfo const &, ConnectionStatus connection_status) {
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
