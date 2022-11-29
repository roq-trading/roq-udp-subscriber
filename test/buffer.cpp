/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/udp_subscriber/tools/buffer.hpp"

using namespace std::literals;  // NOLINT

using namespace roq;
using namespace roq::udp_subscriber;
using namespace roq::udp_subscriber::tools;

// === CONSTANTS ===

namespace {
auto const SESSION_ID_1 = uint16_t{1};
}

// === HELPERS ===

namespace {
auto create_frame(uint32_t seqno, uint8_t fragment = {}, uint8_t fragment_max = {}) {
  return core::udp::Frame{
      .control = {},
      .fragment = fragment,
      .fragment_max = fragment_max,
      .object_type = {},
      .object_id = {},
      .session_id = SESSION_ID_1,
      .seqno = seqno,
      .last_seqno = {},
  };
}
}  // namespace

// === IMPLEMENTATION ===

TEST_CASE("simple", "[buffer]") {
  std::vector<std::byte> payload{64};
  Buffer buffer;
  // initialize
  size_t count = 0;
  REQUIRE(buffer(create_frame(1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{1});
  }));
  REQUIRE(count == 1);
  // normal
  count = 0;
  REQUIRE(buffer(create_frame(2), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{2});
  }));
  REQUIRE(count == 1);
  // gap
  count = 0;
  REQUIRE_FALSE(buffer(
      create_frame(4), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  // re-ordered
  count = 0;
  REQUIRE(buffer(create_frame(3), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) {
    switch (++count) {
      case 1:
        CHECK(header.seqno == uint32_t{3});
        break;
      case 2:
        CHECK(header.seqno == uint32_t{4});
        break;
      default:
        FAIL();
    }
    CHECK(header.session_id == SESSION_ID_1);
  }));
  REQUIRE(count == 2);
  // normal
  count = 0;
  REQUIRE(buffer(create_frame(5), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{5});
  }));
  REQUIRE(count == 1);
}

TEST_CASE("replay", "[buffer]") {
  std::vector<std::byte> payload{64};
  Buffer buffer;
  size_t count = 0;
  REQUIRE(buffer(create_frame(1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{1});
  }));
  REQUIRE(count == 1);
  // replay
  count = 0;
  REQUIRE_FALSE(buffer(
      create_frame(1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  // normal #1
  count = 0;
  REQUIRE(buffer(create_frame(2), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{2});
  }));
  REQUIRE(count == 1);
  // normal #2
  count = 0;
  REQUIRE(buffer(create_frame(3), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{3});
  }));
  REQUIRE(count == 1);
  // replay #1
  count = 0;
  REQUIRE_FALSE(buffer(
      create_frame(2), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  // replay #2
  count = 0;
  REQUIRE_FALSE(buffer(
      create_frame(3), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
}
