#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE( "integer comparison", 
  "the integers should compare equal to themselves")
{
  REQUIRE(5==5);
  REQUIRE(6==6);
  REQUIRE(0==0);
}
