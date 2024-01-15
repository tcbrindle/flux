
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <iostream>
#include <string>
#include <string_view>

#include "test_utils.hpp"

namespace {

constexpr auto  to_string_view = []<typename Seq>(Seq&& seq) // danger Will Robinson
{
    return std::basic_string_view<flux::value_t<Seq>>(flux::data(seq), flux::usize(seq));
};

constexpr bool test_split_with_delim()
{
    using namespace std::string_view_literals;

    {
        auto sv = "the quick brown fox"sv;

        auto split = flux::split(flux::ref(sv), ' ');

        using S = decltype(split);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::contiguous_sequence<flux::element_t<S>>);

        static_assert(flux::multipass_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(flux::contiguous_sequence<flux::element_t<S const>>);

        STATIC_CHECK(check_equal(std::move(split).map(to_string_view),
                std::array{"the"sv, "quick"sv, "brown"sv, "fox"sv}));
    }

    // Leading and trailing delimiters are handled correctly
    {
        auto split = flux::split(" trailing space "sv, ' ').map(to_string_view);

        STATIC_CHECK(check_equal(split, std::array{""sv, "trailing"sv, "space"sv, ""sv}));

        auto cur = split.first();
        split.inc(cur);
        split.inc(cur);
        split.inc(cur);
        STATIC_CHECK(cur.trailing_empty == true);
        STATIC_CHECK(cur != split.last());
        split.inc(cur);
        STATIC_CHECK(cur.trailing_empty == false);
        STATIC_CHECK(cur == split.last());
    }

    // Non-bounded sequences can be split correctly
    {
        auto split = flux::take_while("a b"sv, [](auto) { return true; }).split(' ');

        using S = decltype(split);

        static_assert(flux::multipass_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(flux::random_access_sequence<flux::element_t<S>>);

        STATIC_CHECK(flux::equal(split, std::array{"a"sv, "b"sv}));
    }

    return true;
}

constexpr bool test_split_with_pattern()
{
    using namespace std::string_view_literals;

    // Split with pattern
    {
        int nums[] = {0, 1, 2, 3, 99};

        auto split = flux::split(flux::ref(nums), std::array{1, 2, 3});

        static_assert(
            flux::contiguous_sequence<flux::element_t<decltype(split)>>);

        auto cur = split.first();
        STATIC_CHECK(check_equal(split[cur], flux::single(0)));
        split.inc(cur);
        STATIC_CHECK(check_equal(split[cur], flux::single(99)));
    }

    // String splitting
    {
        auto sv = u"something\r\nsomething\r\nsomething\r\ndark\r\nside"sv;

        auto split = flux::from(sv).split_string(u"\r\n");

        STATIC_CHECK(check_equal(split, std::array{
           u"something"sv, u"something"sv, u"something"sv, u"dark"sv, u"side"sv}));
    }

    return true;
}

constexpr bool test_split_with_predicate()
{
    using namespace std::string_view_literals;

    {
        std::array arr{1, 2, 0, 3, 4, 0, 5};

        auto split = flux::ref(arr).split(flux::pred::eq(0));

        using S = decltype(split);

        static_assert(flux::sequence<S>);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

        static_assert(flux::sequence<S const>);
        static_assert(flux::multipass_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(not flux::bidirectional_sequence<S const>);
        static_assert(not flux::sized_sequence<S const>);

        using E = flux::element_t<S>;
        static_assert(flux::contiguous_sequence<E>);
        static_assert(flux::sized_sequence<E>);

        using EC = flux::element_t<S const>;
        static_assert(flux::contiguous_sequence<EC>);
        static_assert(flux::sized_sequence<EC>);

        auto cur = split.first();
        STATIC_CHECK(check_equal(split[cur], {1, 2}));
        split.inc(cur);
        STATIC_CHECK(check_equal(split[cur], {3, 4}));
        split.inc(cur);
        STATIC_CHECK(cur != split.last());
        STATIC_CHECK(check_equal(split[cur], {5}));
        split.inc(cur);
        STATIC_CHECK(split.is_last(cur));

        STATIC_CHECK(cur == split.last());
    }

    {
        auto const seq = flux::split("two spaces ->  <-"sv, flux::pred::eq(' '))
                            .map(to_string_view);

        STATIC_CHECK(check_equal(seq,
                                 std::array{"two"sv, "spaces"sv, "->"sv, ""sv, "<-"sv}));
    }

    return true;
}

static_assert(test_split_with_delim());
static_assert(test_split_with_pattern());
static_assert(test_split_with_predicate());

}

TEST_CASE("split with delimiter")
{
    bool result = test_split_with_delim();
    REQUIRE(result);
}

TEST_CASE("split with pattern")
{
    bool result = test_split_with_pattern();
    REQUIRE(result);
}

TEST_CASE("split with predicate")
{
    bool result = test_split_with_predicate();
    REQUIRE(result);
}
