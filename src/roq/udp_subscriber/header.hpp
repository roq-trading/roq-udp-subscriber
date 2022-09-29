/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/core/udp/frame.hpp"

namespace roq {
namespace udp_subscriber {

struct Header final {
  uint16_t session_id = {};
  uint32_t seqno = {};
  uint32_t last_seqno = {};
  uint8_t object_type = {};
  uint16_t object_id = {};
  core::udp::Encoding encoding = {};
  bool snapshot = {};
};

}  // namespace udp_subscriber
}  // namespace roq

template <>
struct fmt::formatter<roq::udp_subscriber::Header> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::udp_subscriber::Header const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(session_id=\x{:04x}, )"
        R"(seqno={}, )"
        R"(last_seqno={}, )"
        R"(object_type=\x{:02x}, )"
        R"(object_id=\x{:04x}, )"
        R"(encoding={}, )"
        R"(snapshot={})"
        R"(}})"sv,
        value.session_id,
        value.seqno,
        value.last_seqno,
        value.object_type,
        value.object_id,
        magic_enum::enum_name(value.encoding),
        value.snapshot);
  }
};
