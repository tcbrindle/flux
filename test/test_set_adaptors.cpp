
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <utility>

#include "test_utils.hpp"

namespace {

constexpr bool test_set_union()
{
    {
        int arr1[] = {0,    2,    4,    6};
        int arr2[] = {   1,    3,    5};
        auto union_seq = flux::set_union(flux::ref(arr1), flux::ref(arr2));

        using T = decltype(union_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(union_seq, {0, 1, 2, 3, 4, 5, 6}));
    }

    // Const iteration works as expected
    {
        std::array arr1{0,  2,  4};
        std::array arr2{  1,  3,  5};
        auto union_seq = flux::set_union(flux::mut_ref(arr1), flux::mut_ref(arr2));

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
        int arr1[] = {0,  2,  4};
        int arr2[] = {  1,  3,  5};
        auto yes = [](int) { return true; };

        auto union_seq = flux::set_union(flux::filter(flux::ref(arr1), yes), flux::filter(flux::ref(arr2), yes));

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
        auto union_seq = flux::set_union(std::array{4, 2, 0}, std::array{5, 3, 1}, flux::cmp::reverse_compare);

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
                                         flux::proj(flux::cmp::compare, [] (auto v) { return v.first; }));

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
        std::array arr2{4L, 5L, 6L};

        auto union_seq = flux::set_union(arr1, arr2);

        using T = decltype(union_seq);

        static_assert(std::same_as<flux::element_t<T>, long>);
        static_assert(std::same_as<flux::value_t<T>, long>);
        static_assert(std::same_as<flux::rvalue_element_t<T>, long>);
        static_assert(std::same_as<flux::const_element_t<T>, long>);

        STATIC_CHECK(check_equal(union_seq, {1L, 2L, 3L, 4L, 5L, 6L}));
    }

    // test cursor iteration
    {
        int arr1[] = {0,    2,    4,   6};
        int arr2[] = {   1,    3,    5};
        auto union_seq = flux::set_union(flux::ref(arr1), flux::ref(arr2));

        auto first = flux::first(union_seq);
        auto last = flux::last(union_seq);
        while (first != last) {
            flux::inc(union_seq, first);
        }
        STATIC_CHECK(first == last);
    }

    return true;
}

