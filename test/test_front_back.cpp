
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <string>

#include "test_utils.hpp"

namespace {

constexpr bool test_front()
{
    // Non-member, non-empty
    {
        int arr[] = {1, 2, 3};

        auto opt = flux::front(arr);
        static_assert(std::same_as<decltype(opt), flux::optional<int&>>);

        STATIC_CHECK(opt.has_value());
        STATIC_CHECK(opt.value() == 1);
    }

    // Non-member, empty
    {
        std::array<double, 0> const arr{};

        auto opt = flux::front(arr);
        static_assert(std::same_as<decltype(opt), flux::optional<double const&>>);

        STATIC_CHECK(not opt.has_value());
    }

    // Member, non-const, non-empty
    {
        auto seq = flux::from(std::array{1, 2, 3});

        auto opt = seq.front();
        static_assert(std::same_as<decltype(opt), flux::optional<int&>>);

        STATIC_CHECK(opt.has_value());
        STATIC_CHECK(opt.value() == 1);
    }

    // Member, const, non-empty
    {
        auto const seq = flux::from(std::array{1, 2, 3});

        auto opt = seq.front();
        static_assert(std::same_as<decltype(opt), flux::optional<int const&>>);

        STATIC_CHECK(opt.has_value());
        STATIC_CHECK(opt.value() == 1);
    }

    // Member, non-const, empty
    {
        auto e = flux::empty<std::string>;

        auto opt = e.front();
        static_assert(std::same_as<decltype(opt), flux::optional<std::string&>>);

        STATIC_CHECK(not opt.has_value());
    }

    // Member, const, empty
    {
        auto const seq = flux::from(std::array<int, 0>{});

        auto opt = seq.front();
        static_assert(std::same_as<decltype(opt), flux::optional<int const&>>);

        STATIC_CHECK(not opt.has_value());
    }

    // Cannot call front() on a single-pass sequence
    {
         auto seq = single_pass_only(flux::from(std::array{1, 2, 3}));

         static_assert(not std::invocable<decltype(flux::front), decltype(seq)&>);

         static_assert(not std::invocable<decltype(flux::front), decltype(seq) const&>);

         auto member_front = [](auto&& s) -> decltype(FLUX_FWD(s).front()) {
             return s.front();
         };

         static_assert(not std::invocable<decltype(member_front), decltype(seq)&>);
         static_assert(not std::invocable<decltype(member_front), decltype(seq) const&>);
    }

    return true;
}
static_assert(test_front());

constexpr bool test_back()
{
    // Non-member, non-empty
    {
         int arr[] = {1, 2, 3};

         auto opt = flux::back(arr);
         static_assert(std::same_as<decltype(opt), flux::optional<int&>>);

         STATIC_CHECK(opt.has_value());
         STATIC_CHECK(opt.value() == 3);
    }

    // Non-member, empty
    {
         std::array<double, 0> const arr{};

         auto opt = flux::back(arr);
         static_assert(std::same_as<decltype(opt), flux::optional<double const&>>);

         STATIC_CHECK(not opt.has_value());
    }

    // Member, non-const, non-empty
    {
         auto seq = flux::from(std::array{1, 2, 3});

         auto opt = seq.back();
         static_assert(std::same_as<decltype(opt), flux::optional<int&>>);

         STATIC_CHECK(opt.has_value());
         STATIC_CHECK(opt.value() == 3);
    }

    // Member, const, non-empty
    {
         auto const seq = flux::from(std::array{1, 2, 3});

         auto opt = seq.back();
         static_assert(std::same_as<decltype(opt), flux::optional<int const&>>);

         STATIC_CHECK(opt.has_value());
         STATIC_CHECK(opt.value() == 3);
    }

    // Member, non-const, empty
    {
         auto e = flux::empty<std::string>;

         auto opt = e.back();
         static_assert(std::same_as<decltype(opt), flux::optional<std::string&>>);

         STATIC_CHECK(not opt.has_value());
    }

    // Member, const, empty
    {
         auto const seq = flux::from(std::array<int, 0>{});

         auto opt = seq.back();
         static_assert(std::same_as<decltype(opt), flux::optional<int const&>>);

         STATIC_CHECK(not opt.has_value());
    }

    // Cannot call back() on a multipass (only) sequence
    {
         auto seq = flux::split_string(std::string_view("hello world"), ' ');

         static_assert(not std::invocable<decltype(flux::back), decltype(seq)&>);
         static_assert(not std::invocable<decltype(flux::back), decltype(seq) const&>);

         auto member_back = [](auto&& s) -> decltype(FLUX_FWD(s).back()) {
             return s.front();
         };

         static_assert(not std::invocable<decltype(member_back), decltype(seq)&>);
         static_assert(not std::invocable<decltype(member_back), decltype(seq) const&>);
    }

    // Cannot call back() on a non-bounded sequence
    {
         auto seq = flux::take_while(std::array{1, 2, 3}, [](auto) { return true; });

         static_assert(not std::invocable<decltype(flux::back), decltype(seq)&>);
         static_assert(not std::invocable<decltype(flux::back), decltype(seq) const&>);

         auto member_back = [](auto&& s) -> decltype(FLUX_FWD(s).back()) {
             return s.front();
         };

         static_assert(not std::invocable<decltype(member_back), decltype(seq)&>);
         static_assert(not std::invocable<decltype(member_back), decltype(seq) const&>);
    }

    return true;
}
static_assert(test_back());

}

TEST_CASE("front")
{
    bool res = test_front();
    REQUIRE(res);
}

TEST_CASE("back")
{
    bool res = test_back();
    REQUIRE(res);
}