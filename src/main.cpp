#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <iostream>
#include <set>
#include <unordered_set>

#include "physical_types.h"

TEST_CASE("test singleton equality", "[singleton]")
{
    class HorizontalDim;
    using Horizontal = safe_types::singleton<int, HorizontalDim>;
    Horizontal h1{ 1 };
    Horizontal h2{ 2 };
    REQUIRE(Horizontal{ 1 } != Horizontal{ 2 });
}

TEST_CASE("test simple equality", "[simple]")
{
    REQUIRE(safe_types::meters{ 1000 } == safe_types::kilometers{ 1 });
}

TEST_CASE("test simple less", "[simple]")
{
    REQUIRE(safe_types::meters{ 999 } < safe_types::kilometers{ 1 });
}

TEST_CASE("test simple less or equal", "[simple]")
{
    REQUIRE(safe_types::meters{ 999 } <= safe_types::kilometers{ 1 });
    REQUIRE(safe_types::meters{ 1000 } <= safe_types::kilometers{ 1 });
}

TEST_CASE("test simple sum", "[simple]")
{
    REQUIRE(safe_types::kilometers{ 2 } == safe_types::meters{ 1000 } +safe_types::kilometers{ 1 });
}

TEST_CASE("test comlex multiply", "[complex]")
{
    safe_types::meters d1{ 1000 };
    safe_types::kilometers d2{ 1 };
    const auto mult = d1 * d2;
    REQUIRE(mult.value() == 1000);
    using km2 = decltype(std::declval<safe_types::kilometers>() * std::declval<safe_types::kilometers>());
    REQUIRE(mult == km2{ 1 });
    using m2 = decltype(std::declval<safe_types::meters>() * std::declval<safe_types::meters>());
    REQUIRE(mult == m2{ 1000000 });
}

TEST_CASE("test complex div", "[complex]")
{
    using km2 = decltype(std::declval<safe_types::kilometers>() * std::declval<safe_types::kilometers>());
    auto km = km2{ 1 } / safe_types::kilometers{ 1 };
    REQUIRE(km.value() == 1);
}

TEST_CASE("test degenerated complex div", "[complex]")
{
    const auto mult = safe_types::meters{ 1 } / safe_types::meters{ 1 };
    REQUIRE(mult == 1);
}

TEST_CASE("test simple div by integral", "[simple]")
{
    const auto div = safe_types::meters{ 10 } / 10;
    REQUIRE(div.value() == 1);
}

TEST_CASE("test simple % by simple", "[simple]")
{
    const auto div = safe_types::meters{ 1010 } % safe_types::kilometers{ 1 };
    REQUIRE(div == safe_types::meters{ 10 });
}

TEST_CASE("test complex in set", "[complex]")
{
    std::set<safe_types::meters> meters;
    meters.insert(safe_types::meters{ 1 });
    meters.insert(safe_types::meters{ 2 });
    REQUIRE(meters.find(safe_types::meters{ 1 }) != meters.end());
    REQUIRE(meters.find(safe_types::meters{ 3 }) == meters.end());
}

TEST_CASE("test complex in unordered set", "[complex]")
{
    std::unordered_set<safe_types::meters> meters;
    meters.insert(safe_types::meters{ 1 });
    meters.insert(safe_types::meters{ 2 });
    REQUIRE(meters.find(safe_types::meters{ 1 }) != meters.end());
    REQUIRE(meters.find(safe_types::meters{ 3 }) == meters.end());
}

TEST_CASE("test strings", "[singleton]")
{
    class SomeStringDim;
    using SomeString = safe_types::singleton<std::string, SomeStringDim>;
    SomeString s{""};
    REQUIRE(SomeString{"1"} == SomeString{ "1" });
    std::unordered_set<SomeString> strings;
    strings.insert(s);
    REQUIRE(strings.find(SomeString{ "" }) != strings.end());
    REQUIRE(strings.find(SomeString{ "1" }) == strings.end());
}

TEST_CASE("test order", "[complex]")
{
    using acceleration = decltype(std::declval<safe_types::millimeters>() / std::declval<safe_types::seconds>() / std::declval<safe_types::seconds>());
    const auto g = acceleration{ 9800 };
    const auto newtons1 = safe_types::kilograms{ 70 } * g;
    const auto newtons2 = g * safe_types::kilograms{ 70 };
    static_assert(!std::is_same_v<decltype(newtons1), decltype(newtons2)>, "types should not be equal");
    // but
    REQUIRE(newtons1 == newtons2);
}

TEST_CASE("test conversions", "[complex]")
{
    using meters_per_sec = decltype(std::declval<safe_types::meters>() / std::declval<safe_types::seconds>());
    using kilometers_per_hour = decltype(std::declval<safe_types::kilometers>() / std::declval<safe_types::hours>());

    const auto bykespeed_kmh = kilometers_per_hour{36};
    const auto bykespeed_ms = meters_per_sec(bykespeed_kmh);
    REQUIRE(bykespeed_ms.value() == 10);
}
