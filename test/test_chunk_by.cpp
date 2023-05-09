
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Adapted from Range-v3's views::chunk_by tests
// https://github.com/ericniebler/range-v3/blob/4624c63972c6f2c2871c7b87813c42048ddb80ad/test/view/chunk_by.cpp

//  Copyright Hui Xie 2021
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)


#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <iostream>

namespace {

template <bool = true>
constexpr bool test_chunk_by() {

    using P = std::pair<int, int>;

    std::array<P, 12> const arr = {{
        {1, 1},
        {1, 1},
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {2, 2},
        {2, 2},
        {2, 3},
        {2, 3},
        {2, 3},
        {2, 3},
    }};

    {
        auto seq = flux::chunk_by(
            std::cref(arr), [](P p0, P p1) { return p0.second == p1.second; });

        using S = decltype(seq);

        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::random_access_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

        STATIC_CHECK(seq.count() == 3);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {P{1, 1}, P{1, 1}}));

        seq.inc(cur);
        STATIC_CHECK(check_equal(seq[cur], {P{1, 2}, P{1, 2}, P{1, 2}, P{1, 2}, P{2, 2}, P{2, 2}}));

        seq.inc(cur);
        STATIC_CHECK(check_equal(seq[cur], {P{2, 3}, P{2, 3}, P{2, 3}, P{2, 3}}));

        cur = seq.last();
        STATIC_CHECK(flux::is_last(seq, cur));
    }

    // chunk_by is const-iterable when the underlying sequence is
    {
        auto const seq = flux::ref(arr).chunk_by([](P p0, P p1) { return p0.first == p1.first; });

        STATIC_CHECK(flux::count(seq) == 2);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur),
                                 {P{1, 1}, P{1, 1}, P{1, 2}, P{1, 2}, P{1, 2}, P{1, 2}}));

        STATIC_CHECK(check_equal(flux::read_at(seq, flux::next(seq, cur)),
                                 {P{2, 2}, P{2, 2}, P{2, 3}, P{2, 3}, P{2, 3}, P{2, 3}}));
    }

    // chunk_by is reversible when the underlying sequence is
    {
        auto seq = flux::ref(arr)
                                        .chunk_by([](P p0, P p1) { return p0.first == p1.first; })
                                        .reverse();

        STATIC_CHECK(flux::count(seq) == 2);

        // Note that chunk_by().reverse() delivers the chunks in reverse order,
        // but within each chunk the elements are still in forward order
        // Compare with reverse().chunk_by(), which reverses the elements and
        // *then* splits them into chunks

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur),
                                 {P{2, 2}, P{2, 2}, P{2, 3}, P{2, 3}, P{2, 3}, P{2, 3}}));

        STATIC_CHECK(check_equal(flux::read_at(seq, flux::next(seq, cur)),
                                 {P{1, 1}, P{1, 1}, P{1, 2}, P{1, 2}, P{1, 2}, P{1, 2}}));

    }

    // chunk_by works on empty sequences
    {
        auto seq = flux::chunk_by(flux::empty<int>, std::equal_to<>{});

        STATIC_CHECK(seq.is_empty());
    }

    // chunk_by on a sequence of size 1 doesn't perform a comparison
    {
        auto seq = flux::chunk_by(flux::single(2), [](int, int) -> bool { throw 0; });

        STATIC_CHECK(seq.count() == 1);
        STATIC_CHECK(check_equal(*seq.front(), {2}));
    }


    {
        int a[] = {0, 1, 2, 3, 4, 5};
        auto seq = flux::ref(a).filter(flux::pred::odd).chunk_by(flux::pred::true_);
        STATIC_CHECK(check_equal(seq.front().value(), {1, 3, 5}));
        STATIC_CHECK(seq.count() == 1);
    }

    {
        std::array const arr2{0, 1, 2, 6, 8, 10, 15, 17, 18, 29};
        auto seq = flux::chunk_by(arr2, [](int i, int j) { return j - i < 3; });

        STATIC_CHECK(seq.count() == 4);
        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {0, 1, 2}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {6, 8, 10}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {15, 17, 18}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {29}));
    }

    {
        std::array arr3 = {2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 0};
        auto seq = flux::chunk_by(arr3, std::less<>{});
        STATIC_CHECK(seq.count() == 4);
        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {2, 3, 4, 5}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {0, 1, 2, 3, 4, 5, 6}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {0, 1, 2, 3}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {0}));
    }

    // calling inc() on the last cursor doesn't get us in trouble
    {
        std::array arr4{1, 1, 1, 2, 2};

        auto seq = flux::chunk_by(arr4, std::equal_to<>{});

        auto cur = seq.last();
        seq.inc(cur);

        STATIC_CHECK(seq.is_last(cur));
    }

    // Calling dec() on the first cursor doesn't get us in trouble
    {
        std::array arr4{1, 1, 1, 2, 2};

        auto seq = flux::chunk_by(arr4, std::equal_to<>{});

        auto cur = seq.first();
        seq.dec(cur);

        STATIC_CHECK(cur == seq.first());
    }

    return true;
}
static_assert(test_chunk_by());

}

TEST_CASE("chunk_by")
{
    bool res = test_chunk_by<false>();
    REQUIRE(res);
}
