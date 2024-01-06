
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include "test_utils.hpp"

namespace {

constexpr bool test_single()
{
    {
        auto s = flux::single(1.0f);

        using S = decltype(s);

        static_assert(flux::contiguous_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);

        static_assert(flux::contiguous_sequence<S const>);
        static_assert(flux::sized_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);

        static_assert(std::same_as<flux::element_t<S>, float&>);
        static_assert(std::same_as<flux::value_t<S>, float>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, float&&>);

        static_assert(std::same_as<flux::element_t<S const>, float const&>);
        static_assert(std::same_as<flux::value_t<S const>, float>);
        static_assert(std::same_as<flux::rvalue_element_t<S const>, float const&&>);

        static_assert(flux::size(s) == 1);
        static_assert(flux::size(std::as_const(s)) == 1);


    }

    {
        namespace rng = std::ranges;

        auto view = flux::single(1.0f);

        using V = decltype(view);

        static_assert(rng::contiguous_range<V>);
        static_assert(rng::common_range<V>);
        static_assert(rng::sized_range<V>);

        static_assert(rng::contiguous_range<V const>);
        static_assert(rng::common_range<V const>);
        static_assert(rng::sized_range<V const>);

        static_assert(rng::size(view) == 1);

        float sum = 0.0f;
        for (auto f : view) {
            sum += f;
        }

        STATIC_CHECK(sum == 1.0f);

        std::ranges::sort(view);


    }

    return true;
}
static_assert(test_single());

}


TEST_CASE("single")
{
    bool result = test_single();
    REQUIRE(result);
}