
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>
#include <flux/op/write_to.hpp>

#include "test_utils.hpp"

#include <array>
#include <iostream>
#include <string>
#include <string_view>

namespace {

constexpr auto  to_string_view = []<typename Seq>(Seq&& seq) // danger Will Robinson
{
    return std::basic_string_view<flux::value_t<Seq>>(flux::data(seq), flux::size(seq));
};

constexpr bool test_split()
{
    using namespace std::string_view_literals;

    {
        auto sv = "the quick brown fox"sv;

        auto split = flux::split(flux::ref(sv), ' ');

        using S = decltype(split);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::contiguous_sequence<flux::element_t<S>>);

        static_assert(flux::multipass_sequence<S const>);
        static_assert(flux::contiguous_sequence<flux::element_t<S const>>);

        STATIC_CHECK(check_equal(std::move(split).map(to_string_view),
                std::array{"the"sv, "quick"sv, "brown"sv, "fox"sv}));
    }

    // Leading and trailing delimiters are handled correctly
    {
        auto split = flux::split(" trailing space "sv, ' ').map(to_string_view);

        STATIC_CHECK(check_equal(split, std::array{""sv, "trailing"sv, "space"sv, ""sv}));
    }

    // Non-bounded sequences can be split correctly
    {
        auto split = flux::take_while("a b"sv, [](auto) { return true; }).split(' ');

        using S = decltype(split);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::contiguous_sequence<flux::element_t<S>>);

        STATIC_CHECK(check_equal(std::move(split).map(to_string_view),
                                 std::array{"a"sv, "b"sv}));
    }

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
static_assert(test_split());

}

TEST_CASE("split")
{
    bool result = test_split();
    REQUIRE(result);
}
