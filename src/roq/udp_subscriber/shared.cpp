/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/udp_subscriber/shared.hpp"

#include "roq/utils/update.hpp"

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
    : dispatcher_{dispatcher}, settings{settings}, decoder{roq::codec::fbs::Decoder::create()} {
}

bool Shared::update(Mask<SupportType> value) {
  auto masked = SUPPORTS.logical_and(value);
  return utils::update(supports, masked);
}

}  // namespace udp_subscriber
}  // namespace roq
