// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cstddef>
#include <iostream>
#include <vector>

flux::generator<std::size_t> primes()
{
    std::vector<std::size_t> history;
    auto is_prime = [&](auto x) { 
        return flux::none(history, [x](auto prime) { return (x % prime) == 0; });
    };

    for (auto p : flux::ints(2).filter(is_prime))
    {
        history.push_back(p);
        co_yield p;
    }
}

int main()
{
    // Prints all prime numbers less than 1000
    primes().take_while(flux::pred::lt(1000)).write_to(std::cout);
    std::cout << std::endl;
}
