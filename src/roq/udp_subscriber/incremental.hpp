/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <memory>
#include <vector>

#include "roq/server.hpp"

#include "roq/io/buffer.hpp"
#include "roq/io/context.hpp"

#include "roq/io/net/udp/receiver.hpp"

#include "roq/udp_subscriber/parser.hpp"
#include "roq/udp_subscriber/reader.hpp"
#include "roq/udp_subscriber/shared.hpp"

#include "roq/udp_subscriber/tools/buffer.hpp"

namespace roq {
namespace udp_subscriber {

struct Incremental final : public io::net::udp::Receiver::Handler, public Parser::Handler {
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

  Incremental(Incremental &&) = default;
  Incremental(Incremental const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  // io::net::udp::Receiver::Handler
  void operator()(io::net::udp::Receiver::Read const &) override;
  void operator()(io::net::udp::Receiver::Error const &) override;

  // Parser::Handler
  void operator()(Trace<Parser::Heartbeat> const &, tools::Header const &) override;
  void operator()(Trace<GatewaySettings> const &, tools::Header const &) override;
  void operator()(Trace<StreamStatus> const &, tools::Header const &) override;
  void operator()(Trace<ExternalLatency> const &, tools::Header const &) override;
  void operator()(Trace<GatewayStatus> const &, tools::Header const &) override;
  void operator()(Trace<ReferenceData> const &, tools::Header const &) override;
  void operator()(Trace<MarketStatus> const &, tools::Header const &) override;
  void operator()(Trace<TopOfBook> const &, tools::Header const &) override;
  void operator()(Trace<MarketByPriceUpdate> const &, tools::Header const &) override;
  void operator()(Trace<TradeSummary> const &, tools::Header const &) override;
  void operator()(Trace<StatisticsUpdate> const &, tools::Header const &) override;
  void operator()(Trace<CustomMetricsUpdate> const &, tools::Header const &) override;

  template <typename T>
  bool update(Trace<T> const &, tools::Header const &);

  void publish_stream_status(TraceInfo const &, Mask<SupportType>, ConnectionStatus);

 private:
  Handler &handler_;
  // config
  uint16_t stream_id_ = {};
  // shared
  Shared &shared_;
  // io
  std::vector<std::unique_ptr<io::net::udp::Receiver>> receivers_;
  Reader reader_;
  tools::Buffer buffer_;
  // status
  std::chrono::nanoseconds last_update_time_ = {};
  ConnectionStatus connection_status_ = {};
  Mask<SupportType> supports_;
};

}  // namespace udp_subscriber
}  // namespace roq
