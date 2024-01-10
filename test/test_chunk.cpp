
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <limits>
#include <list>
#include <sstream>

#include "test_utils.hpp"

namespace {

#ifdef _MSC_VER
#define COMPILER_IS_MSVC 1
#else
#define COMPILER_IS_MSVC 0
#endif

constexpr bool compiler_is_msvc = COMPILER_IS_MSVC;

template <flux::sequence Base>
struct NotBidir : flux::inline_sequence_base<NotBidir<Base>> {
    Base base_;

    constexpr explicit NotBidir(Base base) : base_(std::move(base)) {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }

    struct flux_sequence_traits {
        static constexpr auto first(auto& self) { return flux::first(self.base_); }

        static constexpr bool is_last(auto& self, auto const& cur) {
            return flux::is_last(self.base_, cur);
        }

        static constexpr void inc(auto& self, auto& cur) {
            flux::inc(self.base_, cur);
        }

        static constexpr decltype(auto) read_at(auto& self, auto const& cur) {
            return flux::read_at(self.base_, cur);
        }

        static constexpr auto last(auto& self) { return flux::last(self.base_); }

        static constexpr auto size(auto& self) { return flux::size(self.base_); }
    };
};

constexpr bool test_chunk_single_pass()
{
    // Basic single-pass chunk
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = single_pass_only(std::move(arr)).chunk(2);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {1, 2}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(seq.size() == 3);
    }

    // Single-pass chunk, not consuming inner sequences
    {
        auto seq = single_pass_only(std::array{1, 2, 3, 4, 5}).chunk(2);

        STATIC_CHECK(seq.size() == 3);

        auto cur = seq.first();
        for (int i = 0; i < 3; i++) {
            seq.inc(cur);
        }

        STATIC_CHECK(seq.is_last(cur));
    }

    // Single-pass chunk, chunk sz == seq sz, consuming
    {
        auto seq = single_pass_only(std::array{1, 2, 3, 4, 5}).chunk(5);

        STATIC_CHECK(seq.size() == 1);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {1, 2, 3, 4, 5}));
        seq.inc(cur);
        STATIC_CHECK(seq.is_last(cur));
    }

    // Single-pass chunk, chunk sz == seq sz, not consuming
    {
        auto seq = single_pass_only(std::array{1, 2, 3, 4, 5}).chunk(5);

        STATIC_CHECK(seq.size() == 1);

        auto cur = seq.first();
        STATIC_CHECK(seq[cur][seq[cur].first()] == 1);
        seq.inc(cur);
        STATIC_CHECK(seq.is_last(cur));
    }

    // Single-pass chunk, chunk sz > seq sz, consuming
    {
        auto seq = single_pass_only(std::array{1, 2, 3, 4, 5}).chunk(99999);

        STATIC_CHECK(seq.size() == 1);

        auto cur = seq.first();
        STATIC_CHECK(check_equal(seq[cur], {1, 2, 3, 4, 5}));
        seq.inc(cur);
        STATIC_CHECK(seq.is_last(cur));
    }

    // Single-pass chunk, chunk sz > seq sz, not consuming
    {
        auto seq = single_pass_only(std::array{1, 2, 3, 4, 5}).chunk(99999);

        STATIC_CHECK(seq.size() == 1);

        auto cur = seq.first();
        STATIC_CHECK(seq[cur][seq[cur].first()] == 1);
        seq.inc(cur);
        STATIC_CHECK(seq.is_last(cur));
    }

    // Chunked empty single-pass sequence => empty sequence
    {
        auto seq = single_pass_only(flux::copy(flux::empty<int>)).chunk(3);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(flux::is_last(seq, flux::first(seq)));
    }

    // Test round-tripping, chunk -> flatten
    {
        auto seq = single_pass_only(std::array{1, 2, 3, 4, 5}).chunk(2).flatten();

        // FIXME: MSVC doesn't like this during constexpr evaluation
        // Specifically, the problem occurs in the first call to inc(seq, cur),
        // complaining of a read of an uninitialised union member inside flatten's
        // optional<inner_cur>. (The same happens if we use std::optional rather
        // than flux::optional inside flatten_adaptor(), so it doesn't seem to
        // be a bug in our optional implementation.)
        //
        // The same check passes constexpr evaluation in GCC, and runtime
        // eval with both GCC and MSVC (including GCC + UBSan), so I'm inclined
        // to think it's a compiler bug rather than a Flux bug, although it is a
        // little worrisome.
        if (!(std::is_constant_evaluated() && compiler_is_msvc)) {
            STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
        }
    }

    return true;
}
static_assert(test_chunk_single_pass());

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

    // Test round-tripping with flatten
    {
        auto seq = NotBidir(std::array{1, 2, 3, 4, 5}).chunk(2).flatten();

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
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

        auto const seq = flux::chunk(flux::ref(arr), 2);

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

    // Test round-tripping with flatten
    {
        auto seq = NotBidir(std::array{1, 2, 3, 4, 5}).chunk(2).flatten();

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
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
    bool res = test_chunk_single_pass();
    REQUIRE(res);

    res = test_chunk_multipass();
    REQUIRE(res);

    res = test_chunk_bidir();
    REQUIRE(res);

    SECTION("...with istream sequence")
    {
        std::istringstream iss("1 2 3 4 5");
        std::ostringstream out;

        flux::from_istream<int>(iss)
                .chunk(2)
                .write_to(out);

        REQUIRE(out.str() == "[[1, 2], [3, 4], [5]]");
    }

    SECTION("...with bidir only sequence")
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