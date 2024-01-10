
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <algorithm>
#include <numeric>

#include "test_utils.hpp"

namespace {

struct Base {
    int i = 999;
};

struct Derived : Base {
    int j = 1;
};

struct ConvertibleToBaseStar {
    operator Base*() const { return nullptr; }
};

struct Abstract {
    virtual void fn() = 0;
};

template <typename T>
concept can_array_ptr = requires { typename flux::array_ptr<T>; };

// We can't even mention the type array_ptr<T> for abstract, incomplete or non-object T
static_assert(not can_array_ptr<Abstract>);
static_assert(not can_array_ptr<void>);
static_assert(not can_array_ptr<int&>);

constexpr bool test_array_ptr_ctor()
{
    // Default constructor
    {
        flux::array_ptr<int> ptr{};

        STATIC_CHECK(ptr.data() == nullptr);
        STATIC_CHECK(ptr.size() == 0);
    }

    // From-sequence constructor, basic
    {
        std::array arr{1, 2, 3, 4, 5};
        flux::array_ptr<int> ptr(arr);

        STATIC_CHECK(ptr.usize() == arr.size());
        STATIC_CHECK(ptr.data() == arr.data());
    }

    // From-sequence constructor, non-const -> const
    {
        int arr[] = {1, 2, 3, 4, 5};
        flux::array_ptr<int const> ptr(arr);

        STATIC_CHECK(ptr.size() == 5);
        STATIC_CHECK(ptr.data() == arr);

        // constructor is explicit
        auto fn = [](flux::array_ptr<int>) {};
        (void) fn(flux::array_ptr<int>{arr});
        static_assert(not std::invocable<decltype(fn), std::array<int, 3>&>);
    }

    // empty sequences are handled correctly
    {
        auto const& e = flux::empty<int>;
        flux::array_ptr<int> ptr(e);

        STATIC_CHECK(ptr.size() == 0);
        STATIC_CHECK(ptr.is_empty());
        STATIC_CHECK(ptr.data() == nullptr);
    }

    // Cannot construct array_ptr<int> from sequence of const int&
    static_assert(not std::constructible_from<flux::array_ptr<int>, std::array<int, 3> const&>);

    // Cannot construct array_ptr<Base> from sequence of Derived&
    static_assert(std::constructible_from<Base*, Derived*>);
    static_assert(not std::constructible_from<flux::array_ptr<Base>, std::array<Derived, 3>&>);

    // Weird conversion operators are correctly ignored
    static_assert(std::constructible_from<Base*, ConvertibleToBaseStar&>);
    static_assert(not std::constructible_from<flux::array_ptr<Base>, std::array<ConvertibleToBaseStar, 3>&>);

    // We are trivially copyable, movable, *-assignable and destructible
    {
        using AP = flux::array_ptr<int const>;
        std::array arr{1, 2, 3};
        AP p1(arr);

        auto p2 = p1;
        STATIC_CHECK(p2.usize() == arr.size());
        STATIC_CHECK(p2.data() == arr.data());
        static_assert(std::is_trivially_copy_constructible_v<AP>);
        static_assert(std::is_nothrow_copy_constructible_v<AP>);

        auto p3 = std::move(p1);
        STATIC_CHECK(p3.usize() == arr.size());
        STATIC_CHECK(p3.data() == arr.data());
        static_assert(std::is_trivially_move_constructible_v<AP>);
        static_assert(std::is_nothrow_move_constructible_v<AP>);

        int arr2[] = {1, 2, 3, 4, 5};
        auto p4 = flux::array_ptr<int>(arr2);

        p2 = p4;
        STATIC_CHECK(p2.size() == 5);
        STATIC_CHECK(p2.data() == arr2);
        static_assert(std::is_trivially_copy_assignable_v<AP>);
        static_assert(std::is_nothrow_copy_assignable_v<AP>);


        p2 = std::move(p4);
        STATIC_CHECK(p2.size() == 5);
        STATIC_CHECK(p2.data() == arr2);
        static_assert(std::is_trivially_move_assignable_v<AP>);
        static_assert(std::is_nothrow_move_assignable_v<AP>);

        static_assert(std::is_trivially_destructible_v<AP>);
    }

    // Converting constructor
    {
        std::array arr{1, 2, 3};
        flux::array_ptr<int> ptr(arr);

        flux::array_ptr<int const> cptr = ptr;
        STATIC_CHECK(cptr.usize() == arr.size());
        STATIC_CHECK(cptr.data() == arr.data());

        static_assert(std::is_nothrow_convertible_v<flux::array_ptr<int>, flux::array_ptr<int const>>);

        flux::array_ptr<int const volatile> cvptr = cptr;
        STATIC_CHECK(cvptr.usize() == arr.size());
        STATIC_CHECK(cvptr.data() == arr.data());

        auto fn = [](flux::array_ptr<double const>) {};
        static_assert(std::invocable<decltype(fn), flux::array_ptr<double>&>);
    }

    // Cannot lessen CV qualifiers or do derived->base conversion
    static_assert(not std::is_convertible_v<flux::array_ptr<int const>, flux::array_ptr<int>>);
    static_assert(not std::is_convertible_v<flux::array_ptr<int const volatile>, flux::array_ptr<int const>>);
    static_assert(not std::is_convertible_v<flux::array_ptr<Derived>, flux::array_ptr<Base>>);
    static_assert(not std::is_convertible_v<flux::array_ptr<Base>, flux::array_ptr<Derived>>);

    return true;
}
static_assert(test_array_ptr_ctor());

constexpr bool test_array_ptr_ctad()
{
    {
        std::array<int, 3> arr{1, 2, 3};

        auto ap = flux::array_ptr(arr);
        static_assert(std::same_as<flux::array_ptr<int>, decltype(ap)>);

        auto ap2 = flux::array_ptr(std::as_const(arr));
        static_assert(std::same_as<flux::array_ptr<int const>, decltype(ap2)>);
    }

    {
        int volatile arr[] = {1, 2, 3};

        auto ap = flux::array_ptr(arr);
        static_assert(std::same_as<flux::array_ptr<int volatile>, decltype(ap)>);

        auto ap2 = flux::array_ptr(std::as_const(arr));
        static_assert(std::same_as<flux::array_ptr<int volatile const>, decltype(ap2)>);
    }

    return true;
}
static_assert(test_array_ptr_ctad());

// Controversial, this one
constexpr bool test_array_ptr_equality()
{
    // Pointers to different arrays are different
    {
        int arr[] = {1, 2, 3};
        int arr2[] = {1, 2, 3};

        auto ap = flux::array_ptr(arr);
        auto ap2 = flux::array_ptr(arr2);

        STATIC_CHECK(ap == ap);
        STATIC_CHECK(not (ap != ap));
        STATIC_CHECK(not (ap == ap2));
        STATIC_CHECK(ap != ap2);
    }

    // Pointers to the same array with different sizes are different
    {
        int arr[] = {1, 2, 3};

        auto ap = flux::make_array_ptr_unchecked(arr, 3);
        auto ap2 = flux::make_array_ptr_unchecked(arr, 2);

        STATIC_CHECK(ap == ap);
        STATIC_CHECK(not (ap != ap));
        STATIC_CHECK(not (ap == ap2));
        STATIC_CHECK(ap != ap2);

        std::array arr2{1, 2, 3};
        auto take2 = flux::take(arr2, 2);

        STATIC_CHECK(flux::array_ptr(arr2) != flux::array_ptr(take2));
    }

    // Default-constructed ptrs (of the same type) are equal
    {
        flux::array_ptr<int> arr1;
        flux::array_ptr<int> arr2;

        STATIC_CHECK(arr1 == arr2);
    }

    // Can use the converting constructor to compare const and non-const
    {
        int arr[] = {1, 2, 3};
        int const arr2[] = {4, 5, 6};

        STATIC_CHECK(flux::array_ptr<int>(arr) == flux::array_ptr<int const>(arr));
        STATIC_CHECK(flux::array_ptr(arr) != flux::array_ptr(arr2));
    }

    return true;
}
static_assert(test_array_ptr_equality());

constexpr bool test_array_ptr_sequence_impl()
{
    {
        using S = flux::array_ptr<int>;
        static_assert(flux::contiguous_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(not flux::infinite_sequence<S>);
        static_assert(std::same_as<flux::value_t<S>, int>);
        static_assert(std::same_as<flux::element_t<S>, int&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

        static_assert(flux::contiguous_sequence<S const>);
        static_assert(flux::sized_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(not flux::infinite_sequence<S const>);
        static_assert(std::same_as<flux::value_t<S const>, int>);
        static_assert(std::same_as<flux::element_t<S const>, int&>);
        static_assert(std::same_as<flux::rvalue_element_t<S const>, int&&>);

        using C = flux::array_ptr<int const>;
        static_assert(flux::contiguous_sequence<C>);
        static_assert(flux::sized_sequence<C>);
        static_assert(flux::bounded_sequence<C>);
        static_assert(not flux::infinite_sequence<C>);
        static_assert(std::same_as<flux::value_t<C>, int>);
        static_assert(std::same_as<flux::element_t<C>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<C>, int const&&>);

        static_assert(flux::contiguous_sequence<C const>);
        static_assert(flux::sized_sequence<C const>);
        static_assert(flux::bounded_sequence<C const>);
        static_assert(not flux::infinite_sequence<C const>);
        static_assert(std::same_as<flux::value_t<C const>, int>);
        static_assert(std::same_as<flux::element_t<C const>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<C const>, int const&&>);
    }

    auto do_sum = []<typename T>(flux::array_ptr<T> arr) {
        flux::value_t<decltype(arr)> sum{};
        for (auto c = arr.first(); !arr.is_last(c); arr.inc(c)) {
            sum += arr[c];
        }
        return sum;
    };

    // Basic iteration works okay
    {
        std::array arr{1, 2, 3};

        STATIC_CHECK(do_sum(flux::array_ptr(arr)) == 6);
        STATIC_CHECK(do_sum(flux::array_ptr(std::as_const(arr))) == 6);
    }

    // We can mutate via an array_ptr (even one declared const)
    {
        std::array arr{1, 2, 3};
        auto const ptr = flux::array_ptr(arr);

        flux::fill(ptr, 9);

        STATIC_CHECK(check_equal(arr, {9, 9, 9}));
    }

    // ...but not an array_ptr to const
    static_assert(not std::invocable<decltype(flux::fill), flux::array_ptr<int const>&>);

    // We can sort an array ptr, exercising most of the random-access interface
    {
        std::array<int, 100> arr{};
        std::iota(arr.begin(), arr.end(), 0);
        std::ranges::reverse(arr);

        STATIC_CHECK(not std::ranges::is_sorted(arr));

        auto const ptr = flux::array_ptr(arr);
        flux::sort(ptr);

        STATIC_CHECK(std::ranges::is_sorted(arr));
    }

    // Internal iteration works as expected
    {
        std::array const arr{0, 1, 2, 3, 4};
        auto ptr = flux::array_ptr(arr);

        STATIC_CHECK(flux::sum(ptr) == 10);

        auto idx = ptr.find(3);
        STATIC_CHECK(idx == 3);
    }

    // Range interface works as expected
    {
        std::array arr{5, 4, 3, 2, 1};
        auto ptr = flux::array_ptr(arr);
        std::ranges::sort(ptr);
        static_assert(std::ranges::contiguous_range<flux::array_ptr<int>>);
        STATIC_CHECK(std::ranges::is_sorted(arr));
    }

    return true;
}
static_assert(test_array_ptr_sequence_impl());

constexpr bool test_make_array_ptr()
{
    {
        int* p = nullptr;
        auto arr = flux::make_array_ptr_unchecked(p, 0);

        STATIC_CHECK(arr.size() == 0);
        STATIC_CHECK(arr.data() == nullptr);
    }

    {
        int arr[] = {5, 4, 3, 2, 1};
        auto ap = flux::make_array_ptr_unchecked(arr, 4);
        STATIC_CHECK(ap.size() == 4);
        STATIC_CHECK(ap.data() == arr);

        std::ranges::sort(ap);

        STATIC_CHECK(check_equal(arr, {2, 3, 4, 5, 1}));
    }

    return true;
}
static_assert(test_make_array_ptr());

}

TEST_CASE("array_ptr")
{
    REQUIRE(test_array_ptr_ctor());
    REQUIRE(test_array_ptr_ctad());
    REQUIRE(test_array_ptr_equality());
    REQUIRE(test_array_ptr_sequence_impl());
    REQUIRE(test_make_array_ptr());

    SECTION("bounds checking") {
        int arr[] = {0, 1, 2};
        auto ptr = flux::array_ptr(arr);

        // In-bounds reads are okay
        REQUIRE(ptr[0] == 0);

        // Out-of-bounds reads are an error
        REQUIRE_THROWS_AS(ptr[-1], flux::unrecoverable_error);
        REQUIRE_THROWS_AS(ptr[100], flux::unrecoverable_error);

        // Advancing a cursor to the end is okay...
        auto cur = ptr.first();
        ptr.inc(cur, ptr.size());

        // ...but reading from it is an error...
        REQUIRE_THROWS_AS(ptr[cur], flux::unrecoverable_error);
    }
}