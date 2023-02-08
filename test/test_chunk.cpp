
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <limits>
#include <list>
#include <iostream>

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

constexpr bool test_chunk_multipass()
{
    // Basic multipass chunk
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = NotBidir(arr).chunk(2);

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {1, 2}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(seq.size() == 3);
    }

    // Basic multipass chunk, const iteration
    {
        std::array arr{1, 2, 3, 4, 5};

        auto const seq = flux::chunk(NotBidir(std::cref(arr)), 2);

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur), {1, 2}));
        STATIC_CHECK(check_equal(flux::read_at(seq, flux::inc(seq, cur)), {3, 4}));
        STATIC_CHECK(check_equal(flux::read_at(seq, flux::inc(seq, cur)), {5}));
        STATIC_CHECK(flux::is_last(seq, flux::inc(seq, cur)));

        STATIC_CHECK(cur == flux::last(seq));

        STATIC_CHECK(flux::size(seq) == 3);
    }

    // Multipass, chunk size equal to seq size
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = NotBidir(arr).chunk(arr.size());

        auto cur = seq.first();

        STATIC_CHECK(check_equal(seq[cur], {1, 2, 3, 4, 5}));
        seq.inc(cur);
        STATIC_CHECK(seq.is_last(cur));

        STATIC_CHECK(flux::size(seq) == 1);
    }

    // Multipass w/ oversized chunks
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = NotBidir(arr).chunk(10);

        STATIC_CHECK(seq.size() == 1);
        STATIC_CHECK(check_equal(seq.front().value(), {1, 2, 3, 4, 5}));
    }

    // Chunk size == distance_t::max doesn't crash
    {
        std::array arr{1, 2, 3, 4, 5};

        constexpr auto max = std::numeric_limits<flux::distance_t>::max();

        auto seq = NotBidir(arr).chunk(max);

        STATIC_CHECK(seq.size() == 1);
        STATIC_CHECK(check_equal(seq.front().value(), {1, 2, 3, 4, 5}));
    }

    // Multipass chunk with empty sequence
    {
        auto seq = flux::chunk(NotBidir(flux::empty<int>), 10);

        STATIC_CHECK(flux::size(seq) == 0);
        STATIC_CHECK(flux::is_last(seq, flux::first(seq)));
    }

    // Test chunks of size 1
    {
        auto seq = NotBidir(std::array{1, 2, 3, 4, 5}).chunk(1);

        STATIC_CHECK(seq.size() == 5);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {1}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {2}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));
    }

    // Test adaptors
    {
        auto seq = NotBidir(std::array{1, 2, 3, 4, 5}).chunk(2);

        auto r = std::move(seq).map(flux::product).sum();

        STATIC_CHECK(r == (1 * 2) + (3 * 4) + 5);
    }

    return true;
}
static_assert(test_chunk_multipass());

constexpr bool test_chunk_bidir()
{
    // Basic bidir chunk
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::from(arr).chunk(2);

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::random_access_sequence<S>);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {1, 2}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(seq.size() == 3);
    }

    // Basic bidir chunk, const iteration
    {
        std::array arr{1, 2, 3, 4, 5};

        auto const seq = flux::chunk(std::cref(arr), 2);

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::random_access_sequence<S>);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(flux::read_at(seq, cur), {1, 2}));
        STATIC_CHECK(check_equal(flux::read_at(seq, flux::inc(seq, cur)), {3, 4}));
        STATIC_CHECK(check_equal(flux::read_at(seq, flux::inc(seq, cur)), {5}));
        STATIC_CHECK(flux::is_last(seq, flux::inc(seq, cur)));

        STATIC_CHECK(cur == flux::last(seq));

        STATIC_CHECK(flux::size(seq) == 3);
    }

    // Bidir, chunk size equal to seq size
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::from(arr).chunk(arr.size());

        auto cur = seq.first();

        STATIC_CHECK(check_equal(seq[cur], {1, 2, 3, 4, 5}));
        seq.inc(cur);
        STATIC_CHECK(seq.is_last(cur));

        STATIC_CHECK(flux::size(seq) == 1);
    }

    // Bidir w/ oversized chunks
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::chunk(arr,10);

        STATIC_CHECK(seq.size() == 1);
        STATIC_CHECK(check_equal(seq.front().value(), {1, 2, 3, 4, 5}));
    }

    // Chunk size == distance_t::max doesn't crash
    {
        std::array arr{1, 2, 3, 4, 5};

        constexpr auto max = std::numeric_limits<flux::distance_t>::max();

        auto seq = flux::chunk(arr,max);

        STATIC_CHECK(seq.size() == 1);
        STATIC_CHECK(check_equal(seq.front().value(), {1, 2, 3, 4, 5}));
    }

    // Multipass chunk with empty sequence
    {
        auto seq = flux::chunk(flux::empty<int>, 10);

        STATIC_CHECK(flux::size(seq) == 0);
        STATIC_CHECK(flux::is_last(seq, flux::first(seq)));
    }

    // Test chunks of size 1
    {
        auto seq = flux::chunk(std::array{1, 2, 3, 4, 5}, 1);

        STATIC_CHECK(seq.size() == 5);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {1}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {2}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));
    }

    // Test adaptors
    {
        auto seq = flux::chunk(std::array{1, 2, 3, 4, 5}, 2);

        auto r = std::move(seq).map(flux::product).sum();

        STATIC_CHECK(r == (1 * 2) + (3 * 4) + 5);
    }

    // Reversing a chunked sequence works as expected
    {
        auto seq = flux::chunk(std::array{1, 2, 3, 4, 5}, 2).reverse();

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {5}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {1, 2}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(seq.size() == 3);
    }

    // Chunk -> stride -> reverse() works...
    {
        auto arr = std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto seq = flux::chunk(arr, 3).stride(2).reverse();

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {7, 8, 9}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {1, 2, 3}));
    }

    // RA jumps in a chunked sequence work as expected
    {
        auto seq = flux::chunk(std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 3);

        auto cur = seq.first();

        seq.inc(cur, 3);
        STATIC_CHECK(check_equal(seq[cur], {10}));

        seq.inc(cur, -2);
        STATIC_CHECK(check_equal(seq[cur], {4, 5, 6}));
    }

    return true;
}
static_assert(test_chunk_bidir());

}

TEST_CASE("chunk")
{
    bool res = test_chunk_multipass();
    REQUIRE(res);

    res = test_chunk_bidir();
    REQUIRE(res);

    // Test chunk with bidir (only) sequence
    {
        std::list<int> list{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto seq = flux::from_range(list).chunk(3).reverse();

        auto cur = seq.first();
        REQUIRE(check_equal(seq[cur], {10}));
        REQUIRE(check_equal(seq[seq.inc(cur)], {7, 8, 9}));
        REQUIRE(check_equal(seq[seq.inc(cur)], {4, 5, 6}));
        REQUIRE(check_equal(seq[seq.inc(cur)], {1, 2, 3}));

        REQUIRE(seq.is_last(seq.inc(cur)));
        REQUIRE(cur == seq.last());
    }
}