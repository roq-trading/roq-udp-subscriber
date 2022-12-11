/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/reader.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

// === CONSTANTS ===

namespace {
auto const BUFFER_LENGTH = size_t{4096};
}

// === IMPLEMENTATION ===

Reader::Reader() : buffer_(BUFFER_LENGTH) {
}

bool Reader::validate(std::span<std::byte const> const &message) {
  using Frame = core::udp::Frame;
  if (std::size(message) < sizeof(Frame)) {
    log::warn("Unexpected: len(data)={} < len(frame)={}"sv, std::size(message), sizeof(Frame));
    return false;
  }
  return true;
}

}  // namespace udp_subscriber
}  // namespace roq
