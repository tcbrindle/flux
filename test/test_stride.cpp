
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <limits>
#include <list>

namespace {

template <flux::sequence Base>
struct NotBidir : flux::inline_sequence_base<NotBidir<Base>> {
    Base base_;

    constexpr explicit NotBidir(Base base) : base_(std::move(base)) {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }

    struct flux_sequence_traits : flux::detail::passthrough_traits_base<Base> {
        static void dec(...) = delete;
    };
};

constexpr bool test_stride_non_bidir()
{
    // Basic stride, n divides size
    {
        std::array arr{0, 1, 2, 3, 4, 5};

        auto seq = flux::stride(NotBidir(arr), 2);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 2, 4}));
        STATIC_CHECK(seq.last() == flux::last(arr));
        STATIC_CHECK(seq.size() == 3);
    }

    // Basic stride, n does not divide size
    {
        std::array arr{0, 1, 2, 3, 4, 5, 6, 7};

        auto const seq = NotBidir(arr).stride(3);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 3, 6}));
        STATIC_CHECK(flux::last(seq) == flux::last(arr));
        STATIC_CHECK(flux::size(seq) == 3);
    }

    // Stride of 1 returns the original sequence
    {
        std::array arr{0, 1, 2, 3, 4, 5, 6};

        auto const seq = flux::stride(NotBidir(arr), 1);

        STATIC_CHECK(check_equal(seq, arr));
    }

    // Stride >= than sequence size returns sequence of one element
    {
        {
            std::array arr{0, 1, 2, 3, 4, 5};
            auto seq = NotBidir(arr).stride(arr.size());

            auto cur = seq.first();
            STATIC_CHECK(!seq.is_last(cur));
            STATIC_CHECK(seq[cur] == 0);
            cur = seq.next(cur);
            STATIC_CHECK(seq.is_last(cur));
        }

        {
            std::array arr{0, 1, 2, 3, 4, 5};
            auto seq = NotBidir(arr).stride(99999);

            auto cur = seq.first();
            STATIC_CHECK(!seq.is_last(cur));
            STATIC_CHECK(seq[cur] == 0);
            cur = seq.next(cur);
            STATIC_CHECK(seq.is_last(cur));
        }
    }

    // Stride of distance_t::max doesn't break stuff
    {
        std::array arr{0, 1, 2, 3, 4, 5};
        auto seq = NotBidir(arr).stride(std::numeric_limits<flux::distance_t>::max());

        auto cur = seq.first();
        STATIC_CHECK(!seq.is_last(cur));
        STATIC_CHECK(seq[cur] == 0);
        cur = seq.next(cur);
        STATIC_CHECK(seq.is_last(cur));
    }

    // Internal iteration works as expected
    {
        std::array arr{0, 1, 2, 3, 4, 5};

        auto seq = NotBidir(std::ref(arr)).stride(2);

        STATIC_CHECK(seq.sum() == 0 + 2 + 4);

        auto cur = seq.find(4);

        STATIC_CHECK(&seq[cur] == arr.data() + 4);
    }

    return true;
}
static_assert(test_stride_non_bidir());

