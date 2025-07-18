
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include "test_utils.hpp"

namespace {

constexpr bool test_bitset()
{
    {
        std::bitset<32> b{0xCAFEBABE};

        using B = decltype(b);

        static_assert(flux::random_access_sequence<B>);
        static_assert(flux::bounded_sequence<B>);
        static_assert(flux::sized_sequence<B>);

        static_assert(std::same_as<flux::element_t<B>, std::bitset<32>::reference>);
        static_assert(std::same_as<flux::element_t<B const>, bool>);
        static_assert(std::same_as<flux::value_t<B>, bool>);
        static_assert(std::same_as<flux::rvalue_element_t<B>, bool>);
        static_assert(std::same_as<flux::rvalue_element_t<B const>, bool>);

        static_assert(flux::size(b) == 32);

        uint32_t x = 0;
        FLUX_FOR(bool bit, flux::reverse(flux::ref(std::as_const(b)))) {
            x <<= 1;
            x |= uint32_t(bit);
        }

        STATIC_CHECK(x == 0xCAFEBABE);
    }

    return true;
}
static_assert(test_bitset());

}

TEST_CASE("bitset")
{
    bool result = test_bitset();
    REQUIRE(result);

    // Swapping bits
    {
        std::bitset<2> bs{0b01};
        flux::swap_at(bs, 0, 1);
        REQUIRE(bs == std::bitset<2>{0b10});
    }

    // Swapping bits with real bools
    {
        auto seq1 = flux::from(std::bitset<2>{0b00});
        auto seq2 = flux::from(std::array<bool, 2>{true, true});

        flux::swap_with(seq1, flux::first(seq1),
                        seq2, flux::first(seq2));

        REQUIRE(seq1.base() == std::bitset<2>{0b01});
        REQUIRE(check_equal(seq2, {false, true}));
    }

    // Swapping bits with another proxy reference type
    {
        auto seq1 = flux::from(std::bitset<2>{0b00});
        auto vec = std::vector<bool>{true, true};
        auto seq2 = flux::from_range(vec);

        flux::swap_with(seq1, flux::first(seq1),
                       seq2, flux::first(seq2));

        REQUIRE(seq1.base() == std::bitset<2>{0b01});
        REQUIRE(check_equal(seq2, {false, true}));
    }

    // swap_elements between two bitsets
    {
        std::bitset<16> seq1{};
        std::bitset<16> seq2{}; seq2.flip();

        REQUIRE(seq1.none());
        REQUIRE(seq2.all());

        flux::swap_elements(seq1, seq2);

        REQUIRE(seq1.all());
        REQUIRE(seq2.none());
    }

    // swap_elements between a bitset and a vector<bool>
    {
        std::bitset<16> seq1{};
        auto vec = std::vector<bool>(16, true);
        auto seq2 = flux::from_range(vec);

        flux::swap_elements(seq1, seq2);

        REQUIRE(seq1.all());
        REQUIRE(flux::none(seq2, std::identity{}));
    }

    // reversing a bitset in-place
    {
        std::bitset<8> bs{0b01010101};

        flux::inplace_reverse(bs);

        REQUIRE(bs == std::bitset<8>(0b10101010));
    }
}