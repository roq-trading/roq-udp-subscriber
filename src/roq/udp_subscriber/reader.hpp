/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <vector>

#include "roq/core/udp/frame.hpp"

namespace roq {
namespace udp_subscriber {

struct Reader final {
  Reader();

  template <typename Receiver, typename Callback>
  bool recv(Receiver &receiver, Callback callback) {
    auto bytes = receiver.recv(buffer_);
    auto message = std::span{std::data(buffer_), bytes};
    if (validate(message)) {
      auto &frame = *reinterpret_cast<core::udp::Frame const *>(std::data(message));
      auto payload = message.subspan(sizeof(frame));
      callback(frame, payload);
      return true;
    } else {
      return false;
    }
  }

 protected:
  bool validate(std::span<std::byte const> const &message);

 private:
  std::vector<std::byte> buffer_;
};

}  // namespace udp_subscriber
}  // namespace roq
