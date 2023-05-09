
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

template <bool = true>
constexpr bool test_unchecked()
{
    using namespace flux;

    {
        auto seq = unchecked(std::array{5, 4, 3, 2, 1});

        using S = decltype(seq);

        static_assert(contiguous_sequence<S>);
        static_assert(sized_sequence<S>);
        static_assert(bounded_sequence<S>);

        seq.sort();

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
    }

    {
        auto ints = std::array{5, 4, 3, 2, 1};
        auto doubles = std::array{3.0, 2.0, 1.0};

        auto seq = unchecked(zip(ref(ints), ref(doubles)));

        using S = decltype(seq);

        static_assert(random_access_sequence<S>);
        static_assert(bounded_sequence<S>);
        static_assert(sized_sequence<S>);
        static_assert(std::same_as<value_t<S>, std::pair<int, double>>);
        static_assert(std::same_as<element_t<S>, std::pair<int&, double&>>);
        static_assert(std::same_as<rvalue_element_t<S>, std::pair<int&&, double&&>>);

        seq.sort(flux::proj(std::less<>{}, [](auto p) { return p.second; }));

        STATIC_CHECK(check_equal(doubles, {1.0, 2.0, 3.0}));
        STATIC_CHECK(check_equal(ints, {3, 4, 5, 2, 1}));

    }

    return true;
}
static_assert(test_unchecked());


}

TEST_CASE("unchecked adaptor")
{
    auto res = test_unchecked<false>();
    REQUIRE(res);
}
