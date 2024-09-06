
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_CYCLE_HPP_INCLUDED
#define FLUX_ADAPTOR_CYCLE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <bool>
struct cycle_data {
    std::size_t count;
};

template <>
struct cycle_data<true> {};

template <multipass_sequence Base, bool IsInfinite>
struct cycle_adaptor : inline_sequence_base<cycle_adaptor<Base, IsInfinite>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS cycle_data<IsInfinite> data_;

public:
    constexpr explicit cycle_adaptor(decays_to<Base> auto&& base)
        requires IsInfinite
        : base_(FLUX_FWD(base))
    {}

    constexpr cycle_adaptor(decays_to<Base> auto&& base, std::size_t count)
        requires (!IsInfinite)
        : base_(FLUX_FWD(base)),
          data_(count)
    {}

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            // Use an unsigned type to avoid UB on overflow
            std::size_t n = 0;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;

            friend auto operator<=>(cursor_type const&, cursor_type const&)
                -> std::strong_ordering
                requires std::three_way_comparable<cursor_t<Base>, std::strong_ordering>
            = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr bool is_infinite = IsInfinite;

        static constexpr auto first(auto& self)
            -> decltype(cursor_type{flux::first(self.base_)})
        {
            if constexpr (IsInfinite) {
                return cursor_type{flux::first(self.base_)};
            } else {
                auto cur = flux::first(self.base_);
                if (flux::is_last(self.base_, cur)) {
                    return cursor_type{std::move(cur), self.data_.count};
                } else {
                    return cursor_type{std::move(cur)};
                }
            }
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            if constexpr (IsInfinite) {
                return false;
            } else {
                return cur.n >= self.data_.count;
            }
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.base_cur);
            if (flux::is_last(self.base_, cur.base_cur)) {
                cur.base_cur = flux::first(self.base_);
                ++cur.n;
            }
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(static_cast<const_element_t<Base>>(flux::read_at(self.base_, cur.base_cur)))
        {
            return static_cast<const_element_t<Base>>(
                flux::read_at(self.base_, cur.base_cur));
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> const_element_t<Base>
        {
            return static_cast<const_element_t<Base>>(
                flux::read_at_unchecked(self.base_, cur.base_cur));
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<value_t<Base> const&&, rvalue_element_t<Base>>;
            return static_cast<R>(flux::move_at(self.base_, cur.base_cur));
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(auto)
        {
            using R = std::common_reference_t<value_t<Base> const&&, rvalue_element_t<Base>>;
            return static_cast<R>(flux::move_at_unchecked(self.base_, cur.base_cur));
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
        {
            auto constify_pred = [&pred](auto&& elem) {
                return std::invoke(pred, static_cast<const_element_t<Base>>(FLUX_FWD(elem)));
            };

            if constexpr (IsInfinite) {
                std::size_t n = 0;
                while (true) {
                    auto cur = flux::for_each_while(self.base_, constify_pred);
                    if (!flux::is_last(self.base_, cur)) {
                        return cursor_type{std::move(cur), n};
                    }
                    ++n;
                }
            } else {
                for (std::size_t n = 0; n < self.data_.count; ++n) {
                    auto cur = flux::for_each_while(self.base_, constify_pred);
                    if (!flux::is_last(self.base_, cur)) {
                        return cursor_type{std::move(cur), n};
                    }
                }
                return last(self);
            }
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<decltype(self.base_)> &&
                     bounded_sequence<decltype(self.base_)>
        {
            if (cur.base_cur == flux::first(self.base_)) {
                --cur.n;
                cur.base_cur = flux::last(self.base_);
            }
            flux::dec(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset)
            requires random_access_sequence<decltype(self.base_)> &&
                     bounded_sequence<decltype(self.base_)>
        {
            auto const first = flux::first(self.base_);

            auto const sz = flux::size(self.base_);
            if (sz == 0) {
                return;
            }

            auto off = flux::distance(self.base_, first, cur.base_cur);
            off = num::add(off, offset);

            cur.n += static_cast<std::size_t>(off/sz);

            off = off % sz;
            if (off < 0) {
                off +=sz; // differing signs
            }

            cur.base_cur = flux::next(self.base_, first, off);
        }

        static constexpr auto distance(auto& self,
                                       cursor_type const& from,
                                       cursor_type const& to) -> distance_t
            requires random_access_sequence<decltype(self.base_)> &&
                     sized_sequence<decltype(self.base_)>
        {
            auto dist = num::cast<distance_t>(to.n) - num::cast<distance_t>(from.n);
            dist = num::mul(dist, flux::size(self.base_));
            return num::add(dist,
                    flux::distance(self.base_, from.base_cur, to.base_cur));
        }

        // Weirdly, we don't actually need Base to be bounded
        static constexpr auto last(auto& self) -> cursor_type
            requires (!IsInfinite)
        {
            return cursor_type{.base_cur = flux::first(self.base_),
                               .n = self.data_.count};
        }

        static constexpr auto size(auto& self) -> distance_t
            requires (!IsInfinite && sized_sequence<Base>)
        {
            return num::mul(flux::size(self.base_),
                            num::cast<flux::distance_t>(self.data_.count));
        }
    };
};

struct cycle_fn {
    template <adaptable_sequence Seq>
        requires infinite_sequence<Seq> || multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> infinite_sequence auto
    {
        if constexpr (infinite_sequence<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return cycle_adaptor<std::decay_t<Seq>, true>(FLUX_FWD(seq));
        }
    }

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, num::integral auto count) const
        -> multipass_sequence auto
    {
        auto c = num::checked_cast<distance_t>(count);
        if (c < 0) {
            runtime_error("Negative count passed to cycle()");
        }
        return cycle_adaptor<std::decay_t<Seq>, false>(
            FLUX_FWD(seq), num::checked_cast<std::size_t>(c));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto cycle = detail::cycle_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::cycle() &&
    requires infinite_sequence<D> || multipass_sequence<D>
{
    return flux::cycle(std::move(derived()));
}

template <typename D>
constexpr auto inline_sequence_base<D>::cycle(num::integral auto count) &&
    requires multipass_sequence<D>
{
    return flux::cycle(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_ADAPTOR_CYCLE_HPP_INCLUDED
