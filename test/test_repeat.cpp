
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <limits>
#include <string_view>

#include "test_utils.hpp"

namespace {

struct S {
    constexpr S(int i) : i(i) {}
    S(S&&) = default;
    S& operator=(S&&) = default;
    bool operator==(S const&) const = default;

    int i;
};

constexpr bool test_repeat()
{
    // Basic repeat
    {
        auto seq = flux::repeat(3);

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::value_t<S>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        static_assert(std::is_trivially_copyable_v<S>);

        // Check a few elements just to make sure
        auto cur = flux::first(seq);
        for (int i = 0; i < 100; i++) {
            STATIC_CHECK(seq[cur] == 3);
            seq.inc(cur);
        }

        // Check that internal iteration works as expected
        {
            auto counter = 0;
            auto inner_cur =
                flux::for_each_while(seq, [&](int) { return counter++ < 5; });
            STATIC_CHECK(inner_cur == 5);
        }
    }

    // repeat is const-iterable
    {
        auto const seq = flux::repeat(3);

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::infinite_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::value_t<S>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        static_assert(std::is_trivially_copyable_v<S>);

        // Check a few elements just to make sure
        auto cur = flux::first(seq);
        for (int i = 0; i < 100; i++) {
            STATIC_CHECK(flux::read_at(seq, cur) == 3);
            flux::inc(seq, cur);
        }
    }

    // repeat -> take works as expected
    {
        auto seq = flux::repeat(3).take(5);

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, {3, 3, 3, 3, 3}));
    }

    // repeat can wrap around safely
    {
        auto seq = flux::repeat(std::string_view("test"));

        auto cur = std::numeric_limits<std::size_t>::max();

        STATIC_CHECK(seq[cur] == "test");

        seq.inc(cur);
        STATIC_CHECK(cur == std::numeric_limits<std::size_t>::lowest());
        STATIC_CHECK(seq[cur] == "test");

        seq.dec(cur);
        STATIC_CHECK(cur == std::numeric_limits<std::size_t>::max());
        STATIC_CHECK(seq[cur] == "test");
    }

    // random-access increment works
    {
        auto seq = flux::repeat(1.0);

        constexpr auto max_idx = std::numeric_limits<flux::int_t>::max();
        constexpr auto min_idx = std::numeric_limits<flux::int_t>::lowest();

        auto cur = flux::next(seq, seq.first(), max_idx);

        STATIC_CHECK(seq[cur] == 1.0);
        STATIC_CHECK(seq.distance(cur, seq.first()) == -max_idx);

        cur = flux::next(seq, seq.first(), min_idx);

        STATIC_CHECK(seq[cur] == 1.0);
    }

    // repeat works with move-only types
    {
        auto seq = flux::repeat(S(3)).take(5);

        STATIC_CHECK(check_equal(seq, {S(3), S(3), S(3), S(3), S(3)}));
    }

    return true;
}
static_assert(test_repeat());

constexpr bool test_repeat_bounded()
{
    // Basic bounded repeat
    {
        auto seq = flux::repeat(3, 5);

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const &>);
        static_assert(std::same_as<flux::value_t<S>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const &&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const &>);

        static_assert(std::is_trivially_copyable_v<S>);

        STATIC_CHECK(seq.size() == 5);
        STATIC_CHECK(check_equal(seq, {3, 3, 3, 3, 3}));
        STATIC_CHECK(seq.is_last(seq.last()));
        STATIC_CHECK(seq.last() == 5);

        // Check that internal iteration works as expected
        {
            auto cur = flux::for_each_while(seq, flux::pred::true_);
            STATIC_CHECK(cur == seq.last());
        }

        // ...and again with early termination
        {
            auto counter = 0;
            auto cur =
                flux::for_each_while(seq, [&](int) { return counter++ < 3; });
            STATIC_CHECK(cur == 3);
        }
    }

    // bounded repeat can be const-iterated
    {
        auto const seq = flux::repeat(3, 5);

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::value_t<S>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::const_element_t<S>, int const&>);

        static_assert(std::is_trivially_copyable_v<S>);

        STATIC_CHECK(seq.size() == 5);
        STATIC_CHECK(check_equal(seq, {3, 3, 3, 3, 3}));
        STATIC_CHECK(flux::is_last(seq, flux::last(seq)));
        STATIC_CHECK(flux::last(seq) == 5);
    }

    // bounded repeat() can be (uselessly) reversed
    {
        auto seq = flux::repeat(std::string_view("test"), 3).reverse();

        STATIC_CHECK(check_equal(seq, {"test", "test", "test"}));
    }

    // bounded repeat() random-access works as expected
    {
        auto seq = flux::repeat(3, 10);

        auto cur = seq.first();
        seq.inc(cur, 100);
        STATIC_CHECK(seq.is_last(cur));
        STATIC_CHECK(seq.distance(seq.first(), cur) == 100);
        STATIC_CHECK(seq.distance(cur, seq.first()) == -100);

        seq.inc(cur, -100);
        STATIC_CHECK(cur == seq.first());
    }

    // bounded repeat() works with move-only types
    {
        auto seq = flux::repeat(S(3), 5);

        STATIC_CHECK(check_equal(seq, {S(3), S(3), S(3), S(3), S(3)}));
    }

    // repeat(obj, 0) is an empty sequence
    {
        auto seq = flux::repeat(std::string_view("test"), 0);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.size() == 0);
        STATIC_CHECK(seq.is_last(seq.first()));

        bool called = false;
        seq.for_each([&called] (auto) { called = true; });
        STATIC_CHECK(!called);
    }

    return true;
}
static_assert(test_repeat_bounded());

}

TEST_CASE("repeat")
{
    bool res = test_repeat();
    REQUIRE(res);

    res = test_repeat_bounded();
    REQUIRE(res);

    SUBCASE("negative argument to bounded repeat() is caught")
    {
        REQUIRE_THROWS_AS(flux::repeat(3, -100),
                          flux::unrecoverable_error);
    }

    SUBCASE("Unrepresentable distance is caught debug mode")
    {
        if constexpr (flux::config::enable_debug_asserts) {
            auto seq = flux::repeat(3);

            auto cur = std::numeric_limits<std::size_t>::max();

            REQUIRE_THROWS_AS(flux::distance(seq, 0, cur),
                              flux::unrecoverable_error);
        }
    }
}