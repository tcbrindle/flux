// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>

namespace {

template <typename T, std::size_t N>
struct array_iterator : flux::simple_sequence_base<array_iterator<T, N>> {
private:
    std::array<T, N>* array_;
    std::size_t idx_ = 0;

public:
    constexpr explicit array_iterator(std::array<T, N>& arr)
        : array_(std::addressof(arr))
    {}

    constexpr auto maybe_next() -> T*
    {
        if (idx_ < N) {
            return &array_->at(idx_++);
        } else {
            return nullptr;
        }
    }
};

struct ints : flux::simple_sequence_base<ints> {
    int i = 0;

    static constexpr bool is_infinite = true;

    constexpr auto maybe_next() -> std::optional<int>
    {
        return {i++};
    }
};

constexpr bool test_simple_sequence()
{
    {
        std::array<int, 5> arr{1, 2, 3, 4, 5};
        auto iter = array_iterator(arr);

        using I = decltype(iter);
        static_assert(flux::detail::simple_sequence<I>);
        static_assert(flux::sequence<I>);
        static_assert(not flux::multipass_sequence<I>);
        static_assert(not flux::sized_sequence<I>);
        static_assert(not flux::infinite_sequence<I>);

        static_assert(std::same_as<flux::element_t<I>, int&>);
        static_assert(std::same_as<flux::value_t<I>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<I>, int&&>);

        iter.fill(10);

        STATIC_CHECK(check_equal(array_iterator(arr), {10, 10, 10, 10, 10}));
    }

    {
        static_assert(flux::detail::simple_sequence<ints>);
        static_assert(flux::sequence<ints>);
        static_assert(flux::infinite_sequence<ints>);
        static_assert(not flux::multipass_sequence<ints>);
        static_assert(not flux::sized_sequence<ints>);
        static_assert(std::same_as<flux::element_t<ints>, int const&>);
        static_assert(std::same_as<flux::value_t<ints>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<ints>, int const&&>);

        // FIXME: ints{}.take(10).sum()
        int sum = 0;
        ints{}.take(10).for_each([&sum](int const& i) {
            sum += i;
        });

        STATIC_CHECK(sum == 45);
    }

    // Check that we can restart iteration
    {
        std::array<int, 5> arr{1, 2, 3, 4, 5};
        auto iter = array_iterator(arr);

        auto cur = iter.find(3);
        auto slice = flux::slice(iter, std::move(cur), flux::last);

        STATIC_CHECK(check_equal(slice, {3, 4, 5}));
    }

    return true;
}
static_assert(test_simple_sequence());

}

TEST_CASE("simple_sequence")
{
    bool result = test_simple_sequence();
    REQUIRE(result);
}