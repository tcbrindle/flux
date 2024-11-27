
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <coroutine>
#include <memory>
#include <ranges>
#include <utility>

#include "test_utils.hpp"

namespace {

using flux::generator;

auto ints(int from = 0) -> generator<int>
{
    while (true) {
        co_yield from++;
    }
}

auto ints(int from, int const to) -> generator<int>
{
    while (from < to) {
        co_yield from++;
    }
}

auto fib(int a, int b) -> generator<int const&>
{
    while (true) {
        co_yield std::exchange(a, std::exchange(b, a + b));
    }
}

auto pythagorean_triples() -> generator<std::tuple<int, int, int>> {
    for (int z : ints(1)) {
        for (int y : ints(1, z)) {
            for (int x : ints(1, y)) {
                if (x*x + y*y == z*z) {
                    co_yield {x, y, z};
                }
            }
        }
    }
}

auto move_only() ->  generator<std::unique_ptr<int>&&>
{
    for (int i = 0; i < 5; i++) {
        co_yield std::make_unique<int>(i);
    }
}

}

TEST_CASE("generator")
{
    SUBCASE("basic generator tests")
    {
        auto ints = ::ints();

        using I = decltype(ints);

        static_assert(flux::sequence<I>);
        static_assert(not flux::multipass_sequence<I>);
        static_assert(not flux::sized_iterable<I>);
        static_assert(not flux::bounded_sequence<I>);

        static_assert(std::same_as<flux::element_t<I>, int const&>);
        static_assert(std::same_as<flux::value_t<I>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<I>, int const&&>);

        CHECK(check_equal(std::move(ints).take(5), {0, 1, 2, 3, 4}));
    }

    SUBCASE("Generator with move-only element")
    {
        auto gen = ::move_only();

        using G = decltype(gen);

        static_assert(std::same_as<flux::element_t<G>, std::unique_ptr<int>&&>);
        static_assert(std::same_as<flux::value_t<G>, std::unique_ptr<int>>);
        static_assert(std::same_as<flux::rvalue_element_t<G>, std::unique_ptr<int>&&>);

        int i = 0;
        for (auto cur = gen.first(); !gen.is_last(cur); gen.inc(cur)) {
            CHECK(*gen[cur] == i++);
        }
        CHECK(i == 5);
    }

    SUBCASE("ranges integration")
    {
        auto view = ints();

        using V = decltype(view);

        static_assert(std::ranges::input_range<V>);
        static_assert(std::ranges::view<V>);
        static_assert(not std::ranges::forward_range<V>);
        static_assert(std::same_as<std::ranges::range_reference_t<V>, int const&>);
        static_assert(std::same_as<std::ranges::range_value_t<V>, int>);
        static_assert(std::same_as<std::ranges::range_rvalue_reference_t<V>, int const&&>);

        CHECK(std::ranges::equal(std::views::take(std::move(view), 5),
                                 std::views::iota(0, 5)));
    }

    SUBCASE("fibonacci sequence")
    {
        auto seq = fib(0, 1).take(10);

        CHECK(check_equal(seq, {0, 1, 1, 2, 3, 5, 8, 13, 21, 34}));
    }

    SUBCASE("Pythagorean triples")
    {
        auto triples = pythagorean_triples().take(5);

        auto cur = triples.first();

        CHECK((triples[cur] == std::tuple{3, 4, 5}));
        CHECK((triples[triples.inc(cur)] == std::tuple{6, 8, 10}));
        CHECK((triples[triples.inc(cur)] == std::tuple{5, 12, 13}));
        CHECK((triples[triples.inc(cur)] == std::tuple{9, 12, 15}));
        CHECK((triples[triples.inc(cur)] == std::tuple{8, 15, 17}));
    }
}
