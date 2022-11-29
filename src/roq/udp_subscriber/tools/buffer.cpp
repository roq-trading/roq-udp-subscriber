/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/tools/buffer.hpp"

#include <algorithm>
#include <limits>

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {
namespace tools {

namespace {
auto const MAX_PAYLOAD = core::udp::MAX_PAYLOAD_LENGTH;
auto const MAX_BUFFER = MAX_PAYLOAD * size_t{256};
auto const MAX_BUFFERS = size_t{16};
}  // namespace

Buffer::Buffer() : assembly_(MAX_BUFFERS) {
}

Buffer::Status Buffer::update(core::udp::Frame const &frame, std::span<std::byte const> const &payload) {
  log::info<5>("frame={}, len(payload)={}"sv, frame, std::size(payload));
  auto seqno = frame.seqno;
  log::info<5>("seqno={}, next={}"sv, seqno, next_seqno_);
  // reset
  if (session_id_ != frame.session_id) {
    if (session_id_)
      log::warn(R"(+++ SESSION RESET \x{:04x} +++)"sv, frame.session_id);
    session_id_ = frame.session_id;
    next_seqno_ = seqno;
  }
  // replay
  if (is_replay(seqno))
    return Status::READY;
  // simple
  if (seqno == next_seqno_ && frame.fragment_max == 0) {
    advance();
    // log::debug("next={}"sv, next_seqno_);
    return Status::DISPATCH;
  }
  // fragmented or out-of-sequence
  auto result = Status::BUFFERING;
  auto process = [&]() -> bool {
    return get_buffer(seqno, [&](auto &item) {
      auto index = frame.fragment;
      if (!item.available[index]) {
        item.available.set(index);
        auto offset = frame.fragment * MAX_PAYLOAD;
        std::copy(std::begin(payload), std::end(payload), std::begin(item.payload) + offset);
        if (index == frame.fragment_max)
          item.size = offset + std::size(payload);
        ++item.count;
        assert(!item.ready);
        if (item.count == (static_cast<size_t>(frame.fragment_max) + 1)) {  // note! +1
          item.ready = true;
          if (seqno == next_seqno_)
            result = Status::READY;
          assert(item.available.count() == item.count);
          //
          item.last_seqno = frame.last_seqno;
          item.object_type = frame.object_type;
          item.object_id = frame.object_id;
          item.encoding = core::udp::encoding_from_control(frame.control);
          item.snapshot = core::udp::snapshot_from_control(frame.control);
        }
      } else {
        log::warn("Duplicated fragment"sv);
      }
    });
  };
  if (process()) {
    // all good
  } else {
    if (distance(seqno) == MAX_BUFFERS) {
      // special case: we might have some "good" messages towards the end of the buffer
      auto next_seqno = seqno;
      auto ok = true;
      // note! reverse
      for (size_t offset = 0; offset < MAX_BUFFERS; ++offset) {
        auto seqno = next_seqno_ + (MAX_BUFFERS - offset - 1);
        auto &item = get_item(seqno);
        if (ok && item.ready) {
          next_seqno = seqno;
          continue;
        }
        ok = false;
        item.reset();
      }
      assert(!ok);  // we shouldn't get here if there wasn't an issue
      log::warn("+++ SEQUENCE GAP {} --> {} +++"sv, next_seqno_, next_seqno);
      next_seqno_ = next_seqno;
      // XXX copy + dispatch
    } else {
      // gap is too big: reset
      reset();
      log::warn("+++ SEQUENCE GAP {} --> {} +++"sv, next_seqno_, seqno);
      next_seqno_ = seqno;
    }
    if (process()) {
      // all good
    } else {
      log::fatal("Unexpected: buffer is full"sv);
    }
    // result = Status::RESET;
  }
  // log::debug("next={}"sv, next_seqno_);
  return result;
}

namespace {
// deals with overflow
template <typename T>
constexpr size_t diff(T lhs, T rhs) {
  if (lhs >= rhs)
    return lhs - rhs;
  return lhs + (std::numeric_limits<T>::max() - rhs) + 1;
}
// normal
static_assert(diff<uint8_t>(0, 0) == 0);
static_assert(diff<uint8_t>(1, 0) == 1);
static_assert(diff<uint8_t>(2, 0) == 2);
static_assert(diff<uint8_t>(255, 255) == 0);
static_assert(diff<uint8_t>(0, 255) == 1);
static_assert(diff<uint8_t>(1, 255) == 2);
// replay
static_assert(diff<uint8_t>(0, 1) == 255);
static_assert(diff<uint8_t>(0, 2) == 254);
static_assert(diff<uint8_t>(255, 0) == 255);
static_assert(diff<uint8_t>(254, 0) == 254);
static_assert(diff<uint8_t>(255, 1) == 254);
static_assert(diff<uint8_t>(254, 1) == 253);
}  // namespace

size_t Buffer::distance(uint32_t seqno) {
  return diff(seqno, next_seqno_);
}

bool Buffer::is_replay(uint32_t seqno) {
  constexpr auto const THRESHOLD = 3 * (uint32_t{1} << 30);
  auto tmp = diff(seqno, next_seqno_);
  return tmp > THRESHOLD;
}

void Buffer::advance() {
  get_buffer(next_seqno_, [&](auto &item) {
    assert(!item.ready);
    assert(item.count == 0);
    assert(item.available.count() == 0);
    assert(item.size == 0);
  });
  ++next_seqno_;
}

Buffer::Item &Buffer::get_item(uint32_t seqno) {
  assert(distance(seqno) < MAX_BUFFERS);
  return assembly_[seqno % MAX_BUFFERS];
}

template <typename Callback>
bool Buffer::get_buffer(uint32_t seqno, Callback callback) {
  if (distance(seqno) < MAX_BUFFERS) {
    callback(assembly_[seqno % MAX_BUFFERS]);
    return true;
  } else {
    return false;
  }
}

void Buffer::reset() {
  for (auto &item : assembly_)
    item.reset();
}

// Item

Buffer::Item::Item() : payload(MAX_BUFFER) {
}

void Buffer::Item::reset() {
  ready = false;
  count = {};
  available.reset();
  size = {};
}

}  // namespace tools
}  // namespace udp_subscriber
}  // namespace roq
