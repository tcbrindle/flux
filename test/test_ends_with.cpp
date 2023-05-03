
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include <array>
#include <iostream>

#include "test_utils.hpp"

namespace {

struct S {
    int i;

    constexpr int get() const { return i; }
};

constexpr bool test_ends_with()
{
    // Basic ends_with for two reversible sequences
    {
        std::array const arr1{1, 2, 3, 4, 5};
        int arr2[] = {3, 4, 5};

        STATIC_CHECK(flux::ends_with(arr1, arr2));

        STATIC_CHECK(flux::ends_with(arr1, arr1));

        STATIC_CHECK(not flux::ends_with(arr2, arr1));

        STATIC_CHECK(flux::ends_with(arr2, arr2));
    }

    // Member ends_with for two reversible sequences
    {
        std::array const arr1{1, 2, 3, 4, 5};
        int arr2[] = {3, 4, 5};

        STATIC_CHECK(flux::ref(arr1).ends_with(arr2));

        STATIC_CHECK(flux::ref(arr1).ends_with(arr1));

        STATIC_CHECK(not flux::ref(arr2).ends_with(arr1));

        STATIC_CHECK(flux::ref(arr2).ends_with(arr2));
    }

    // ends_with, non-reversible non-sized sequences
    {
        auto seq1 = flux::ints(1).take_while(flux::pred::leq(10));
        double seq2[] = {8.0, 9.0, 10.0};

        STATIC_CHECK(flux::ends_with(seq1, seq2));

        STATIC_CHECK(not flux::ends_with(seq2, seq1));

        STATIC_CHECK(flux::ends_with(seq1, seq1));

        STATIC_CHECK(flux::ends_with(seq2, seq2));
    }

    // Same size but different elements
    {
        std::array const arr1{1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 3, 4, 6};

        STATIC_CHECK(not flux::ends_with(arr1, arr2));
        STATIC_CHECK(not flux::ends_with(arr2, arr1));

        // and again with non-bidir, non-sized seq
        auto non_bidir = flux::take_while(arr1, flux::pred::true_);

        STATIC_CHECK(not flux::ends_with(non_bidir, arr2));
        STATIC_CHECK(not flux::ends_with(arr2, non_bidir));
        STATIC_CHECK(flux::ends_with(non_bidir, arr1));
        STATIC_CHECK(flux::ends_with(arr1, non_bidir));
    }

    // Test with custom comparator
    {
        S arr1[] = {{1}, {2}, {3}, {4}, {5}};
        std::array arr2 = {4, 5};

        auto cmp = [](S s, int i) { return s.i == i; };

        STATIC_CHECK(flux::ends_with(arr1, arr2, cmp));

        STATIC_CHECK(flux::ref(arr1).ends_with(arr2, cmp));

        auto seq = flux::ref(arr1).take_while(flux::pred::true_);

        STATIC_CHECK(flux::ends_with(seq, arr2, cmp));
        STATIC_CHECK(seq.ends_with(arr2, cmp));
    }

    // Test with projections
    {
        S arr1[] = {{1}, {2}, {3}, {4}, {5}};
        std::array arr2 = {4, 5};

        STATIC_CHECK(flux::ends_with(arr1, arr2, flux::proj2(std::equal_to{}, &S::i)));

        STATIC_CHECK(flux::ref(arr1).ends_with(arr2, flux::proj2(std::equal_to{}, &S::get)));

        auto seq = flux::ref(arr1).take_while(flux::pred::true_);

        STATIC_CHECK(flux::ends_with(seq, arr2, flux::proj2(std::equal_to{}, &S::i)));
        STATIC_CHECK(seq.ends_with(arr2, flux::proj2(std::equal_to{}, &S::get)));

        STATIC_CHECK(flux::ends_with(seq, arr1, flux::proj(std::equal_to{}, &S::i)));
    }

    return true;
}
static_assert(test_ends_with());

}

TEST_CASE("ends_with")
{
    bool result = test_ends_with();
    REQUIRE(result);
}