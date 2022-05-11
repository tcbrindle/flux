
#include <flux/op/all_any_none.hpp>

#include <flux/ranges/from_range.hpp>

#include <flux/op/write_to.hpp>

#include <iostream>

namespace {

constexpr auto gt_zero = [](auto i) { return i > 0; };

template <typename T>
constexpr bool test_all(std::initializer_list<T> ilist)
{
    return flux::all(ilist, gt_zero) == std::all_of(ilist.begin(), ilist.end(), gt_zero);
}
static_assert(test_all<int>({}));
static_assert(test_all({1, 2, 3, 4, 5}));
static_assert(test_all({1.0, 2.0, -3.0, 4.0}));

}

TEST_CASE("all with vector")
{
    std::vector<int> vec{1, 2, 3, 4, 5};

    static_assert(flux::contiguous_sequence<decltype(vec)>);

    bool req = flux::all(vec, gt_zero);

    REQUIRE(req);
}

TEST_CASE("write to??")
{
    /*int arr[][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };*/
    std::vector<std::vector<int>> arr = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };


    flux::write_to(arr, std::cout) << std::endl;
}