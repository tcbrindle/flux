
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <array>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "test_utils.hpp"

namespace {

template <typename T>
struct my_allocator {
    using value_type = T;
    using is_always_equal = std::true_type;

    my_allocator() = default;

    template <typename U>
    my_allocator(my_allocator<U> const&) {}

    T* allocate(std::size_t n) { return static_cast<T*>(std::malloc(n * sizeof(T))); }
    void deallocate(T* p, std::size_t) { std::free(p); }

    friend constexpr bool operator==(my_allocator const&, my_allocator const) { return true;  }

};

template <typename T, typename A = std::allocator<T>>
struct test_vector {

    using value_type = T;
    using allocator_type = A;

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

template <flux::sequence Seq, typename A>
test_vector(flux::from_sequence_t, Seq&&, A const&) -> test_vector<flux::value_t<Seq>, A>;

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

        SECTION("insert construction")
        {
            SECTION("...to vector")
            {
                std::vector<int> vec1{1, 2, 3, 4, 5};
                auto vec2 = single_pass_only(flux::ref(vec1)).to<std::vector<int>>();
                CHECK(vec1 == vec2);
            }

            SECTION("...to list")
            {
                std::istringstream iss{"1 2 3 4 5"};
                auto list = flux::from_istream<int>(iss).to<std::list<int>>();
                CHECK(check_equal(flux::from_range(list), {1, 2, 3, 4, 5}));
            }

            SECTION("...to set")
            {
                std::istringstream iss{"5 4 3 2 1"};
                auto set = flux::from_istream<int>(iss).to<std::set<int>>();
                CHECK(check_equal(flux::from_range(set), {1, 2, 3, 4, 5}));
            }
        }

        SECTION("insert construction with allocator")
        {
            using A = my_allocator<int>;

            SECTION("...to vector")
            {
                std::vector<int> vec1{1, 2, 3, 4, 5};
                auto vec2 = single_pass_only(flux::ref(vec1)).to<std::vector<int, A>>(A{});
                CHECK(std::ranges::equal(vec1, vec2));
            }

            SECTION("...to list")
            {
                std::istringstream iss{"1 2 3 4 5"};
                auto list = flux::from_istream<int>(iss).to<std::list<int, A>>(A{});
                CHECK(check_equal(flux::from_range(list), {1, 2, 3, 4, 5}));
            }

            SECTION("...to set")
            {
                std::istringstream iss{"5 4 3 2 1"};
                auto set = flux::from_istream<int>(iss)
                              .to<std::set<int, std::less<>, A>>(A{});
                CHECK(check_equal(flux::from_range(set), {1, 2, 3, 4, 5}));
            }
        }

        SECTION("recursive to() calls")
        {
            std::string const str = "The quick brown fox";
            auto vec = flux::split(flux::ref(str), ' ').to<std::vector<std::string>>();

            CHECK(check_equal(vec, {"The", "quick", "brown", "fox"}));
        }

        SECTION("recursive to() calls with allocator")
        {
            using Alloc = my_allocator<std::string>;

            std::string const str = "The quick brown fox";
            auto vec = flux::split(flux::ref(str), ' ').to<std::vector<std::string, Alloc>>(Alloc{});

            using V = decltype(vec);
            static_assert(std::same_as<typename V::allocator_type, Alloc>);

            CHECK(check_equal(vec, {"The", "quick", "brown", "fox"}));
        }

        SECTION("from set_union adaptor")
        {
            auto union_seq = flux::set_union(std::array{1,2,3}, std::array{4,5});
            auto vec = flux::to<std::vector<int>>(union_seq);

            CHECK(check_equal(vec, {1,2,3,4,5}));
        }
    }

    SECTION("...using CTAD")
    {
        SECTION("vector->vector construction")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};
            auto vec2 = flux::to<std::vector>(vec1);

            using V = decltype(vec2);
            static_assert(std::same_as<typename V::value_type, int>);
            static_assert(std::same_as<typename V::allocator_type, std::allocator<int>>);

            CHECK(vec1 == vec2);
        }

        SECTION("vector->vector construction with allocator")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};
            auto vec2 = flux::to<std::vector>(vec1, my_allocator<int>{});

            using V = decltype(vec2);
            static_assert(std::same_as<typename V::value_type, int>);
            static_assert(std::same_as<typename V::allocator_type, my_allocator<int>>);

            CHECK(std::ranges::equal(vec1, vec2));
        }

        SECTION("zipped sequence to map")
        {
            auto zipped = flux::zip(std::array{1, 2, 3},
                                   std::vector<std::string>{"1", "2", "3"});

            auto map = flux::to<std::map>(zipped);

            using M = decltype(map);

            static_assert(std::same_as<typename M::key_type, int>);
            static_assert(std::same_as<typename M::mapped_type, std::string>);

            const auto reqd = std::map<int, std::string>{
                {1, "1"}, {2, "2"}, {3, "3"}
            };

            CHECK(map == reqd);
        }

        SECTION("from_sequence construction")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};

            auto vec2 = flux::to<test_vector>(vec1);

            using V = decltype(vec2);
            static_assert(std::same_as<typename V::value_type, int>);

            CHECK(std::ranges::equal(vec1, vec2));
        }

        SECTION("from_sequence construction with allocator")
        {
            std::vector<int> vec1{1, 2, 3, 4, 5};

            auto vec2 = flux::to<test_vector>(vec1, my_allocator<int>{});

            using V = decltype(vec2);
            static_assert(std::same_as<typename V::value_type, int>);
            static_assert(std::same_as<typename V::allocator_type, my_allocator<int>>);

            CHECK(std::ranges::equal(vec1, vec2));
        }

        SECTION("view construction")
        {
            auto seq = flux::filter(std::vector{1, 2, 3, 4, 5},
                                    [](int i) { return i % 2 != 0; });

            auto vec2 = flux::to<std::vector>(seq);

            using V = decltype(vec2);
            static_assert(std::same_as<typename V::value_type, int>);

            CHECK(check_equal(vec2, {1, 3, 5}));
        }

        SECTION("view construction with allocator")
        {
            auto seq = flux::filter(std::vector{1, 2, 3, 4, 5},
                                    [](int i) { return i % 2 != 0; });

            auto vec2 = flux::to<std::vector>(seq, my_allocator<int>{});

            using V = decltype(vec2);
            static_assert(std::same_as<typename V::value_type, int>);
            static_assert(std::same_as<typename V::allocator_type, my_allocator<int>>);

            CHECK(check_equal(vec2, {1, 3, 5}));
        }

        SECTION("insert construction")
        {
            SECTION("...to vector")
            {
                std::vector<int> vec1{1, 2, 3, 4, 5};
                auto vec2 = single_pass_only(flux::ref(vec1)).to<std::vector>();
                using V = decltype(vec2);
                static_assert(std::same_as<typename V::value_type, int>);
                CHECK(vec1 == vec2);
            }

            SECTION("...to list")
            {
                std::istringstream iss{"1 2 3 4 5"};
                auto list = flux::from_istream<int>(iss).to<std::list>();
                using L = decltype(list);
                static_assert(std::same_as<typename L::value_type, int>);
                CHECK(check_equal(flux::from_range(list), {1, 2, 3, 4, 5}));
            }

            SECTION("...to set")
            {
                std::istringstream iss{"5 4 3 2 1"};
                auto set = flux::from_istream<int>(iss).to<std::set>();
                using S = decltype(set);
                static_assert(std::same_as<typename S::value_type, int>);
                CHECK(check_equal(flux::from_range(set), {1, 2, 3, 4, 5}));
            }
        }
    }
}
