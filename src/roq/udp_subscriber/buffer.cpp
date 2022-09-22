/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/buffer.hpp"

#include <limits>

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

namespace {
const constexpr size_t MAX_BUFFERS = 16;
}

Buffer::Buffer() : assembly_(MAX_BUFFERS) {
}

Buffer::Status Buffer::update_helper(core::udp::Frame const &frame, std::span<std::byte const> const &payload) {
  log::debug("frame={}, len(payload)={}"sv, frame, std::size(payload));
  auto result = Status::UNDEFINED;
  if (frame.source_seqno == (seqno_ + 1) && frame.fragment_number_max == 0) {  // simple
    seqno_ = frame.source_seqno;
    result = Status::SIMPLE;
  } else {
    if (get_buffer(frame, [&](auto &item) {  // fragmented or out of sequence
          auto index = frame.fragment_number;
          auto last = index == frame.fragment_number_max;
          // log::debug("index={}, last={}"sv, index, last);
          if (!item.available[index]) {
            auto offset = frame.fragment_number * core::udp::MAX_PAYLOAD_LENGTH;
            std::memcpy(std::data(item.payload) + offset, std::data(payload), std::size(payload));
            if (last)
              item.size = offset + std::size(payload);
            item.available.set(index);
            ++item.count;
            item.ready = item.count == static_cast<size_t>(frame.fragment_number_max) + 1;  // note! +1
            /*
            log::debug(
                "offset={}, availabe={}, count={}, size={}, ready={}"sv,
                offset,
                item.available.count(),
                item.count,
                item.size,
                item.ready);
            */
            if (item.ready) {
              // exclude heartbeat
              if (!(frame.fragment_number == 0 && frame.fragment_number_max == 0 && std::empty(payload)))
                assert(item.size);
              assert(item.available.count() == item.count);
            }
          } else {
            log::warn("Duplicated fragment"sv);
          }
          if (frame.source_seqno == (seqno_ + 1) && item.ready) {
            result = Status::MULTIPLE;
          } else {
            result = Status::BUFFERING;
          }
        })) {
    } else {
      for (auto &item : assembly_)
        item.reset();
      seqno_ = frame.source_seqno;
      // XXX FIXME avoid dropping this update
      result = Status::RESET;
    }
  }
  assert(result != Status::UNDEFINED);
  return result;
}

std::span<std::byte const> Buffer::get_next(core::udp::Frame const &frame) {
  if (distance(frame) == 1) {
    auto &item = assembly_[frame.source_seqno % MAX_BUFFERS];
    if (item.ready) {
      assert(item.size);
      auto result = std::span{std::data(item.payload), item.size};
      item.reset();
      ++seqno_;
      return result;
    }
  }
  return {};
}

template <typename Callback>
bool Buffer::get_buffer(core::udp::Frame const &frame, Callback callback) {
  if (distance(frame) < MAX_BUFFERS) {
    callback(assembly_[frame.source_seqno % MAX_BUFFERS]);
    return true;
  } else {
    return false;
  }
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

size_t Buffer::distance(core::udp::Frame const &frame) {
  return diff(frame.source_seqno, seqno_);
}

// Item

Buffer::Item::Item() : payload(core::udp::MAX_PAYLOAD_LENGTH * 256) {
}

void Buffer::Item::reset() {
  ready = false;
  count = {};
  available.reset();
  size = {};
}

}  // namespace udp_subscriber
}  // namespace roq
