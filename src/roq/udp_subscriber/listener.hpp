/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/server.hpp"

#include "roq/io/buffer.hpp"
#include "roq/io/context.hpp"

#include "roq/io/net/udp/receiver.hpp"

#include "roq/udp_subscriber/parser.hpp"
#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

class Listener final : public io::net::udp::Receiver::Handler, public Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<TopOfBook const> const &, bool is_last) = 0;
    virtual void operator()(Trace<CustomMetrics const> const &, bool is_last) = 0;
  };

  Listener(Handler &, io::Context &, uint16_t stream_id, Shared &);

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  // io::net::udp::Receiver::Handler
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

  // Parser::Handler
  void operator()(Trace<TopOfBook const> const &) override;
  void operator()(Trace<CustomMetricsUpdate const> const &) override;

 private:
  Handler &handler_;
  // config
  uint16_t stream_id_ = {};
  // shared
  Shared &shared_;
  // io
  std::unique_ptr<io::net::udp::Receiver> receiver_;
  io::Buffer receive_buffer_;
};

}  // namespace udp_subscriber
}  // namespace roq
