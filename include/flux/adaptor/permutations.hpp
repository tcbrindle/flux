// Copyright stuff here? Not really sure on that as I've never done
// it

#ifndef FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED
#define FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED

#include "flux/core/concepts.hpp"
#include "flux/core/inline_sequence_base.hpp"
#include "flux/adaptor/permutations_base.hpp"
#include "flux/adaptor/cartesian_base.hpp" // tuple_repeated_t
#include <cstddef>
#include <flux/core.hpp>
#include <numeric>
#include <flux.hpp>

namespace flux {
namespace detail {

template <flux::sequence Base, flux::distance_t SubsequenceSize>
    requires(SubsequenceSize > 0) && flux::bounded_sequence<Base> && flux::multipass_sequence<Base>
struct permutations_adaptor
    : public flux::inline_sequence_base<permutations_adaptor<Base, SubsequenceSize>> {
private:
    Base base_;

public:
    constexpr permutations_adaptor(Base&& base) : base_(std::move(base)) { }

    struct flux_sequence_traits : flux::default_sequence_traits {
    private:
        using self_t = permutations_adaptor;

        struct cursor_type {
            std::size_t permutation_index_;

            [[nodiscard]] constexpr auto operator<=>(const cursor_type& other) const
            {
                return permutation_index_ <=> other.permutation_index_;
            }
            [[nodiscard]] constexpr bool operator==(const cursor_type& other) const
            {
                return permutation_index_ == other.permutation_index_;
            }
        };

    public:
        // TODO: remove vector type
        using value_type = tuple_repeated_t<value_t<Base>, SubsequenceSize>;

        inline static constexpr bool is_infinite = false;

        static constexpr auto first(self_t& self) -> cursor_type
        {
            return {.permutation_index_ = 0};
        }

        static constexpr auto is_last(self_t& self, const cursor_type& cursor) -> bool
        {
            return false;
        }

        static constexpr auto inc(self_t& self, cursor_type& cursor) -> void
        {
            /*
            const auto k = SubsequenceSize;
            const auto n = self.cache_.size();

            cursor.permutation_index_ += 1;

            for (const auto i : flux::iota(std::size_t {0}, std::size_t {k}).reverse()) {
                if (cursor.cycles_[i] == 0) {
                    cursor.cycles_[i] = n - i - 1;
                    std::rotate(cursor.indices_.begin() + i, cursor.indices_.begin() + i + 1,
                                cursor.indices_.end());
                } else {
                    const auto swap_index = n - cursor.cycles_[i];
                    std::swap(cursor.indices_[i], cursor.indices_[swap_index]);
                    cursor.cycles_[i] -= 1;
                    return;
                }
            }
            */
        }

        static constexpr auto read_at(self_t& self, const cursor_type& cursor) -> value_type
        {
            return {};
        }
    };
};

template <std::size_t SubsequenceSize>
    requires(SubsequenceSize > 0)
struct permutations_fn {
    template <sequence Seq>
        requires(not infinite_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> auto
    {
        return permutations_adaptor<std::decay_t<Seq>, SubsequenceSize>(FLUX_FWD(seq));
    }
};

} // namespace detail

FLUX_EXPORT
template <std::size_t SubsequenceSize>
inline constexpr auto permutations = detail::permutations_fn<SubsequenceSize> {};

// clang-format off
FLUX_EXPORT
template <typename Derived>
template <std::size_t SubsequenceSize>
    requires(SubsequenceSize > 0)
constexpr auto inline_sequence_base<Derived>::permutations() &&
    requires(not infinite_sequence<Derived>)
{
    return flux::permutations<SubsequenceSize>(std::move(derived()));
}
// clang-format on
//
} // namespace flux

#endif
