/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <deque>
#include <string>
#include <utility>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/core/fbs/decoder.hpp"

#include "roq/market/mbp/sequencer.hpp"

#include "roq/udp_subscriber/settings.hpp"

namespace roq {
namespace udp_subscriber {

struct Shared final {
  explicit Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  bool update(Mask<SupportType>);

 private:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;

  std::vector<Measurement> measurements;

  core::fbs::Decoder decoder;

  uint16_t session_id = {};

  struct State final {
    bool ready = false;
    std::optional<uint32_t> last_seqno = {};
  };
  utils::unordered_map<std::pair<uint8_t, uint16_t>, State> state;

  utils::unordered_map<std::string, market::mbp::Sequencer> mbp_sequencer;

  std::vector<MBPUpdate> final_bids, final_asks;

  Mask<SupportType> supports;
};

}  // namespace udp_subscriber
}  // namespace roq
