/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/udp_subscriber/gateway/parser.hpp"

#include "roq/logging.hpp"

#include "roq/udp_subscriber/gateway/fbs_parser.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {
namespace gateway {

// === IMPLEMENTATION ===

size_t Parser::dispatch(Handler &handler, tools::Header const &header, std::span<std::byte const> const &payload, TraceInfo const &trace_info, Shared &shared) {
  if (!std::empty(payload)) {
    auto ok = false;
    switch (header.encoding) {
      using enum core::udp::Encoding;
      case UNDEFINED:
        break;
      case NATIVE:
        break;
      case FLATBUFFERS:
        FBSParser::dispatch_helper(handler, payload, trace_info, shared, header);
        ok = true;
        break;
    }
    if (!ok) {
      log::warn("Unexpected: header={}"sv, header);
    }
  } else {
    auto heartbeat = Heartbeat{};
    create_trace_and_dispatch(handler, trace_info, heartbeat, header);
  }
  return std::size(payload);
}

}  // namespace gateway
}  // namespace udp_subscriber
}  // namespace roq
