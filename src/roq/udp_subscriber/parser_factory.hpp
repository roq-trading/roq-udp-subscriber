/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/udp_subscriber/parser.hpp"

namespace roq {
namespace udp_subscriber {

struct ParserFactory final {
  static std::unique_ptr<Parser> create();
};

}  // namespace udp_subscriber
}  // namespace roq
