/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/udp_subscriber/parser_factory.hpp"

#include "roq/utils/compare.hpp"

#include "roq/logging.hpp"

#include "roq/udp_subscriber/flags.hpp"

#include "roq/udp_subscriber/fbs_parser.hpp"
#include "roq/udp_subscriber/json_parser.hpp"

using namespace std::literals;

namespace roq {
namespace udp_subscriber {

std::unique_ptr<Parser> ParserFactory::create() {
  auto encoding = Flags::encoding();
  if (utils::case_insensitive_compare(encoding, "json"sv) == 0) {
    return std::make_unique<JSONParser>();
  } else if (
      utils::case_insensitive_compare(encoding, "fbs"sv) == 0 ||
      utils::case_insensitive_compare(encoding, "flatbuffers"sv) == 0) {
    return std::make_unique<FBSParser>();
  } else {
    log::fatal(R"(Unexpected: encoding="{}")"sv, encoding);
  }
}

}  // namespace udp_subscriber
}  // namespace roq
