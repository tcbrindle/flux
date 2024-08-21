
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <string>

#include "test_utils.hpp"

TEST_CASE("istreambuf")
{
    {
        std::istringstream iss("hello world");

        auto& seq = *iss.rdbuf();

        using S = decltype(seq);

        static_assert(flux::sequence<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);

        static_assert(std::same_as<flux::element_t<S>, char>);
        static_assert(std::same_as<flux::rvalue_element_t<S>, char>);
        static_assert(std::same_as<flux::value_t<S>, char>);

        std::string str;
        FLUX_FOR(char c, seq) {
            str.push_back(c);
        }

        REQUIRE(str == "hello world");
    }

    {
        std::wistringstream iss(L"hello world");

        auto seq = flux::from_istreambuf(iss.rdbuf());

        static_assert(flux::sequence<decltype(seq)>);
        static_assert(std::same_as<flux::element_t<decltype(seq)>, wchar_t>);

        std::wstring str;
        FLUX_FOR(auto c, seq) {
            str.push_back(c);
        }

        REQUIRE(str == L"hello world");
    }

    {
        std::basic_istringstream<char32_t> iss(U"hello world");

        auto seq = flux::from_istreambuf(iss);

        static_assert(flux::sequence<decltype(seq)>);
        static_assert(std::same_as<flux::element_t<decltype(seq)>, char32_t>);

        std::u32string str;
        FLUX_FOR(auto c, seq) {
            str.push_back(c);
        }

        REQUIRE(str == U"hello world");
    }

    // Make sure assertion fires
    {
        std::basic_streambuf<char, std::char_traits<char>>* ptr = nullptr;
        REQUIRE_THROWS_AS(flux::from_istreambuf(ptr), flux::unrecoverable_error);
    }
}