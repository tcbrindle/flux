
#include "catch.hpp"

#include <flux.hpp>
#include <flux/op/write_to.hpp>

#include "test_utils.hpp"

#include <array>
#include <iostream>
#include <vector>

namespace {

constexpr bool test_chain()
{
    // Basic chaining
    {
        int arr1[] = {0, 1, 2};
        int arr2[] = {3, 4, 5};
        int arr3[] = {6, 7, 8};

        auto seq = flux::chain(arr1, arr2, arr3);

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        STATIC_CHECK(flux::size(seq) == 9);
        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5, 6, 7, 8}));

        auto idx1 = seq.next(seq.first());
        auto idx2 = seq.prev(seq.last());

        STATIC_CHECK(seq.distance(idx1, idx2) == 7);

        // Make sure we're really multipass, not pretending
        auto idx = seq.find(4);
        (void) seq.next(idx);
        STATIC_CHECK(seq[idx] == 4);
    }

    // Chaining single-pass sequences works as expected
    {
        int arr1[] = {0, 1, 2};
        int arr2[] = {3, 4, 5};

        auto seq = flux::chain(single_pass_only(flux::from(arr1)),
                               single_pass_only(flux::from(arr2)));

        using S = decltype(seq);

        static_assert(flux::sequence<S>);
        static_assert(!flux::multipass_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5}));
    }

    // Reverse works when all chained sequences are bidir + bounded
    {
        int arr1[] = {0, 1, 2};
        int arr2[] = {3, 4, 5};

        auto seq = flux::chain(arr1, arr2).reverse();

        STATIC_CHECK(check_equal(seq, {5, 4, 3, 2, 1, 0}));
    }

    // Empty sequences are ignored as they should be
    {
        int arr1[] = {0, 1, 2};
        std::array arr2 = {3, 4, 5};

        auto seq = flux::chain(arr1,
                               flux::empty<int const>,
                               arr2,
                               flux::empty<int const>,
                               std::array{6, 7, 8});

        STATIC_CHECK(flux::size(seq) == 9);
        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5, 6, 7, 8}));
    }

    // We can sort across RA sequences
    {
        int arr1[] = {9, 8, 7};
        std::array arr2 = {6, 5, 4};

        auto seq = flux::chain(arr1,
                               flux::empty<int>,
                               arr2,
                               flux::empty<int>,
                               std::array{3, 2, 1});

        std::ranges::sort(seq.view());

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    return true;
}
static_assert(test_chain());

}

TEST_CASE("chain")
{
    bool result = test_chain();
    REQUIRE(result);
}