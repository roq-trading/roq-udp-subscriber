/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/trace.hpp"

#include "roq/custom_metrics_update.hpp"
#include "roq/top_of_book.hpp"

#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

struct Parser {
  struct Handler {
    virtual void operator()(Trace<TopOfBook const> const &) = 0;
    virtual void operator()(Trace<CustomMetrics const> const &) = 0;
  };

  virtual ~Parser() {}

  virtual size_t dispatch(Handler &, std::span<std::byte const> const &buffer, TraceInfo const &, Shared &) = 0;
};

}  // namespace udp_subscriber
}  // namespace roq
