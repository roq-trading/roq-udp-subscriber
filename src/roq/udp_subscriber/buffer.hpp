/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <bitset>
#include <vector>

#include "roq/core/udp/frame.hpp"

#include "roq/udp_subscriber/header.hpp"

namespace roq {
namespace udp_subscriber {

struct Buffer final {
  enum class Status {
    BUFFERING,
    DISPATCH,
    READY,
  };

  Buffer();

  template <typename Callback>
  void operator()(core::udp::Frame const &frame, std::span<std::byte const> const &payload, Callback callback) {
    auto status = update(frame, payload);
    switch (status) {
      using enum Status;
      case BUFFERING:
        break;
      case DISPATCH: {
        Header header{
            .session_id = frame.session_id,
            .seqno = frame.seqno,
            .last_seqno = frame.last_seqno,
            .object_type = frame.object_type,
            .object_id = frame.object_id,
            .encoding = frame.encoding,
        };
        callback(header, payload);
        [[fallthrough]];  // note! possible re-ordering
      }
      case READY:
        while (true) {
          auto &item = get_item(next_seqno_);
          if (!item.ready)
            break;
          Header header{
              .session_id = session_id_,
              .seqno = next_seqno_,
              .last_seqno = item.last_seqno,
              .object_type = item.object_type,
              .object_id = item.object_id,
              .encoding = item.encoding,
          };
          auto payload = std::span{std::data(item.payload), item.size};
          callback(header, payload);
          item.reset();
          advance();
        }
        break;
    }
  }

 protected:
  Status update(core::udp::Frame const &, std::span<std::byte const> const &payload);

  size_t distance(uint32_t seqno);

  void advance();

  struct Item final {
    Item();
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

}  // namespace udp_subscriber
}  // namespace roq
