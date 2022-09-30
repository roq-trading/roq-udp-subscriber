/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/server.hpp"

#include "roq/io/buffer.hpp"
#include "roq/io/context.hpp"

#include "roq/io/net/udp/receiver.hpp"

#include "roq/udp_subscriber/buffer.hpp"
#include "roq/udp_subscriber/parser.hpp"
#include "roq/udp_subscriber/reader.hpp"
#include "roq/udp_subscriber/shared.hpp"

namespace roq {
namespace udp_subscriber {

class Incremental final : public io::net::udp::Receiver::Handler, public Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<GatewaySettings> const &) = 0;
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<GatewayStatus> const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
    virtual void operator()(Trace<TradeSummary> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<CustomMetrics> const &, bool is_last) = 0;
  };

  Incremental(Handler &, io::Context &, uint16_t stream_id, Shared &);

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  // io::net::udp::Receiver::Handler
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

  // Parser::Handler
  void operator()(Trace<Parser::Heartbeat> const &, Header const &) override;
  void operator()(Trace<GatewaySettings> const &, Header const &) override;
  void operator()(Trace<StreamStatus> const &, Header const &) override;
  void operator()(Trace<ExternalLatency> const &, Header const &) override;
  void operator()(Trace<GatewayStatus> const &, Header const &) override;
  void operator()(Trace<ReferenceData> const &, Header const &) override;
  void operator()(Trace<MarketStatus> const &, Header const &) override;
  void operator()(Trace<TopOfBook> const &, Header const &) override;
  void operator()(Trace<MarketByPriceUpdate> const &, Header const &) override;
  void operator()(Trace<TradeSummary> const &, Header const &) override;
  void operator()(Trace<StatisticsUpdate> const &, Header const &) override;
  void operator()(Trace<CustomMetricsUpdate> const &, Header const &) override;

  template <typename T>
  bool update(Trace<T> const &, Header const &);

  void publish_stream_status(TraceInfo const &, ConnectionStatus);

 private:
  Handler &handler_;
  // config
  uint16_t stream_id_ = {};
  // shared
  Shared &shared_;
  // io
  std::unique_ptr<io::net::udp::Receiver> receiver_;
  Reader reader_;
  Buffer buffer_;
  // status
  std::chrono::nanoseconds last_update_time_ = {};
  ConnectionStatus connection_status_ = {};
};

}  // namespace udp_subscriber
}  // namespace roq