constexpr bool test_stride_bidir()
{
    // Basic stride, n divides size
    {
        std::array arr{0, 1, 2, 3, 4, 5};

        auto seq = flux::stride(arr, 2);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(not flux::contiguous_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 2, 4}));
        STATIC_CHECK(seq.last().cur == flux::last(arr));
        STATIC_CHECK(seq.size() == 3);
        STATIC_CHECK(seq.distance(seq.first(), seq.last()) == 3);
        STATIC_CHECK(seq.distance(seq.last(), seq.first()) == -3);
    }

    // Basic stride, n does not divide size
    {
        std::array arr{0, 1, 2, 3, 4, 5, 6, 7};

        auto const seq = flux::stride(arr,3);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 3, 6}));
        STATIC_CHECK(flux::last(seq).cur == flux::last(arr));
        STATIC_CHECK(flux::size(seq) == 3);
        STATIC_CHECK(flux::distance(seq, flux::first(seq), flux::last(seq)) == 3);
        STATIC_CHECK(flux::distance(seq, flux::last(seq), flux::first(seq)) == -3);
    }

    // Reversing gives the expected result
    {
        // stride divides size
        {
            std::array arr{0, 1, 2, 3, 4, 5};

            auto stride_then_rev = flux::stride(arr, 3).reverse();
            auto rev_then_stride = flux::reverse(arr).stride(3);

            STATIC_CHECK(check_equal(stride_then_rev, {3, 0}));
            STATIC_CHECK(check_equal(rev_then_stride, {5, 2}));
        }

        // stride does not divide size
        {
            std::array arr{0, 1, 2, 3, 4, 5, 6};

            auto stride_then_rev = flux::stride(arr, 3).reverse();
            auto rev_then_stride = flux::reverse(arr).stride(3);

            STATIC_CHECK(check_equal(stride_then_rev, {6, 3, 0}));
            STATIC_CHECK(check_equal(rev_then_stride, {6, 3, 0}));
        }

        // ...and again
        {
            std::array arr{0, 1, 2, 3, 4, 5, 6, 7};

            auto stride_then_rev = flux::stride(arr, 3).reverse();
            auto rev_then_stride = flux::reverse(arr).stride(3);

            STATIC_CHECK(check_equal(stride_then_rev, {6, 3, 0}));
            STATIC_CHECK(check_equal(rev_then_stride, {7, 4, 1}));
        }
    }

    // Stride of 1 returns the original sequence
    {
        std::array arr{0, 1, 2, 3, 4, 5, 6};

        auto const seq = flux::stride(arr, 1);

        STATIC_CHECK(check_equal(seq, arr));
        STATIC_CHECK(check_equal(flux::reverse(seq), flux::reverse(arr)));
    }

    // Stride >= than sequence size returns sequence of one element
    {
        {
            std::array arr{0, 1, 2, 3, 4, 5};
            auto seq = flux::ref(arr).stride(arr.size());

            auto cur = seq.first();
            STATIC_CHECK(!seq.is_last(cur));
            STATIC_CHECK(seq[cur] == 0);
            cur = seq.next(cur);
            STATIC_CHECK(seq.is_last(cur));
        }

        {
            std::array arr{0, 1, 2, 3, 4, 5};
            auto seq = flux::ref(arr).stride(99999);

            auto cur = seq.first();
            STATIC_CHECK(!seq.is_last(cur));
            STATIC_CHECK(seq[cur] == 0);
            cur = seq.next(cur);
            STATIC_CHECK(seq.is_last(cur));
        }
    }

    // Stride of distance_t::max doesn't break stuff
    {
        std::array arr{0, 1, 2, 3, 4, 5};
        auto seq = flux::ref(arr).stride(std::numeric_limits<flux::distance_t>::max());

        auto cur = seq.first();
        STATIC_CHECK(!seq.is_last(cur));
        STATIC_CHECK(seq[cur] == 0);
        cur = seq.next(cur);
        STATIC_CHECK(seq.is_last(cur));
    }

    // In-bounds RA jumps work as expected
    {
        std::array arr{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto seq = flux::stride(arr, 3);

        auto cur = seq.first();
        seq.inc(cur, 2);

        STATIC_CHECK(seq[cur] = 6);

        seq.inc(cur, -2);

        STATIC_CHECK(seq[cur] == 0);

        // Jump of zero size does nothing
        STATIC_CHECK(flux::next(seq, cur, 0) == cur);
    }

    // Out-of-bounds RA jumps are saturating
    {
        std::array arr{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto seq = flux::stride(arr, 3);

        auto cur = seq.first();
        seq.inc(cur, 10);

        STATIC_CHECK(cur.cur == 10);
        STATIC_CHECK(seq.is_last(cur));

        seq.inc(cur, -2);

        STATIC_CHECK(seq[cur] == 6);

        cur = seq.first();
        seq.inc(cur, -1);
        STATIC_CHECK(seq[cur] == 0);
    }

    // Internal iteration works as expected
    {
        std::array arr{0, 1, 2, 3, 4, 5, 6};

        auto seq = flux::ref(arr).stride(2);

        STATIC_CHECK(seq.sum() == 0 + 2 + 4 + 6);

        auto cur = seq.find(4);

        STATIC_CHECK(&seq[cur] == arr.data() + 4);

        cur = seq.find(99999);
        STATIC_CHECK(seq.is_last(cur));
        seq.dec(cur);

        STATIC_CHECK(&seq[cur] == arr.data() + 6);
    }


    // Can we sort a strided array?
    {
        std::array arr{9, 8, 7, 6, 5, 4, 3, 2, 1};

        flux::ref(arr).stride(3).sort();

        STATIC_CHECK(check_equal(arr, {3, 8, 7, 6, 5, 4, 9, 2, 1}));
    }

    return true;
}
static_assert(test_stride_bidir());

}

TEST_CASE("stride")
{
    bool res = test_stride_non_bidir();
    REQUIRE(res);

    res = test_stride_bidir();
    REQUIRE(res);

    // Test with bidir-but-not-RA sequence
    {
        auto list = std::list<int>{1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto seq = flux::from_range(list).stride(3);

        REQUIRE(seq.size() == 3);
        REQUIRE(check_equal(seq, {1, 4, 7}));
        REQUIRE(seq.sum() == 12);

        auto rev = std::move(seq).reverse();

        REQUIRE(check_equal(rev, {7, 4, 1}));
        REQUIRE(rev.sum() == 12);
    }

    // detail::advance tests to keep CodeCov happy
    {
        {
            auto seq = NotBidir(std::array{1, 2, 3, 4, 5});
            auto cur = seq.first();
            auto cur2 = cur;

            // advance by zero places should do nothing
            auto r = flux::detail::advance(seq, cur, 0);
            REQUIRE(cur == cur2);
            REQUIRE(r == 0);

            // advance with negative offset for non-bidir sequence is a runtime error
            REQUIRE_THROWS_AS(flux::detail::advance(seq, cur, -2), flux::unrecoverable_error);
        }

        {
            auto seq = flux::from(std::array{1, 2, 3, 4, 5}).stride(2);
            auto cur = seq.first();
            auto cur2 = cur;

            auto r = flux::detail::advance(seq, cur, 0);
            REQUIRE(cur <=> cur2 == std::strong_ordering::equal);
            REQUIRE(r == 0);
        }
    }
}