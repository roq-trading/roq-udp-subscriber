/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/snapshot.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "roq/utils/update.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === HELPERS ===

namespace {
auto create_receiver_helper(auto &handler, auto &context, auto &address, auto port) {
  auto address_2 = std::empty(address) ? "127.0.0.1"sv : address;  // note! default is localhost
  auto network_address = io::NetworkAddress::create_blocking(address_2, port);
  auto socket_options = Mask{
      io::SocketOption::REUSE_ADDRESS,
  };
  auto receiver = context.create_udp_receiver(handler, network_address, socket_options);
  return receiver;
}

auto create_receivers(auto &handler, auto &settings, auto &context) {
  auto address = settings.udp.snapshot_address;
  auto port = settings.udp.snapshot_port;
  if (std::empty(port))
    log::fatal("Unexpected: port is missing"sv);
  if (std::size(port) > 1 && std::size(address) > 1 && std::size(port) != std::size(address))
    log::fatal("Unexpected: mismatched length of address and port"sv);
  std::vector<std::unique_ptr<io::net::udp::Receiver>> result;
  auto length = std::max(std::size(address), std::size(port));
  for (size_t i = 0; i < length; ++i) {
    auto address_ = std::empty(address) ? std::string{} : address[std::min(i, std::size(address) - 1)];
    auto port_ = port[std::min(i, std::size(port) - 1)];
    auto sender = create_receiver_helper(handler, context, address_, port_);
    result.emplace_back(std::move(sender));
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Snapshot::Snapshot(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, shared_{shared},
      receivers_{create_receivers(*this, shared.settings, context)} {
}

void Snapshot::operator()(Event<Start> const &) {
}

void Snapshot::operator()(Event<Stop> const &) {
}

void Snapshot::operator()(Event<Timer> const &event) {
  if (!last_update_time_.count())
    return;
  if ((last_update_time_ + shared_.settings.common.udp_heartbeat_timeout) < event.value.now) {
    last_update_time_ = {};
    TraceInfo trace_info;
    publish_stream_status(trace_info, supports_, ConnectionStatus::DISCONNECTED);
  }
}

void Snapshot::operator()(metrics::Writer &) {
}

void Snapshot::operator()(io::net::udp::Receiver::Read const &read) {
  TraceInfo trace_info;
  auto parse = [&](auto &header, auto &payload) {
    log::info<5>("header={}, len(payload)={}"sv, header, std::size(payload));
    // note! different session id drops
    if (header.session_id != shared_.session_id)
      return;
    auto &state = shared_.state[{header.object_type, header.object_id}];
    auto include = [&header, &state]() {
      if (header.object_type == 0x0)  // always
        return true;
      if (state.ready) {
        return false;
      } else {
        if (state.last_seqno.has_value()) {
          if ((*state.last_seqno) <= header.last_seqno) {  // note! we last_seqno from incremental channel
            return true;
          } else {
            return false;
          }
        } else {
          return true;  // always release when the object is not ready and we're not waiting
        }
      }
    }();
    if (include) {
      state.last_seqno = header.last_seqno;  // note! last_seqno is from the incremental channel
      if (header.object_type != 0x0) {
        log::info<4>(
            R"(+++ OBJECT READY +++ (object_type=\x{:02x}, object_id=\x{:04x}, last_seqno={}))"sv,
            header.object_type,
            header.object_id,
            *state.last_seqno);
      }
      // parse
      auto bytes = Parser::dispatch(*this, header, payload, trace_info, shared_);
      if (bytes != std::size(payload))
        log::warn("Unexpected: bytes={}, len(payload)={}"sv, bytes, std::size(payload));
    }
  };
  if (reader_.recv(read.receiver, [&](auto &frame, auto &payload) {
        log::info<5>("frame={}, len(payload)={}"sv, frame, std::size(payload));
        buffer_(frame, payload, parse);
      })) {
  } else {
    log::warn("Unexpected: invalid datagram"sv);
  }
}

void Snapshot::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

void Snapshot::operator()(Trace<Parser::Heartbeat> const &event, tools::Header const &header) {
  update(event, header);
}

void Snapshot::operator()(Trace<GatewaySettings> const &event, tools::Header const &header) {
  if (update(event, header))
    handler_(event);
}

void Snapshot::operator()(Trace<StreamStatus> const &event, tools::Header const &header) {
  update(event, header);
}

void Snapshot::operator()(Trace<ExternalLatency> const &, tools::Header const &) {
  log::fatal("Unexpected"sv);
}

void Snapshot::operator()(Trace<GatewayStatus> const &event, tools::Header const &header) {
  if (update(event, header))
    handler_(event);
}

void Snapshot::operator()(Trace<ReferenceData> const &event, tools::Header const &header) {
  if (update(event, header))
    handler_(event, true);
}

void Snapshot::operator()(Trace<MarketStatus> const &event, tools::Header const &header) {
  if (update(event, header))
    handler_(event, true);
}

void Snapshot::operator()(Trace<TopOfBook> const &, tools::Header const &) {
  log::fatal("Unexpected"sv);
}

void Snapshot::operator()(Trace<MarketByPriceUpdate> const &event, tools::Header const &header) {
  if (update(event, header)) {
    auto &trace_info = event.trace_info;
    auto &market_by_price_update = event.value;
    auto symbol = market_by_price_update.symbol;
    auto &sequencer = shared_.mbp_sequencer[symbol];
    try {
      auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence) {
        log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
        auto market_by_price_update_2 = MarketByPriceUpdate{
            .stream_id = stream_id_,
            .exchange = shared_.settings.exchange,
            .symbol = symbol,
            .bids = {const_cast<MBPUpdate *>(std::data(bids)), std::size(bids)},  // FIXME
            .asks = {const_cast<MBPUpdate *>(std::data(asks)), std::size(asks)},  // FIXME
            .update_type = UpdateType::SNAPSHOT,
            .exchange_time_utc = {},
            .exchange_sequence = sequencer.last_sequence(),
            .price_decimals = {},
            .quantity_decimals = {},
            .checksum = {},
        };
        Trace event(trace_info, market_by_price_update_2);
        shared_(event, true, [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, false); });
      };
      auto request_snapshot = [&]([[maybe_unused]] auto retries) {
        // XXX ???
      };
      sequencer(
          market_by_price_update.bids,
          market_by_price_update.asks,
          header.last_seqno,  // note! last_seqno correlates with incremental
          false,
          publish_snapshot,
          request_snapshot);
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      sequencer.clear();
      // XXX ???
    }
  }
}

void Snapshot::operator()(Trace<TradeSummary> const &, tools::Header const &) {
  log::fatal("Unexpected"sv);
}

void Snapshot::operator()(Trace<StatisticsUpdate> const &event, tools::Header const &header) {
  if (update(event, header))
    handler_(event, true);
}

void Snapshot::operator()(Trace<CustomMetricsUpdate> const &event, tools::Header const &header) {
  if (update(event, header)) {
    auto &[trace_info, value] = event;
    auto custom_metrics = CustomMetrics{
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
bool Snapshot::update(Trace<T> const &event, tools::Header const &) {
  auto &trace_info = event.trace_info;
  auto updated = [&]() {
    auto result = !last_update_time_.count();
    using value_type = typename std::remove_cvref<T>::type;
    if constexpr (std::is_same<value_type, GatewayStatus>::value) {
      result |= shared_.update(event.value.supported);
    }
    return result;
  }();
  if (updated)
    publish_stream_status(trace_info, shared_.supports, ConnectionStatus::READY);
  last_update_time_ = trace_info.source_receive_time;
  return true;
}

void Snapshot::publish_stream_status(
    TraceInfo const &trace_info, Mask<SupportType> supports, ConnectionStatus connection_status) {
  if (utils::update(supports_, supports) || utils::update(connection_status_, connection_status)) {
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = supports_,
        .transport = Transport::UDP,
        .protocol = Protocol::ROQ,
        .encoding = {Encoding::FBS},
        .priority = Priority::SECONDARY,
        .connection_status = connection_status_,
        .interface = {},
        .authority = {},
        .path = {},
        .proxy = {},
    };
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

}  // namespace udp_subscriber
}  // namespace roq
