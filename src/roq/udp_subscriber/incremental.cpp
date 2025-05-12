/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/udp_subscriber/incremental.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "roq/utils/common.hpp"
#include "roq/utils/update.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === CONSTANTS ===

namespace {
auto LOCALHOST = "127.0.0.1"sv;

auto const SOCKET_OPTIONS = Mask{
    io::SocketOption::REUSE_ADDRESS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_receiver_helper(auto &handler, auto &context, auto &address, auto port) {
  auto address_2 = [&]() -> std::string_view {
    if (std::empty(address)) {
      return LOCALHOST;  // note! default is localhost
    }
    return address;
  }();
  auto network_address = io::NetworkAddress::create_blocking(address_2, port);
  auto receiver = context.create_udp_receiver(handler, network_address, SOCKET_OPTIONS);
  return receiver;
}

template <typename T>
auto create_receivers(auto &handler, auto &settings, auto &context) {
  using result_type = std::remove_cvref_t<T>;
  result_type result;
  auto address = settings.udp.incremental_address;
  auto port = settings.udp.incremental_port;
  if (std::empty(port)) {
    log::fatal("Unexpected: port is missing"sv);
  }
  if (std::size(port) > 1 && std::size(address) > 1 && std::size(port) != std::size(address)) {
    log::fatal("Unexpected: mismatched length of address and port"sv);
  }
  auto length = std::max(std::size(address), std::size(port));
  for (size_t i = 0; i < length; ++i) {
    auto address_ = [&]() -> std::string {
      if (std::empty(address)) {
        return {};
      }
      return address[std::min(i, std::size(address) - 1)];
    }();
    auto port_ = port[std::min(i, std::size(port) - 1)];
    auto sender = create_receiver_helper(handler, context, address_, port_);
    result.emplace_back(std::move(sender));
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Incremental::Incremental(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, shared_{shared}, receivers_{create_receivers<decltype(receivers_)>(*this, shared.settings, context)} {
}

void Incremental::operator()(Event<Start> const &) {
}

void Incremental::operator()(Event<Stop> const &) {
}

void Incremental::operator()(Event<Timer> const &event) {
  if (!last_update_time_.count()) {
    return;
  }
  if ((last_update_time_ + shared_.settings.misc.udp_heartbeat_timeout) < event.value.now) {
    last_update_time_ = {};
    TraceInfo trace_info;
    publish_stream_status(trace_info, supports_, ConnectionStatus::DISCONNECTED);
  }
}

void Incremental::operator()(metrics::Writer &) {
}

void Incremental::operator()(io::net::udp::Receiver::Read const &read) {
  TraceInfo trace_info;
  auto parse = [&](auto &header, auto &payload) {
    log::info<5>("header={}, len(payload)={}"sv, header, std::size(payload));
    if (header.session_id != shared_.session_id) {  // note! different session_id will reset
      log::warn(R"(+++ SESSION RESET +++ (session_id=\x{:04x}))"sv, header.session_id);
      shared_.session_id = header.session_id;
      shared_.state.clear();
      // XXX mark all objects stale
    }
    auto &state = shared_.state[{header.object_type, header.object_id}];
    auto bytes = Parser::dispatch(*this, header, payload, trace_info, shared_);
    if (bytes != std::size(payload)) {
      log::warn("Unexpected: bytes={}, len(payload)={}"sv, bytes, std::size(payload));
    }
    if (state.ready) {
      state.last_seqno = header.seqno;
    } else {
      if (header.snapshot) {
        state.ready = true;
        state.last_seqno = header.seqno;
        if (header.object_type != 0x0) {
          log::info<4>(
              R"(+++ OBJECT READY +++ (object_type=\x{:02x}, object_id=\x{:04x}, last_seqno={}))"sv, header.object_type, header.object_id, *state.last_seqno);
        }
      } else {
        if (!state.last_seqno.has_value()) {  // wait for snapshot
          state.last_seqno = header.seqno;
          if (header.object_type != 0x0) {
            log::info<4>(
                R"(+++ OBJECT WAITING +++ (object_type=\x{:02x}, object_id=\x{:04x}, last_seqno={}))"sv,
                header.object_type,
                header.object_id,
                *state.last_seqno);
          }
        }
      }
    }
  };
  auto helper = [&](auto &frame, auto &payload) {
    log::info<5>("frame={}, len(payload)={}"sv, frame, std::size(payload));
    buffer_(frame, payload, parse);
  };
  if (reader_.recv(read.receiver, helper)) {
  } else {
    log::warn("Unexpected: invalid datagram"sv);
  }
}

void Incremental::operator()(io::net::udp::Receiver::Error const &error) {
  log::fatal("Error: what={}"sv, error.what);
}

void Incremental::operator()(Trace<Parser::Heartbeat> const &event, tools::Header const &header) {
  update(event, header);
}

void Incremental::operator()(Trace<GatewaySettings> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event);
  }
}

void Incremental::operator()(Trace<StreamStatus> const &event, tools::Header const &header) {
  update(event, header);
}

void Incremental::operator()(Trace<ExternalLatency> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event);
  }
}

void Incremental::operator()(Trace<GatewayStatus> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event);
  }
}

