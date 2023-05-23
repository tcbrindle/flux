
// Based on https://github.com/brevzin/rivers/blob/main/bench/benchmark.cxx
// Copyright (c) 2021 Barry Revzin
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <nanobench.h>

#include <flux.hpp>

#include "ranges_concat.hpp"

#include <algorithm>
#include <cstdlib>
#include <numeric>

namespace an = ankerl::nanobench;

int main(int argc, char** argv)
{
    int const n_iters = argc > 1 ? std::atoi(argv[1]) : 200;

    std::vector<int> bunch_of_ints(1'000'000);
    std::iota(bunch_of_ints.begin(), bunch_of_ints.end(), 0);

    auto is_even = [](int x) { return x % 2 == 0; };
    auto triple = [](int x) { return 3 * x; };

    {
        auto bench = an::Bench().minEpochIterations(n_iters).relative(true);

        bench.run("transform_filter_handwritten", [&] {
            int res = 0;
            for (int i : bunch_of_ints) {
                i = triple(i);
                if (is_even(i)) { res += i; }
            }
            an::doNotOptimizeAway(res);
        });

        bench.run("transform_filter_ranges", [&] {
            namespace rv = std::views;
            auto r =
                bunch_of_ints | rv::transform(triple) | rv::filter(is_even);

            int res = 0;
            for (int i : r) { res += i; }
            an::doNotOptimizeAway(res);
        });

        bench.run("transform_filter_flux", [&] {
            auto r = flux::ref(bunch_of_ints).map(triple).filter(is_even);
            int res = r.sum();
            an::doNotOptimizeAway(res);
        });
    }

    auto moar_ints = bunch_of_ints;
    std::reverse(moar_ints.begin(), moar_ints.end());

    {
        auto bench = an::Bench().minEpochIterations(n_iters).relative(true);

        bench.run("concat_handwritten", [&] {
            int res = 0;
            for (int i : bunch_of_ints) { res += i; }
            for (int i : moar_ints) { res += i; }
            an::doNotOptimizeAway(res);
        });

        bench.run("concat_ranges", [&] {
            namespace rv = std::views;
            auto r = rv::concat(bunch_of_ints, moar_ints);
            int res = 0;
            for (int i : r) { res += i; }
            an::doNotOptimizeAway(res);
        });

        bench.run("concat_flux", [&] {
            auto r =
                flux::chain(flux::ref(bunch_of_ints), flux::ref(moar_ints));
            int res = r.sum();
            an::doNotOptimizeAway(res);
        });
    }

    {
        auto bench = an::Bench().minEpochIterations(n_iters).relative(true);

        bench.run("concat_take_transform_filter_handwritten", [&] {
            int res = 0;
            int take = 1'500'000;
            for (int i : bunch_of_ints) {
                if (take == 0) { break; }
                --take;
                i = triple(i);
                if (is_even(i)) { res += i; }
            }
            for (int i : moar_ints) {
                if (take == 0) { break; }
                --take;
                i = triple(i);
                if (is_even(i)) { res += i; }
            }
            an::doNotOptimizeAway(res);
        });

        bench.run("concat_take_transform_filter_ranges", [&] {
            namespace rv = std::views;
            auto r = rv::concat(bunch_of_ints, moar_ints) |
                     rv::take(1'500'000) | rv::transform(triple) |
                     rv::filter(is_even);
            int res = 0;
            for (int i : r) { res += i; }
            an::doNotOptimizeAway(res);
        });

        bench.run("concat_take_transform_filter_flux", [&] {
            int res =
                flux::chain(flux::ref(bunch_of_ints), flux::ref(moar_ints))
                    .take(1'500'000)
                    .map(triple)
                    .filter(is_even)
                    .sum();
            an::doNotOptimizeAway(res);
        });
    }
}