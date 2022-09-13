/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/shared.hpp"

#include "roq/udp_subscriber/flags.hpp"

namespace roq {
namespace udp_subscriber {

Shared::Shared(server::Dispatcher &dispatcher) : dispatcher_(dispatcher), decoder(1024, 1024, 1024, 1024, 1024, 1024) {
}

}  // namespace udp_subscriber
}  // namespace roq
