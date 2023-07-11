
#include "catch.hpp"

#include <flux/op/equal.hpp>
#include <flux/op/take_while.hpp>
#include <flux/source/empty.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

struct S {
    int i;
};

class T {
    int i;
public:
    constexpr T(int i) : i(i) {}
    constexpr int get() const { return i; }
};

constexpr bool test_equal()
{
    // Basic equal
    {
        int arr1[] = {1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 3, 4, 5};

        STATIC_CHECK(flux::equal(arr1, arr2));
    }

    // Basic equal, same size but different elements
    {
        int arr1[] = {1, 2, 3, 4, 5};
        int arr2[] = {1, 2, 99, 4, 5};

        STATIC_CHECK(not flux::equal(arr1, arr2));
    }

    // Different but comparable element types
    {
        int arr1[] = {1, 2, 3, 4, 5};
        float arr2[] = {1.f, 2.f, 3.f, 4.f, 5.f};

        STATIC_CHECK(flux::equal(arr1, arr2));
    }

    // Differing lengths, both sized sequences
    {
        int arr1[] = {1, 2, 3, 4, 5};
        int arr2[] = {1};

        STATIC_CHECK(not flux::equal(arr1, arr2));
        STATIC_CHECK(not flux::equal(arr2, arr1));
    }

    // Differing lengths, not sized
    {
        auto yes = [](int) { return true; };
        auto seq1 = flux::take_while(std::array{1, 2, 3, 4, 5}, yes);
        auto seq2 = flux::take_while(std::array{1}, yes);

        static_assert(not flux::sized_sequence<decltype(seq1)>);

        STATIC_CHECK(not flux::equal(seq1, seq2));
        STATIC_CHECK(not flux::equal(seq2, seq1));
    }

    // Custom comparator
    {
        S arr1[] = {{1}, {2}, {3}, {4}, {5}};
        T arr2[] = {{1}, {2}, {3}, {4}, {5}};

        STATIC_CHECK(flux::equal(arr1, arr2, [](S const& s, T const& t) {
            return s.i ==t.get();
        }));
    }

    // Test with projections
    {
        S arr1[] = {{1}, {2}, {3}, {4}, {5}};
        T arr2[] = {{1}, {2}, {3}, {4}, {5}};

        STATIC_CHECK(flux::equal(arr1, arr2, flux::proj2(std::equal_to<>{}, &S::i, &T::get)));
    }

    // Two empty sequences compare equal if their element types are comparable
    {
        std::array<int, 0> seq1{};
        auto seq2 = flux::take_while(flux::ref(seq1), [](int) { return true; }); // not sized

        STATIC_CHECK(flux::equal(seq1, seq2));

        static_assert(flux::equal(flux::empty<int>, flux::empty<float>));
    }

    return true;
}
static_assert(test_equal());

}

TEST_CASE("equal")
{
    bool result = test_equal();
    REQUIRE(result);
}
