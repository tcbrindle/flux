
#include "catch.hpp"

#include <flux/source/empty.hpp>

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
{}