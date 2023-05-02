// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cstddef>
#include <ios>
#include <iostream>
#include <ostream>

struct stats_t
{
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t chars = 0;
    bool is_last_space = true;
};

auto collect_stats = [](flux::sequence auto&& seq)
{
    return flux::fold(FLUX_FWD(seq), [](stats_t&& stats, auto val) 
    {
        stats.chars++;
        if (not stats.is_last_space and std::isspace(val))
            stats.words++;
        if (val == '\n')
            stats.lines++;
        stats.is_last_space = std::isspace(val);
        return FLUX_FWD(stats);
    }, stats_t{});
};

std::ostream& operator<<(std::ostream& s, stats_t stats)
{
    return s << stats.lines << ' ' << stats.words << ' ' << stats.chars;
}

int main()
{
    // Print newline, word, and byte counts from std input (like gnu wc)
    std::noskipws(std::cin);
    auto result = flux::from_istream<char>(std::cin)._(collect_stats);
    if (not result.is_last_space)
        result.words++;
    std::cout << result << std::endl;
}