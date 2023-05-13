
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>
#include <utility>

namespace {

constexpr bool test_set_union()
{
    {
        int arr1[] = {0, 2, 4, 6};
        int arr2[] = {1, 3, 5};
        auto union_seq = flux::set_union(flux::ref(arr1), flux::ref(arr2));

        using T = decltype(union_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(union_seq, {0, 1, 2, 3, 4, 5, 6}));
    }

    // Const iteration works as expected
    {
        std::array arr1{0, 2, 4};
        std::array arr2{1, 3, 5};
        auto union_seq = flux::set_union(flux::ref(arr1), flux::ref(arr2));

        using Seq = decltype(union_seq);

        static_assert(flux::sequence<Seq>);
        static_assert(flux::multipass_sequence<Seq>);
        static_assert(not flux::sized_sequence<Seq>);

        static_assert(std::same_as<flux::element_t<Seq>, int&>);
        static_assert(std::same_as<flux::value_t<Seq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Seq>, int&&>);

        STATIC_CHECK(check_equal(union_seq, {0, 1, 2, 3, 4, 5}));

        const auto const_union_seq = flux::set_union(arr1, arr2);
        using ConstSeq = decltype(const_union_seq);

        static_assert(std::same_as<flux::element_t<ConstSeq>, int const&>);
        static_assert(std::same_as<flux::value_t<ConstSeq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<ConstSeq>, int const&&>);

        STATIC_CHECK(check_equal(std::as_const(const_union_seq), {0, 1, 2, 3, 4, 5}));
    }

    // Non-const-iterable sequences
    {
        int arr1[] = {0, 2, 4};
        int arr2[] = {1, 3, 5};
        auto yes = [](int) { return true; };

        auto union_seq = flux::set_union(flux::filter(flux::ref(arr1), yes), flux::filter(flux::from(arr2), yes));

        using T = decltype(union_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(union_seq, {0, 1, 2, 3, 4, 5}));
    }

    // test first seq empty
    {
        auto union_seq = flux::set_union(flux::empty<int>, std::array{1, 3, 5});

        using T = decltype(union_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(union_seq, {1, 3, 5}));
    }

    // test second seq empty
    {
        auto union_seq = flux::set_union(std::array{1, 3, 5}, flux::empty<int>);

        using T = decltype(union_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(union_seq, {1, 3, 5}));
    }

    // custom compare
    {
        auto union_seq = flux::set_union(std::array{4, 2, 0}, std::array{5, 3, 1}, std::ranges::greater{});

        using T = decltype(union_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(union_seq, {5, 4, 3, 2, 1, 0 }));
    }

    // projection
    {
        std::array<std::pair<int, char>, 3> arr1{{{0, 'a'}, {2, 'b'}, {4, 'c'}}};
        std::array<std::pair<int, char>, 3> arr2{{{1, 'x'}, {3, 'y'}, {5, 'z'}}};

        auto union_seq = flux::set_union(flux::ref(arr1), flux::ref(arr2), 
                                         flux::proj(std::ranges::less{}, [] (auto v) { return v.first; }));

        using T = decltype(union_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(union_seq, std::array<std::pair<int, char>, 6>{
                        {{0, 'a'}, {1, 'x'}, {2, 'b'}, {3, 'y'}, {4, 'c'}, {5, 'z'}}}));
    }

    // test with repeating values
    {
        std::array arr1{1, 2, 3, 3, 3};
        std::array arr2{2, 3, 3, 4};

        auto union_seq = flux::set_union(arr1, arr2);

        STATIC_CHECK(check_equal(union_seq, {1, 2, 3, 3, 3, 4}));
    }

    // test with different (but compatible) types
    {
        std::array arr1{1, 2, 3, 4, 5};
        std::array arr2{4.0, 5.0, 6.0};

        auto union_seq = flux::set_union(arr1, arr2);

        using T = decltype(union_seq);

        static_assert(std::same_as<flux::element_t<T>, double>);
        static_assert(std::same_as<flux::value_t<T>, double>);
        static_assert(std::same_as<flux::rvalue_element_t<T>, double>);
        static_assert(std::same_as<flux::const_element_t<T>, double>);

        STATIC_CHECK(check_equal(union_seq, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}));
    }

    return true;
}

static_assert(test_set_union());

}

TEST_CASE("set_union")
{
    bool result = test_set_union();
    REQUIRE(result);
}