constexpr bool test_set_difference()
{
    {
        int arr1[] = {0, 1, 2, 3, 4, 5, 6};
        int arr2[] = {   1,    3,    5};
        auto diff_seq = flux::set_difference(flux::ref(arr1), flux::ref(arr2));

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {0, 2, 4, 6}));
    }

    // Const iteration works as expected
    {
        std::array arr1{0, 1, 2, 3, 4, 5, 6};
        std::array arr2{   1,    3,    5};
        auto diff_seq = flux::set_difference(flux::mut_ref(arr1), flux::mut_ref(arr2));

        using Seq = decltype(diff_seq);

        static_assert(flux::sequence<Seq>);
        static_assert(flux::multipass_sequence<Seq>);
        static_assert(not flux::sized_sequence<Seq>);

        static_assert(std::same_as<flux::element_t<Seq>, int&>);
        static_assert(std::same_as<flux::value_t<Seq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Seq>, int&&>);

        STATIC_CHECK(check_equal(diff_seq, {0, 2, 4, 6}));

        const auto const_diff_seq = flux::set_difference(arr1, arr2);
        using ConstSeq = decltype(const_diff_seq);

        static_assert(std::same_as<flux::element_t<ConstSeq>, int const&>);
        static_assert(std::same_as<flux::value_t<ConstSeq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<ConstSeq>, int const&&>);

        STATIC_CHECK(check_equal(std::as_const(const_diff_seq), {0, 2, 4, 6}));
    }

    // Non-const-iterable sequences
    {
        int arr1[] = {0, 1, 2, 3, 4, 5, 6};
        int arr2[] = {   1,    3,    5};
        auto yes = [](int) { return true; };

        auto diff_seq = flux::set_difference(flux::filter(flux::ref(arr1), yes), flux::filter(flux::ref(arr2), yes));

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {0, 2, 4, 6}));
    }

    // test first seq empty
    {
        auto diff_seq = flux::set_difference(flux::empty<int>, std::array{1, 3, 5});

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, flux::empty<int>));
    }

    // test second seq empty
    {
        auto diff_seq = flux::set_difference(std::array{1, 3, 5}, flux::empty<int>);

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {1, 3, 5}));
    }

    // custom compare
    {
        auto diff_seq = flux::set_difference(std::array{5, 4, 3, 2, 1, 0}, std::array{4, 2, 0},
                                             flux::cmp::reverse_compare);

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {5, 3, 1}));
    }

    // projection
    {
        std::array<std::pair<int, char>, 4> arr1{{{0, 'a'}, {1, 'b'}, {2, 'c'}, {3, 'd'}}};
        std::array<std::pair<int, char>, 3> arr2{{{1, 'x'}, {2, 'y'}, {5, 'z'}}};

        auto diff_seq = flux::set_difference(flux::ref(arr1), flux::ref(arr2), 
                                             flux::proj(flux::cmp::compare, [] (auto v) { return v.first; }));

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, std::array<std::pair<int, char>, 2>{{{0, 'a'}, {3, 'd'}}}));
    }

    // test with repeating values
    {
        std::array arr1{1, 2, 3, 3, 3};
        std::array arr2{   2, 3, 3};

        auto diff_seq = flux::set_difference(arr1, arr2);

        STATIC_CHECK(check_equal(diff_seq, {1, 3}));
    }

    // test different value types
    {
        std::array<int, 4> arr1{1, 2, 3, 4};
        std::array<char, 2> arr2{  2, 3};

        auto diff_seq = flux::set_difference(arr1, arr2);

        using Seq = decltype(diff_seq);

        static_assert(flux::sequence<Seq>);
        static_assert(flux::multipass_sequence<Seq>);
        static_assert(not flux::sized_sequence<Seq>);

        static_assert(std::same_as<flux::element_t<Seq>, int&>);
        static_assert(std::same_as<flux::value_t<Seq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Seq>, int&&>);

        STATIC_CHECK(check_equal(diff_seq, {1, 4}));
    }

    return true;
}

