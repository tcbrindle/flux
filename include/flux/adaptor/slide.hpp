
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_SLIDE_HPP_INCLUDED
#define FLUX_ADAPTOR_SLIDE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/adaptor/stride.hpp>

namespace flux {

namespace detail {

template <multipass_sequence Base>
struct slide_adaptor : inline_iter_base<slide_adaptor<Base>> {
private:
    Base base_;
    distance_t win_sz_;

public:
    constexpr slide_adaptor(decays_to<Base> auto&& base, distance_t win_sz)
        : base_(FLUX_FWD(base)),
          win_sz_(win_sz)
    {}

    struct flux_iter_traits : default_iter_traits {
    private:
        struct cursor_type {
            cursor_t<Base> from;
            cursor_t<Base> to;

            friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs)
                -> bool
            {
                return lhs.from == rhs.from;
            }

            friend constexpr auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
                -> std::strong_ordering
                requires ordered_cursor<cursor_t<Base>>
            {
                return lhs.from <=> rhs.from;
            }
        };

    public:
        static constexpr auto first(auto& self) -> cursor_type {
            auto cur = flux::first(self.base_);
            auto end = cur;
            advance(self.base_, end, num::sub(self.win_sz_, distance_t{1}));

            return cursor_type{.from = std::move(cur), .to = std::move(end)};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.to);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            flux::inc(self.base_, cur.from);
            flux::inc(self.base_, cur.to);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::take(flux::slice(self.base_, cur.from, flux::last), self.win_sz_))
        {
            return flux::take(flux::slice(self.base_, cur.from, flux::last), self.win_sz_);
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base> && bidirectional_sequence<Base>
        {
            auto end = flux::last(self.base_);
            auto cur = end;
            advance(self.base_, cur, num::sub(distance_t{1}, self.win_sz_));
            return cursor_type{.from = std::move(cur), .to = std::move(end)};
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            flux::dec(self.base_, cur.from);
            flux::dec(self.base_, cur.to);
        }

        static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset) -> void
            requires random_access_sequence<Base>
        {
            flux::inc(self.base_, cur.from, offset);
            flux::inc(self.base_, cur.to, offset);
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> distance_t
            requires random_access_sequence<Base>
        {
            return flux::distance(self.base_, from.from, to.from);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_iterable<Base>
        {
            auto s = num::add(num::sub(flux::size(self.base_), self.win_sz_), distance_t{1});
            return (cmp::max)(s, distance_t{0});
        }
    };
};

struct slide_fn {
    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, num::integral auto win_sz) const
        -> sequence auto
    {
        return slide_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq),
                                                num::checked_cast<distance_t>(win_sz));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto slide = detail::slide_fn{};

template <typename D>
constexpr auto inline_iter_base<D>::slide(num::integral auto win_sz) &&
        requires multipass_sequence<D>
{
    FLUX_ASSERT(win_sz > 0);
    return flux::slide(std::move(derived()), win_sz);
}

} // namespace slide

#endif // FLUX_ADAPTOR_SLIDE_HPP_INCLUDED
