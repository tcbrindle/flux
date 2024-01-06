
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#ifdef USE_MODULES
import flux;
#else
#include <flux/source/empty.hpp>
#endif

namespace {

auto e = flux::empty<double>;
auto f = flux::empty<double>;

static_assert(flux::contiguous_sequence<decltype(e)>);
static_assert(flux::sized_sequence<decltype(e)>);
static_assert(flux::bounded_sequence<decltype(e)>);
static_assert(std::same_as<flux::element_t<decltype(e)>, double&>);

static_assert(e.first() == f.first());
static_assert(!(e.first() < f.first()));
static_assert(e.first() == e.last());
static_assert(e.size() == 0);
static_assert(e.distance(e.first(), e.last()) == 0);
static_assert(e.data() == nullptr);
static_assert(flux::is_empty(e));

}

TEST_CASE("empty")
{
    // Make sure the assertion fires
    REQUIRE_THROWS_AS(e[e.first()], flux::unrecoverable_error);

    REQUIRE(e.first() == f.first());
    REQUIRE(!(e.first() < f.first()));
    REQUIRE(e.first() == e.last());
    REQUIRE(e.next(e.first()) == e.last());
    REQUIRE(e.prev(e.last()) == e.first());
    REQUIRE(e.size() == 0);
    REQUIRE(e.distance(e.first(), e.last()) == 0);
    REQUIRE(e.data() == nullptr);
    REQUIRE(e.is_empty());
    REQUIRE(flux::is_empty(e));
}
