
// Copyright (c) 2021 Barry Revzin
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <nanobench.h>

#include <flux.hpp>

#include <numeric>
#include <ranges>
#include <iostream>

namespace an = ankerl::nanobench;

// Kernels are placed in a separate translation unit to prevent compilers from
// optimizing them based on the input that we'll be giving them and to make it
// easier to study their compiled assembly.
extern void memset_2d_reference(double* A, flux::distance_t N, flux::distance_t M);
extern void memset_2d_std_cartesian_product_iota(double* A, flux::distance_t N, flux::distance_t M);
extern void memset_2d_flux_cartesian_product_iota(double* A, flux::distance_t N, flux::distance_t M);
extern void memset_diagonal_2d_reference(double* A, flux::distance_t N, flux::distance_t M);
extern void memset_diagonal_2d_std_cartesian_product_iota_filter(double* A, flux::distance_t N, flux::distance_t M);
extern void memset_diagonal_2d_flux_cartesian_product_iota_filter(double* A, flux::distance_t N, flux::distance_t M);

int main(int argc, char** argv)
{
    int const n_iters = argc > 1 ? std::atoi(argv[1]) : 40;

    constexpr flux::distance_t N = 1024;
    constexpr flux::distance_t M = 2048;
    std::vector<double> A(N * M);

    const auto run_benchmark =
    [] (auto& bench, auto& A, auto N, auto M, auto name, auto func, auto check) {
        std::iota(A.begin(), A.end(), 0);
        bench.run(name, [&] { func(A.data(), N, M); });
        check(A, N, M);
    };

    {
        const auto check_2d = [] (auto& A, auto N, auto M) {
            const auto it = std::ranges::find_if_not(A, [&] (auto e) { return e == 0.0; });
            if (it != A.end())
                throw false;
        };

        auto bench = an::Bench()
            .minEpochIterations(n_iters)
            .relative(true)
            .performanceCounters(false);

        const auto run_2d_benchmark_impl = [&] (auto name, auto func) {
            run_benchmark(bench, A, N, M, name, func, check_2d);
        };

        #define run_2d_benchmark(func) run_2d_benchmark_impl(#func, func)

        run_2d_benchmark(memset_2d_reference);
        run_2d_benchmark(memset_2d_std_cartesian_product_iota);
        run_2d_benchmark(memset_2d_flux_cartesian_product_iota);
    }

    {
        const auto check_diagonal_2d = [] (auto& A, auto N, auto M) {
            for (auto i : std::views::iota(0, N))
                for (auto j : std::views::iota(0, M)) {
                    if (i == j) {
                        if (A[i * M + j] != 0.0) throw false;
                    } else {
                        if (A[i * M + j] != i * M + j) throw false;
                    }
                }
        };

        auto bench = an::Bench()
            .minEpochIterations(n_iters)
            .relative(true)
            .performanceCounters(false);

        const auto run_diagonal_2d_benchmark_impl = [&] (auto name, auto func) {
            run_benchmark(bench, A, N, M, name, func, check_diagonal_2d);
        };

        #define run_diagonal_2d_benchmark(func) run_diagonal_2d_benchmark_impl(#func, func)

        run_diagonal_2d_benchmark(memset_diagonal_2d_reference);
        run_diagonal_2d_benchmark(memset_diagonal_2d_std_cartesian_product_iota_filter);
        run_diagonal_2d_benchmark(memset_diagonal_2d_flux_cartesian_product_iota_filter);
    }
}
