
// Copyright (c) 2021 Barry Revzin
// Copyright (c) 2023 NVIDIA Corporation
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <nanobench.h>

#include <flux.hpp>

#include <numeric>

namespace an = ankerl::nanobench;

extern void memset_2d_reference(double* A, std::size_t N, std::size_t M);

extern void memset_2d_flux_cartesian_product_iota(double* A, std::size_t N, std::size_t M);

int main(int argc, char** argv)
{
    int const n_iters = argc > 1 ? std::atoi(argv[1]) : 40;

    constexpr std::size_t N = 1024;
    constexpr std::size_t M = 2048;
    std::vector<double> A(N * M);

    {
        auto bench = an::Bench().minEpochIterations(n_iters).relative(true);

        std::iota(A.begin(), A.end(), 0);

        bench.run("memset_2d_handwritten",
            [&] { memset_2d_reference(A.data(), N, M); });

        if (auto it = std::ranges::find_if_not(A, [&] (auto e) { return e == 0; }); it != A.end())
            throw false;

        std::iota(A.begin(), A.end(), 0);

        bench.run("memset_2d_flux_cartesian_product_iota",
            [&] { memset_2d_flux_cartesian_product_iota(A.data(), N, M); });

        if (auto it = std::ranges::find_if_not(A, [&] (auto e) { return e == 0; }); it != A.end())
            throw false;
    }
}
