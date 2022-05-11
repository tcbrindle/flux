
#include "catch.hpp"

#include <flux/source/bitset.hpp>
#include <flux/op/all_any_none.hpp>
#include <flux/op/inplace_reverse.hpp>
#include <flux/op/reverse.hpp>
#include <flux/op/swap_elements.hpp>
#include <flux/ranges/from_range.hpp>

#include "test_utils.hpp"

#include <iostream>

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
        FLUX_FOR(bool bit, flux::reverse(std::as_const(b))) {
            x <<= 1;
            x |= bit;
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
        auto seq2 = flux::from(std::vector<bool>{true, true});

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
        std::vector<bool> seq2(16, true);

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