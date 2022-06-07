
#include "catch.hpp"

//#define FLUX_ENABLE_BOUNDS_CHECKING
#include <flux.hpp>

#include "test_utils.hpp"

#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <string_view>


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

constexpr bool test_sort_contexpr()
{
    {
        int arr[] = {9, 7, 5, 3, 1, 4, 6, 8, 0, 2};
        flux::sort(arr);
        STATIC_CHECK(std::is_sorted(arr, arr + 10));
    }

    {
        using namespace std::string_view_literals;
        std::array arr = {
            "delta"sv,
            "charlie"sv,
            "alpha"sv,
            "bravo"sv
        };

        flux::sort(arr, [](auto lhs, auto rhs) {
            return rhs < lhs;
        });

        STATIC_CHECK(std::is_sorted(arr.crbegin(), arr.crend()));
    }

    {
        using namespace std::string_view_literals;
        std::array arr = {
            "alpha"sv,
            "bravo"sv,
            "charlie"sv,
            "delta"sv
        };

        flux::zip(std::array{3, 2, 4, 1}, arr)
            .sort(std::ranges::greater{},
                  [](auto const& elem) { return std::get<0>(elem); });

        STATIC_CHECK(check_equal(arr, {"charlie"sv, "alpha"sv, "bravo"sv, "delta"sv}));
    }

    return true;
}
static_assert(test_sort_contexpr());


std::mt19937 gen{};

void test_already_sorted(int sz)
{
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, int{0});

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_reverse_sorted(int sz)
{
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, int{0});
    std::reverse(ptr, ptr + sz);

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_randomised(int sz) {
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, int{0});
    std::shuffle(ptr, ptr + sz, gen);

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_all_equal(int sz) {
    auto* ptr = new int[sz];
    std::fill(ptr, ptr + sz, 10);

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_sort(int sz)
{
    test_already_sorted(sz);
    test_reverse_sorted(sz);
    test_randomised(sz);
    test_all_equal(sz);
}

struct Int {
    int i;
    Int& operator++() { ++i; return *this; }
};

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

void test_heapsort(int sz)
{
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, 0);
    std::shuffle(ptr, ptr + sz, gen);

    auto seq = span_seq(ptr, sz);
    auto cmp = std::ranges::less{};
    auto proj = std::identity{};
    flux::detail::make_heap(seq, cmp, proj);
    flux::detail::sort_heap(seq, cmp, proj);

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_adapted_deque_sort(int sz)
{
    std::deque<std::string> deque(sz);
    std::generate(deque.begin(), deque.end(), [i = 0]() mutable {
        return std::to_string(i++);
    });
    std::shuffle(deque.begin(), deque.end(), gen);

    CHECK(not std::is_sorted(deque.cbegin(), deque.cend())); // seems unlikely, anyway

    flux::from(deque)
        .take(sz/2)
        .sort();

    CHECK(std::is_sorted(deque.cbegin(), deque.cbegin() + sz/2));
}

}

TEST_CASE("sort")
{
    CHECK(test_sort_contexpr());

    test_sort(0);
    test_sort(1);
    test_sort(10);
    test_sort(100);
    test_sort(1'000);
    test_sort(10'000);
    test_sort(100'000);
    test_sort(1'000'000);

    test_sort_projected(0);
    test_sort_projected(1);
    test_sort_projected(10);
    test_sort_projected(100);
    test_sort_projected(100'000);

    test_adapted_deque_sort(100'000);

    // Test our heapsort implementation, because I don't know how to
    // synthesise a test case in which pqdsort hits this
    test_heapsort(0);
    test_heapsort(1);
    test_heapsort(10);
    test_heapsort(100);
    test_heapsort(100'000);
}