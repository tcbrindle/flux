// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cstddef>
#include <iostream>
#include <vector>

struct interval_t
{
    std::size_t begin;
    std::size_t end;
};

std::ostream& operator<<(std::ostream& s, const interval_t& i)
{
    return s << '(' << i.begin << ',' << i.end << ')';
}

bool is_overlapped(interval_t a, interval_t b)
{
    return a.end >= b.begin;
}

auto merge = [](flux::sequence auto seq) -> interval_t
{
    auto begin = flux::front(seq)->begin;
    auto end = flux::max(seq, flux::proj(std::less<>{}, &interval_t::end))->end;
    return {begin, end};
};

int main()
{
    std::vector<interval_t> intervals = {{2, 4}, {7, 9}, {11, 13}, {6, 7}, {0, 3}};

    // sort intervals according to begin
    flux::sort(intervals, flux::proj(std::less{}, &interval_t::begin));

    flux::from(intervals)
         .chunk_by(is_overlapped)
         .map(merge)
         .write_to(std::cout);
                
    std::cout << std::endl;

    // prints [(0,4), (6,9), (11,13)]
}