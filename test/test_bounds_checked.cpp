
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/op/bounds_checked.hpp>
#include <flux/op/map.hpp>
#include <flux/ranges/view.hpp>
#include <flux/source/istream.hpp>

#include <sstream>

#include "test_utils.hpp"

TEST_CASE("C array bounds checking")
{
    {
        int arr[] = {0, 1, 2, 3, 4};

        auto seq = flux::ref(arr);

        SECTION("Can read from in-bounds indices")
        {
            auto cur = seq.first();
            REQUIRE(seq[cur] == 0);
            REQUIRE(seq.move_at(cur) == 0);
        }

        SECTION("Can advance within bounds")
        {
            auto cur = seq.first();
            for (; !seq.is_last(cur); seq.inc(cur)) {}
            REQUIRE(seq.is_last(cur));
        }

        SECTION("Reading past the end is an error")
        {
            auto cur = seq.last();
            REQUIRE_THROWS_AS(seq[cur], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(cur), flux::unrecoverable_error);
        }

        SECTION("Reading before the start is an error")
        {
            auto cur = seq.prev(seq.first());
            REQUIRE_THROWS_AS(seq[cur], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(cur), flux::unrecoverable_error);
        }

        SECTION("Can decrement within bounds")
        {
            auto cur = seq.last();
            while(seq.dec(cur) != seq.first()) {}
            REQUIRE(cur == seq.first());
        }

        SECTION("Random reads are an error")
        {
            REQUIRE_THROWS_AS(seq[100], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(100), flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq[-100], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(-100), flux::unrecoverable_error);
        }

        SECTION("Views are bounds checked as well")
        {
            auto view = seq.view();

            auto first = view.begin();
            auto last = view.end();

            (void) *first;

            REQUIRE_THROWS_AS(*last, flux::unrecoverable_error);
            REQUIRE_THROWS_AS(first[10], flux::unrecoverable_error);
        }
    }
}