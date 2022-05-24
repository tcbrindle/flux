
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux/op/to.hpp>
#include <flux/op/filter.hpp>
#include <flux/op/zip.hpp>
#include <flux/ranges/from_range.hpp>
#include <flux/source/istream.hpp>

#include <array>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "test_utils.hpp"

namespace {

template <typename T>
struct my_allocator : std::allocator<T> {};

template <typename T, typename A = std::allocator<T>>
struct test_vector {

    test_vector() = default;

    template <flux::sequence Seq>
    test_vector(flux::from_sequence_t, Seq&& seq)
    {
        flux::output_to(seq, std::back_inserter(vec_));
    }

    template <flux::sequence Seq>
    test_vector(flux::from_sequence_t, Seq&& seq, A const& alloc)
        : vec_(alloc)
    {
        flux::output_to(seq, std::back_inserter(vec_));
    }

    auto begin() { return vec_.begin(); }
    auto end() { return vec_.end(); }

private:
    std::vector<T, A> vec_;
};


}

TEST_CASE("to")
{
    SECTION("...with explicit value type")
    {
        SECTION("vector->vector construction")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};
            auto vec2 = flux::to<std::vector<int>>(vec1);

            CHECK(vec1 == vec2);
        }

        SECTION("vector->vector construction with allocator")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};
            auto vec2 = flux::to<std::vector<int, my_allocator<int>>>(vec1, my_allocator<int>{});

            CHECK(std::ranges::equal(vec1, vec2));
        }

        SECTION("zipped sequence to map")
        {
            auto zipped = flux::zip(std::array{1, 2, 3},
                                   std::vector<std::string>{"1", "2", "3"});

            const auto map = flux::to<std::map<int, std::string>>(zipped);

            const auto reqd = std::map<int, std::string>{
                {1, "1"}, {2, "2"}, {3, "3"}
            };

            CHECK(map == reqd);
        }

        SECTION("from_sequence construction")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};

            auto vec2 = flux::to<test_vector<int>>(vec1);

            CHECK(std::ranges::equal(vec1, vec2));
        }

        SECTION("from_sequence construction with allocator")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};

            auto vec2 = flux::to<test_vector<int, my_allocator<int>>>(vec1, my_allocator<int>{});

            CHECK(std::ranges::equal(vec1, vec2));
        }

        SECTION("view construction")
        {
            auto seq = flux::filter(std::vector{1, 2, 3, 4, 5},
                                  [](int i) { return i % 2 != 0; });

            auto vec2 = flux::to<std::vector<int>>(seq);

            CHECK(check_equal(vec2, {1, 3, 5}));
        }

        SECTION("view construction with allocator")
        {
            auto seq = flux::filter(std::vector{1, 2, 3, 4, 5},
                                  [](int i) { return i % 2 != 0; });

            auto vec2 = flux::to<std::vector<int, my_allocator<int>>>(seq, my_allocator<int>{});

            CHECK(check_equal(vec2, {1, 3, 5}));
        }

        SECTION("fallback construction")
        {
            SECTION("...to vector")
            {
                std::vector<int> vec1{1, 2, 3, 4, 5};
                auto vec2 = single_pass_only(flux::from(vec1)).to<std::vector<int>>();
                CHECK(vec1 == vec2);
            }

            SECTION("...to list")
            {
                std::istringstream iss{"1 2 3 4 5"};
                auto list = flux::from_istream<int>(iss).to<std::list<int>>();
                CHECK(check_equal(list, {1, 2, 3, 4, 5}));
            }

            SECTION("...to set")
            {
                std::istringstream iss{"5 4 3 2 1"};
                auto set = flux::from_istream<int>(iss).to<std::set<int>>();
                CHECK(check_equal(set, {1, 2, 3, 4, 5}));
            }
        }

        SECTION("fallback construction with allocator")
        {
            using A = my_allocator<int>;

            SECTION("...to vector")
            {
                std::vector<int> vec1{1, 2, 3, 4, 5};
                auto vec2 = single_pass_only(flux::from(vec1)).to<std::vector<int, A>>(A{});
                CHECK(std::ranges::equal(vec1, vec2));
            }

            SECTION("...to list")
            {
                std::istringstream iss{"1 2 3 4 5"};
                auto list = flux::from_istream<int>(iss).to<std::list<int, A>>(A{});
                CHECK(check_equal(list, {1, 2, 3, 4, 5}));
            }

            SECTION("...to set")
            {
                std::istringstream iss{"5 4 3 2 1"};
                auto set = flux::from_istream<int>(iss)
                              .to<std::set<int, std::less<>, A>>(A{});
                CHECK(check_equal(set, {1, 2, 3, 4, 5}));
            }
        }
    }
}