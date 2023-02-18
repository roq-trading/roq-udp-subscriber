/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/node_hash_map.h>

#include <chrono>
#include <deque>
#include <string>
#include <utility>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/fbs/decoder.hpp"

#include "roq/core/mbp/sequencer.hpp"

namespace roq {
namespace udp_subscriber {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  bool update(Mask<SupportType>);

 private:
  server::Dispatcher &dispatcher_;

 public:
  std::vector<Measurement> measurements;

  core::fbs::Decoder decoder;

  uint16_t session_id = {};

  struct State final {
    bool ready = false;
    std::optional<uint32_t> last_seqno = {};
  };
  absl::node_hash_map<std::pair<uint8_t, uint16_t>, State> state;

  absl::flat_hash_map<Symbol, core::mbp::Sequencer> mbp_sequencer;

  std::vector<MBPUpdate> final_bids, final_asks;

  Mask<SupportType> supports;
};

}  // namespace udp_subscriber
}  // namespace roq
