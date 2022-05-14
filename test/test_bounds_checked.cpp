
#include "catch.hpp"

#include <flux/op/bounds_checked.hpp>
#include <flux/op/map.hpp>
#include <flux/ranges/view.hpp>
#include <flux/source/istream.hpp>

#include <sstream>

#include "test_utils.hpp"

namespace {

// Make sure everything is constexpr as long as we stay in-bounds
constexpr bool test_bounds_checked()
{
    {
        int arr[] = {0, 1, 2, 3, 4};

        auto seq = flux::bounds_checked(arr).map([](int i) { return i * 2; });

        using S = decltype(seq);

        static_assert(flux::random_access_sequence<S>);
        static_assert(flux::bounded_sequence<S>);
        static_assert(flux::sized_sequence<S>);

        static_assert(flux::random_access_sequence<S const>);
        static_assert(flux::bounded_sequence<S const>);
        static_assert(flux::sized_sequence<S const>);

        STATIC_CHECK(seq.size() == 5);

        STATIC_CHECK(check_equal(seq, {0, 2, 4, 6, 8}));

        int sum = 0;
        for (int i : seq.view()) {
            sum += i;
        }

        STATIC_CHECK(sum == 20);
    }

    return true;
}
static_assert(test_bounds_checked());

}

TEST_CASE("bounds_checked")
{
    {
        int arr[] = {0, 1, 2, 3, 4};

        auto seq = flux::bounds_checked(arr);

        SECTION("Can read from in-bounds indices")
        {
            auto cur = seq.first();
            REQUIRE(seq[cur] == 0);
            REQUIRE(seq.move_at(cur) == 0);
        }

        SECTION("Can advance within bounds")
        {
            auto cur = seq.first();
            for (; !seq.is_last(cur); seq.inc(cur)) {}
            REQUIRE(seq.is_last(cur));
        }

        SECTION("Advancing past the end is an error")
        {
            auto cur = seq.last();
            REQUIRE_THROWS_AS(seq.inc(cur), std::out_of_range);
        }

        SECTION("Reading past the end is an error")
        {
            auto cur = seq.last();
            REQUIRE_THROWS_AS(seq[cur], std::out_of_range);
            REQUIRE_THROWS_AS(seq.move_at(cur), std::out_of_range);
        }

        SECTION("Can decrement within bounds")
        {
            auto cur = seq.last();
            while(seq.dec(cur) != seq.first()) {}
            REQUIRE(cur == seq.first());
        }

        SECTION("Decrementing before the start is an error")
        {
            auto cur = seq.first();
            REQUIRE_THROWS_AS(seq.dec(cur), std::out_of_range);
        }

        SECTION("Random access jumps out of bounds are an error")
        {
            auto cur = seq.first();
            REQUIRE_THROWS_AS(seq.inc(cur, 100), std::out_of_range);
            // Advancing to the end of the range (but no further) is okay
            REQUIRE(seq.is_last(flux::next(seq, cur, seq.size())));

            REQUIRE_THROWS_AS(seq.inc(cur, -100), std::out_of_range);

            REQUIRE(flux::next(seq, seq.last(), -seq.size()) == seq.first());
        }

        SECTION("Random reads are an error")
        {
            REQUIRE_THROWS_AS(seq[100], std::out_of_range);
            REQUIRE_THROWS_AS(seq.move_at(100), std::out_of_range);
        }

        SECTION("Subsequences are bounds-checked too")
        {
            auto slice = flux::slice(seq, 2, 4);
            REQUIRE(slice.size() == 2);

            auto f = slice.first();
            auto l = slice.last();
            REQUIRE_THROWS_AS(slice.inc(l), std::out_of_range);
            REQUIRE_THROWS_AS(slice.dec(f), std::out_of_range);
        }

        SECTION("Views are bounds checked as well")
        {
            auto view = seq.view();

            auto first = view.begin();
            auto last = view.end();

            (void) *first;

            REQUIRE_THROWS_AS(*last, std::out_of_range);
            REQUIRE_THROWS_AS(++last, std::out_of_range);
            REQUIRE_THROWS_AS(last++, std::out_of_range);
            REQUIRE_THROWS_AS(--first, std::out_of_range);
            REQUIRE_THROWS_AS(first--, std::out_of_range);
            REQUIRE_THROWS_AS(first += 10, std::out_of_range);
            REQUIRE_THROWS_AS(last -= 10, std::out_of_range);
            REQUIRE_THROWS_AS(first[10], std::out_of_range);
        }
    }

    SECTION("Single-pass sequences can be bounds-checked")
    {
        std::istringstream iss("1 2 3 ");

        auto seq = flux::bounds_checked(flux::from_istream<int>(iss));

        auto cur = seq.first();
        REQUIRE(seq[cur] == 1); seq.inc(cur);
        REQUIRE(seq[cur] == 2); seq.inc(cur);
        REQUIRE(seq[cur] == 3); seq.inc(cur);

        REQUIRE_THROWS_AS(seq[cur], std::out_of_range);
        REQUIRE_THROWS_AS(seq.inc(cur), std::out_of_range);
    }
}