
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SET_ALGORITHMS_HPP_INCLUDED
#define FLUX_OP_SET_ALGORITHMS_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/requirements.hpp>

#include <utility>

namespace flux::detail {

template <sequence Base1, sequence Base2, typename Cmp>
struct set_union_adaptor
    : flux::inline_sequence_base<set_union_adaptor<Base1, Base2, Cmp>>
{
private:
    FLUX_NO_UNIQUE_ADDRESS Base1 base1_;
    FLUX_NO_UNIQUE_ADDRESS Base2 base2_;
    FLUX_NO_UNIQUE_ADDRESS Cmp cmp_;

public:
    constexpr set_union_adaptor(decays_to<Base1> auto&& base1, decays_to<Base2> auto&& base2, Cmp cmp)
        : base1_(FLUX_FWD(base1)),
          base2_(FLUX_FWD(base2)),
          cmp_(cmp)
    {}

    struct flux_sequence_traits {
    private:

        struct cursor_type {
            cursor_t<Base1> base1_cursor;
            cursor_t<Base2> base2_cursor;
            enum : bool {first, second} active_ = first;

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool
                requires std::equality_comparable<cursor_t<Base1>> &&
                         std::equality_comparable<cursor_t<Base2>>
                = default;
        };

        template <typename Self>
        static inline constexpr bool maybe_const_iterable
            = std::is_const_v<Self> ? (flux::sequence<Base1 const> && flux::sequence<Base2 const>) : true;

        template <typename Self>
        static constexpr void update(Self& self, cursor_type& cur) {
            if (flux::is_last(self.base1_, cur.base1_cursor)) {
                cur.active_ = cursor_type::second;
                return;
            }

            if (flux::is_last(self.base2_, cur.base2_cursor)) {
                cur.active_ = cursor_type::first;
                return;
            }

            if (std::invoke(self.cmp_, flux::read_at(self.base2_, cur.base2_cursor), 
                                       flux::read_at(self.base1_, cur.base1_cursor))) {
                cur.active_ = cursor_type::second;
                return;
            }

            if (not std::invoke(self.cmp_, flux::read_at(self.base1_, cur.base1_cursor), 
                                           flux::read_at(self.base2_, cur.base2_cursor))) {
                flux::inc(self.base2_, cur.base2_cursor);
            }

            cur.active_ = cursor_type::first;
        }

    public:
        using value_type = std::common_type_t<value_t<Base1>, value_t<Base2>>;

        inline static constexpr bool is_infinite = flux::infinite_sequence<Base1> || 
                                                   flux::infinite_sequence<Base2>;

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto cur = cursor_type{.base1_cursor = flux::first(self.base1_),
                                   .base2_cursor = flux::first(self.base2_)};
            update(self, cur);
            return cur;
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base1_, cur.base1_cursor) && 
                   flux::is_last(self.base2_, cur.base2_cursor);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            if (cur.active_ == cursor_type::first) {
                flux::inc(self.base1_, cur.base1_cursor);
            } else {
                flux::inc(self.base2_, cur.base2_cursor);
            }

            update(self, cur);
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> std::common_reference_t<element_t<Base1>, element_t<Base2>>
        {
            if (cur.active_ == cursor_type::first) {
                return flux::read_at(self.base1_, cur.base1_cursor);
            } else {
                return flux::read_at(self.base2_, cur.base2_cursor);
            }
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto last(Self& self) -> cursor_type
            requires bounded_sequence<Base1> && bounded_sequence<Base2>
        {
            return cursor_type{flux::last(self.base1_), flux::last(self.base2_)};
        }

        template <typename Self>
            requires maybe_const_iterable<Self>
        static constexpr auto move_at(Self& self, cursor_type const& cur)
            -> std::common_reference_t<decltype(flux::move_at(self.base1_, cur.base1_cursor)), 
                                       decltype(flux::move_at(self.base2_, cur.base2_cursor))>
        {
            if (cur.active_ == cursor_type::first) {
                return flux::move_at(self.base1_, cur.base1_cursor);
            } else {
                return flux::move_at(self.base2_, cur.base2_cursor);
            }
        }
        
    };
};

template <typename T1, typename T2>
concept set_op_compatible =
    std::common_reference_with<element_t<T1>, element_t<T2>> &&
    std::common_reference_with<rvalue_element_t<T1>, rvalue_element_t<T2>> &&
    requires { typename std::common_type_t<value_t<T1>, value_t<T2>>; };

struct set_union_fn {
    template <adaptable_sequence Seq1, adaptable_sequence Seq2, typename Cmp = std::ranges::less>
        requires set_op_compatible<Seq1, Seq2> && 
                 strict_weak_order_for<Cmp, Seq1> && 
                 strict_weak_order_for<Cmp, Seq2> 
    [[nodiscard]]
    constexpr auto operator()(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) const
    {
        return set_union_adaptor<std::decay_t<Seq1>, std::decay_t<Seq2>, Cmp>(FLUX_FWD(seq1), FLUX_FWD(seq2), cmp);
    }
};

} // namespace detail

namespace flux {

inline constexpr auto set_union = detail::set_union_fn{};

} // namespace flux

#endif // namespace FLUX_OP_SET_ALGORITHMS_HPP_INCLUDED