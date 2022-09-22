/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/reader.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

Reader::Reader() : buffer_(4096) {
}

bool Reader::validate(std::span<std::byte const> const &message) {
  using Frame = core::udp::Frame;
  if (std::size(message) < sizeof(Frame)) {
    log::warn("Unexpected: len(data)={} < len(frame)={}"sv, std::size(message), sizeof(Frame));
    return false;
  }
  if (message[0] != core::udp::MAGIC) {
    log::warn(R"(Unexpected: magic=\x{:02x} != \x{:02x})"sv, message[0], core::udp::MAGIC);
    return false;
  }
  return true;
}

}  // namespace udp_subscriber
}  // namespace roq
