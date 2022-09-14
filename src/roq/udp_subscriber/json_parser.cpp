/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/json_parser.hpp"

#include <nlohmann/json.hpp>

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

namespace {
std::string_view get_string_view(auto &obj) {
  if (obj.is_null())
    return {};
  return obj.template get<std::string_view>();
}
}  // namespace

void JSONParser::dispatch_helper(
    Handler &handler,
    std::span<std::byte const> const &payload,
    TraceInfo const &trace_info,
    Shared &shared,
    core::udp::Frame const &frame) {
  auto message = std::string_view{reinterpret_cast<char const *>(std::data(payload)), std::size(payload)};
  auto json = nlohmann::json::parse(message);
  auto type = json[0].get<std::string_view>();
  log::debug(R"(type="{}")"sv, type);
  if (type.compare("TopOfBook"sv) == 0) {
  } else if (type.compare("CustomMetricsUpdate"sv) == 0) {
    auto &measurements = shared.measurements;
    measurements.clear();
    auto obj = json[1];
    auto label = obj["label"sv].get<std::string_view>();
    auto account = get_string_view(obj["account"sv]);
    auto exchange = obj["exchange"sv].get<std::string_view>();
    auto symbol = obj["symbol"sv].get<std::string_view>();
    for (auto &item : obj["measurements"sv]) {
      auto name = item["name"sv].get<std::string_view>();
      auto value = item["value"sv].get<double>();
      measurements.push_back({name, value});
    }
    auto update_type = obj["update_type"sv].get<std::string_view>();
    CustomMetricsUpdate const custom_metrics_update{
        .user = {},
        .label = label,
        .account = account,
        .exchange = exchange,
        .symbol = symbol,
        .measurements = measurements,
        .update_type = magic_enum::enum_cast<UpdateType>(update_type).value(),
    };
    log::debug("{} {}"sv, frame, custom_metrics_update);
    create_trace_and_dispatch(handler, trace_info, custom_metrics_update, frame);
  } else {
    log::warn(R"(Unexpected: type="{}")"sv, type);
  }
}

}  // namespace udp_subscriber
}  // namespace roq
