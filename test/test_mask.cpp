
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include "test_utils.hpp"

#include <flux.hpp>

#include <array>
#include <iostream>

namespace {

constexpr bool test_mask()
{
    // Basic mask
    {
        std::array values{1, 2, 3, 4, 5};
        std::array mask{true, false, true, false, true};

        auto masked = flux::mask(values, mask);

        using S = decltype(masked);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(masked, {1, 3, 5}));
        STATIC_CHECK(check_equal(flux::reverse(masked), {5, 3, 1}));
    }

    // mask is const-iterable when both sequences are
    {
        std::array values{1, 2, 3, 4, 5};
        std::array mask{true, false, true, false, true};

        auto const masked = flux::mask(values, mask);

        using S = decltype(masked);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(masked, {1, 3, 5}));
        STATIC_CHECK(check_equal(flux::reverse(masked), {5, 3, 1}));
    }

    // mask with single-pass base sequence is single-pass
    {
        auto values = flux::scan(std::array{1, 2, 3, 4, 5}, std::plus{});
        auto mask = std::array{0, 1, 0, 1, 0};

        auto masked = flux::mask(std::move(values), mask);

        using S = decltype(masked);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(masked, {3, 10}));
    }

    // mask with single-pass selectors sequence is single-pass
    {
        auto values = std::array{1, 2, 3, 4, 5};
        auto mask = single_pass_only(std::array{false, false, false, true, false});

        auto masked = flux::mask(std::move(values), std::move(mask));

        using S = decltype(masked);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(masked, {4}));
    }

    // mask with shorter base sequence
    {
        std::array values{1, 2, 3, 4, 5};
        auto mask = flux::cycle(std::array{true, false});

        auto masked = flux::from(values).mask(mask);

        using S = decltype(masked);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);

        STATIC_CHECK(check_equal(masked, {1, 3, 5}));
    }

    // mask with shorter selectors sequence
    {
        auto masked = flux::ints().mask(std::array{true, false, true});

        using S = decltype(masked);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);

        STATIC_CHECK(check_equal(masked, {0, 2}));
    }

    // mask with two infinite sequences is infinite
    {
        auto masked = flux::ints().mask(flux::cycle(std::array{0, 1}));

        using S = decltype(masked);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(flux::infinite_sequence<S>);

        flux::cursor auto cur = flux::first(masked);
        STATIC_CHECK(masked[cur] == 1);
        masked.inc(cur);
        STATIC_CHECK(masked[cur] == 3);
        masked.dec(cur);
        STATIC_CHECK(masked[cur] == 1);
    }

    // mask with empty selectors sequence is empty
    {
        auto masked = flux::ints().mask(flux::empty<bool>);

        STATIC_CHECK(masked.is_empty());
    }

    // mask with empty values sequence is empty
    {
        auto masked = flux::mask(flux::empty<double>, flux::repeat(true));

        STATIC_CHECK(masked.is_empty());
    }

    // masked with all selectors true is the same as the original
    {
        std::array values{1, 2, 3, 4, 5};

        auto masked = flux::ref(values).mask(flux::repeat(true));

        STATIC_CHECK(check_equal(values, masked));
    }

    // mask with all selectors false is empty
    {
        std::array values{1, 2, 3, 4, 5};

        auto masked = flux::ref(values).mask(flux::repeat(false));

        STATIC_CHECK(masked.is_empty());
    }

    // mask can be used to implement filter()
    {
        std::array values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto masked = flux::mask(flux::ref(values),
                                 flux::ref(values).map(flux::pred::even));

        STATIC_CHECK(check_equal(masked, {2, 4, 6, 8, 10}));
    }

    return true;
}
static_assert(test_mask());

}

TEST_CASE("mask")
{
    auto res = test_mask();
    REQUIRE(res);
}