
#include <array>

#include "test_utils.hpp"

namespace {

constexpr bool test_cache_last()
{
    // cache_last turns an unbounded sequence into a bounded one
    {
        std::array arr{1, 2, 3, 4, 5};
        auto seq = flux::take_while(flux::ref(arr), [](int){ return true; });

        static_assert(not flux::bounded_sequence<decltype(seq)>);

        auto cached = flux::cache_last(std::move(seq));

        using C = decltype(cached);

        static_assert(flux::sequence<C>);
        static_assert(flux::random_access_sequence<C>);
        static_assert(flux::bounded_sequence<C>);

        STATIC_CHECK(cached.count() == 5);

        static_assert(std::ranges::common_range<C>);
    }

    // For a bounded sequence, cache_last is equivalent to flux::from
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr);
        auto cached = flux::cache_last(flux::ref(arr));

        STATIC_CHECK(&seq.base() == &cached.base());
        STATIC_CHECK(&cached.base() == &arr);
    }

    // Example from docs
    {
        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        flux::mut_ref(arr)
            .take_while([](int i) { return i <= 5; })
            .cache_last()
            .inplace_reverse();

        STATIC_CHECK(check_equal(arr, {5, 4, 3, 2, 1, 6, 7, 8, 9, 10}));
    }

    return true;
}
static_assert(test_cache_last());

}

TEST_CASE("cache_last")
{
    bool result = test_cache_last();
    REQUIRE(result);
}
