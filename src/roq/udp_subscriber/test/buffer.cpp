/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/logging.hpp"

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
  assert(std::size(payload) == 64);
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
  REQUIRE_FALSE(buffer(create_frame(4), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
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

TEST_CASE("simple_fragmented", "[buffer]") {
  std::vector<std::byte> payload{64};
  assert(std::size(payload) == 64);
  Buffer buffer;
  // initialize
  size_t count = 0;
  REQUIRE_FALSE(buffer(create_frame(1, 0, 1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  REQUIRE(buffer(create_frame(1, 1, 1), payload, [&](auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{1});
  }));
  REQUIRE(count == 1);
  // reordered fragments
  count = 0;
  REQUIRE_FALSE(buffer(create_frame(2, 1, 1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  REQUIRE(buffer(create_frame(2, 0, 1), payload, [&](auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{2});
  }));
  REQUIRE(count == 1);
  // gap
  count = 0;
  REQUIRE_FALSE(buffer(create_frame(4, 0, 1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  count = 0;
  REQUIRE_FALSE(buffer(create_frame(4, 1, 1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  // re-ordered
  count = 0;
  REQUIRE_FALSE(buffer(create_frame(3, 0, 1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  count = 0;
  REQUIRE(buffer(create_frame(3, 1, 1), payload, [&](auto &header, [[maybe_unused]] auto &payload) {
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
  assert(std::size(payload) == 64);
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
  REQUIRE_FALSE(buffer(create_frame(1), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
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
  REQUIRE_FALSE(buffer(create_frame(2), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
  // replay #2
  count = 0;
  REQUIRE_FALSE(buffer(create_frame(3), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
  REQUIRE(count == 0);
}

TEST_CASE("bug", "[buffer]") {
  std::vector<std::byte> payload{64};
  assert(std::size(payload) == 64);
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
  // lost #3
  for (uint32_t seqno = 4; seqno < 19; ++seqno) {
    count = 0;
    REQUIRE_FALSE(buffer(create_frame(seqno), payload, [&]([[maybe_unused]] auto &header, [[maybe_unused]] auto &payload) { ++count; }));
    REQUIRE(count == 0);
  }
  // overflow
  count = 0;
  REQUIRE(buffer(create_frame(19), payload, [&](auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.seqno == uint32_t{3} + count);
    CHECK(header.session_id == SESSION_ID_1);
  }));
  REQUIRE(count == 16);
  // normal
  count = 0;
  REQUIRE(buffer(create_frame(20), payload, [&](auto &header, [[maybe_unused]] auto &payload) {
    ++count;
    CHECK(header.session_id == SESSION_ID_1);
    CHECK(header.seqno == uint32_t{20});
  }));
  REQUIRE(count == 1);
}
