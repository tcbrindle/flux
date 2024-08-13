
// Copyright (c) 2024 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include <iostream>

#include "test_utils.hpp"

namespace {

using namespace std::string_view_literals;

constexpr bool test_flatten_with_single_pass()
{
    // Single-pass outer, multipass inner, sequence pattern
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::flatten_with(single_pass_only(std::move(arr)), "-"sv);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, char const&>);
        static_assert(std::same_as<flux::value_t<S>, char>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, char const&&>);

        STATIC_CHECK(check_equal(seq, "111-222-333"sv));
    }

    // Single-pass outer, multipass inner, value pattern
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::flatten_with(single_pass_only(std::move(arr)), '-');

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, "111-222-333"sv));
    }

    // Multipass outer, prvalue inner
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::map(arr, [](auto sv) { return sv; }).flatten_with('-');

        STATIC_CHECK(check_equal(seq, "111-222-333"sv));
    }

    // flatten_with with an empty pattern is the same as flatten()
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::flatten_with(single_pass_only(std::move(arr)), ""sv);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, "111222333"sv));
    }

    // Empty inner sequence is handled correctly, with delims on both sides
    {
        std::array<std::string_view, 6> arr{
            "123"sv, ""sv, "456"sv, ""sv, "7"sv, "89"sv
        };
        
        auto seq = single_pass_only(std::move(arr)).flatten_with('-');

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, "123--456--7-89"sv));
    }

    // Empty outer sequence is handled correctly
    {
        auto arr = std::array<std::array<int, 3>, 0>{};

        auto seq = flux::flatten_with(single_pass_only(std::move(arr)), 999);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(seq.count() == 0);
    }


    return true;
}
static_assert(test_flatten_with_single_pass());

}

TEST_CASE("flatten_with")
{
    bool res = test_flatten_with_single_pass();
    REQUIRE(res);
}