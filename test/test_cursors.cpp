
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <sstream>

namespace {

constexpr bool test_cursors()
{
    // Basic cursors adaptor
    {
        std::array arr{100, 200, 300, 400, 500};

        auto indices = flux::cursors(arr);

        using S = decltype(indices);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, flux::index_t>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, flux::index_t>);
        static_assert(std::same_as<flux::const_element_t<S>, flux::index_t>);
        static_assert(std::same_as<flux::value_t<S>, flux::index_t>);

        STATIC_CHECK(indices.size() == flux::size(arr));
        STATIC_CHECK(check_equal(indices, {0, 1, 2, 3, 4}));

        auto cur = indices.last();

        STATIC_CHECK(flux::distance(arr, flux::first(arr), cur) == 5);

        auto rev = flux::reverse(indices);

        STATIC_CHECK(check_equal(rev, {4, 3, 2, 1, 0}));
    }

    // Cursors adaptor works with infinite sequences
    {
        auto seq = flux::repeat(10);

        auto curs = seq.cursors();

        using S = decltype(curs);

        static_assert(flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(flux::infinite_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, std::size_t>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, std::size_t>);
        static_assert(std::same_as<flux::const_element_t<S>, std::size_t>);
        static_assert(std::same_as<flux::value_t<S>, std::size_t>);

        STATIC_CHECK(check_equal(flux::take(curs, 5), {0u, 1u, 2u, 3u, 4u}));
    }

    // Cursors adaptor works with adapted sequences
    {
        auto const arr = std::array{101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

        auto evens = flux::filter(arr, flux::pred::even);
        auto indices_of_evens = evens.cursors();

        STATIC_CHECK(flux::count(indices_of_evens) == 5);
        STATIC_CHECK(check_equal(indices_of_evens, {1, 3, 5, 7, 9}));
    }

    // Cursors adaptor can be used to produce a mutating cycle adaptor
    {
        std::array arr{1, 2, 3, 4, 5};

        auto read_array = [&arr](auto cur) -> decltype(auto) {
            return flux::read_at(arr, cur);
        };

        auto mut_cycled = flux::cursors(arr).cycle().map(read_array);

        auto cur = flux::next(mut_cycled, mut_cycled.first(), 102);
        mut_cycled[cur] = 99;

        STATIC_CHECK(check_equal(arr, {1, 2, 99, 4, 5}));
    }

    return true;
}
static_assert(test_cursors());

}

TEST_CASE("cursors")
{
    bool result = test_cursors();
    REQUIRE(result);

    {
        std::ostringstream oss{};

        auto seq = flux::zip(std::array{1, 2, 3, 4, 5},
                             std::array{5, 4, 3, 2, 1});

        for (auto idx : seq.cursors()) {
            auto [a, b] = seq[idx];
            oss << a << b << ' ';
        }

        REQUIRE(oss.str() == "15 24 33 42 51 ");
    }
}