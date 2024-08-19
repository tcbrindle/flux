
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <vector>

#include "test_utils.hpp"

TEST_CASE("C array bounds checking")
{
    {
        int arr[] = {0, 1, 2, 3, 4};

        auto seq = flux::ref(arr);

        SUBCASE("Can read from in-bounds indices")
        {
            auto cur = seq.first();
            REQUIRE(seq[cur] == 0);
            REQUIRE(seq.move_at(cur) == 0);
        }

        SUBCASE("Can advance within bounds")
        {
            auto cur = seq.first();
            for (; !seq.is_last(cur); seq.inc(cur)) {}
            REQUIRE(seq.is_last(cur));
        }

        SUBCASE("Reading past the end is an error")
        {
            auto cur = seq.last();
            REQUIRE_THROWS_AS(seq[cur], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(cur), flux::unrecoverable_error);
        }

        SUBCASE("Reading before the start is an error")
        {
            auto cur = seq.first() - 1;
            REQUIRE_THROWS_AS(seq[cur], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(cur), flux::unrecoverable_error);
        }

        SUBCASE("Can decrement within bounds")
        {
            auto cur = seq.last();
            while(seq.dec(cur) != seq.first()) {}
            REQUIRE(cur == seq.first());
        }

        SUBCASE("Random reads are an error")
        {
            REQUIRE_THROWS_AS(seq[100], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(100), flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq[-100], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(-100), flux::unrecoverable_error);
        }

        SUBCASE("Views are bounds checked as well")
        {
            auto first = seq.begin();
            auto last = seq.end();

            REQUIRE_THROWS_AS(*last, flux::unrecoverable_error);
            REQUIRE_THROWS_AS(first[10], flux::unrecoverable_error);
        }
    }
}

TEST_CASE("vector bounds checking")
{
    std::vector<int> vec{0, 1, 2, 3, 4};

    auto seq = flux::ref(vec);

    SUBCASE("Can read from in-bounds indices")
    {
        auto cur = seq.first();
        REQUIRE(seq[cur] == 0);
        REQUIRE(seq.move_at(cur) == 0);
    }

    SUBCASE("Can advance within bounds")
    {
        auto cur = seq.first();
        for (; !seq.is_last(cur); seq.inc(cur)) {}
        REQUIRE(seq.is_last(cur));
    }

    SUBCASE("Reading past the end is an error")
    {
        auto cur = seq.last();
        REQUIRE_THROWS_AS(seq[cur], flux::unrecoverable_error);
        REQUIRE_THROWS_AS(seq.move_at(cur), flux::unrecoverable_error);
    }

    SUBCASE("Reading before the start is an error")
    {
        auto cur = seq.first() - 1;
        REQUIRE_THROWS_AS(seq[cur], flux::unrecoverable_error);
        REQUIRE_THROWS_AS(seq.move_at(cur), flux::unrecoverable_error);
    }

    SUBCASE("Can decrement within bounds")
    {
        auto cur = seq.last();
        while(seq.dec(cur) != seq.first()) {}
        REQUIRE(cur == seq.first());
    }

    SUBCASE("Random reads are an error")
    {
        REQUIRE_THROWS_AS(seq[100], flux::unrecoverable_error);
        REQUIRE_THROWS_AS(seq.move_at(100), flux::unrecoverable_error);
        REQUIRE_THROWS_AS(seq[-100], flux::unrecoverable_error);
        REQUIRE_THROWS_AS(seq.move_at(-100), flux::unrecoverable_error);
    }

    SUBCASE("Storage invalidation is okay")
    {
        auto vec2 = vec;
        auto cur = flux::next(vec2, flux::first(vec2), 2);
        REQUIRE(flux::read_at(vec2, cur) == 2);

        vec2.resize(vec2.capacity() + 1); // force a realloc
        REQUIRE(flux::read_at(vec2, cur) == 2);

        vec2.clear();
        vec2.shrink_to_fit(); // for good measure
        REQUIRE_THROWS_AS(flux::read_at(vec2, cur), flux::unrecoverable_error);
    }

    SUBCASE("Range interface is bounds checked as well")
    {
        auto first = seq.begin();
        auto last = seq.end();

        REQUIRE_THROWS_AS(*last, flux::unrecoverable_error);
        REQUIRE_THROWS_AS(first[10], flux::unrecoverable_error);
    }
}