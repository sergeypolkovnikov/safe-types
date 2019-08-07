#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "safe_types.h"

class HorizontalDim;
using Horizontal = safe_types::singleton<int, HorizontalDim>;

class DistanceDim;
using meters = safe_types::simple_type<int, std::ratio<1>, DistanceDim>;
using kilometers = safe_types::simple_type<int, std::kilo, DistanceDim>;

TEST_CASE("test singleton equality", "[singleton]")
{
    Horizontal h1{ 1 };
    Horizontal h2{ 2 };
    REQUIRE(h1 != h2);
}

TEST_CASE("test simple equality", "[simple]")
{
    meters d1{ 1000 };
    kilometers d2{ 1 };
    REQUIRE(d1 == d2);
}

TEST_CASE("test simple sum", "[simple]")
{
    meters d1{ 1000 };
    kilometers d2{ 1 };
    REQUIRE(kilometers{2} == d1 + d2);
}

TEST_CASE("test comlex multiply", "[complex]")
{
    meters d1{ 1000 };
    kilometers d2{ 1 };
    auto mult = d1 * d2;
    REQUIRE(mult.value() == 1000);
    using km2 = decltype(std::declval<kilometers>() * std::declval<kilometers>());
    REQUIRE(mult == km2{ 1 });
    using m2 = decltype(std::declval<meters>() * std::declval<meters>());
    REQUIRE(mult == m2{ 1000000 });
}

TEST_CASE("test complex div", "[complex]")
{
    using km2 = decltype(std::declval<kilometers>() * std::declval<kilometers>());
    auto km = km2{ 1 } / kilometers{ 1 };
    REQUIRE(km.value() == 1);
}

TEST_CASE("test degenerated complex div", "[complex]")
{
    auto mult = meters{ 1 } / meters{ 1 };
    REQUIRE(std::is_same_v<decltype(mult), int>);
}
