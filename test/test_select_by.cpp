
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include "test_utils.hpp"

#include <flux.hpp>

#include <array>
#include <iostream>

namespace {

constexpr bool test_select_by()
{
    // Basic select_by
    {
        std::array values{1, 2, 3, 4, 5};
        std::array selectors{true, false, true, false, true};

        auto selected = flux::select_by(values, selectors);

        using S = decltype(selected);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(selected, {1, 3, 5}));
        STATIC_CHECK(check_equal(flux::reverse(selected), {5, 3, 1}));
    }

    // select_by is const-iterable when both sequences are
    {
        std::array values{1, 2, 3, 4, 5};
        std::array selectors{true, false, true, false, true};

        auto const selected = flux::select_by(values, selectors);

        using S = decltype(selected);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(selected, {1, 3, 5}));
        STATIC_CHECK(check_equal(flux::reverse(selected), {5, 3, 1}));
    }

    // select_by with single-pass base sequence is single-pass
    {
        auto values = flux::scan(std::array{1, 2, 3, 4, 5}, std::plus{});
        auto selectors = std::array{0, 1, 0, 1, 0};

        auto selected = flux::select_by(std::move(values), selectors);

        using S = decltype(selected);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(selected, {3, 10}));
    }

    // select_by with single-pass selectors sequence is single-pass
    {
        auto values = std::array{1, 2, 3, 4, 5};
        auto selectors = single_pass_only(std::array{false, false, false, true, false});

        auto selected = flux::select_by(std::move(values), std::move(selectors));

        using S = decltype(selected);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        STATIC_CHECK(check_equal(selected, {4}));
    }

    // select_by with shorter base sequence
    {
        std::array values{1, 2, 3, 4, 5};
        auto selectors = flux::cycle(std::array{true, false});

        auto selected = flux::from(values).select_by(selectors);

        using S = decltype(selected);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);

        STATIC_CHECK(check_equal(selected, {1, 3, 5}));
    }

    // select_by with shorter selectors sequence
    {
        auto selected = flux::ints().select_by(std::array{true, false, true});

        using S = decltype(selected);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);

        STATIC_CHECK(check_equal(selected, {0, 2}));
    }

    // select_by with two infinite sequences is infinite
    {
        auto selected = flux::ints().select_by(flux::cycle(std::array{0, 1}));

        using S = decltype(selected);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(flux::infinite_sequence<S>);

        flux::cursor auto cur = flux::first(selected);
        STATIC_CHECK(selected[cur] == 1);
        selected.inc(cur);
        STATIC_CHECK(selected[cur] == 3);
        selected.dec(cur);
        STATIC_CHECK(selected[cur] == 1);
    }

    // select_by with empty selectors sequence is empty
    {
        auto selected = flux::ints().select_by(flux::empty<bool>);

        STATIC_CHECK(selected.is_empty());
    }

    // select_by with empty values sequence is empty
    {
        auto selected = flux::select_by(flux::empty<double>, flux::repeat(true));

        STATIC_CHECK(selected.is_empty());
    }

    // select_by with all selectors true is the same as the original
    {
        std::array values{1, 2, 3, 4, 5};

        auto selected = flux::ref(values).select_by(flux::repeat(true));

        STATIC_CHECK(check_equal(values, selected));
    }

    // select_by with all selectors false is empty
    {
        std::array values{1, 2, 3, 4, 5};

        auto selected = flux::ref(values).select_by(flux::repeat(false));

        STATIC_CHECK(selected.is_empty());
    }

    // select_by can be used to implement filter()
    {
        std::array values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto selected = flux::select_by(flux::ref(values),
                                        flux::ref(values).map(flux::pred::even));

        STATIC_CHECK(check_equal(selected, {2, 4, 6, 8, 10}));
    }

    return true;
}
static_assert(test_select_by());

}

TEST_CASE("select_by")
{
    auto res = test_select_by();
    REQUIRE(res);
}