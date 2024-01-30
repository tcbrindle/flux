
#include "catch.hpp"

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

#include "test_utils.hpp"

namespace {

template <typename T>
struct span_seq {
    T* ptr_;
    std::size_t sz_;

    struct flux_sequence_traits {
        static constexpr std::size_t first(span_seq const&) { return 0; }
        static constexpr bool is_last(span_seq const& self, std::size_t i) { return i == self.sz_; }
        static constexpr std::size_t& inc(span_seq const&, std::size_t& i) { return ++i; }
        static constexpr T& read_at(span_seq const& self, std::size_t i) { return self.ptr_[i]; }
        static constexpr std::size_t last(span_seq const& self) { return self.sz_; }
        static constexpr std::size_t& dec(span_seq const&, std::size_t& i) { return --i; }
        static constexpr std::size_t& inc(span_seq const&, std::size_t& i, flux::distance_t o)
        {
            return i += static_cast<std::size_t>(o);
        }
        static constexpr flux::distance_t distance(span_seq const&, std::size_t from, std::size_t to)
        {
            return static_cast<flux::distance_t>(to) - static_cast<flux::distance_t>(from);
        }
        static constexpr flux::distance_t size(span_seq const& self) {
            return static_cast<flux::distance_t>(self.sz_);
        }
        static constexpr T* data(span_seq const& self) { return self.ptr_; }
    };
};

template <typename T>
span_seq(T*, size_t) -> span_seq<T>;

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
            return rhs <=> lhs;
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

        flux::zip(std::array{3, 2, 4, 1}, flux::mut_ref(arr))
            .sort(flux::proj(flux::cmp::reverse_compare,
                           [](auto const& elem) { return std::get<0>(elem); }));

        STATIC_CHECK(check_equal(arr, {"charlie"sv, "alpha"sv, "bravo"sv, "delta"sv}));
    }

    return true;
}
static_assert(test_sort_contexpr());


std::mt19937 gen{};

void test_already_sorted(unsigned sz)
{
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, int{0});

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_reverse_sorted(unsigned sz)
{
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, int{0});
    std::reverse(ptr, ptr + sz);

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_randomised(unsigned sz) {
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, int{0});
    std::shuffle(ptr, ptr + sz, gen);

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_all_equal(unsigned sz) {
    auto* ptr = new int[sz];
    std::fill(ptr, ptr + sz, 10);

    flux::sort(span_seq(ptr, sz));

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}

void test_sort(unsigned sz)
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

void test_sort_projected(unsigned sz)
{
    auto* ptr = new Int[sz];
    std::iota(ptr, ptr + sz, Int{0});
    std::shuffle(ptr, ptr + sz, gen);

    flux::sort(span_seq(ptr, sz), flux::proj(flux::cmp::compare, &Int::i));

    CHECK(std::is_sorted(ptr, ptr + sz, [](Int lhs, Int rhs) {
        return lhs.i < rhs.i;
    }));
    delete[] ptr;
}

#ifndef USE_MODULES
void test_heapsort(unsigned sz)
{
    auto* ptr = new int[sz];
    std::iota(ptr, ptr + sz, 0);
    std::shuffle(ptr, ptr + sz, gen);

    auto seq = span_seq(ptr, sz);
    auto cmp = std::ranges::less{};
    flux::detail::make_heap(seq, cmp);
    flux::detail::sort_heap(seq, cmp);

    CHECK(std::is_sorted(ptr, ptr + sz));
    delete[] ptr;
}
#endif

void test_adapted_deque_sort(unsigned sz)
{
    std::deque<std::string> deque(sz);
    std::generate(deque.begin(), deque.end(), [i = 0]() mutable {
        return std::to_string(i++);
    });
    std::shuffle(deque.begin(), deque.end(), gen);

    CHECK(not std::is_sorted(deque.cbegin(), deque.cend())); // seems unlikely, anyway

    flux::from_range(deque)
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

#ifndef USE_MODULES
    // Test our heapsort implementation, because I don't know how to
    // synthesise a test case in which pqdsort hits this
    test_heapsort(0);
    test_heapsort(1);
    test_heapsort(10);
    test_heapsort(100);
    test_heapsort(100'000);
#endif
}