void Incremental::operator()(Trace<ReferenceData> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event, true);
  }
}

void Incremental::operator()(Trace<MarketStatus> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event, true);
  }
}

void Incremental::operator()(Trace<TopOfBook> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event, true);
  }
}

void Incremental::operator()(Trace<MarketByPriceUpdate> const &event, tools::Header const &header) {
  if (update(event, header)) {
    auto &trace_info = event.trace_info;
    auto &market_by_price_update = event.value;
    auto symbol = market_by_price_update.symbol;
    auto &sequencer = shared_.mbp_sequencer[symbol];
    if (utils::is_snapshot(market_by_price_update.update_type)) {
      log::debug(R"(PUBLISH SNAPSHOT symbol="{}")"sv, symbol);
      sequencer.initialize(header.seqno);
      shared_(event, true, [&]([[maybe_unused]] auto &market_by_price) {});
    } else {
      try {
        auto create_update = [&](auto &bids, auto &asks, auto update_type, auto exchange_sequence) -> MarketByPriceUpdate {
          return {
              .stream_id = stream_id_,
              .exchange = market_by_price_update.exchange,
              .symbol = market_by_price_update.symbol,
              .bids = bids,
              .asks = asks,
              .update_type = update_type,
              .exchange_time_utc = market_by_price_update.exchange_time_utc,
              .exchange_sequence = exchange_sequence,
              .price_precision = market_by_price_update.price_precision,
              .quantity_precision = market_by_price_update.price_precision,
              .checksum = market_by_price_update.checksum,
          };
        };
        auto publish_update = [&](auto &bids, auto &asks) {
          log::debug("bids=[{}], asks=[{}]"sv, fmt::join(bids, ", "sv), fmt::join(asks, ", "sv));
          auto market_by_price_update_2 = create_update(bids, asks, UpdateType::INCREMENTAL, market_by_price_update.exchange_sequence);
          Trace event{trace_info, market_by_price_update_2};
          shared_(event, true, shared_.final_bids, shared_.final_asks, [&]([[maybe_unused]] auto &market_by_price) {});
        };
        auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, auto retries, auto delay) {
          log::debug(
              R"(PUBLISH SNAPSHOT symbol="{}", sequence={}, retries={}, delay={})"sv,
              symbol,
              sequence,
              retries,
              std::chrono::duration_cast<std::chrono::milliseconds>(delay));
          auto market_by_price_update_2 = create_update(bids, asks, UpdateType::SNAPSHOT, sequencer.last_sequence());
          Trace event{trace_info, market_by_price_update_2};
          shared_(event, true, [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, false); });
        };
        auto request_snapshot = [&]([[maybe_unused]] auto retries) {
          // XXX ???
        };
        sequencer(
            market_by_price_update.bids,
            market_by_price_update.asks,
            header.last_seqno + 1,
            header.seqno,
            header.last_seqno,
            publish_update,
            publish_snapshot,
            request_snapshot);
      } catch (BadState &) {
        log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
        sequencer.clear();
        // XXX ???
      }
    }
  }
}

void Incremental::operator()(Trace<TradeSummary> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event, true);
  }
}

void Incremental::operator()(Trace<StatisticsUpdate> const &event, tools::Header const &header) {
  if (update(event, header)) {
    handler_(event, true);
  }
}

void Incremental::operator()(Trace<CustomMetricsUpdate> const &event, tools::Header const &header) {
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
    create_trace_and_dispatch(handler_, trace_info, custom_metrics, true, value.sending_time_utc);
  }
}

template <typename T>
bool Incremental::update(Trace<T> const &event, tools::Header const &) {
  auto &trace_info = event.trace_info;
  auto updated = [&]() {
    auto result = !last_update_time_.count();
    using value_type = typename std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<value_type, GatewayStatus>) {
      // note! let snapshot update (for now, later we can do something with seqno)
    }
    // note! instead we inherit from snapshot
    result |= utils::compare(supports_, shared_.supports) != 0;
    return result;
  }();
  if (updated) {
    publish_stream_status(trace_info, shared_.supports, ConnectionStatus::READY);
  }
  last_update_time_ = trace_info.source_receive_time;
  return true;
}

void Incremental::publish_stream_status(TraceInfo const &trace_info, Mask<SupportType> supports, ConnectionStatus connection_status) {
  if (utils::update(supports_, supports) || utils::update(connection_status_, connection_status)) {
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = supports_,
        .transport = Transport::UDP,
        .protocol = Protocol::ROQ,
        .encoding = {Encoding::FBS},
        .priority = Priority::PRIMARY,
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
