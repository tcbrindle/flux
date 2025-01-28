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

    auto seq = flux::mut_ref(arr).permutations<3>();

    using SeqType = decltype(seq);
    using CurType = flux::cursor_t<SeqType>;

    // Sequence
    static_assert(flux::sequence<SeqType>);
    static_assert(flux::multipass_sequence<SeqType>);
    static_assert(flux::bounded_sequence<SeqType>);
    static_assert(flux::sized_sequence<SeqType>);
    static_assert(not flux::infinite_sequence<SeqType>);
    static_assert(not flux::random_access_sequence<SeqType>);
    static_assert(not flux::contiguous_sequence<SeqType>);

    // Cursors
    static_assert(flux::regular_cursor<CurType>);
    static_assert(flux::ordered_cursor<CurType>);

    // Elements
    static_assert(std::same_as<flux::element_t<SeqType>, std::vector<int>>);
    static_assert(std::same_as<flux::value_t<SeqType>, std::vector<int>>);
    static_assert(flux::random_access_sequence<flux::element_t<SeqType>>);
    static_assert(flux::random_access_sequence<flux::value_t<SeqType>>);

    return true;
}

constexpr auto test_permutations() -> bool
{
    // Simple Array Comparison
    {
        auto arr = std::array {1, 2, 3};
        auto seq = flux::mut_ref(arr).permutations<3>();

        // Sizes
        STATIC_CHECK(seq.size() == 6);

        auto cur = flux::first(seq);
        auto test_comp = std::array {1, 2, 3};

        // Forward Iteration Permutations
        while (not flux::is_last(seq, cur)) {
            STATIC_CHECK(check_equal(flux::read_at(seq, cur), test_comp));

            flux::inc(seq, cur);
            std::ranges::next_permutation(test_comp);
        }
    }

    return true;
}

constexpr auto compare_permutations_with_python_itertools() -> bool
{
    // "flux" string permutations
    {
        /*
        # Python code to generate comparison output
        from itertools import permutations

        perms = [''.join(p) for p in permutations("flux", 4)]
        formatted = '{' + ', '.join(f'"{x}"' for x in perms) + '}'
        print(formatted)
        */

        auto reference = std::array<std::string, 24> {
            "flux", "flxu", "fulx", "fuxl", "fxlu", "fxul", "lfux", "lfxu",
            "lufx", "luxf", "lxfu", "lxuf", "uflx", "ufxl", "ulfx", "ulxf",
            "uxfl", "uxlf", "xflu", "xful", "xlfu", "xluf", "xufl", "xulf"};

        auto str = std::string("flux");
        auto permutations = flux::permutations<4>(flux::ref(str));
        auto first = flux::first(permutations);

        for (auto i : flux::ints().take(24)) {
            auto p = flux::read_at(permutations, first);
            auto r = flux::ref(reference.at(static_cast<size_t>(i)));

            STATIC_CHECK(check_equal(p, r));

            flux::inc(permutations, first);
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

    bool comparison = compare_permutations_with_python_itertools();
    REQUIRE(comparison);
}
