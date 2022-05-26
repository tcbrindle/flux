
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/op/fold.hpp>
#include <flux/op/filter.hpp>
#include <flux/op/map.hpp>
#include <flux/ranges/from_range.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

constexpr bool test_fold()
{
    {
        std::array arr{1, 2, 3, 4, 5};

        auto sum = flux::fold(arr, std::plus<>{});

        static_assert(std::same_as<decltype(sum), int>);

        STATIC_CHECK(sum == 15);
    }

    {
        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        auto prod = flux::from(arr)
                       .filter([](int i) { return i % 2 == 0; })
                       .map([](int i) { return i + i; })
                       .fold(std::multiplies<>{}, int64_t{1});

        static_assert(std::same_as<decltype(prod), int64_t>);

        STATIC_CHECK(prod == 122'880);
    }

    return true;
}
static_assert(test_fold());

}

TEST_CASE("fold")
{
    bool result = test_fold();
    REQUIRE(result);

    // Populate a vector in a really inefficient way
    {
        auto out = flux::fold(std::array{1, 2, 3, 4, 5}, [](auto vec, int val) {
            vec.push_back(val);
            return vec;
        }, std::vector<int>{});

        REQUIRE((out == std::vector{1, 2, 3, 4, 5}));
    }
}