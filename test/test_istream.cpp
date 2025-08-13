
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>

#include "test_utils.hpp"

TEST_CASE("istream")
{
    {
        std::istringstream iss{"0 1 2 3 4"};

        auto seq = flux::from_istream<int>(iss);

        using S = decltype(seq);

        static_assert(flux::iterable<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);

        static_assert(std::same_as<flux::iterable_element_t<S>, int const&>);
        static_assert(std::same_as<flux::iterable_value_t<S>, int>);

        REQUIRE(check_equal(seq, {0, 1, 2, 3, 4}));
    }

    {
        std::istringstream iss{"0 1 2 3 4 5 6 7 8     9 10"};

        auto seq = flux::from_istream<int>(iss)
                      .filter([](int i) { return i >= 5; })
                      .map([](int i) { return i * 2; })
                      .take(3);

        static_assert(flux::iterable<decltype(seq)>);

        REQUIRE(check_equal(seq, {10, 12, 14}));
    }

    SUBCASE("take(5) with istream")
    {
        std::istringstream iss("1 2 3 4 5 6 7 8 9 10");

        auto it = flux::from_istream<int>(iss).take(5);

        REQUIRE(check_equal(it, {1, 2, 3, 4, 5}));

        int i = 0;
        iss >> i;
        REQUIRE(i == 6);
    }
}