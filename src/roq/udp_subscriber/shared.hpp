/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/node_hash_map.h>

#include <chrono>
#include <deque>
#include <string>
#include <utility>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/fbs/decoder.hpp"

namespace roq {
namespace udp_subscriber {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }
  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  /*
  template <typename Callback>
  bool find_state(auto object_type, auto object_id, Callback callback) {
    auto iter = waiting.find({object_type, object_id});
    if (iter != std::end(waiting)) {
      callback((*iter).second);
      return true;
    }
    return false;
  }
  */

 private:
  server::Dispatcher &dispatcher_;

 public:
  std::vector<Measurement> measurements;

  core::fbs::Decoder decoder;

  uint16_t session_id = {};

  struct State final {
    bool ready = false;
    std::optional<uint32_t> last_seqno = {};  // begin of collection or last update if good
  };
  absl::node_hash_map<std::pair<uint8_t, uint16_t>, State> state;
};

}  // namespace udp_subscriber
}  // namespace roq
