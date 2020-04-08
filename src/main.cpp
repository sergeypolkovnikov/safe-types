#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <iostream>
#include <set>
#include <unordered_set>

#include "physical_types.h"

TEST_CASE("test singleton equality", "[singleton]")
{
    static_assert(std::is_same_v<safe_types::internal::parameter_for_copy_t<int>, const int>, "parameter_for_copy_t should be int");

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

TEST_CASE("test internal::trim", "[complex]")
{
    using type1 = safe_types::internal::tuple_dim<safe_types::DistanceDim, safe_types::DurationDim>;
    using sametype = safe_types::internal::tuple_dim<safe_types::DurationDim, safe_types::DistanceDim>;
    using type2 = safe_types::internal::tuple_dim<safe_types::DurationDim>;
    REQUIRE(std::is_same<safe_types::internal::trim<type1, type1>::num, safe_types::internal::tuple_dim<>>::value == true);
    REQUIRE(std::is_same<safe_types::internal::trim<type1, sametype>::num, safe_types::internal::tuple_dim<>>::value == true);
    REQUIRE(std::is_same<safe_types::internal::trim<type1, type2>::num, safe_types::internal::tuple_dim<>>::value == false);
}

TEST_CASE("test type equality", "[complex]")
{
    using one_dim = safe_types::internal::tuple_dim<safe_types::DistanceDim, safe_types::DurationDim>;
    using two_dim = safe_types::internal::tuple_dim<safe_types::DurationDim, safe_types::DistanceDim>;
    using one_type = safe_types::internal::dim_ratio<one_dim, safe_types::internal::tuple_dim<>>;
    using two_type = safe_types::internal::dim_ratio<two_dim, safe_types::internal::tuple_dim<>>;
    REQUIRE(safe_types::is_same<one_type, two_type>::value == true);
}

TEST_CASE("test order", "[complex]")
{
    using acceleration = decltype(std::declval<safe_types::millimeters>() / std::declval<safe_types::seconds>() / std::declval<safe_types::seconds>());
    const auto g = acceleration{ 9800 };
    const auto newtons1 = safe_types::kilograms{ 70 } * g;
    const auto newtons2 = g * safe_types::kilograms{ 70 };
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

TEST_CASE("test div modulus int", "[complex]")
{
    const auto bytes_count = safe_types::bytes{ 1025 };
    const auto modul_bytes_count = bytes_count % 1024;
    REQUIRE(modul_bytes_count.value() == 1);
}

TEST_CASE("test div modulus", "[complex]")
{
    const auto bytes_count = safe_types::bytes{1025};
    const auto modul_bytes_count = bytes_count % safe_types::kilobytes{ 1 };
    REQUIRE(modul_bytes_count.value() == 1);
}

TEST_CASE("test degenerated multiply", "[complex]")
{
    using DenomSeconds = decltype(1 / std::declval<safe_types::seconds>());
    const auto degenerated_value = safe_types::hours{ 1 } * DenomSeconds{ 60 };
    INFO(typeid(decltype(degenerated_value)).name());
    static_assert(std::is_same_v<std::decay_t<decltype(degenerated_value)>, DenomSeconds::underlying_type>, "degenerated value should be the same type of seconds");
    REQUIRE(degenerated_value == 60);
}

struct A
{
    static size_t count;
    A() { count++; }
    A(A const&) { count++; }
    A& operator= (A const&) { count++; return *this; }
    A(A &&) { }
    A& operator= (A &&) noexcept { return *this; }
};

size_t A::count = 0;

TEST_CASE("copy", "[complex]")
{
    class ADim;
    using AS = safe_types::singleton<A, ADim>;
    AS a{ A() }; //1
    AS b{ A() }; //2
    auto c = b; //3
    auto d = std::move(b); // 3
    REQUIRE(A::count == 3);
}
