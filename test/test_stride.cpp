
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <limits>

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

}

TEST_CASE("stride")
{
    auto res = test_stride_non_bidir();
    REQUIRE(res);
}