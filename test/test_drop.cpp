
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "test_utils.hpp"

namespace {

constexpr bool test_drop() {

    {
        int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto dropped = flux::drop(flux::ref(arr), 5);

        using D = decltype(dropped);

        static_assert(flux::contiguous_sequence<D>);
        static_assert(flux::sized_iterable<D>);
        static_assert(flux::bounded_sequence<D>);

        static_assert(flux::contiguous_sequence<D const>);
        static_assert(flux::sized_iterable<D const>);
        static_assert(flux::bounded_sequence<D const>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(flux::data(dropped) == arr + 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));

        auto const& c_dropped = dropped;
        STATIC_CHECK(flux::size(c_dropped) == 5);
        STATIC_CHECK(flux::data(c_dropped) == arr + 5);
        STATIC_CHECK(check_equal(c_dropped, {5, 6, 7, 8, 9}));
    }

    {
        auto dropped = flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                           .drop(5);

        using D = decltype(dropped);

        static_assert(flux::contiguous_sequence<D>);
        static_assert(flux::sized_iterable<D>);
        static_assert(flux::bounded_sequence<D>);

        static_assert(flux::contiguous_sequence<D const>);
        static_assert(flux::sized_iterable<D const>);
        static_assert(flux::bounded_sequence<D const>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));

        auto const& c_dropped = dropped;
        STATIC_CHECK(flux::size(c_dropped) == 5);
        STATIC_CHECK(check_equal(c_dropped, {5, 6, 7, 8, 9}));
    }

    {
        auto dropped = single_pass_only(flux::from(std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}))
                           .drop(5);

        using D = decltype(dropped);

        static_assert(flux::sequence<D>);
        static_assert(not flux::multipass_sequence<D>);
        static_assert(flux::sized_iterable<D>);
        static_assert(flux::bounded_sequence<D>);

        STATIC_CHECK(flux::size(dropped) == 5);
        STATIC_CHECK(check_equal(dropped, {5, 6, 7, 8, 9}));
    }

    // test dropping zero items
    {
        auto dropped = flux::drop(std::array{1, 2, 3, 4, 5}, 0);

        STATIC_CHECK(dropped.size() == 5);
        STATIC_CHECK(check_equal(dropped, {1, 2, 3, 4, 5}));
    }

    // test dropping all items
    {
        auto const arr = std::array{1, 2, 3, 4, 5};

        auto dropped = flux::ref(arr).drop(5);

        STATIC_CHECK(dropped.is_empty());
        STATIC_CHECK(dropped.size() == 0);
        STATIC_CHECK(dropped.distance(dropped.first(), dropped.last()) == 0);
        STATIC_CHECK(flux::equal(dropped, flux::empty<int>));
        STATIC_CHECK(dropped.data() == arr.data() + 5);
    }

    // test dropping too many items
    {
        auto const arr = std::array{1, 2, 3, 4, 5};

        auto dropped = flux::ref(arr).drop(1000UL);

        STATIC_CHECK(dropped.is_empty());
        STATIC_CHECK(dropped.size() == 0);
        STATIC_CHECK(dropped.distance(dropped.first(), dropped.last()) == 0);
        STATIC_CHECK(flux::equal(dropped, flux::empty<int>));
        STATIC_CHECK(dropped.data() == arr.data() + 5);
    }

    return true;
}
static_assert(test_drop());

constexpr bool issue_132a()
{
    auto result = flux::from(std::array{1, 2})
                      .filter(flux::pred::even)
                      .drop(2)
                      .drop(1);
    STATIC_CHECK(flux::is_empty(result));
    return true;
}
static_assert(issue_132a());

void issue_132b()
{
    using namespace flux;

    auto intersperse = [](auto r, auto e) -> auto {
        return flux::map(std::move(r), [e](auto const& x) -> auto {
                   return std::vector{e, x};
               }).flatten().drop(1);
    };

    auto sfml_argument = [](std::string_view) -> std::string { return "abc";  };

    auto sfml_argument_list = [&](std::span<std::string_view> mf) -> std::string {
        return "(" + intersperse(drop(mf, 1).map(sfml_argument), std::string(", ")).flatten().to<std::string>() + ")";
    };

    std::vector<std::string_view> v {"point"};
    (void) sfml_argument_list(v);
}

}

TEST_CASE("drop")
{
    bool result = test_drop();
    REQUIRE(result);

    // Test dropping a negative number of elements
    {
        std::list list{1, 2, 3, 4, 5};

        REQUIRE_THROWS_AS(flux::drop(flux::from_range(list), -1), flux::unrecoverable_error);

        REQUIRE_THROWS_AS(flux::from_range(list).drop(-1000), flux::unrecoverable_error);
    }

    issue_132b();

}