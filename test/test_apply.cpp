
#include <array>
#include <algorithm>

#include "test_utils.hpp"

// We don't have an in-place rotate yet, so let's borrow the STL's
auto rotate_by = []<flux::random_access_sequence Seq>(Seq&& seq, flux::distance_t places)
    -> Seq&&
{
    auto const sz = flux::size(seq);

    while (places < 0) {
        places += sz;
    }
    while (places >= sz) {
        places -= sz;
    }

    std::ranges::rotate(seq, seq.begin() + places);

    return FLUX_FWD(seq);
};


constexpr bool test_apply()
{
    // Checking lvalue sequence
    {
        auto seq = flux::from(std::array{1, 2, 3, 4, 5});
        auto& r = seq._(rotate_by, -1);
        STATIC_CHECK(&r == &seq);
        STATIC_CHECK(check_equal(seq, {5, 1, 2, 3, 4}));
    }

    // Checking rvalue sequence
    {
        auto seq = flux::from(std::array{1, 2, 3, 4, 5})._(rotate_by, -1).take(3);
        STATIC_CHECK(check_equal(seq, {5, 1, 2}));
    }

    // Checking const [l|r]value
    {
        auto sum = [](auto& seq) {
            int s = 0;
            FLUX_FOR(int i, seq) { s += i; }
            return s;
        };

        auto const seq = flux::from(std::array{1, 2, 3, 4, 5});
        STATIC_CHECK(seq._(sum) == 15);

        STATIC_CHECK(std::move(seq)._(sum) == 15);
    }

    return true;
}
static_assert(test_apply());

TEST_CASE("apply")
{
    REQUIRE(test_apply());
}