
#include "nanobench.h"

#include <flux.hpp>

#include <algorithm>
#include <numeric>
#include <random>

namespace an = ankerl::nanobench;

static constexpr int test_sz = 100'000;

template <typename SortFn, typename Vec>
static void test_sort(const char* name, SortFn& sort, const Vec& vec, an::Bench& bench)
{
    bench.run(name, [&] {
        Vec copy = vec;
        sort(copy);
        bench.doNotOptimizeAway(copy);
    });
}

int main()
{
    {
        std::vector<int> vec(test_sz);
        std::mt19937 gen{std::random_device{}()};
        std::uniform_int_distribution dist(0, test_sz);
        std::generate(vec.begin(), vec.end(), [&] { return dist(gen); });

        auto bench = an::Bench().relative(true).minEpochIterations(10);

        test_sort("random ints (std)", std::ranges::sort, vec, bench);
        test_sort("random ints (flux)", flux::sort, vec, bench);
    }

    {
        std::vector<int> vec(test_sz);
        std::iota(vec.begin(), vec.end(), 0);
        auto bench = an::Bench().relative(true).minEpochIterations(10);
        test_sort("sorted ints (std)", std::ranges::sort, vec, bench);
        test_sort("sorted ints (flux)", flux::sort, vec, bench);
    }

    {
        std::vector<int> vec(test_sz);
        std::iota(vec.begin(), vec.end(), 0);
        std::ranges::reverse(vec);
        auto bench = an::Bench().relative(true).minEpochIterations(10);
        test_sort("reverse sorted ints (std)", std::ranges::sort, vec, bench);
        test_sort("reverse sorted ints (flux)", flux::sort, vec, bench);
    }

    {
        std::vector<int> vec(test_sz);
        std::iota(vec.begin(), vec.begin() + test_sz/2, 0);
        std::iota(vec.begin() + test_sz/2, vec.end(), 0);
        std::reverse(vec.begin() + test_sz/2, vec.end());

        auto bench = an::Bench().relative(true).minEpochIterations(10);
        test_sort("organpipe ints (std)", std::ranges::sort, vec, bench);
        test_sort("organpipe ints (flux)", flux::sort, vec, bench);
    }

    {
        std::vector<double> vec(test_sz);
        std::mt19937 gen{std::random_device{}()};
        std::uniform_real_distribution<double> dist(0, test_sz);
        std::generate(vec.begin(), vec.end(), [&] { return dist(gen); });

        auto bench = an::Bench().relative(true).minEpochIterations(10);

        test_sort("random doubles (std)", std::ranges::sort, vec, bench);
        test_sort("random doubles (flux)", flux::sort, vec, bench);
    }

    {
        std::vector<std::string> vec(test_sz);
        std::mt19937 gen{std::random_device{}()};
        std::uniform_int_distribution dist(0, test_sz);
        std::generate(vec.begin(), vec.end(), [&] { return std::to_string(dist(gen)); });

        auto bench = an::Bench().relative(true);

        test_sort("random strings (std)", std::ranges::sort, vec, bench);
        test_sort("random strings (flux)", flux::sort, vec, bench);
    }
}