
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>

#include "test_utils.hpp"

namespace {

constexpr bool test_drop_while()
{
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto seq = flux::drop_while(flux::ref(arr), [](int i) { return i < 5; });

        using S = decltype(seq);

        static_assert(flux::contiguous_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>); // because bounded + RA

        static_assert(flux::contiguous_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(flux::sized_sequence<S const>);

        STATIC_CHECK(seq.size() == 5);
        STATIC_CHECK(seq.data() == arr + 5);
        STATIC_CHECK(check_equal(seq, {5, 6, 7, 8, 9}));

        auto const& c_seq = seq;
        STATIC_CHECK(c_seq.size() == 5);
        STATIC_CHECK(c_seq.data() == arr + 5);
        STATIC_CHECK(check_equal(seq, {5, 6, 7, 8, 9}));
    }

    // Single-pass sequences are okay
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto seq = single_pass_only(flux::ref(arr)).drop_while([](int i) { return i < 5; });

        using S = decltype(seq);

        static_assert(not flux::multipass_sequence<S>);

        STATIC_CHECK(check_equal(seq, {5, 6, 7, 8, 9}));
    }

    // We don't filter longer than we should
    {
        int arr[] = {2, 2, 2, 3, 4, 5, 6, 7, 8, 9};

        auto seq = flux::drop_while(flux::ref(arr), [](int i) { return i % 2 == 0; });

        STATIC_CHECK(check_equal(seq, {3, 4, 5, 6, 7, 8, 9}));
    }

    // We can drop everything
    {
        auto yes = [](auto) { return true; };

        auto seq = flux::drop_while(std::array{1, 2, 3, 4, 5, 6, 7, 8, 9}, yes);

        STATIC_CHECK(seq.is_empty());
    }

    // We can drop nothing
    {
        auto no = [](auto) { return false; };

        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9};

        STATIC_CHECK(check_equal(flux::drop_while(flux::ref(arr), no), arr));
    }

    return true;
}
static_assert(test_drop_while());

}

TEST_CASE("drop_while")
{
    bool result = test_drop_while();
    REQUIRE(result);
}