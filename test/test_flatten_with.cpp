
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

constexpr bool test_flatten_with_iterable()
{
    {
        // Contrive a not-sequence-of-not-sequences
        auto true_ = flux::pred::true_;
        std::array arr{
            std::views::filter("111"sv, true_),
            std::views::filter("222"sv, true_),
            std::views::filter("333"sv, true_)};
        auto view = std::views::filter(std::move(arr), true_);

        auto iter = flux::flatten_with(std::move(view), "-"sv);

        using F = decltype(iter);
        static_assert(flux::iterable<F>);
        static_assert(not flux::sequence<F>);
        static_assert(not flux::sized_iterable<F>);
        static_assert(std::same_as<flux::element_t<F>, char const&>);
        static_assert(std::same_as<flux::value_t<F>, char>);

        STATIC_CHECK(check_equal(iter, "111-222-333"sv));
    }

    return true;
}
static_assert(test_flatten_with_iterable());

constexpr bool test_flatten_with_single_pass()
{
    // Single-pass outer, multipass inner, sequence pattern
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::flatten_with(single_pass_only(std::move(arr)), "-"sv);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
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
        static_assert(not flux::sized_iterable<S>);
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
        static_assert(not flux::sized_iterable<S>);
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
        static_assert(not flux::sized_iterable<S>);
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
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(seq.count() == 0);
    }

    return true;
}
static_assert(test_flatten_with_single_pass());

constexpr bool test_flatten_with_multipass()
{
    // multipass outer, multipass inner, sequence pattern
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::flatten_with(arr, "-"sv);

        using S = decltype(seq);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, char const&>);
        static_assert(std::same_as<flux::value_t<S>, char>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, char const&&>);

        static_assert(flux::bidirectional_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(std::same_as<flux::element_t<S const>, char const&>);
        static_assert(std::same_as<flux::value_t<S const>, char>);
        static_assert(std::same_as<flux::rvalue_element_t<S const>, char const&&>);

        STATIC_CHECK(check_equal(seq, "111-222-333"sv));
    }

    // multipass outer, multipass inner, value pattern
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::flatten_with(arr, '&');

        STATIC_CHECK(check_equal(seq, "111&222&333"sv));
    }

    // Reversing works correctly
    {
        std::array<std::string_view, 3> arr{"123", "456", "789"};

        auto seq = flux::ref(arr).flatten_with("abc"sv).reverse();

        STATIC_CHECK(check_equal(seq, "987cba654cba321"sv));
    }

    // Empty pattern is equivalent to flatten()
    {
        std::array<std::string_view, 3> arr{"111", "222", "333"};
        auto seq = flux::flatten_with(arr, ""sv);

        STATIC_CHECK(check_equal(seq, "111222333"sv));
    }

    // Empty source is handled correctly
    {
        std::array<std::array<int, 3>, 0> arr{};

        auto seq = flux::flatten_with(arr, 0);

        STATIC_CHECK(seq.is_empty());
    }

    // Iterating to the end gives last()
    {
        std::array<std::string_view, 3> arr{"a"sv, "b"sv, "c"sv};

        auto seq = flux::flatten_with(arr, '-');

        auto cur = seq.first();
        while (!seq.is_last(cur)) {
            seq.inc(cur);
        }

        STATIC_CHECK(cur == seq.last());
    }

    return true;
}
static_assert(test_flatten_with_multipass());

}

TEST_CASE("flatten_with")
{
    bool res = test_flatten_with_iterable();
    REQUIRE(res);

    res = test_flatten_with_single_pass();
    REQUIRE(res);

    res = test_flatten_with_multipass();
    REQUIRE(res);
}