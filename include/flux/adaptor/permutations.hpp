// Copyright stuff here? Not really sure on that as I've never done
// it

#ifndef FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED
#define FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED

#include "flux/core/concepts.hpp"
#include "flux/core/inline_sequence_base.hpp"
#include "flux/macros.hpp"
#include "flux/adaptor/permutations_base.hpp"
#include <cstddef>
#include <flux/core.hpp>
#include <numeric>

namespace flux {
namespace detail {

template <flux::sequence Base>
    requires flux::bounded_sequence<Base>
class permutations_adaptor : public flux::inline_sequence_base<permutations_adaptor<Base>> {
private:
    Base base_;

    // Uninitialized: Input sequence is not cached, output can't be generated
    // Cached: Input sequence is cached (copied) into the cache_ member variable
    enum class state_t { Uninitialized, Cached };
    state_t state_ {state_t::Uninitialized};

    using inner_value_t = flux::value_t<Base>;
    std::vector<inner_value_t> cache_; // This caches the input sequence

    std::size_t size_;

    [[nodiscard]] constexpr std::size_t number_permutations()
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

        size_ = factorial(cache_.size());

        state_ = state_t::Cached;
    }

    constexpr auto cache_base() -> void
        requires flux::sized_sequence<Base>
    {
        const auto size = static_cast<std::size_t>(flux::size(base_));
        cache_.resize(size);
        std::ranges::copy(base_, cache_.begin());

        size_ = factorial(cache_.size());

        state_ = state_t::Cached;
    }

public:
    constexpr permutations_adaptor(Base&& base) : base_(std::move(base)) { }

    struct flux_sequence_traits : flux::default_sequence_traits {
    private:
        using self_t = permutations_adaptor;

        struct cursor_type {
            std::vector<std::size_t> indices_;
            std::size_t index_ {0};

            [[nodiscard]] constexpr auto operator<=>(const cursor_type& other) const
            {
                return index_ <=> other.index_;
            }

            [[nodiscard]] constexpr bool operator==(const cursor_type& other) const
            {
                return index_ == other.index_;
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

            // variable aliases for readability
            const auto base_length = self.cache_.size();

            // fill up the indices from 0 to size-1
            std::vector<std::size_t> indices(base_length);
            std::iota(indices.begin(), indices.end(), 0);

            // return the cursor
            return {.indices_ = indices, .index_ = 0};
        }

        static constexpr auto last(self_t& self) -> cursor_type
        {
            if (not self.is_cached()) {
                self.cache_base();
            }

            // variable aliases for readability
            const auto base_length = self.cache_.size();

            // fill up the indices from 0 to size-1 in reverse (as this is the last)
            std::vector<std::size_t> indices(base_length);
            std::iota(indices.rbegin(), indices.rend(), 0);

            // return the cursor
            return {.indices_ = indices, .index_ = self.number_permutations()};
        }

        static constexpr auto is_last([[maybe_unused]] self_t& self, const cursor_type& cursor)
            -> bool
        {
            return cursor.index_ >= self.number_permutations();
        }

        static constexpr auto inc([[maybe_unused]] self_t& self, cursor_type& cursor) -> void
        {
            std::next_permutation(cursor.indices_.begin(), cursor.indices_.end());
            cursor.index_ += 1;
        }

        static constexpr auto dec([[maybe_unused]] self_t& self, cursor_type& cursor) -> void
        {
            std::prev_permutation(cursor.indices_.begin(), cursor.indices_.end());
            cursor.index_ -= 1;
        }

        static constexpr auto read_at(self_t& self, const cursor_type& cursor) -> value_type
        {
            if (not self.is_cached()) {
                self.cache_base();
            }
            return reindex_vec<inner_value_t>(self.cache_, cursor.indices_);
        }

        static constexpr auto read_at_unchecked(self_t& self, const cursor_type& cursor)
            -> value_type
        {
            return reindex_vec<inner_value_t>(self.cache_, cursor.indices_);
        }

        static constexpr auto size(self_t& self) -> flux::distance_t
        {
            return static_cast<flux::distance_t>(self.number_permutations());
        }
    };
};

struct permutations_fn {
    template <sequence Seq>
        requires(not infinite_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> bidirectional_sequence auto
    {
        return permutations_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto permutations = detail::permutations_fn {};

FLUX_EXPORT
template <typename Derived>
    constexpr auto inline_sequence_base<Derived>::permutations()
    && requires(not infinite_sequence<Derived>) { return flux::permutations(std::move(derived())); }

} // namespace flux
#endif
