/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/udp_subscriber/config.hpp"
#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

class Gateway final : public server::Handler {
 public:
  Gateway(server::Dispatcher &, Config const &);

 protected:
  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  void operator()(metrics::Writer &) override;

 private:
  server::Dispatcher &dispatcher_;
  // io
  std::unique_ptr<io::Context> context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
};

}  // namespace udp_subscriber
}  // namespace roq
