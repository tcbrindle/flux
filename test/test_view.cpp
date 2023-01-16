
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include <algorithm>
#include <array>
#include <list>
#include <iostream>

#include "test_utils.hpp"

namespace {

constexpr bool test_view()
{
    namespace rng = std::ranges;

    {
        int arr[] = {0, 1, 2, 3, 4};
        auto view = flux::ref(arr);

        using V = decltype(view);

        static_assert(rng::view<V>);
        static_assert(rng::contiguous_range<V>);
        static_assert(rng::common_range<V>);
        static_assert(rng::sized_range<V>);

        static_assert(std::same_as<rng::range_reference_t<V>, int&>);
        static_assert(std::same_as<rng::range_rvalue_reference_t<V>, int&&>);
        static_assert(std::same_as<rng::range_value_t<V>, int>);

        STATIC_CHECK(rng::size(view) == 5);
        STATIC_CHECK(rng::equal(view, arr));
        STATIC_CHECK(view.data() == arr);
        STATIC_CHECK(view.begin()[3] == arr[3]);
    }

    {
        auto view = flux::from(std::array{1, 2, 3, 4, 5});

        using V = decltype(view);

        static_assert(rng::view<V>);
        static_assert(rng::contiguous_range<V>);
        static_assert(rng::common_range<V>);
        static_assert(rng::sized_range<V>);

        static_assert(std::same_as<rng::range_reference_t<V>, int&>);
        static_assert(std::same_as<rng::range_rvalue_reference_t<V>, int&&>);
        static_assert(std::same_as<rng::range_value_t<V>, int>);

        STATIC_CHECK(rng::size(view) == 5);
        STATIC_CHECK(rng::equal(view, std::array{1, 2, 3, 4, 5}));
    }

    {
        using List = std::list<int>;
        using V = decltype(flux::from_range(FLUX_DECLVAL(List&)));

        static_assert(rng::bidirectional_range<V>);
        static_assert(rng::sized_range<V>);
        static_assert(rng::common_range<V>);
    }

    // Range -> Sequence -> View -> Sequence -> View (!)
    {
        auto arr = std::array{1, 2, 3, 4, 5};
        auto view1 = arr | std::views::filter([](int i) { return i % 2 == 0; });
        auto seq = flux::from_range(view1);
        auto view2 = seq | std::views::transform([](int i) { return i * 2; });
        auto view3 = flux::from_range(std::move(view2));

        using V = decltype(view3);

        static_assert(rng::view<V>);
        static_assert(rng::bidirectional_range<V>);

        STATIC_CHECK(rng::equal(view3, std::array{4, 8}));
    }

    {
        int arr[] = {1, 2, 3, 4, 5};
        auto view = single_pass_only(flux::from(arr));

        using V = decltype(view);

        using S = decltype(view.end());
        using I = rng::iterator_t<V>;
        static_assert(std::sentinel_for<S, I>);

        //static_assert(rng::view<V>);
        static_assert(rng::input_range<V>);
        static_assert(std::same_as<rng::range_reference_t<V>, int&>);
    }

    return true;
}
static_assert(test_view());

}

TEST_CASE("view")
{
    bool result = test_view();
    REQUIRE(result);
}