
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

constexpr bool test_prescan()
{
    // prescan returns the same as std::exclusive_scan with one extra value
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).prescan(std::plus<>{}, 0);

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);

        std::array<int, 5> req{};
        std::exclusive_scan(arr.cbegin(), arr.cend(), req.begin(), 0);

        STATIC_CHECK(flux::starts_with(seq, req));

        STATIC_CHECK(seq[seq.first()] == flux::fold(arr, std::plus{}, 0));
    }

    // map -> prescan returns the same as std::transform_exclusive_scan with one extra value
    {
        auto const in = std::array{1, 2, 3, 4, 5};

        auto square = [](int i) { return i * i; };

        std::array<int, 6> out_flux{};
        std::array<int, 5> out_std{};

        flux::map(in, square)
            .prescan(std::plus<>{}, 0)
            .output_to(out_flux.begin());

        std::transform_exclusive_scan(in.cbegin(), in.cend(), out_std.begin(),
                                      0, std::plus<>{}, square);

        STATIC_CHECK(flux::starts_with(out_flux, out_std));

        STATIC_CHECK(out_flux.back() == flux::map(in, square).fold(std::plus{}, 0));
    }

    // prescan of an empty sequence is returns just the initial element
    {
        auto seq = flux::prescan(flux::empty<int>, std::plus<>{}, 100);

        STATIC_CHECK(not seq.is_empty());
        STATIC_CHECK(seq[seq.first()] == 100);
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // Can resume correctly after internal iteration
    {
        auto seq = flux::prescan(std::array{1, 2, 3, 4, 5}, std::plus<>{}, 0);

        auto cur = seq.find(6);

        STATIC_CHECK(not seq.is_last(cur));
        STATIC_CHECK(seq[cur] == 6);
        STATIC_CHECK(seq[seq.inc(cur)] == 10);
        STATIC_CHECK(seq[seq.inc(cur)] == 15);
        STATIC_CHECK(seq.is_last(seq.inc(cur)));
    }

    return true;
}
static_assert(test_prescan());

constexpr bool test_scan_first()
{
    // scan_first returns the same as std::inclusive_scan
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).scan_first(std::plus<>{});

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

    // map -> scan_first returns the same as std::transform_inclusive_scan
    {
        auto const in = std::array{1, 2, 3, 4, 5};

        auto square = [](int i) { return i * i; };

        std::array<int, 5> out_flux{}, out_std{};

        flux::map(in, square).scan_first(std::plus<>{}).output_to(out_flux.begin());

        std::transform_inclusive_scan(in.cbegin(), in.cend(), out_std.begin(),
                                      std::plus<>{}, square);

        STATIC_CHECK(out_flux == out_std);
    }

    // scan_first of an empty sequence is empty
    {
        auto seq = flux::scan_first(flux::empty<int>, std::plus<>{});

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // Can resume correctly after internal iteration
    {
        auto seq = flux::scan_first(std::array{1, 2, 3, 4, 5}, std::plus<>{});

        auto cur = seq.find(6);

        STATIC_CHECK(not seq.is_last(cur));
        STATIC_CHECK(seq[cur] == 6);
        STATIC_CHECK(seq[seq.inc(cur)] == 10);
        STATIC_CHECK(seq[seq.inc(cur)] == 15);
        STATIC_CHECK(seq.is_last(seq.inc(cur)));
    }
    return true;
}
static_assert(test_scan_first());

}

TEST_CASE("scan")
{
    bool res = test_inclusive_scan();
    REQUIRE(res);

    res = test_prescan();
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

    SECTION("prescan with stringstream")
    {
        std::istringstream iss("1 2 3 4 5");

        auto seq = flux::prescan(flux::from_istream<int>(iss),
                                        std::plus<>{}, 100);

        static_assert(flux::sequence<decltype(seq)>);
        static_assert(not flux::multipass_sequence<decltype(seq)>);
        static_assert(not flux::sized_sequence<decltype(seq)>);
        static_assert(not flux::bounded_sequence<decltype(seq)>);

        REQUIRE(check_equal(seq, {100, 101, 103, 106, 110, 115}));
    }

    SECTION("scan_first with stringstream")
    {
        std::istringstream iss("1 2 3 4 5");

        auto seq = flux::from_istream<int>(iss).scan_first(std::plus<>{});

        static_assert(flux::sequence<decltype(seq)>);
        static_assert(not flux::multipass_sequence<decltype(seq)>);
        static_assert(not flux::sized_sequence<decltype(seq)>);
        static_assert(not flux::bounded_sequence<decltype(seq)>);

        REQUIRE(check_equal(seq, {1, 3, 6, 10, 15}));
    }
}
