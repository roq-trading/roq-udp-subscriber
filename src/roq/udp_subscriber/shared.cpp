/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/udp_subscriber/shared.hpp"

#include "roq/utils/update.hpp"

#include "roq/udp_subscriber/flags.hpp"

namespace roq {
namespace udp_subscriber {

// === CONSTANTS ===

namespace {
auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    // SupportType::MARKET_BY_PRICE,
    // SupportType::MARKET_BY_ORDER,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher_{dispatcher}, settings{settings}, decoder{1024, 1024, 1024, 1024, 1024, 1024},
      final_bids(settings.cache.mbp_max_depth), final_asks(settings.cache.mbp_max_depth) {
}

bool Shared::update(Mask<SupportType> value) {
  auto masked = SUPPORTS.logical_and(value);
  return utils::update(supports, masked);
}

}  // namespace udp_subscriber
}  // namespace roq
