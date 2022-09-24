/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/buffer.hpp"

#include <algorithm>
#include <limits>

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

namespace {
const constexpr size_t MAX_PAYLOAD = core::udp::MAX_PAYLOAD_LENGTH;
const constexpr size_t MAX_BUFFER = MAX_PAYLOAD * size_t{256};
const constexpr size_t MAX_BUFFERS = 16;
}  // namespace

Buffer::Buffer() : assembly_(MAX_BUFFERS) {
}

Buffer::Status Buffer::update(core::udp::Frame const &frame, std::span<std::byte const> const &payload) {
  // log::debug("frame={}, len(payload)={}"sv, frame, std::size(payload));
  auto seqno = frame.seqno;
  // log::debug("seqno={}, next={}"sv, seqno, next_seqno_);
  // reset
  if (session_id_ != frame.session_id) {
    if (session_id_)
      log::warn("+++ SESSION RESET {} +++"sv, frame.session_id);
    session_id_ = frame.session_id;
    next_seqno_ = seqno;
  }
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
          assert(item.available.count() == item.count);  // DEBUG
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
      auto next_seqno = seqno;
      auto ok = true;
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
      reset();
      log::warn("+++ SEQUENCE GAP {} --> {} +++"sv, next_seqno_, seqno);
      next_seqno_ = seqno;
    }
    if (process()) {
      // all good
    } else {
      log::fatal("Unexpected"sv);
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
static_assert(diff<uint8_t>(0, 0) == 0);
static_assert(diff<uint8_t>(1, 0) == 1);
static_assert(diff<uint8_t>(2, 0) == 2);
static_assert(diff<uint8_t>(255, 255) == 0);
static_assert(diff<uint8_t>(0, 255) == 1);
static_assert(diff<uint8_t>(1, 255) == 2);
}  // namespace

size_t Buffer::distance(uint32_t seqno) {
  return diff(seqno, next_seqno_);
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

}  // namespace udp_subscriber
}  // namespace roq
