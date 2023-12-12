
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <array>
#include <cassert>
#include <string_view>
#include <vector>
#include <iostream>

using namespace std::string_view_literals;

int main()
{
    using flux::equal;

    // We can split a sequence using a single delimiter
    auto seq1 = flux::split("here are some words"sv, ' ');
    assert(equal(seq1, std::array{"here"sv, "are"sv, "some"sv, "words"sv}));


    // Consecutive delimiters will result in empty subsequences in the output
    auto seq2 = flux::split("some,,,commas"sv, ',');
    assert(equal(seq2, std::array{"some"sv, ""sv, ""sv, "commas"sv}));


    // If the sequence ends with a delimiter, the final subsequence will be empty
    auto seq3 = flux::split("Two. Sentences."sv, '.');
    assert(equal(seq3, std::array{"Two"sv, " Sentences"sv, ""sv}));


    // We can also split a sequence with a pattern
    auto seq4 = flux::split(std::vector{1, 2, 3, 4, 5}, std::array{2, 3});
    assert(equal(seq4, std::vector{std::vector{1}, std::vector{4, 5}}));


    // Repeated, non-overlapping patterns result in empty subsequences
    auto seq5 = flux::split("Hello!!!!World"sv, "!!"sv);
    assert(equal(seq5, std::array{"Hello"sv, ""sv, "World"sv}));


    // Overlapping patterns are only matched once
    auto seq6 = flux::split("Hello!!!World"sv, "!!"sv);
    assert(equal(seq6, std::array{"Hello"sv, "!World"sv}));


    // If the sequence begins with the pattern, the first subsequence will
    // be empty...
    auto seq7 = flux::split("!!Hello"sv, "!!"sv);
    assert(equal(seq7, std::array{""sv, "Hello"sv}));


    // ... and likewise if it ends with the pattern
    auto seq8 = flux::split("Hello!!"sv, "!!"sv);
    assert(equal(seq8, std::array{"Hello"sv, ""sv}));


    // Lastly, we can split using a predicate function
    auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

    auto seq9 = flux::split("These1are2some3words"sv, is_digit);
    assert(equal(seq9, std::array{"These"sv, "are"sv, "some"sv, "words"sv}));


    // As usual, consecutive "true" elements in the input will produce
    // empty subsequences in the output
    auto seq10 = flux::split("A123B"sv, is_digit);
    assert(equal(seq10, std::array{"A"sv, ""sv, ""sv, "B"sv}));


    // It can be useful combine splitting with a "not empty" filter
    auto is_space = [](char c) { return std::isspace(static_cast<unsigned char>(c)); };

    auto seq11 = flux::split("Alpha  Bravo\t\rCharlie \n"sv, is_space)
                        .filter(std::not_fn(flux::is_empty));
    assert(equal(seq11, std::array{"Alpha"sv, "Bravo"sv, "Charlie"sv}));
}