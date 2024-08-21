
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_MASK_HPP_INCLUDED
#define FLUX_OP_MASK_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <sequence Base, sequence Mask>
struct mask_adaptor : inline_sequence_base<mask_adaptor<Base, Mask>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Mask mask_;

public:
    constexpr mask_adaptor(decays_to<Base> auto&& base, decays_to<Mask> auto&& mask)
        : base_(FLUX_FWD(base)),
          mask_(FLUX_FWD(mask))
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            cursor_t<Mask> mask_cur;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base>> &&
                         std::equality_comparable<cursor_t<Mask>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable =
            std::is_const_v<Self>
                ? sequence<Base const> && sequence<Mask const>
                : true;

    public:
        using value_type = value_t<Base>;

        static inline constexpr bool is_infinite =
            infinite_sequence<Base> && infinite_sequence<Mask>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto base_cur = flux::first(self.base_);
            auto mask_cur = flux::first(self.mask_);

            while (!flux::is_last(self.base_, base_cur) && !flux::is_last(self.mask_, mask_cur)) {
                if (static_cast<bool>(flux::read_at(self.mask_, mask_cur))) {
                    break;
                }
                flux::inc(self.base_, base_cur);
                flux::inc(self.mask_, mask_cur);
            }
            return cursor_type{std::move(base_cur), std::move(mask_cur)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.base_cur) ||
                    flux::is_last(self.mask_, cur.mask_cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            while (!flux::is_last(self.base_, flux::inc(self.base_, cur.base_cur)) &&
                   !flux::is_last(self.mask_, flux::inc(self.mask_, cur.mask_cur))) {
                if (static_cast<bool>(flux::read_at(self.mask_, cur.mask_cur))) {
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
        static constexpr auto move_at(Self& self, cursor_type const& cur) -> decltype(auto)
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
                     bounded_sequence<Mask>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{.base_cur = flux::last(self.base_),
                               .mask_cur = flux::last(self.mask_)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self> &&
                     bidirectional_sequence<Base> &&
                     bidirectional_sequence<Mask>
        static constexpr auto dec(Self& self, cursor_type& cur) -> void
        {
            do {
                flux::dec(self.base_, cur.base_cur);
                flux::dec(self.mask_, cur.mask_cur);
            } while (!static_cast<bool>(flux::read_at(self.mask_, cur.mask_cur)));
        }
    };
};

struct mask_fn {
    template <adaptable_sequence Base, adaptable_sequence Mask>
        requires boolean_testable<element_t<Mask>>
    [[nodiscard]]
    constexpr auto operator()(Base&& base, Mask&& mask) const
    {
        return mask_adaptor<std::decay_t<Base>, std::decay_t<Mask>>(
            FLUX_FWD(base), FLUX_FWD(mask));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto mask = detail::mask_fn{};

template <typename D>
template <adaptable_sequence Mask>
    requires detail::boolean_testable<element_t<Mask>>
constexpr auto inline_sequence_base<D>::mask(Mask&& mask_) &&
{
    return flux::mask(std::move(derived()), FLUX_FWD(mask_));
}

} // namespace flux

#endif // FLUX_OP_MASK_HPP_INCLUDED
