
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_ADJACENT_FILTER_HPP_INCLUDED
#define FLUX_OP_ADJACENT_FILTER_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <multipass_sequence Base, typename Pred>
struct adjacent_filter_adaptor
    : inline_sequence_base<adjacent_filter_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

public:
    constexpr adjacent_filter_adaptor(decays_to<Base> auto&& base, Pred pred)
        : base_(FLUX_FWD(base)),
          pred_(std::move(pred))
    {}

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{flux::first(self.base_)};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base_, cur.base_cur))
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base_, cur.base_cur))
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            auto temp = cur.base_cur;
            flux::inc(self.base_, cur.base_cur);

            while (!flux::is_last(self.base_, cur.base_cur)) {
                if (std::invoke(self.pred_,
                                flux::read_at(self.base_, temp),
                                flux::read_at(self.base_, cur.base_cur))) {
                    break;
                }
                flux::inc(self.base_, cur.base_cur);
            }
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            auto first = flux::first(self.base_);
            FLUX_DEBUG_ASSERT(cur.base_cur != first);

            flux::dec(self.base_, cur.base_cur);

            while (cur.base_cur != first) {
                auto temp = flux::prev(self.base_, cur.base_cur);

                if (std::invoke(self.pred_, flux::read_at(self.base_, temp),
                                flux::read_at(self.base_, cur.base_cur))) {
                    break;
                }
                cur.base_cur = std::move(temp);
            }
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type{flux::last(self.base_)};
        }
    };
};

struct adjacent_filter_fn {
    template <adaptable_sequence Seq, typename Pred>
        requires multipass_sequence<Seq> &&
                 std::predicate<Pred&, element_t<Seq>, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const -> multipass_sequence auto
    {
        return adjacent_filter_adaptor<std::decay_t<Seq>, Pred>(
            FLUX_FWD(seq), std::move(pred));
    }
};

struct dedup_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq> &&
                 std::equality_comparable<element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> multipass_sequence auto
    {
        return adjacent_filter_fn{}(FLUX_FWD(seq), std::ranges::not_equal_to{});
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto adjacent_filter = detail::adjacent_filter_fn{};
FLUX_EXPORT inline constexpr auto dedup = detail::dedup_fn{};

template <typename D>
template <typename Pred>
    requires multipass_sequence<D> &&
             std::predicate<Pred&, element_t<D>, element_t<D>>
constexpr auto inline_sequence_base<D>::adjacent_filter(Pred pred) &&
{
    return flux::adjacent_filter(std::move(derived()), std::move(pred));
}

template <typename D>
constexpr auto inline_sequence_base<D>::dedup() &&
    requires multipass_sequence<D> &&
             std::equality_comparable<element_t<D>>
{
    return flux::dedup(std::move(derived()));
}

} // namespace flux

#endif // FLUX_OP_ADJACENT_FILTER_HPP_INCLUDED
