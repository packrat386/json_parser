#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <json_parser.hpp>

#include <fstream>

TEST_CASE("simple example test") {
  std::ifstream input("test/data/simple.json");
  
  REQUIRE_NOTHROW(json_parser::parse(input));
}
