
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

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
        std::vector<double> v = {0.25, 0.75};
        auto r = flux::fold(v, std::plus(), 1);

        static_assert(std::same_as<decltype(r), double>);

        STATIC_CHECK(r == 2.0);
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

constexpr bool test_fold_first()
{
    {
        int arr[] = {1, 2, 3, 4, 5};

        auto sum = flux::fold_first(arr, std::plus{});

        STATIC_CHECK(sum.value() == 15);
    }

    // fold_first over an empty sequence produces no value
    {
        auto opt = flux::fold_first(flux::empty<double>, std::plus{});
        STATIC_CHECK(not opt.has_value());
    }

    {
        auto min_fn = [](auto seq) {
            return seq.fold_first([](int so_far, int elem) {
                return elem < so_far ? elem : so_far;
            });
        };

        int arr[] = {5, 4, 1, 3, -1};

        auto min = min_fn(flux::ref(arr));

        STATIC_CHECK(min.value() == -1);
    }

    return true;
}
static_assert(test_fold_first());

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