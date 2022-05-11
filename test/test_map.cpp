
#include "catch.hpp"

#include <flux.hpp>
#include <flux/ranges/from_range.hpp>

#include "test_utils.hpp"

namespace {

constexpr bool test_map()
{
    {
        int arr[] = {0, 1, 2, 3, 4};

        auto mapped = flux::map(arr, [](int& i) { return i * 2; });

        using M = decltype(mapped);

        static_assert(flux::random_access_sequence<M>);
        static_assert(not flux::contiguous_sequence<M>);
        static_assert(flux::bounded_sequence<M>);
        static_assert(flux::sized_sequence<M>);

        static_assert(std::same_as<int, flux::element_t<M>>);
        static_assert(std::same_as<int, flux::rvalue_element_t<M>>);

        static_assert(flux::random_access_sequence<M const>);
        static_assert(not flux::contiguous_sequence<M const>);
        static_assert(flux::bounded_sequence<M const>);
        static_assert(flux::sized_sequence<M const>);

        STATIC_CHECK(check_equal(mapped, {0, 2, 4, 6, 8}));
        STATIC_CHECK(mapped.size() == 5);
    }

    {
        auto mapped = flux::map(std::array{0, 1, 2, 3, 4}, [](int const& i) -> int const& { return i; });

        using M = decltype(mapped);

        static_assert(flux::random_access_sequence<M>);
        static_assert(not flux::contiguous_sequence<M>);
        static_assert(flux::bounded_sequence<M>);
        static_assert(flux::sized_sequence<M>);

        static_assert(std::same_as<int const&, flux::element_t<M>>);
        static_assert(std::same_as<int const&&, flux::rvalue_element_t<M>>);
        static_assert(std::same_as<int, flux::value_t<M>>);

        static_assert(flux::random_access_sequence<M const>);
        static_assert(not flux::contiguous_sequence<M const>);
        static_assert(flux::bounded_sequence<M const>);
        static_assert(flux::sized_sequence<M const>);

        STATIC_CHECK(check_equal(mapped, {0, 1, 2, 3, 4}));
        STATIC_CHECK(mapped.size() == 5);
    }

    {
        int arr[] = {0, 1, 2, 3, 4};

        auto idx = flux::from(arr).map([](int i) { return i * 2; }).find(4);

        STATIC_CHECK(idx == 2);
    }

    {
        std::pair<int, int> pairs[] = { {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}};

        auto mapped = flux::from(pairs).map(&std::pair<int, int>::first);

        using M = decltype(mapped);
        static_assert(flux::random_access_sequence<M>);
        static_assert(std::same_as<int&, flux::element_t<M>>);
        static_assert(std::same_as<int&&, flux::rvalue_element_t<M>>);
        static_assert(std::same_as<int, flux::value_t<M>>);

        STATIC_CHECK(check_equal(mapped, {0, 1, 2, 3, 4}));
    }

    {
        constexpr auto seq = flux::from(std::array{0, 1, 2, 3, 4})
                                 .map([](int i) { return i * 2; })
                                 .reverse()
                                 .take(3);

        static_assert(flux::random_access_sequence<decltype(seq)>);
        static_assert(flux::size(seq) == 3);
        static_assert(check_equal(seq, {8, 6, 4}));
    }

    {
        auto times_two = [](int i) { return i * 2; };

        int arr[] = {0, 1, 2, 3, 4};

        auto view = flux::map(arr, times_two).view();

        using V = decltype(view);

        static_assert(std::ranges::view<V>);
        static_assert(std::ranges::random_access_range<V>);
        static_assert(not std::ranges::contiguous_range<V>);
        static_assert(std::ranges::common_range<V>);
        static_assert(std::ranges::sized_range<V>);

        STATIC_CHECK(std::ranges::equal(view | std::views::transform(times_two),
                                        std::array{0, 4, 8, 12, 16}));
    }

    return true;
}
static_assert(test_map());

}

TEST_CASE("map")
{
    bool result = test_map();
    REQUIRE(result);
}