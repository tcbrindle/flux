
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

#include "test_utils.hpp"

#include <array>

#include <iostream>

namespace {

template <typename Lhs, typename Rhs>
constexpr bool tuple_equal(Lhs const& lhs, Rhs const& rhs)
{
    auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return ((std::get<Is>(lhs) == std::get<Is>(rhs)) && ...);
    };

    return std::tuple_size_v<Lhs> == std::tuple_size_v<Rhs> &&
            impl(std::make_index_sequence<std::tuple_size_v<Lhs>>{});
}

constexpr bool test_pairwise()
{
    // Basic pairwise
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).pairwise();

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(not flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

        static_assert(std::same_as<flux::element_t<S>, std::pair<int&, int&>>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, std::pair<int&&, int&&>>);
        static_assert(std::same_as<flux::value_t<S>, std::pair<int, int>>);


//        STATIC_CHECK(seq.size() == 4);
//        STATIC_CHECK(seq.distance(seq.first(), seq.last()) == 4);

        auto cur = flux::first(seq);
        STATIC_CHECK(tuple_equal(seq[cur], std::pair{1, 2}));
        STATIC_CHECK(tuple_equal(seq[seq.inc(cur)], std::tuple{2, 3}));
        STATIC_CHECK(tuple_equal(seq[seq.inc(cur)], std::array{3, 4}));
        STATIC_CHECK(tuple_equal(seq[seq.inc(cur)], std::pair{4, 5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

//        STATIC_CHECK(cur == seq.last());
//        STATIC_CHECK(seq.is_last(seq.last()));
    }

    // const iteration works if the underlying is const-iterable
    {
        auto const seq = flux::pairwise(std::array{1, 2, 3, 4, 5});

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(not flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

//        STATIC_CHECK(flux::size(seq) == 4);

        auto cur = flux::first(seq);
        STATIC_CHECK(tuple_equal(flux::read_at(seq, cur), std::array{1, 2}));
        flux::inc(seq, cur);
        STATIC_CHECK(tuple_equal(flux::read_at(seq, cur), std::pair{2, 3}));
        flux::inc(seq, cur);
        STATIC_CHECK(tuple_equal(flux::read_at(seq, cur), std::pair{3, 4}));
        flux::inc(seq, cur);
        STATIC_CHECK(tuple_equal(flux::read_at(seq, cur), std::pair{4, 5}));
        flux::inc(seq, cur);
        STATIC_CHECK(flux::is_last(seq, cur));
    }

    // pairwise with empty sequence is an empty sequence
    {
        auto seq = flux::pairwise(flux::empty<int>);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // pairwise with two-element sequence has one element
    {
        auto seq = flux::pairwise(std::array{1, 2});

        STATIC_CHECK(flux::count(seq) == 1);
        STATIC_CHECK(tuple_equal(seq.front().value(), std::pair{1, 2}));
    }

#if 0
    // Reverse iteration works when underlying is bidir + bounded
    {
        auto seq = flux::pairwise(std::array{1, 2, 3, 4, 5}, 2).reverse();

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(seq[cur], {4, 5}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {2, 3}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {1, 2}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(seq.is_last(seq.last()));
    }
#endif

    return true;
}
static_assert(test_pairwise());

constexpr bool test_adjacent()
{
    // Basic striding
    {
        std::array arr{1, 2, 3, 4, 5};

        auto seq = flux::ref(arr).adjacent<3>();

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(not flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

//        STATIC_CHECK(seq.size() == 4);
//        STATIC_CHECK(seq.distance(seq.first(), seq.last()) == 4);

        auto cur = flux::first(seq);
        STATIC_CHECK(tuple_equal(seq[cur], std::array{1, 2, 3}));
        STATIC_CHECK(tuple_equal(seq[seq.inc(cur)], std::array{2, 3, 4}));
        STATIC_CHECK(tuple_equal(seq[seq.inc(cur)], std::array{3, 4, 5}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

   //     STATIC_CHECK(cur == seq.last());
   //     STATIC_CHECK(seq.is_last(seq.last()));
    }

    // const iteration works if the underlying is const-iterable
    {
        auto const seq = flux::adjacent<3>(std::array{1, 2, 3, 4, 5});

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(not flux::bidirectional_sequence<S>);
        static_assert(not flux::random_access_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);
        static_assert(not flux::sized_sequence<S>);

        //STATIC_CHECK(flux::size(seq) == 3);

        auto cur = flux::first(seq);
        STATIC_CHECK(tuple_equal(flux::read_at(seq, cur), std::tuple{1, 2, 3}));
        flux::inc(seq, cur);
        STATIC_CHECK(tuple_equal(flux::read_at(seq, cur), std::tuple{2, 3, 4}));
        flux::inc(seq, cur);
        STATIC_CHECK(tuple_equal(flux::read_at(seq, cur), std::tuple{3, 4, 5}));
        flux::inc(seq, cur);
        STATIC_CHECK(flux::is_last(seq, cur));
    }

    // adjacent with window size > sequence size is an empty sequence
    {
        auto seq = flux::adjacent<10>(std::array{1, 2, 3});

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // adjacent with empty sequence is an empty sequence
    {
        auto seq = flux::adjacent<5>(flux::empty<int>);

        STATIC_CHECK(seq.is_empty());
        STATIC_CHECK(seq.is_last(seq.first()));
    }

    // adjacent with window size == sequence size has one element
    {
        auto seq = flux::adjacent<5>(std::array{1, 2, 3, 4, 5});

        STATIC_CHECK(flux::count(seq) == 1);
        STATIC_CHECK(tuple_equal(seq.front().value(), std::array{1, 2, 3, 4, 5}));
    }

    // adjacent<n> + stride(n) is equivalent to chunk(n), if n divides size
    {
        std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

        auto adj_n_stride = flux::adjacent<3>(arr).stride(3);

        auto chunk = flux::chunk(arr, 3);

        auto tuple_to_array = []<typename T>(T&& tuple) {
            using array_t =  std::array<std::decay_t<std::tuple_element_t<0, T>>,
                                        std::tuple_size_v<T>>;
            return std::apply([](auto&&... args) {
                return array_t{FLUX_FWD(args)...};
            }, FLUX_FWD(tuple));
        };

        STATIC_CHECK(flux::equal(adj_n_stride, chunk, flux::equal, tuple_to_array));
    }
#if 0
    // Reverse iteration works when underlying is bidir + bounded
    {
        auto seq = flux::adjacent<3>(std::array{1, 2, 3, 4, 5}).reverse();

        using S = decltype(seq);
        static_assert(flux::multipass_sequence<S>);
        static_assert(flux::bidirectional_sequence<S>);
        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        auto cur = flux::first(seq);
        STATIC_CHECK(check_equal(seq[cur], {4, 5}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {3, 4}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {2, 3}));
        STATIC_CHECK(check_equal(seq[seq.inc(cur)], {1, 2}));
        STATIC_CHECK(seq.is_last(seq.inc(cur)));

        STATIC_CHECK(cur == seq.last());
        STATIC_CHECK(seq.is_last(seq.last()));
    }
#endif
    return true;
}
static_assert(test_adjacent());

}

TEST_CASE("adjacent")
{
    bool res = test_pairwise();
    REQUIRE(res);

    res = test_adjacent();
    REQUIRE(res);
}