constexpr bool test_set_symmetric_difference()
{
    {
        int arr1[] = {   1,    3,    5   };
        int arr2[] = {0, 1, 2, 3, 4, 5, 6};
        auto diff_seq = flux::set_symmetric_difference(flux::ref(arr1), flux::ref(arr2));

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {0, 2, 4, 6}));
    }

    // Const iteration works as expected
    {
        std::array arr1{1, 2, 3, 4, 5, 6, 7, 8};
        std::array arr2{            5,    7,    9, 10};
        auto diff_seq = flux::set_symmetric_difference(flux::mut_ref(arr1), flux::mut_ref(arr2));

        using Seq = decltype(diff_seq);

        static_assert(flux::sequence<Seq>);
        static_assert(flux::multipass_sequence<Seq>);
        static_assert(not flux::sized_sequence<Seq>);

        static_assert(std::same_as<flux::element_t<Seq>, int&>);
        static_assert(std::same_as<flux::value_t<Seq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Seq>, int&&>);

        STATIC_CHECK(check_equal(diff_seq, {1, 2, 3, 4, 6, 8, 9, 10}));

        const auto const_diff_seq = flux::set_symmetric_difference(arr1, arr2);
        using ConstSeq = decltype(const_diff_seq);

        static_assert(std::same_as<flux::element_t<ConstSeq>, int const&>);
        static_assert(std::same_as<flux::value_t<ConstSeq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<ConstSeq>, int const&&>);

        STATIC_CHECK(check_equal(std::as_const(const_diff_seq), {1, 2, 3, 4, 6, 8, 9, 10}));
    }

    // Non-const-iterable sequences
    {
        int arr1[] = {0, 1, 2, 3, 4};
        int arr2[] = {   1, 2,      5};
        auto yes = [](int) { return true; };

        auto diff_seq = flux::set_symmetric_difference(flux::filter(flux::ref(arr1), yes), flux::filter(flux::ref(arr2), yes));

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {0, 3, 4, 5}));
    }

    // test first seq empty
    {
        auto diff_seq = flux::set_symmetric_difference(flux::empty<int>, std::array{1, 3, 5});

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {1, 3, 5}));
    }

    // test second seq empty
    {
        auto diff_seq = flux::set_symmetric_difference(std::array{1, 3, 5}, flux::empty<int>);

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {1, 3, 5}));
    }

    // custom compare
    {
        auto diff_seq = flux::set_symmetric_difference(std::array{5, 4, 3, 2, 1, 0}, std::array{6, 4, 2, 0},
                                                       flux::cmp::reverse_compare);

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, {6, 5, 3, 1}));
    }

    // projection
    {
        std::array<std::pair<int, char>, 4> arr1{{{0, 'a'}, {1, 'b'}, {2, 'c'}, {3, 'd'}}};
        std::array<std::pair<int, char>, 3> arr2{{{1, 'x'}, {2, 'y'}, {5, 'z'}}};

        auto diff_seq = flux::set_symmetric_difference(flux::ref(arr1), flux::ref(arr2), 
                                                       flux::proj(flux::cmp::compare, [] (auto v) { return v.first; }));

        using T = decltype(diff_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(diff_seq, std::array<std::pair<int, char>, 3>{{{0, 'a'}, {3, 'd'}, {5, 'z'}}}));
    }

    // test with repeating values
    {
        std::array arr1{1, 2, 3, 3, 3};
        std::array arr2{   2, 3, 3,   6};

        auto diff_seq = flux::set_symmetric_difference(arr1, arr2);

        STATIC_CHECK(check_equal(diff_seq, {1, 3, 6}));
    }

    // test different value types
    {
        std::array<int, 4> arr1{1, 2, 3, 4};
        std::array<long, 3> arr2{2L, 3L, 5L};

        auto diff_seq = flux::set_symmetric_difference(arr1, arr2);

        using Seq = decltype(diff_seq);

        static_assert(flux::sequence<Seq>);
        static_assert(flux::multipass_sequence<Seq>);
        static_assert(not flux::sized_sequence<Seq>);

        static_assert(std::same_as<flux::element_t<Seq>, long>);
        static_assert(std::same_as<flux::value_t<Seq>, long>);
        static_assert(std::same_as<flux::rvalue_element_t<Seq>, long>);
        static_assert(std::same_as<flux::const_element_t<Seq>, long>);

        STATIC_CHECK(check_equal(diff_seq, {1L, 4L, 5L}));
    }

    return true;
}

