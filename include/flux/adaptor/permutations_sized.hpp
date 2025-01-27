// Copyright stuff here? Not really sure on that as I've never done
// it

#ifndef FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED
#define FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED

#include "flux/core/concepts.hpp"
#include "flux/core/inline_sequence_base.hpp"
#include "flux/adaptor/permutations_base.hpp"
#include <cstddef>
#include <flux/core.hpp>
#include <numeric>
#include <flux.hpp>

namespace flux {
namespace detail {

template <flux::sequence Base, flux::distance_t SubsequenceSize>
    requires flux::bounded_sequence<Base> && (SubsequenceSize > 0)
struct permutations_sized_adaptor
    : public flux::inline_sequence_base<permutations_sized_adaptor<Base, SubsequenceSize>> {
private:
    Base base_;

    // Uninitialized: Input sequence is not cached, output can't be generated
    // Cached: Input sequence is cached (copied) into the cache_ member variable
    enum class state_t { Uninitialized, Cached };
    state_t state_ {state_t::Uninitialized};

    using inner_value_t = flux::value_t<Base>;
    std::vector<inner_value_t> cache_;
    std::size_t size_;

    [[nodiscard]] constexpr std::size_t count_permutations()
    {
        if (not is_cached()) {
            cache_base();
        }

        return size_;
    }

    [[nodiscard]] constexpr bool is_cached() const noexcept { return state_ == state_t::Cached; }

    constexpr auto cache_base() -> void
    {
        for (const auto elem : base_) {
            cache_.emplace_back(elem);
        }

        if (SubsequenceSize <= cache_.size()) {
            size_ = factorial(cache_.size()) / factorial(cache_.size() - SubsequenceSize);
        } else {
            size_ = 0;
        }

        state_ = state_t::Cached;
    }

    constexpr auto cache_base() -> void
        requires flux::sized_sequence<Base>
    {
        const auto size = static_cast<std::size_t>(flux::size(base_));
        cache_.resize(size);
        std::ranges::copy(base_, cache_.begin());

        if (SubsequenceSize <= size) {
            size_ = factorial(cache_.size()) / factorial(cache_.size() - SubsequenceSize);
        } else {
            size_ = 0;
        }

        state_ = state_t::Cached;
    }

public:
    constexpr permutations_sized_adaptor(Base&& base) : base_(std::move(base)) { }

    struct flux_sequence_traits : flux::default_sequence_traits {
    private:
        using self_t = permutations_sized_adaptor;

        struct cursor_type {
            std::vector<std::size_t> indices_;
            std::vector<std::size_t> cycles_;
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
        using value_type = std::vector<inner_value_t>;

        inline static constexpr bool is_infinite = false;

        static constexpr auto first(self_t& self) -> cursor_type
        {
            if (not self.is_cached()) {
                self.cache_base();
            }

            // variable alias for readability
            const auto base_length = self.cache_.size();

            // Set indices to range [0, 1, ..., size-1]
            std::vector<std::size_t> indices(base_length);
            std::iota(indices.begin(), indices.end(), 0);

            // Set cycles to [i, i+1, ..., size] where i = size - subsequence_size
            std::vector<std::size_t> cycles(SubsequenceSize);
            std::iota(cycles.rbegin(), cycles.rend(), base_length - SubsequenceSize);

            // return the cursor
            return {.indices_ = indices, .cycles_ = cycles, .permutation_index_ = 0};
        }

        static constexpr auto last(self_t& self) -> cursor_type
        {
            if (not self.is_cached()) {
                self.cache_base();
            }

            // variable aliases for readability
            const auto base_length = self.cache_.size();

            // fill up the indices from 0 to size-1 in reverse because it's the end
            std::vector<std::size_t> indices(base_length);
            std::iota(indices.rbegin(), indices.rend(), 0);

            // fill up the cycles with 0s because it's the end
            std::vector<std::size_t> cycles(SubsequenceSize, 0);

            // return the cursor
            return {.indices_ = indices, .cycles_ = cycles, .permutation_index_ = self.count_permutations()};
        }

        static constexpr auto is_last(self_t& self, const cursor_type& cursor) -> bool
        {
            return cursor.permutation_index_ >= self.count_permutations();
        }

        static constexpr auto inc(self_t& self, cursor_type& cursor) -> void
        {
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
        }

        static constexpr auto read_at(self_t& self, const cursor_type& cursor) -> value_type
        {
            if (not self.is_cached()) {
                self.cache_base();
            }
            return reindex_vec<inner_value_t>(self.cache_, cursor.indices_, SubsequenceSize);
        }

        static constexpr auto read_at_unchecked(self_t& self, const cursor_type& cursor)
            -> value_type
        {
            return reindex_vec<inner_value_t>(self.cache_, cursor.indices_, SubsequenceSize);
        }

        static constexpr auto size(self_t& self) -> flux::distance_t
            requires flux::sized_sequence<Base>
        {
            return self.count_permutations();
        }
    };
};

template <std::size_t SubsequenceSize>
    requires(SubsequenceSize > 0)
struct permutations_sized_fn {
    template <sequence Seq>
        requires(not infinite_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> sized_sequence auto
    {
        return permutations_sized_adaptor<std::decay_t<Seq>, SubsequenceSize>(FLUX_FWD(seq));
    }
};

} // namespace detail

FLUX_EXPORT
template <std::size_t SubsequenceSize>
inline constexpr auto permutations_sized = detail::permutations_sized_fn<SubsequenceSize> {};

// clang-format off
FLUX_EXPORT
template <typename Derived>
template <std::size_t SubsequenceSize>
    requires(SubsequenceSize > 0)
constexpr auto inline_sequence_base<Derived>::permutations_sized() &&
    requires(not infinite_sequence<Derived>)
{
    return flux::permutations_sized<SubsequenceSize>(std::move(derived()));
}
// clang-format on
//
} // namespace flux

#endif
