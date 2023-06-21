
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SELECT_BY_HPP_INCLUDED
#define FLUX_OP_SELECT_BY_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <sequence Base, sequence Selectors>
struct select_by_adaptor : inline_sequence_base<select_by_adaptor<Base, Selectors>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Selectors selectors_;

public:
    constexpr select_by_adaptor(decays_to<Base> auto&& base, decays_to<Selectors> auto&& selectors)
        : base_(FLUX_FWD(base)),
          selectors_(FLUX_FWD(selectors))
    {}

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            cursor_t<Selectors> selector_cur;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base>> &&
                         std::equality_comparable<cursor_t<Selectors>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable =
            std::is_const_v<Self>
                ? sequence<Base const> && sequence<Selectors const>
                : true;

    public:
        static inline constexpr bool is_infinite =
            infinite_sequence<Base> && infinite_sequence<Selectors>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto base_cur = flux::first(self.base_);
            auto selector_cur = flux::first(self.selectors_);

            while (!flux::is_last(self.base_, base_cur) && !flux::is_last(self.selectors_, selector_cur)) {
                if (static_cast<bool>(flux::read_at(self.selectors_, selector_cur))) {
                    break;
                }
                flux::inc(self.base_, base_cur);
                flux::inc(self.selectors_, selector_cur);
            }
            return cursor_type{std::move(base_cur), std::move(selector_cur)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur) ||
                    flux::is_last(self.selectors_, cur.selector_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            while (!flux::is_last(self.base_, flux::inc(self.base_, cur.base_cur)) &&
                   !flux::is_last(self.selectors_, flux::inc(self.selectors_, cur.selector_cur))) {
                if (static_cast<bool>(flux::read_at(self.selectors_, cur.selector_cur))) {
                    break;
                }
            }
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at_unchecked(Self& self, cursor_type const& cur)
            -> decltype(auto)
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at(auto& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at_unchecked(Self& self, cursor_type const& cur)
            -> decltype(auto)
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self> &&
                     bounded_sequence<Base> &&
                     bounded_sequence<Selectors>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{.base_cur = flux::last(self.base_),
                               .selector_cur = flux::last(self.selectors_)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self> &&
                     bidirectional_sequence<Base> &&
                     bidirectional_sequence<Selectors>
        static constexpr auto dec(Self& self, cursor_type& cur) -> void
        {
            do {
                flux::dec(self.base_, cur.base_cur);
                flux::dec(self.selectors_, cur.selector_cur);
            } while (!static_cast<bool>(flux::read_at(self.selectors_, cur.selector_cur)));
        }
    };
};

struct select_by_fn {
    template <adaptable_sequence Base, adaptable_sequence Selectors>
        requires boolean_testable<element_t<Selectors>>
    [[nodiscard]]
    constexpr auto operator()(Base&& base, Selectors&& selectors) const
    {
        return select_by_adaptor<std::decay_t<Base>, std::decay_t<Selectors>>(
            FLUX_FWD(base), FLUX_FWD(selectors));
    }
};

} // namespace detail

inline constexpr auto select_by = detail::select_by_fn{};

template <typename D>
template <adaptable_sequence Selectors>
    requires detail::boolean_testable<element_t<Selectors>>
constexpr auto inline_sequence_base<D>::select_by(Selectors&& selectors) &&
{
    return flux::select_by(std::move(derived()), FLUX_FWD(selectors));
}

} // namespace flux

#endif // FLUX_OP_FILTER_BY_HPP_INCLUDED
