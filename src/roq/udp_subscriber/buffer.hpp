/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <bitset>
#include <vector>

#include "roq/core/udp/frame.hpp"

namespace roq {
namespace udp_subscriber {

struct Buffer final {
  enum class Status {
    UNDEFINED,
    SIMPLE,
    BUFFERING,
    MULTIPLE,
    RESET,
  };

  Buffer();

  template <typename Callback>
  void operator()(core::udp::Frame const &frame, std::span<std::byte const> const &payload, Callback callback) {
    auto status = update_helper(frame, payload);
    switch (status) {
      using enum Status;
      case UNDEFINED:
        assert(false);
        break;
      case SIMPLE:
        callback(frame, payload);
        break;
      case BUFFERING:
        break;
      case MULTIPLE:
        while (true) {
          // XXX maybe we also need the seqno here?
          auto payload = get_next(frame);
          if (std::empty(payload))
            break;
          callback(frame, payload);  // XXX wrong frame
        }
        break;
      case RESET:
        break;
    }
  }

 protected:
  Status update_helper(core::udp::Frame const &, std::span<std::byte const> const &payload);
  std::span<std::byte const> get_next(core::udp::Frame const &);

  template <typename Callback>
  bool get_buffer(core::udp::Frame const &, Callback);

  size_t distance(core::udp::Frame const &);

 private:
  uint32_t session_id_ = {};
  uint32_t seqno_ = {};

  struct Item final {
    Item();
    void reset();

    bool ready = false;
    size_t count = {};
    std::bitset<256> available;
    std::vector<std::byte> payload;
    size_t size = {};
  };
  std::vector<Item> assembly_;
};

}  // namespace udp_subscriber
}  // namespace roq
