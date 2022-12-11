/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/shared.hpp"

#include "roq/udp_subscriber/flags.hpp"

namespace roq {
namespace udp_subscriber {

Shared::Shared(server::Dispatcher &dispatcher)
    : dispatcher_(dispatcher), decoder(1024, 1024, 1024, 1024, 1024, 1024),
      final_bids(server::Flags::cache_mbp_max_depth()), final_asks(server::Flags::cache_mbp_max_depth()) {
}

}  // namespace udp_subscriber
}  // namespace roq