constexpr bool test_set_intersection()
{
    {
        int arr1[] = {0, 1, 2, 3};
        int arr2[] = {1, 3, 5};
        auto inter_seq = flux::set_intersection(flux::ref(arr1), flux::ref(arr2));

        using T = decltype(inter_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(inter_seq, {1, 3}));
    }

    // Const iteration works as expected
    {
        std::array arr1{0, 1, 2, 3};
        std::array arr2{1, 3, 5};
        auto inter_seq = flux::set_intersection(flux::mut_ref(arr1), flux::mut_ref(arr2));

        using Seq = decltype(inter_seq);

        static_assert(flux::sequence<Seq>);
        static_assert(flux::multipass_sequence<Seq>);
        static_assert(not flux::sized_sequence<Seq>);

        static_assert(std::same_as<flux::element_t<Seq>, int&>);
        static_assert(std::same_as<flux::value_t<Seq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Seq>, int&&>);

        STATIC_CHECK(check_equal(inter_seq, {1, 3}));

        const auto const_inter_seq = flux::set_intersection(arr1, arr2);
        using ConstSeq = decltype(const_inter_seq);

        static_assert(std::same_as<flux::element_t<ConstSeq>, int const&>);
        static_assert(std::same_as<flux::value_t<ConstSeq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<ConstSeq>, int const&&>);

        STATIC_CHECK(check_equal(std::as_const(const_inter_seq), {1, 3}));
    }

    // Non-const-iterable sequences
    {
        int arr1[] = {0, 1, 2, 3};
        int arr2[] = {1, 3, 5};
        auto yes = [](int) { return true; };

        auto inter_seq = flux::set_intersection(flux::filter(flux::ref(arr1), yes), flux::filter(flux::ref(arr2), yes));

        using T = decltype(inter_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(inter_seq, {1, 3}));
    }

    // test first seq empty
    {
        auto inter_seq = flux::set_intersection(flux::empty<int>, std::array{1, 3, 5});

        using T = decltype(inter_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(inter_seq, flux::empty<int>));
    }

    // test second seq empty
    {
        auto inter_seq = flux::set_intersection(std::array{1, 3, 5}, flux::empty<int>);

        using T = decltype(inter_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(inter_seq, flux::empty<int>));
    }

    // custom compare
    {
        auto inter_seq = flux::set_intersection(std::array{3, 2, 1, 0}, std::array{5, 3, 1},
                                                flux::cmp::reverse_compare);

        using T = decltype(inter_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(inter_seq, {3, 1}));
    }

    // projection
    {
        std::array<std::pair<int, char>, 4> arr1{{{0, 'a'}, {1, 'b'}, {2, 'c'}, {3, 'd'}}};
        std::array<std::pair<int, char>, 3> arr2{{{1, 'x'}, {2, 'y'}, {5, 'z'}}};

        auto inter_seq = flux::set_intersection(flux::ref(arr1), flux::ref(arr2), 
                                             flux::proj(flux::cmp::compare, [] (auto v) { return v.first; }));

        using T = decltype(inter_seq);
        static_assert(flux::sequence<T>);
        static_assert(flux::multipass_sequence<T>);
        static_assert(not flux::sized_sequence<T>);

        STATIC_CHECK(check_equal(inter_seq, std::array<std::pair<int, char>, 2>{{{1, 'b'}, {2, 'c'}}}));
    }

    // test with repeating values
    {
        std::array arr1{1, 2, 3, 3, 3};
        std::array arr2{2, 3, 3};

        auto inter_seq = flux::set_intersection(arr1, arr2);

        STATIC_CHECK(check_equal(inter_seq, {2, 3, 3}));
    }

    // test different value types
    {
        std::array<int, 4> arr1{1, 2, 3, 4};
        std::array<char, 2> arr2{2, 3};

        auto inter_seq = flux::set_intersection(arr1, arr2);

        using Seq = decltype(inter_seq);

        static_assert(flux::sequence<Seq>);
        static_assert(flux::multipass_sequence<Seq>);
        static_assert(not flux::sized_sequence<Seq>);

        static_assert(std::same_as<flux::element_t<Seq>, int&>);
        static_assert(std::same_as<flux::value_t<Seq>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<Seq>, int&&>);

        STATIC_CHECK(check_equal(inter_seq, {2, 3}));
    }

    return true;
}

static_assert(test_set_union());
static_assert(test_set_difference());
static_assert(test_set_symmetric_difference());
static_assert(test_set_intersection());

}

TEST_CASE("set_union")
{
    bool result = test_set_union();
    REQUIRE(result);
}

TEST_CASE("set_difference")
{
    bool result = test_set_difference();
    REQUIRE(result);
}

TEST_CASE("set_symmetric_difference")
{
    bool result = test_set_symmetric_difference();
    REQUIRE(result);
}

TEST_CASE("set_intersection")
{
    bool result = test_set_intersection();
    REQUIRE(result);
}
