
#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <list>

namespace {

constexpr bool test_reverse()
{
    {
        int arr[] = {0, 1, 2, 3, 4};

        auto reversed = flux::reverse(arr);

        using R = decltype(reversed);

        static_assert(flux::random_access_sequence<R>);
        static_assert(not flux::contiguous_sequence<R>);
        static_assert(flux::bounded_sequence<R>);
        static_assert(flux::sized_sequence<R>);

        static_assert(flux::random_access_sequence<R const>);
        static_assert(not flux::contiguous_sequence<R const>);
        static_assert(flux::bounded_sequence<R const>);
        static_assert(flux::sized_sequence<R const>);

        STATIC_CHECK(flux::size(reversed) == 5);
        STATIC_CHECK(check_equal(reversed, {4, 3, 2, 1, 0}));
    }

    {
        auto reversed = flux::from(std::array{0, 1, 2, 3, 4}).reverse();

        using R = decltype(reversed);

        static_assert(flux::random_access_sequence<R>);
        static_assert(not flux::contiguous_sequence<R>);
        static_assert(flux::bounded_sequence<R>);
        static_assert(flux::sized_sequence<R>);

        static_assert(flux::random_access_sequence<R const>);
        static_assert(not flux::contiguous_sequence<R const>);
        static_assert(flux::bounded_sequence<R const>);
        static_assert(flux::sized_sequence<R const>);

        STATIC_CHECK(flux::size(reversed) == 5);
        STATIC_CHECK(check_equal(reversed, {4, 3, 2, 1, 0}));
    }

    {
        std::array arr{0, 1, 2, 3, 4};

        auto seq = flux::from(arr).reverse().reverse();

        using S = decltype(seq);

        static_assert(flux::contiguous_sequence<S>);
        static_assert(std::same_as<S, decltype(flux::from(arr))>);

        STATIC_CHECK(seq.data() == arr.data());
    }

    {
        std::array arr{0, 1, 2, 3, 4};

        auto seq = flux::from(arr).reverse().reverse().reverse();

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(std::same_as<S, decltype(flux::reverse(arr))>);

        STATIC_CHECK(check_equal(seq, {4, 3, 2, 1, 0}));
    }

    return true;
}
static_assert(test_reverse());

}

TEST_CASE("reverse")
{
    bool result = test_reverse();
    REQUIRE(result);

    {
        auto rlist = flux::reverse(std::list{0, 1, 2, 3, 4});

        using R = decltype(rlist);

        static_assert(flux::regular_index<flux::index_t<R>>);
      //  static_assert(flux::bidirectional_sequence<R>);
     //   static_assert(not flux::random_access_sequence<R>);
        // FIXME: GCC ICE
        // static_assert(flux::sized_sequence<R>);
       // static_assert(flux::bounded_sequence<R>);

       // REQUIRE(check_equal(rlist, {4, 3, 2, 1, 0}));
    }
}