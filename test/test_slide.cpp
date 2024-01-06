
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>

#include "test_utils.hpp"

namespace {

constexpr bool test_slide()
{
    // Basic sliding
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).slide(2);

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        STATIC_CHECK(seq.size() == 4);
        STATIC_CHECK(seq.distance(seq.first(), seq.last()) == 4);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(seq[cur], {1, 2}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {2, 3}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {4, 5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(seq.is_last(seq.last()));
    }

    // const iteration works if the underlying is const-iterable
    {
        auto const seq = flux::slide(std::array{1, 2, 3, 4, 5}, 2);

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        STATIC_CHECK(flux::size(seq) == 4);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur), {1, 2}));
        flux::inc(seq, cur);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur), {2, 3}));
        flux::inc(seq, cur);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur), {3, 4}));
        flux::inc(seq, cur);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur), {4, 5}));
        flux::inc(seq, cur);
        STATIC_CHECK(flux::is_last(seq, cur));
    }

    // slide with window size > sequence size is an empty sequence
    {
        auto seq = flux::slide(std::array{1, 2, 3}, 10);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // slide with empty sequence is an empty sequence
    {
        auto seq = flux::slide(flux::empty<int>, 5);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // slide with window size == sequence size has one element
    {
        auto seq = flux::slide(std::array{1, 2, 3, 4, 5}, 5);

        STATIC_CHECK(flux::count(seq) == 1);
        STATIC_CHECK(check_equal(seq.front().value(), {1, 2, 3, 4, 5}));
    }

    // slide(n) + stride(n) is equivalent to chunk(n), if n divides size
    {
        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

        auto slide_n_stride = flux::slide(arr, 3).stride(3);

        auto chunk = flux::chunk(arr, 3);

        // I love that this actually works
        STATIC_CHECK(flux::equal(slide_n_stride, chunk, flux::equal));
    }

    // Reverse iteration works when underlying is bidir + bounded
    {
        auto seq = flux::slide(std::array{1, 2, 3, 4, 5}, 2).reverse();

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(seq[cur], {4, 5}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {2, 3}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {1, 2}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(seq.is_last(seq.last()));
    }

    return true;
}
static_assert(test_slide());

}

TEST_CASE("slide")
{
    bool res = test_slide();
    REQUIRE(res);
}