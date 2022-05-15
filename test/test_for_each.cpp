
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/op/for_each.hpp>
#include <flux/ranges.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

struct S {
    constexpr void p() const { *p_ += i_; }

    int *p_;
    int i_;
};

constexpr bool test_for_each()
{
    {
        int sum = 0;
        auto fun = [&](int i) { sum += i; };
        auto arr = std::array{0, 2, 4, 6};

        flux::for_each(arr, fun);

        STATIC_CHECK(sum == 12);
    }

    {
        int sum = 0;
        auto fun = [&](int& i) { sum += i; };
        auto arr = std::array{0, 2, 4, 6};

        flux::from(arr).for_each(fun);

        STATIC_CHECK(sum == 12);
    }

    {
        int sum = 0;
        auto arr = std::array<S, 4>{ S{&sum, 0}, S{&sum, 2}, S{&sum, 4}, S{&sum, 6}};

        flux::for_each(arr, &S::p);

        STATIC_CHECK(sum == 12);
    }

    {
        int sum = 0;
        auto arr = std::array<S, 4>{ S{&sum, 0}, S{&sum, 2}, S{&sum, 4}, S{&sum, 6}};

        flux::for_each(arr, [&sum](int i) { sum += i; },  &S::i_);

        STATIC_CHECK(sum == 12);
    }

    {
        struct counter {
            constexpr void operator()(int i) { sum += i; }
            int sum = 0;
        };

        auto ilist = {0, 2, 4, 6};
        auto result = flux::for_each(ilist, counter{});

        STATIC_CHECK(result.sum == 12);
    }

    return true;
}
static_assert(test_for_each());

}

TEST_CASE("for_each")
{
    bool result = test_for_each();
    REQUIRE(result);
}