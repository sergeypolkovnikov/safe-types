#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "safe_types.h"

class HorizontalDim;
using Horizontal = safe_types::singleton<int, HorizontalDim>;

class DistanceDim;
using meters = safe_types::simple_type<int, std::ratio<1>, DistanceDim>;
using kilometers = safe_types::simple_type<int, std::kilo, DistanceDim>;

TEST_CASE("test singleton equality", "[singletoneq]")
{
    Horizontal h1{ 1 };
    Horizontal h2{ 2 };
    REQUIRE(h1 != h2);
}

TEST_CASE("test simple equality", "[simpleeq]")
{
    meters d1{ 1000 };
    kilometers d2{ 1 };
    REQUIRE(d1 == d2);
}

TEST_CASE("test simple sum", "[simplesum]")
{
    meters d1{ 1000 };
    kilometers d2{ 1 };
    REQUIRE(kilometers{2} == d1 + d2);
}

