/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <bitset>
#include <vector>

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/tools/header.hpp"

namespace roq {
namespace udp_subscriber {
namespace tools {

struct Buffer final {
  enum class Status {
    BUFFERING,
    DISPATCH,
    READY,
  };

  Buffer();

  Buffer(Buffer &&) = default;
  Buffer(Buffer const &) = delete;

  template <typename Callback>
  bool operator()(core::udp::Frame const &frame, std::span<std::byte const> const &payload, Callback callback) {
    auto result = false;
    auto status = update(frame, payload);
    switch (status) {
      using enum Status;
      case BUFFERING:
        break;
      case DISPATCH: {
        auto header = Header{
            .session_id = frame.session_id,
            .seqno = frame.seqno,
            .last_seqno = frame.last_seqno,
            .object_type = frame.object_type,
            .object_id = frame.object_id,
            .encoding = core::udp::encoding_from_control(frame.control),
            .snapshot = core::udp::snapshot_from_control(frame.control),
        };
        result = true;
        callback(header, payload);
        [[fallthrough]];  // note! possible re-ordering
      }
      case READY:
        while (true) {
          auto &item = get_item(next_seqno_);
          if (!item.ready)
            break;
          auto header = Header{
              .session_id = session_id_,
              .seqno = next_seqno_,
              .last_seqno = item.last_seqno,
              .object_type = item.object_type,
              .object_id = item.object_id,
              .encoding = item.encoding,
              .snapshot = item.snapshot,
          };
          auto payload = std::span{std::data(item.payload), item.size};
          result = true;
          callback(header, payload);
          item.reset();
          advance();
        }
        break;
    }
    return result;
  }

 protected:
  Status update(core::udp::Frame const &, std::span<std::byte const> const &payload);

  size_t distance(uint32_t seqno);
  bool is_replay(uint32_t seqno);

  void advance();

  struct Item final {
    Item();

    Item(Item &&) = default;
    Item(Item const &) = delete;

    void reset();

    bool ready = false;
    size_t count = {};
    std::bitset<256> available;
    std::vector<std::byte> payload;
    size_t size = {};
    //
    uint32_t last_seqno = {};
    uint8_t object_type = {};
    uint16_t object_id = {};
    core::udp::Encoding encoding = {};
    bool snapshot = {};
  };

  Item &get_item(uint32_t seqno);

  template <typename Callback>
  bool get_buffer(uint32_t seqno, Callback);

  void reset();

 private:
  uint16_t session_id_ = {};
  uint32_t next_seqno_ = {};
  std::vector<Item> assembly_;
};

}  // namespace tools
}  // namespace udp_subscriber
}  // namespace roq
