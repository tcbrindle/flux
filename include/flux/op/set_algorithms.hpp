
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SET_ALGORITHMS_HPP_INCLUDED
#define FLUX_OP_SET_ALGORITHMS_HPP_INCLUDED

#include <flux/core.hpp>

#include <utility>

namespace flux::detail {

template <sequence Base1, sequence Base2, typename Cmp>
struct set_union_adaptor
    : flux::inline_sequence_base<set_union_adaptor<Base1, Base2, Cmp>>
{
private:
    Base1 base1_;
    Base2 base2_;
    Cmp cmp_;

public:
    constexpr set_union_adaptor(decays_to<Base1> auto&& base1, decays_to<Base2> auto&& base2, Cmp cmp)
        : base1_(std::move(base1)),
          base2_(std::move(base2)),
          cmp_(cmp)
    {}

    struct flux_sequence_traits {
    private:

        struct cursor_type {
            cursor_t<Base1> base1_cursor;
            cursor_t<Base2> base2_cursor;
            enum {FIRST, SECOND} active_ = FIRST;
        };

    public:
        using value_type = value_t<Base1>;

        inline static constexpr bool is_infinite = flux::infinite_sequence<Base1> or flux::infinite_sequence<Base2>;

        template <typename Self>
        static constexpr void update(Self& self, cursor_type& cur) {
            if (flux::is_last(self.base1_, cur.base1_cursor)) {
                cur.active_ = cursor_type::SECOND;
                return;
            }

            if (flux::is_last(self.base2_, cur.base2_cursor)) {
                cur.active_ = cursor_type::FIRST;
                return;
            }

            if (std::invoke(self.cmp_, flux::read_at(self.base2_, cur.base2_cursor), 
                                       flux::read_at(self.base1_, cur.base1_cursor))) {
                cur.active_ = cursor_type::SECOND;
                return;
            }

            if (not std::invoke(self.cmp_, flux::read_at(self.base1_, cur.base1_cursor), 
                                           flux::read_at(self.base2_, cur.base2_cursor))) {
                flux::inc(self.base2_, cur.base2_cursor);
            }

            cur.active_ = cursor_type::FIRST;
        }

        template <typename Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            auto cur = cursor_type{.base1_cursor = flux::first(self.base1_),
                                   .base2_cursor = flux::first(self.base2_)};
            update(self, cur);
            return cur;
        }

        template <typename Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base1_, cur.base1_cursor) and 
                   flux::is_last(self.base2_, cur.base2_cursor);
        }

        template <typename Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            if (cur.active_ == cursor_type::FIRST) {
                flux::inc(self.base1_, cur.base1_cursor);
            }

            if (cur.active_ == cursor_type::SECOND) {
                flux::inc(self.base2_, cur.base2_cursor);
            }

            update(self, cur);
        }

        template <typename Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur)
            -> decltype(auto)
        {
            return cur.active_ == cursor_type::FIRST ? 
                        flux::read_at(self.base1_, cur.base1_cursor):
                        flux::read_at(self.base2_, cur.base2_cursor);
        }
    };
};

template <typename T1, typename T2>
concept have_common_ref =
    requires { typename std::common_reference_t<T1, T2>; } and
    (std::convertible_to<T1, std::common_reference_t<T1, T2>> and 
     std::convertible_to<T1, std::common_reference_t<T1, T2>>);

template <typename T1, typename T2>
concept compatible =
    have_common_ref<element_t<T1>, element_t<T2>> and
    have_common_ref<rvalue_element_t<T1>, rvalue_element_t<T2>> and
    requires { typename std::common_type_t<value_t<T1>, value_t<T2>>; };

struct set_union_fn {
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less>
        requires compatible<Seq1, Seq2> and 
                 strict_weak_order_for<Cmp, Seq1> and 
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

template <typename D>
template <sequence Seq, typename Cmp>
constexpr auto inline_sequence_base<D>::set_union(Seq&& seq, Cmp cmp)
{
    return flux::set_union(derived(), FLUX_FWD(seq), std::move(cmp));
}

} // namespace flux

#endif // namespace FLUX_OP_SET_ALGORITHMS_HPP_INCLUDED