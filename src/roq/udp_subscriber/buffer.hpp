/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <bitset>
#include <vector>

#include "roq/core/udp/frame.hpp"

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
      case DISPATCH:
        callback(frame, payload);  // XXX frame.source_seqno
        [[fallthrough]];           // note!
      case READY:
        while (true) {
          auto &item = get_item(next_seqno_);
          if (!item.ready)
            break;
          auto payload = std::span{std::data(item.payload), item.size};
          callback(frame, payload);  // XXX next_seqno_
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
  };

  Item &get_item(uint32_t seqno);

  template <typename Callback>
  bool get_buffer(uint32_t seqno, Callback);

  void reset();

 private:
  uint32_t session_id_ = {};
  uint32_t next_seqno_ = {};
  std::vector<Item> assembly_;
};

}  // namespace udp_subscriber
}  // namespace roq
