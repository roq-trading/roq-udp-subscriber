/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/parser.hpp"

#include "roq/logging.hpp"

#include "roq/debug/hex/message.hpp"

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/fbs_parser.hpp"
#include "roq/udp_subscriber/json_parser.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

size_t Parser::dispatch(
    Handler &handler, std::span<std::byte const> const &buffer, TraceInfo const &trace_info, Shared &shared) {
  using Frame = core::udp::Frame;
  if (std::size(buffer) < sizeof(Frame)) {
    log::warn("Unexpected: len(data)={} < len(frame)={}"sv, std::size(buffer), sizeof(Frame));
    return 0;
  }
  Frame frame;
  std::memcpy(&frame, std::data(buffer), sizeof(frame));
  if (frame.magic != core::udp::MAGIC) {
    log::warn(R"(Unexpected: magic=\x{:02x} != \x{:02x})"sv, frame.magic, core::udp::MAGIC);
    return 0;
  }
  // ...
  if (frame.total_fragments == 0 && frame.fragment_number == 0) {
    auto payload = buffer.subspan(sizeof(frame));
    if (!std::empty(payload)) {
      switch (frame.encoding) {
        using enum core::udp::Encoding;
        case UNDEFINED:
          break;
        case NATIVE:
          break;
        case FLATBUFFERS:
          FBSParser::dispatch_helper(handler, payload, trace_info, shared, frame);
          break;
        case JSON:
          JSONParser::dispatch_helper(handler, payload, trace_info, shared, frame);
          break;
      }
    } else {
      // heartbeat
      log::info<1>("HEARTBEAT"sv);
    }
    return std::size(buffer);
  } else {
    log::fatal("Unexpected: fragments"sv);
  }
  return 0;
}

}  // namespace udp_subscriber
}  // namespace roq
