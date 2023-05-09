
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include <algorithm>
#include <array>
#include <numeric>

#include "test_utils.hpp"

namespace {

template <bool = true>
constexpr bool test_range_iface()
{
    namespace rng = std::ranges;

    {
        auto seq = flux::map(std::array{1, 2, 3, 4, 5}, std::identity{});

        using R = decltype(seq);

        static_assert(rng::random_access_range<R>);
        static_assert(rng::sized_range<R>);
        static_assert(rng::common_range<R>);

        static_assert(std::same_as<rng::range_reference_t<R>, int&>);
        static_assert(std::same_as<rng::range_difference_t<R>, flux::distance_t>);
        static_assert(std::same_as<rng::range_value_t<R>, int>);
        static_assert(std::same_as<rng::range_rvalue_reference_t<R>, int&&>);
        static_assert(std::same_as<rng::range_size_t<R>, flux::distance_t>);

        static_assert(rng::random_access_range<R const>);
        static_assert(rng::sized_range<R const>);
        static_assert(rng::common_range<R const>);

        static_assert(std::same_as<rng::range_reference_t<R const>, int const&>);
        static_assert(std::same_as<rng::range_difference_t<R const>, flux::distance_t>);
        static_assert(std::same_as<rng::range_value_t<R const>, int>);
        static_assert(std::same_as<rng::range_rvalue_reference_t<R const>, int const&&>);
        static_assert(std::same_as<rng::range_size_t<R const>, flux::distance_t>);

        STATIC_CHECK(std::ranges::equal(seq, std::array{1, 2, 3, 4, 5}));

        STATIC_CHECK(std::ranges::equal(std::as_const(seq), std::array{1, 2, 3, 4, 5}));

#if defined(__cpp_lib_three_way_comparison)
#if __cpp_lib_three_way_comparison >= 201907L
        {
            constexpr auto check = std::array{1, 2, 3, 4, 5};
            auto res = std::lexicographical_compare_three_way(
                seq.begin(), seq.end(), check.begin(), check.end());
            STATIC_CHECK(std::is_eq(res));
        }
#endif
#endif
    }


    {
        std::array arr{5, 4, 3, 2, 1};

        using I = decltype(flux::begin(arr));
        using CI = decltype(flux::begin(std::as_const(arr)));

        static_assert(std::contiguous_iterator<I>);
        static_assert(std::sized_sentinel_for<I, I>);

        static_assert(std::contiguous_iterator<CI>);
        static_assert(std::sized_sentinel_for<CI, CI>);

        // Check we can initialise a const_iter from non-const
        CI iter = flux::begin(arr);

        STATIC_CHECK(std::accumulate(iter, flux::end(std::as_const(arr)), 0) == 15);

        {
            auto arr2 = arr;
            auto sub = rng::subrange(flux::begin(arr2), flux::end(arr2));

            STATIC_CHECK(sub.size() == arr2.size());
            STATIC_CHECK(sub.data() == arr2.data());

            rng::sort(sub);
            STATIC_CHECK(rng::equal(sub, std::array{1, 2, 3, 4, 5}));
        }

        {
            auto arr2 = arr;
            auto sub = std::ranges::subrange(flux::begin(arr2), std::default_sentinel);

            STATIC_CHECK(rng::distance(sub) == flux::size(arr));
            STATIC_CHECK(sub.data() == arr2.data());

            rng::sort(sub);
            STATIC_CHECK(rng::equal(sub, std::array{1, 2, 3, 4, 5}));
        }
    }

    {
        int arr[] = {1, 2, 3, 4, 5};
        auto seq = single_pass_only(flux::from(arr));

        using V = decltype(seq);

        using S = decltype(seq.end());
        using I = rng::iterator_t<V>;
        static_assert(std::sentinel_for<S, I>);

        static_assert(rng::input_range<V>);
        static_assert(not rng::forward_range<V>);
        static_assert(std::same_as<rng::range_reference_t<V>, int&>);

        STATIC_CHECK(rng::equal(seq, arr));
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

    return true;
}
static_assert(test_range_iface());

}

TEST_CASE("range interface")
{
    bool res = test_range_iface<false>();
    REQUIRE(res);
}
