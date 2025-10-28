
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

        static_assert(flux::iterable<S>);
        static_assert(not flux::multipass_sequence<S>);
        static_assert(not flux::sized_sequence<S>);
        static_assert(not flux::bounded_sequence<S>);

        static_assert(std::same_as<flux::iterable_element_t<S>, char>);
        static_assert(std::same_as<flux::iterable_value_t<S>, char>);

        std::string str;
        flux::for_each(seq, [&](char c) { str.push_back(c); });

        REQUIRE(str == "hello world");
    }

    {
        std::wistringstream iss(L"hello world");

        auto& seq = *iss.rdbuf();

        static_assert(flux::iterable<decltype(seq)>);
        static_assert(std::same_as<flux::iterable_element_t<decltype(seq)>, wchar_t>);

        std::wstring str;
        flux::for_each(seq, [&](wchar_t c) { str.push_back(c); });

        REQUIRE(str == L"hello world");
    }

    {
        std::basic_istringstream<char32_t> iss(U"hello world");

        auto& seq = *iss.rdbuf();

        static_assert(flux::iterable<decltype(seq)>);
        static_assert(std::same_as<flux::iterable_element_t<decltype(seq)>, char32_t>);

        std::u32string str;
        flux::for_each(seq, [&](char32_t c) { str.push_back(c); });

        REQUIRE(str == U"hello world");
    }

    SUBCASE("take(5) with streambuf")
    {
        std::istringstream iss("123456789");

        auto& buf = *iss.rdbuf();

        auto taken = flux::mut_ref(buf).take(5);

        std::string str;
        flux::for_each(taken, [&](char c) { str.push_back(c); });

        REQUIRE(str == "12345");
        REQUIRE(buf.sgetc() == '6');
    }
}