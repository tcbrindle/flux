
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <algorithm>
#include <array>
#include <string_view>

#include "test_utils.hpp"

namespace {

constexpr bool test_zip_for_each()
{
    // two sequences with stateful function obj
    {
        int arr1[] = {1, 2, 3, 4, 5};
        std::array const arr2 = {100.0, 200.0, 300.0};

        struct counter {
            int int_sum = 0;
            double double_sum = 0.0;

            constexpr auto operator()(int i, double d) {
                int_sum += i;
                double_sum += d;
            }
        };

        auto c = flux::zip_for_each(counter{}, arr1, arr2);

        // Make sure we're getting the right type back out
        static_assert(std::same_as<counter, decltype(c)>);

        STATIC_CHECK(c.int_sum == 1 + 2 + 3);
        STATIC_CHECK(c.double_sum == 100.0 + 200.0 + 300.0);
    }

    // zip_for_each with no sequences never calls the fn
    {
        bool called = false;
        flux::zip_for_each([&] { called = true; });

        STATIC_CHECK(called == false);
    }

    return true;
}
static_assert(test_zip_for_each());

constexpr bool test_zip_find_if()
{
    // successful, two sequences
    {
        int arr1[] = {1, 2, 3, 4, 5};
        std::array const arr2{5, 4, 3, 2, 1};

        auto [idx1, idx2] = flux::zip_find_if(std::equal_to{}, arr1, arr2);

        STATIC_CHECK(idx1 == 2);
        STATIC_CHECK(idx2 == 2);
    }

    // unsuccessful, two sequences
    {
        int arr1[] = {1, 2, 3, 4, 5};
        std::array const arr2{100.0, 200.0, 300.0};

        auto [idx1, idx2] = flux::zip_find_if([](int i, double d) {
            return i == 1 && d < 100;
        }, arr1, arr2);

        STATIC_CHECK(idx1 == 3); // We didn't exhaust sequence 1
        STATIC_CHECK(idx2 == 3);
        STATIC_CHECK(flux::is_last(arr2, idx2));
    }

    // successful, one sequence, equivalent to find_if
    {
        int arr[] = {1, 2, 3, 4, 5};

        auto [idx] = flux::zip_find_if(flux::pred::eq(3), arr);

        STATIC_CHECK(idx == 2);
    }

    // unsuccessful, one sequence
    {
        int arr[] = {1, 2, 3, 4, 5};

        auto [idx] = flux::zip_find_if(flux::pred::gt(10), arr);

        STATIC_CHECK(flux::is_last(arr, idx));
    }

    // zero sequences (just make sure it compiles)
    {
        auto r = flux::zip_find_if([] { return true; });
        STATIC_CHECK(r == std::tuple<>{});
    }

    // Check we can use zip_find_if to implement the STL's adjacent_find
    {
        int const arr[] = {1, 2, 3, 3, 4, 5, 6};

        auto adjacent_find = [](flux::sequence auto&& seq) {
            auto [a, b] = flux::zip_find_if(std::equal_to{}, seq, flux::ref(seq).drop(1));
            if (flux::is_last(seq, b)) {
                return b;
            } else {
                return a;
            }
        };

        auto r = adjacent_find(arr);

        STATIC_CHECK(r == 2);
    }

    // Check we can use zip_find_if to implement the STL's mismatch
    {
        int const arr1[] = {1, 2, 3, 4, 5};
        std::array arr2 = {1, 2, 3, 5, 4};

        auto [iter1, iter2] = std::ranges::mismatch(arr1, arr2);

        auto [cur1, cur2] = flux::zip_find_if(std::not_equal_to{}, arr1, arr2);

        STATIC_CHECK(*iter1 == arr1[cur1]);
        STATIC_CHECK(*iter2 == arr2[size_t(cur2)]);

    }

    return true;
}
static_assert(test_zip_find_if());

constexpr bool test_zip_fold()
{
    // Summing two sequences at the same time
    {
        int arr1[] = {1, 2, 3, 4, 5};
        std::array const arr2 = {100.0, 200.0, 300.0};

        struct counter {
            int int_sum;
            double double_sum;
        };

        auto r = flux::zip_fold([](counter c, int i, double d) {
            c.int_sum += i;
            c.double_sum += d;
            return c;
        }, counter{}, arr1, arr2);

        static_assert(std::same_as<counter, decltype(r)>);

        STATIC_CHECK(r.int_sum == 6);
        STATIC_CHECK(r.double_sum == 600.0);
    }

    // We can implement something like an adjacent_fold
    {
        int const arr[] = {1, 2, 3, 4, 5};

        auto plus = [](auto... a) { return (a + ...); };

        auto sum = flux::zip_fold(plus, 0, arr, flux::ref(arr).drop(1), flux::ref(arr).drop(2));

        STATIC_CHECK(sum == (1 + 2 + 3) + (2 + 3 + 4) + (3 + 4 + 5));
    }

    return true;
}
static_assert(test_zip_fold());

}

TEST_CASE("zip algorithms")
{
    bool r = test_zip_for_each();
    REQUIRE(r);

    r = test_zip_find_if();
    REQUIRE(r);

    r = test_zip_fold();
    REQUIRE(r);
}