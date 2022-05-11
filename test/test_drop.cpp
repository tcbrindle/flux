
#include "catch.hpp"

#include <flux/op/drop.hpp>
#include <flux/op/reverse.hpp>
#include <flux/ranges/from_range.hpp>

#include "test_utils.hpp"

#include <array>
#include <iostream>

namespace {

constexpr bool test_drop() {

    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto dropped = flux::drop(arr, 5);

        using D = decltype(dropped);

        static_assert(flux::contiguous_sequence<D>);
        static_assert(flux::sized_sequence<D>);
        static_assert(flux::bounded_sequence<D>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(flux::data(dropped) == arr + 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));
    }

    {
        auto dropped = flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                           .drop(5);

        using D = decltype(dropped);

        static_assert(flux::contiguous_sequence<D>);
        static_assert(flux::sized_sequence<D>);
        static_assert(flux::bounded_sequence<D>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));
    }

    {
        auto dropped = single_pass_only(flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}))
                           .drop(5);

        using D = decltype(dropped);

        static_assert(flux::sequence<D>);
        static_assert(not flux::multipass_sequence<D>);
        static_assert(flux::sized_sequence<D>);
        static_assert(flux::bounded_sequence<D>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));
    }

    return true;
}
static_assert(test_drop());

}

TEST_CASE("drop")
{
    bool result = test_drop();
    REQUIRE(result);
}