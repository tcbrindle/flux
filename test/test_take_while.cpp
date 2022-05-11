
#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

namespace {

struct ints {
    int from = 0;

    static constexpr int first() { return 0; }
    static constexpr bool is_last(int) { return false; }
    constexpr int read_at(int idx) const { return from + idx; }
    static constexpr int& inc(int& idx, int o = 1) { return idx += o; }
    static constexpr int& dec(int& idx) { return --idx; }
    static constexpr int distance(int from, int to) { return to - from; }
};

constexpr bool test_take_while()
{
    {
        auto seq = flux::take_while(ints{10}, [](int i) { return i != 25; });

        using S = decltype(seq);

        static_assert(flux::lens<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

        STATIC_CHECK(check_equal(
            seq, {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}));
    }

    {
        auto seq = flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                        .take_while([](int i) { return i != 50; });

        using S = decltype(seq);

        static_assert(flux::lens<S>);
        static_assert(flux::contiguous_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

        STATIC_CHECK(check_equal(seq, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    // Check with mutable predicate
    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int count = 0;
        auto mutable_only = flux::take_while(arr, [count](int) mutable { return ++count <= 5; });

        using M = decltype(mutable_only);
        static_assert(flux::contiguous_sequence<M>);
        static_assert(not flux::sequence<M const>);

        STATIC_CHECK(check_equal(mutable_only, {0, 1, 2, 3, 4}));
    }

    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto idx = flux::from(arr)
                     .take_while([](int i) { return i < 5; })
                     .find(99);

        STATIC_CHECK(idx == flux::next(arr, flux::first(arr), 5));
    }

    {
        auto seq = flux::from(ints{})
                       .filter([](int i) { return i % 2 == 0; })
                       .take_while([](int i) { return i <= 10; })
                       .map([](int i) { return i * i; })
                       .drop(1);

        STATIC_CHECK(check_equal(seq, {4, 16, 36, 64, 100}));
    }

    return true;
}
static_assert(test_take_while());

}

TEST_CASE("take_while")
{
    bool result = test_take_while();
    REQUIRE(result);
}