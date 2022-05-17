
#include "catch.hpp"

//#define FLUX_ENABLE_BOUNDS_CHECKING
#include <flux.hpp>

#include "test_utils.hpp"

#include <memory>
#include <random>
#include <algorithm>
#include <functional>
#include <iostream>
#include <span>
#include <numeric>

namespace {

template <typename T>
struct span_seq {
    T* ptr_;
    std::size_t sz_;

    static constexpr std::size_t first() { return 0; }
    constexpr bool is_last(std::size_t i) const { return i == sz_; }
    constexpr std::size_t& inc(std::size_t& i) const { return ++i; }
    constexpr T& read_at(std::size_t i) const { return ptr_[i]; }
    constexpr std::size_t last() const { return sz_; }
    static constexpr std::size_t& dec(std::size_t& i) { return --i; }
    static constexpr std::size_t& inc(std::size_t& i, std::ptrdiff_t o) { return i += o; }
    static constexpr std::ptrdiff_t distance(std::size_t from, std::size_t to)
    {
        return static_cast<std::ptrdiff_t>(to) - static_cast<std::ptrdiff_t>(from);
    }
    constexpr std::size_t size() { return sz_; }
    constexpr T* data() const { return ptr_; }
};

std::mt19937 gen{};

struct Int {
    int i;
    Int& operator++() { ++i; return *this; }
};

void test_sort(int sz) {
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, int{0});
    std::shuffle(ptr, ptr + sz, gen);

    flux::sort(span_seq(ptr, sz));
//    std::ranges::sort(std::span(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_sort_projected(int sz)
{
    auto* ptr = new Int[sz];
    std::iota(ptr, ptr + sz, Int{0});
    std::shuffle(ptr, ptr + sz, gen);

    flux::sort(span_seq(ptr, sz), {}, &Int::i);

    CHECK(std::is_sorted(ptr, ptr + sz, [](Int lhs, Int rhs) {
        return lhs.i < rhs.i;
    }));
    delete[] ptr;
}

}

TEST_CASE("sort")
{
    test_sort(0);
    test_sort(1);
    test_sort(10);
    test_sort(100);
    test_sort(1'000);
    test_sort(10'000);
    test_sort(100'000);
    test_sort(1'000'000);
 //   test_sort(10'000'000);

    test_sort_projected(0);
    test_sort_projected(1);
    test_sort_projected(10);
    test_sort_projected(100);
    test_sort_projected(100'000);
}