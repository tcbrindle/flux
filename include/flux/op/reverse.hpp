
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REVERSE_HPP_INCLUDED
#define FLUX_OP_REVERSE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

namespace flux {

namespace detail {

template <typename BaseCur>
struct rev_cur {
    BaseCur base_cur;

    friend bool operator==(rev_cur const&, rev_cur const&)
        requires std::equality_comparable<BaseCur>
        = default;

    friend std::strong_ordering operator<=>(rev_cur const& lhs, rev_cur const& rhs)
        requires std::three_way_comparable<BaseCur>
    {
        return rhs <=> lhs;
    }
};

template <typename B>
rev_cur(B&&) -> rev_cur<std::remove_cvref_t<B>>;

template <lens Base>
    requires bidirectional_sequence<Base> &&
             bounded_sequence<Base>
struct reverse_adaptor : lens_base<reverse_adaptor<Base>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;

    friend struct sequence_iface<reverse_adaptor>;

public:
    constexpr explicit reverse_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }
};

template <typename>
inline constexpr bool is_reverse_adaptor = false;

template <typename Base>
inline constexpr bool is_reverse_adaptor<reverse_adaptor<Base>> = true;

struct reverse_fn {
    template <adaptable_sequence Seq>
        requires bidirectional_sequence<Seq> &&
                 bounded_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
        -> lens auto
    {
        if constexpr (is_reverse_adaptor<std::decay_t<Seq>>) {
            return FLUX_FWD(seq).base();
        } else {
            return reverse_adaptor(flux::from(FLUX_FWD(seq)));
        }
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::reverse_adaptor<Base>>
{
    using value_type = value_t<Base>;
    using distance_type = distance_t<Base>;

    static constexpr auto first(auto& self)
    {
        return detail::rev_cur(flux::last(self.base_));
    }

    static constexpr auto last(auto& self)
    {
        return detail::rev_cur(flux::first(self.base_));
    }

    static constexpr auto is_last(auto& self, auto const& cur) -> bool
    {
        return cur.base_cur == flux::first(self.base_);
    }

    static constexpr auto read_at(auto& self, auto const& cur) -> decltype(auto)
    {
        return flux::read_at(self.base_, flux::prev(self.base_, cur.base_cur));
    }

    static constexpr auto inc(auto& self, auto& cur) -> auto&
    {
        flux::dec(self.base_, cur.base_cur);
        return cur;
    }

    static constexpr auto dec(auto& self, auto& cur) -> auto&
    {
        flux::inc(self.base_, cur.base_cur);
        return cur;
    }

    static constexpr auto inc(auto& self, auto& cur, auto dist) -> auto&
        requires random_access_sequence<decltype(self.base_)>
    {
        flux::inc(self.base_, cur.base_cur, -dist);
        return cur;
    }

    static constexpr auto distance(auto& self, auto const& from, auto const& to)
        requires random_access_sequence<decltype(self.base_)>
    {
        return -flux::distance(self.base_, from.base_cur, to.base_cur);
    }

    // FIXME: GCC11 ICE
#if GCC_ICE
    static constexpr auto size(auto& self)
        requires sized_sequence<decltype(self.base_)>
    {
        return flux::size(self.base_);
    }
#endif

    static constexpr auto move_at(auto& self, auto const& cur) -> decltype(auto)
    {
        return flux::move_at(self.base_, cur.base_cur);
    }

    static constexpr auto for_each_while(auto& self, auto&& pred)
    {
        auto cur = flux::last(self.base_);
        const auto end = flux::first(self.base_);

        while (cur != end) {
            if (!std::invoke(pred, flux::read_at(self.base_, flux::prev(self.base_, cur)))) {
                break;
            }
            flux::dec(self.base_, cur);
        }

        return detail::rev_cur(cur);
    }
};

inline constexpr auto reverse = detail::reverse_fn{};

template <typename D>
constexpr auto lens_base<D>::reverse() &&
    requires bidirectional_sequence<D> && bounded_sequence<D>
{
    return flux::reverse(std::move(derived()));
}

} // namespace flux

#endif
