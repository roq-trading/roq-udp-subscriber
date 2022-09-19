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

class Snapshot final : public io::net::udp::Receiver::Handler, public Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
    virtual void operator()(Trace<CustomMetrics> const &, bool is_last) = 0;
  };

  Snapshot(Handler &, io::Context &, uint16_t stream_id, Shared &);

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  // io::net::udp::Receiver::Handler
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

  // Parser::Handler
  void operator()(Trace<Parser::Heartbeat> const &, core::udp::Frame const &) override;
  void operator()(Trace<ReferenceData> const &, core::udp::Frame const &) override;
  void operator()(Trace<MarketStatus> const &, core::udp::Frame const &) override;
  void operator()(Trace<TopOfBook> const &, core::udp::Frame const &) override;
  void operator()(Trace<CustomMetricsUpdate> const &, core::udp::Frame const &) override;

  template <typename T>
  bool update(Trace<T> const &, core::udp::Frame const &);

  void publish_stream_status(TraceInfo const &, ConnectionStatus);

 private:
  Handler &handler_;
  // config
  uint16_t stream_id_ = {};
  // shared
  Shared &shared_;
  // io
  std::unique_ptr<io::net::udp::Receiver> receiver_;
  io::Buffer receive_buffer_;
  // status
  std::chrono::nanoseconds last_update_time_ = {};
  ConnectionStatus connection_status_ = {};
  // EXPERIMENTAL
  uint32_t source_session_id_ = {};
  uint32_t source_seqno_ = {};
};

}  // namespace udp_subscriber
}  // namespace roq
