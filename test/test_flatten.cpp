
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <string>
#include <string_view>
#include <ranges>
#include <vector>

#include "test_utils.hpp"


#if defined(_GLIBCXX_RELEASE)
#  if _GLIBCXX_RELEASE < 12
#    define NO_CONSTEXPR_VECTOR
#  endif
#endif

/*
 * We have three completely separate implementations of flatten.
 * The first is iterable only. The second is single-pass sequence only,
 * while the third can go all the way to bidirectional.
 *
 * The multipass version is used when all of the following are true:
 *  * the outer sequence is multipass
 *  * the element type of the outer sequence is a reference type
 *  * the inner sequence is multipass
 *
 * Otherwise, the single-pass version is used when the outer and
 * the inner are both sequences, else the basic iterable version
 * is used.
 */

namespace {

constexpr bool test_flatten_iterable()
{
    // Outer is iterable only
    {
        std::array<std::array<int, 3>, 3> arr{
            std::array{1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        };

        auto view = arr | std::views::transform(std::identity{});

        auto flattened = flux::ref(view).flatten();

        using F = decltype(flattened);
        static_assert(flux::iterable<F>);
        static_assert(flux::iterable<F const>);
        static_assert(not flux::sequence<F>);

        STATIC_CHECK(flattened.all(flux::pred::positive));
        STATIC_CHECK(check_equal(flattened, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    // Outer is multipass, inner is iterable only
    {
        std::array<std::array<int, 3>, 3> arr{
            std::array{1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        };

        auto views = flux::map(arr, [](auto inner) {
            return std::views::transform(std::move(inner), std::identity{});
        });

        auto flattened = flux::ref(views).flatten();

        using F = decltype(flattened);
        static_assert(flux::iterable<F>);
        static_assert(flux::iterable<F const>);
        static_assert(not flux::sequence<F>);

        STATIC_CHECK(flattened.all(flux::pred::positive));
        STATIC_CHECK(check_equal(flattened, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    return true;
}
static_assert(test_flatten_iterable());

constexpr bool test_flatten_single_pass()
{
    // Single-pass source sequence, inner sequence is multipass
    {
        std::array<std::array<int, 3>, 3> arr{
            std::array{1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        };
        auto seq = flux::flatten(single_pass_only(std::move(arr)));

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    // Multipass sequence, returns prvalue
    {
        std::array<std::array<int, 3>, 3> arr{
            std::array{1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        };

        auto seq = flux::map(arr, [](auto s) { return s; }).flatten();

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    // Multipass but returns single-pass
    {
        auto arr = std::array{
            single_pass_only(flux::single(1)),
            single_pass_only(flux::single(2)),
            single_pass_only(flux::single(3))
        };

        auto seq = flux::flatten(std::move(arr));

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3}));
    }

    // Short-circuiting internal iteration
    {
        std::array<std::array<int, 3>, 3> const arr{
            std::array{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

        auto seq = flux::flatten(single_pass_only(std::ref(arr)));
        auto five = seq.find(5);

        STATIC_CHECK(&seq[five] == &arr[1][1]);
    }

    // Empty outer sequence is handled correctly
    {
        auto arr = std::array<std::array<int, 3>, 0>{};

        auto seq = flux::flatten(single_pass_only(std::move(arr)));

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(seq.count() == 0);
    }

#ifndef NO_CONSTEXPR_VECTOR
    // Empty inner sequence is skipped correctly
    {
        std::vector<std::vector<int>> vec_of_vecs{
            {1, 2, 3}, {}, {4, 5, 6}, {}, {7}, {}, {8, 9}
        };

        auto seq = single_pass_only(std::move(vec_of_vecs)).flatten();

        using S = decltype(seq);
        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }
#endif // NO_CONSTEXPR_VECTOR

    // Test awkward single-pass flatten with a non-assignable inner sequence
    {
        int k = 0;
        auto seq = flux::ints(0, 2)
                    .map([&k](auto i) {
                           return flux::ints(0, 2).map([i, &k](auto j) { return i + j + k; });
                       })
                    .flatten();

        STATIC_CHECK(check_equal(seq, {0, 1, 1, 2}));
    }

    return true;
}
static_assert(test_flatten_single_pass());

constexpr bool test_flatten_multipass()
{
    {
        std::array<std::array<int, 3>, 3> const arr{
            std::array{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

        auto const seq = flux::flatten(arr);

        static_assert(flux::bidirectional_sequence<decltype(seq)>);
        static_assert(flux::bounded_sequence<decltype(seq)>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }

    {
        std::array<std::array<int, 3>, 3> const arr{
            std::array{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

        STATIC_CHECK(flux::flatten(arr).sum() == 45);
    }

    // Short-circuiting internal iter
    {
        std::array<std::array<int, 3>, 3> const arr{
            std::array{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

        auto seq = flux::flatten(std::ref(arr));
        auto five = seq.find(5);

        STATIC_CHECK(&seq[five] == &arr[1][1]);
    }

    // reversing
    {
        std::array<std::array<int, 3>, 3> const arr{
            std::array{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

        auto seq = flux::reverse(flux::flatten(arr));

        static_assert(flux::multipass_sequence<decltype(seq)>);

        STATIC_CHECK(check_equal(seq, {9, 8, 7, 6, 5, 4, 3, 2, 1}));
    }

    // Empty outer sequence is handled correctly
    {
        auto arr = std::array<std::array<int, 3>, 0>{};

        auto seq = flux::flatten(arr);

        using S = decltype(seq);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(seq.is_empty());
    }

#ifndef NO_CONSTEXPR_VECTOR
    // Empty inner sequence is skipped correctly
    {
        std::vector<std::vector<int>> vec_of_vecs{
            {1, 2, 3}, {}, {4, 5, 6}, {}, {7}, {}, {8, 9}
        };

        auto seq = flux::flatten(std::move(vec_of_vecs));

        using S = decltype(seq);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(not flux::sized_iterable<S>);
        static_assert(flux::bounded_sequence<S>);

        STATIC_CHECK(check_equal(seq, {1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }
#endif // NO_CONSTEXPR_VECTOR

    // Iterating to the end gives last()
    {
        std::array<std::string_view, 3> arr{"a", "b", "c"};

        auto seq = flux::flatten(arr);

        auto cur = seq.first();
        while (!seq.is_last(cur)) {
            seq.inc(cur);
        }

        STATIC_CHECK(cur == seq.last());
    }

    return true;
}
static_assert(test_flatten_multipass());

#ifndef NO_CONSTEXPR_VECTOR
constexpr
#endif
bool issue_150()
{
    const std::vector<std::string_view> vec{"a", "b", "c"};

    auto str = flux::ref(vec).flatten().to<std::string>();

    return str == "abc";
}
#ifndef NO_CONSTEXPR_VECTOR
static_assert(issue_150());
#endif

}

TEST_CASE("flatten")
{
    bool it = test_flatten_iterable();
    REQUIRE(it);

    bool sp = test_flatten_single_pass();
    REQUIRE(sp);

    bool mp = test_flatten_multipass();
    REQUIRE(mp);

    bool res = issue_150();
    REQUIRE(res);
}
