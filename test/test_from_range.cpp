
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <forward_list>
#include <list>
#include <map>
#include <ranges>
#include <sstream>
#include <utility>
#include <vector>

#include "test_utils.hpp"

#if defined(_GLIBCXX_RELEASE)
#  if _GLIBCXX_RELEASE < 12
#    define RVALUE_VIEWS_NOT_SUPPORTED
#  endif
#endif

namespace {

constexpr bool test_from_range()
{
    {
        std::array<int, 4> arr{10, 2, 3, 4};

        auto seq = flux::from_range(flux::ref(arr));

        using S = decltype(seq);

        static_assert(flux::sequence<S>);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::contiguous_sequence<S>);
        static_assert(flux::sized_sequence<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(flux::read_at(seq, flux::first(seq)) == 10);
        STATIC_CHECK(flux::move_at(seq, flux::prev(seq, flux::last(seq))) == 4);
        STATIC_CHECK(flux::usize(seq) == arr.size());
        STATIC_CHECK(flux::data(seq)[0] == 10);
    }

    return true;
}
static_assert(test_from_range());

// https://github.com/tcbrindle/flux/issues/244
constexpr bool issue_244()
{
    std::string_view str("abcdefghijklmnopqrstuvwxyz");

    auto chunks = flux::from_range(str).chunk(3);

    std::string out;

    flux::for_each(chunks, [&out](auto c) { flux::for_each(c, [&out](char n) { out += n; }); });

    return out == str;
}
static_assert(issue_244());
}

TEST_CASE("from range")
{
    REQUIRE(test_from_range());
    REQUIRE(issue_244());

    SUBCASE("bounds checking") {
        std::vector<int> vec{0, 1, 2, 3, 4};

        auto seq = flux::from_range(vec);

        SUBCASE("Can read in-bounds")
        {
            auto cur = seq.first();
            REQUIRE(seq[cur] == 0);
            REQUIRE(seq.move_at(cur) == 0);
        }

        SUBCASE("Can advance within bounds")
        {
            auto cur = seq.first();
            for (; !seq.is_last(cur); seq.inc(cur)) {}
            REQUIRE(seq.is_last(cur));
        }

        SUBCASE("Reading past the end is an error")
        {
            auto cur = seq.last();
            REQUIRE_THROWS_AS(seq[cur], flux::unrecoverable_error);
            REQUIRE_THROWS_AS(seq.move_at(cur), flux::unrecoverable_error);
        }

        SUBCASE("Advancing past the end is an error")
        {
            auto cur = seq.last();
            REQUIRE_THROWS_AS(seq.next(cur), flux::unrecoverable_error);
        }

        SUBCASE("Can decrement within bounds")
        {
            auto cur = seq.last();
            while(seq.dec(cur) != seq.first()) {}
            REQUIRE(cur == seq.first());
        }

        SUBCASE("Decrementing before the start is an error")
        {
            auto cur = seq.first();
            REQUIRE_THROWS_AS(seq.dec(cur), flux::unrecoverable_error);
        }

        SUBCASE("Random-access movements are bounds-checked")
        {
            auto cur = seq.first();
            REQUIRE_THROWS_AS(seq.inc(cur, 100), flux::unrecoverable_error);

            REQUIRE_THROWS_AS(seq.inc(cur, -200), flux::unrecoverable_error);
        }
    }

    SUBCASE("...with std::list...") {
        std::list<int> list{1, 2, 3, 4, 5};

        SUBCASE("non-const argument, non-const seq, non-const from_range")
        {
            auto seq = flux::from_range(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("non-const argument, const seq, non-const from_range")
        {
            auto const seq = flux::from_range(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, non-const seq, non-const from_range")
        {
            auto seq = flux::from_range(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, const seq, non-const from_range")
        {
            auto const seq = flux::from_range(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

#ifndef RVALUE_VIEWS_NOT_SUPPORTED
        SUBCASE("prvalue argument, non-const seq, non-const from_range")
        {
            auto seq = flux::from_range(flux::copy(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

            static_assert(not flux::sequence<S const>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }
#endif

        SUBCASE("non-const argument, non-const seq, from_crange")
        {
            auto seq = flux::from_crange(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("non-const argument, const seq, from_crange")
        {
            auto const seq = flux::from_crange(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, non-const seq, from_crange")
        {
            auto seq = flux::from_crange(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, const seq, from_crange")
        {
            auto const seq = flux::from_crange(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

#ifndef RVALUE_VIEWS_NOT_SUPPORTED
        SUBCASE("prvalue argument, non-const seq, from_crange")
        {
            auto seq = flux::from_crange(flux::copy(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("prvalue argument, const seq, from_crange")
        {
            auto const seq = flux::from_crange(flux::copy(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }
#endif
    }

    SUBCASE("...with std::forward_list...") {
        std::forward_list<int> list{1, 2, 3, 4, 5};

        SUBCASE("non-const argument, non-const seq, non-const from_range")
        {
            auto seq = flux::from_range(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("non-const argument, const seq, non-const from_range")
        {
            auto const seq = flux::from_range(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, non-const seq, non-const from_range")
        {
            auto seq = flux::from_range(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, const seq, non-const from_range")
        {
            auto const seq = flux::from_range(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

#ifndef RVALUE_VIEWS_NOT_SUPPORTED
        SUBCASE("prvalue argument, non-const seq, non-const from_range")
        {
            auto seq = flux::from_range(flux::copy(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int&&>);

            static_assert(not flux::sequence<S const>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }
#endif

        SUBCASE("non-const argument, non-const seq, from_crange")
        {
            auto seq = flux::from_crange(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("non-const argument, const seq, from_crange")
        {
            auto const seq = flux::from_crange(list);

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, non-const seq, from_crange")
        {
            auto seq = flux::from_crange(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("const argument, const seq, from_crange")
        {
            auto const seq = flux::from_crange(std::as_const(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

#ifndef RVALUE_VIEWS_NOT_SUPPORTED
        SUBCASE("prvalue argument, non-const seq, from_crange")
        {
            auto seq = flux::from_crange(flux::copy(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }

        SUBCASE("prvalue argument, const seq, from_crange")
        {
            auto const seq = flux::from_crange(flux::copy(list));

            using S = decltype(seq);
            static_assert(flux::sequence<S>);
            static_assert(flux::multipass_sequence<S>);
            static_assert(not flux::bidirectional_sequence<S>);
            static_assert(not flux::random_access_sequence<S>);
            static_assert(not flux::contiguous_sequence<S>);
            static_assert(flux::bounded_sequence<S>);
            static_assert(not flux::sized_sequence<S>);

            static_assert(std::same_as<flux::value_t<S>, int>);
            static_assert(std::same_as<flux::element_t<S>, int const&>);
            static_assert(std::same_as<flux::rvalue_element_t<S>, int const&&>);

            REQUIRE(check_equal(seq, {1, 2, 3, 4, 5}));
        }
#endif
    }

    SUBCASE("with input range") {

        std::istringstream iss("1 2 3 4 5");
        auto view = std::ranges::basic_istream_view<int, char>(iss);

        auto seq = flux::from_range(view);

        using S = decltype(seq);

        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);

        static_assert(not flux::sequence<S const>);

        static_assert(not std::copyable<flux::cursor_t<S>>);
    }

}