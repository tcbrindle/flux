
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>

#include "test_utils.hpp"

namespace {

constexpr bool test_read_only()
{
    // read_only with iterables
    {
        auto iter = iterable_only(std::array{1, 2, 3, 4, 5});

        auto read_only = flux::read_only(iter);

        using R = decltype(read_only);

        static_assert(flux::iterable<R>);
        static_assert(flux::sized_iterable<R>);

        static_assert(std::same_as<flux::iterable_element_t<R>, int const&>);
        static_assert(std::same_as<flux::iterable_value_t<R>, int>);

        STATIC_CHECK(check_equal(read_only, {1, 2, 3, 4, 5}));
    }

    // Mutable lvalue ref -> const lvalue ref
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::read_only(flux::mut_ref(arr));
        using S = decltype(seq);

        static_assert(flux::contiguous_sequence<S>);
        static_assert(flux::read_only_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::value_t<S>, int>);

        STATIC_CHECK(flux::data(seq) == arr.data());
        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
    }

    // Const lvalue ref is unchanged
    {
        std::array const arr{1, 2, 3, 4, 5};

        auto seq = flux::read_only(flux::ref(arr));
        using S = decltype(seq);

        static_assert(flux::contiguous_sequence<S>);
        static_assert(flux::read_only_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::value_t<S>, int>);

        STATIC_CHECK(flux::data(seq) == arr.data());
        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
    }

    // Mutable rvalue ref -> const rvalue ref
    {
        std::array arr{1, 2, 3, 4, 5};
        auto seq = flux::mut_ref(arr)
            .map([](int& elem) -> int&&  { return std::move(elem); })
            .read_only();

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::read_only_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::value_t<S>, int>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
    }

    // Const rvalue ref is unchanged
    {
        std::array const arr{1, 2, 3, 4, 5};
        auto seq = flux::ref(arr)
            .map([](int const& elem) -> int const&&  {
                return std::move(elem);
            })
            .read_only();

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::read_only_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::read_only_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int const&&>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);
        static_assert(std::same_as<flux::value_t<S>, int>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
    }

    // By-value is unchanged
    {
        std::array const arr{1, 2, 3, 4, 5};
        auto seq = flux::ref(arr)
                           .map([](int const& elem) { return elem; })
                           .read_only();

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::read_only_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(std::same_as<flux::element_t<S>, int>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, int>);
        static_assert(std::same_as<flux::value_t<S>, int>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5}));
    }

    {
        std::array<int, 3> arr1{1, 2, 3};
        std::array<double, 3> arr2{100.0, 200.0, 300.0};

        auto seq = flux::zip(arr1, arr2).read_only();
        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        // static_assert(flux::read_only_sequence<S>);
        // This needs the C++23 tuple/pair converting constructors and
        // basic_common_reference specialisations to work properly
#ifdef FLUX_HAVE_CPP23_TUPLE_COMMON_REF
        static_assert(std::same_as<flux::element_t<S>, std::pair<int const&, double const&>>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, std::pair<int const&&, double const&&>>);
#endif
        static_assert(std::same_as<flux::value_t<S>, std::pair<int, double>>);
    }

    return true;
}
static_assert(test_read_only());

}

TEST_CASE("read_only")
{
    bool res = test_read_only();
    REQUIRE(res);
}