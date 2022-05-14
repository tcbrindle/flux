
#include "catch.hpp"

#include <flux/core/default_impls.hpp>
#include <flux/op/find.hpp>
#include <flux/op/from.hpp>
#include <flux/ranges/from_range.hpp>

#include <array>
#include <forward_list>
#include <vector>

namespace {

struct S {
    int i_;
};


using find_fn = decltype(flux::find);

static_assert(std::invocable<find_fn, int[10], float>);

// Incompatible value type
static_assert(not std::invocable<find_fn, int[10], S>);

// Not equality comparable
static_assert(not std::invocable<find_fn, S[10], S>);

// Okay
static_assert(std::invocable<find_fn, S[10], int, decltype(&S::i_)>);

// Non-callable projection
static_assert(not std::invocable<find_fn, int[10], int, int>);

constexpr bool test_find()
{
    {
        int const ints[] = {0, 1, 2, 3, 4, 5};

        auto cur = flux::find(ints, 3);
        if (cur != 3) {
            return false;
        }

        cur = flux::find(ints, 99);
        if (!flux::is_last(ints, cur)) {
            return false;
        }

        auto lens = flux::from(ints);

        cur = lens.find(3);
        if (cur != 3) {
            return false;
        }

        cur = lens.find(99);
        if (!lens.is_last(cur)) {
            return false;
        }
    }

    {
        S ss[] = { S{1}, S{2}, S{3}, S{4}, S{5} };

        auto cur = flux::find(ss, 3, &S::i_);
        if (cur != 2) {
            return false;
        }

        cur = flux::find(ss, 99, &S::i_);
        if (!flux::is_last(ss, cur)) {
            return false;
        }
    }

    return true;
}
static_assert(test_find());

}

TEST_CASE("find")
{
    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto iter = flux::find(vec, 3);
        REQUIRE(iter == vec.begin() + 2);
    }

    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto iter = flux::from(vec).find(99);
        REQUIRE(iter == vec.end());
    }

}
