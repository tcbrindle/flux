// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cstddef>
#include <ostream>
#include <string>
#include <random>
#include <iostream>
#include <iomanip>
#include <map>

template<typename T>
flux::generator<int> randu(T min, T max)
{
    std::mt19937 rng;
    std::uniform_int_distribution dist(min, max);

    while (true)
        co_yield dist(rng);
}

template<typename T>
flux::generator<int> randn(T mean, T stddev = 1.0)
{
    std::random_device rng;
    std::normal_distribution dist(mean, stddev);

    while (true)
        co_yield std::round(dist(rng));
}

using hist_t = std::map<int, int>;

auto to_histogram = [](hist_t&& so_far, auto x)
{
    ++so_far[x];
    return std::move(so_far);
};

void print_histogram(const hist_t& hist)
{
    for (auto [bin, count]: hist){
        std::cout << std::setw(2) << bin << ' ' << std::string(count / 200, '*') << '\n';
    }
};

int main()
{
    std::cout << "Uniform distribution from 0 to 10\n";
    print_histogram(randu(0, 10).take(10000).fold(to_histogram, hist_t{}));
    std::cout << '\n';

    std::cout << "Normal distribution with mean 5 and stddev 2\n";
    print_histogram(randn(5.0, 2.0).take(10000).fold(to_histogram, hist_t{}));
    std::cout << '\n';
}