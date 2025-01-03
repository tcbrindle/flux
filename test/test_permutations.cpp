
#include "flux/core/concepts.hpp"
#include "flux/core/ref.hpp"
#include "test_utils.hpp"
#include <algorithm>

#ifdef USE_MODULES
import flux;
#else
#    include <flux/adaptor/permutations.hpp>
#endif

namespace {

auto test_permutations_types() -> bool
{
    auto arr = std::array {1, 2, 3};

    auto seq = flux::mut_ref(arr).permutations();

    using SeqType = decltype(seq);
    using CurType = flux::cursor_t<SeqType>;

    // Sequence
    static_assert(flux::sequence<SeqType>);
    static_assert(flux::multipass_sequence<SeqType>);
    static_assert(flux::bidirectional_sequence<SeqType>);
    static_assert(flux::bounded_sequence<SeqType>);
    static_assert(flux::sized_sequence<SeqType>);
    static_assert(not flux::infinite_sequence<SeqType>);
    static_assert(not flux::random_access_sequence<SeqType>);

    // Cursors
    static_assert(flux::regular_cursor<CurType>);
    static_assert(flux::ordered_cursor<CurType>);

    // Elements
    static_assert(std::same_as<flux::element_t<SeqType>, std::vector<int>>);
    static_assert(std::same_as<flux::value_t<SeqType>, std::vector<int>>);

    return true;
}

constexpr auto test_permutations() -> bool
{
    // Simple Array Comparison
    {
        auto arr = std::array {1, 2, 3};
        auto seq = flux::mut_ref(arr).permutations();

        // Sizes
        STATIC_CHECK(seq.size() == 6);

        // Values
        auto cur = flux::first(seq);
        auto test_comp = std::array {1, 2, 3};

        while (not flux::is_last(seq, cur)) {
            if (not check_equal(flux::read_at(seq, cur), test_comp)) {
                return false;
            }
            flux::inc(seq, cur);
            std::ranges::next_permutation(test_comp);
        }
    }

    return true;
}

} // namespace

TEST_CASE("permutations")
{
    bool types = test_permutations_types();
    REQUIRE(types);

    bool functionality = test_permutations();
    REQUIRE(functionality);
}
