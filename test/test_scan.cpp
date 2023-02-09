
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <numeric>
#include <sstream>

namespace {

constexpr bool test_inclusive_scan()
{
    // scan returns the same as std::inclusive_scan
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).scan(std::plus<>{});

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);

        std::array<int, 5> req{};
        std::inclusive_scan(arr.cbegin(), arr.cend(), req.begin());

        STATIC_CHECK(check_equal(seq, req));
    }

    // map -> scan returns the same as std::transform_inclusive_scan
    {
        auto const in = std::array{1, 2, 3, 4, 5};

        auto square = [](int i) { return i * i; };

        std::array<int, 5> out_flux{}, out_std{};

        flux::map(in, square).scan(std::plus<>{}).output_to(out_flux.begin());

        std::transform_inclusive_scan(in.cbegin(), in.cend(), out_std.begin(),
                                      std::plus<>{}, square);

        STATIC_CHECK(out_flux == out_std);
    }

    // scan of an empty sequence is empty
    {
        auto seq = flux::scan(flux::empty<int>, std::plus<>{}, 0);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // Can resume correctly after internal iteration
    {
        auto seq = flux::scan(std::array{1, 2, 3, 4, 5}, std::plus<>{});

        auto cur = seq.find(6);

        STATIC_CHECK(not seq.is_last(cur));
        STATIC_CHECK(seq[cur] == 6);
        STATIC_CHECK(seq[seq.inc(cur)] == 10);
        STATIC_CHECK(seq[seq.inc(cur)] == 15);
        STATIC_CHECK(seq.is_last(seq.inc(cur)));
    }

    return true;
}
static_assert(test_inclusive_scan());

constexpr bool test_exclusive_scan()
{
    // exclusive_scan returns the same as std::exclusive_scan
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).exclusive_scan(std::plus<>{}, 0);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);

        std::array<int, 5> req{};
        std::exclusive_scan(arr.cbegin(), arr.cend(), req.begin(), 0);

        STATIC_CHECK(check_equal(seq, req));
    }

    // map -> exclusive_scan returns the same as std::transform_exclusive_scan
    {
        auto const in = std::array{1, 2, 3, 4, 5};

        auto square = [](int i) { return i * i; };

        std::array<int, 5> out_flux{}, out_std{};

        flux::map(in, square)
            .exclusive_scan(std::plus<>{}, 0)
            .output_to(out_flux.begin());

        std::transform_exclusive_scan(in.cbegin(), in.cend(), out_std.begin(),
                                      0, std::plus<>{}, square);

        STATIC_CHECK(out_flux == out_std);
    }

    // scan of an empty sequence is empty
    {
        auto seq = flux::scan(flux::empty<int>, std::plus<>{}, 0);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // Can resume correctly after internal iteration
    {
        auto seq = flux::exclusive_scan(std::array{1, 2, 3, 4, 5}, std::plus<>{}, 0);

        auto cur = seq.find(6);

        STATIC_CHECK(not seq.is_last(cur));
        STATIC_CHECK(seq[cur] == 6);
        STATIC_CHECK(seq[seq.inc(cur)] == 10);
        STATIC_CHECK(seq.is_last(seq.inc(cur)));
    }

    return true;
}
static_assert(test_exclusive_scan());

}

TEST_CASE("scan")
{
    bool res = test_inclusive_scan();
    REQUIRE(res);

    res = test_exclusive_scan();
    REQUIRE(res);

    SECTION("inclusive scan with stringstream")
    {
        std::istringstream iss("1 2 3 4 5");

        auto seq = flux::from_istream<int>(iss).scan(std::plus<>{}, 100);

        static_assert(flux::sequence<decltype(seq)>);
        static_assert(not flux::multipass_sequence<decltype(seq)>);
        static_assert(not flux::sized_sequence<decltype(seq)>);
        static_assert(not flux::bounded_sequence<decltype(seq)>);

        REQUIRE(check_equal(seq, {101, 103, 106, 110, 115}));
    }

    SECTION("exclusive scan with stringstream")
    {
        std::istringstream iss("1 2 3 4 5");

        auto seq = flux::exclusive_scan(flux::from_istream<int>(iss),
                                        std::plus<>{}, 100);

        static_assert(flux::sequence<decltype(seq)>);
        static_assert(not flux::multipass_sequence<decltype(seq)>);
        static_assert(not flux::sized_sequence<decltype(seq)>);
        static_assert(not flux::bounded_sequence<decltype(seq)>);

        REQUIRE(check_equal(seq, {100, 101, 103, 106, 110}));
    }
}
