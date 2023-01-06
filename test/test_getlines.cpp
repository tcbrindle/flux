// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <sstream>
#include <vector>

constexpr const auto& test_str1 = "Line1\nLine2\nLine3";

TEST_CASE("getlines")
{
    std::istringstream iss(test_str1);

    auto seq = flux::getlines(iss);

    static_assert(flux::sequence<decltype(seq)>);
    static_assert(!flux::multipass_sequence<decltype(seq)>);

    auto cur = seq.first();
    REQUIRE(seq[cur] == "Line1");
    seq.inc(cur);
    REQUIRE(seq[cur] == "Line2");
    seq.inc(cur);
    REQUIRE(seq[cur] == "Line3");
    seq.inc(cur);
    REQUIRE(seq.is_last(cur));

    // Make sure assertion fires
    REQUIRE_THROWS_AS(seq.inc(cur), flux::unrecoverable_error);
}

TEST_CASE("getlines to vector")
{
    std::istringstream iss(test_str1);

    auto const vec = flux::getlines(iss).to<std::vector>();

    static_assert(std::same_as<decltype(vec)::value_type, std::string>);

    REQUIRE(vec == std::vector<std::string>{"Line1", "Line2", "Line3"});
}

TEST_CASE("getlines with custom delimiter")
{
    using namespace std::string_view_literals;

    std::istringstream iss("Lorem ipsum dolor sit amet");

    auto seq = flux::getlines(iss, ' ');

    REQUIRE(flux::equal(seq, flux::split_string("Lorem ipsum dolor sit amet"sv, ' ')));
}

constexpr const auto& test_str2 = L"Line1\nLine2\nLine3";

TEST_CASE("getlines with wide strings")
{
    std::wistringstream iss(test_str2);

    auto const vec = flux::getlines(iss).to<std::vector>();

    static_assert(std::same_as<decltype(vec)::value_type, std::wstring>);

    REQUIRE(vec == std::vector<std::wstring>{L"Line1", L"Line2", L"Line3"});
}
