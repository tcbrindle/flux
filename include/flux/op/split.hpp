
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SPLIT_HPP_INCLUDED
#define FLUX_OP_SPLIT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/find.hpp>
#include <flux/op/from.hpp>
#include <flux/op/search.hpp>
#include <flux/op/slice.hpp>
#include <flux/source/single.hpp>

namespace flux {

namespace detail {

template <multipass_sequence Base, multipass_sequence Pattern>
struct split_adaptor : inline_sequence_base<split_adaptor<Base, Pattern>> {
private:
    Base base_;
    Pattern pattern_;

    friend struct sequence_traits<split_adaptor>;

public:
    constexpr split_adaptor(decays_to<Base> auto&& base, decays_to<Pattern> auto&& pattern)
        : base_(FLUX_FWD(base)),
          pattern_(FLUX_FWD(pattern))
    {}
};

struct split_fn {
    template <adaptable_sequence Seq, adaptable_sequence Pattern>
        requires multipass_sequence<Seq> &&
                 multipass_sequence<Pattern> &&
                 std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pattern&& pattern) const
    {
        return split_adaptor<std::decay_t<Seq>, std::decay_t<Pattern>>(
                    FLUX_FWD(seq), FLUX_FWD(pattern));
    }

    template <adaptable_sequence Seq>
        requires multipass_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, value_t<Seq> delim) const
    {
        return (*this)(FLUX_FWD(seq), flux::single(std::move(delim)));
    }
};

template <typename>
inline constexpr bool is_single_seq = false;

template <typename T>
inline constexpr bool is_single_seq<single_sequence<T>> = true;

} // namespace detail

template <typename Base, typename Pattern>
struct sequence_traits<detail::split_adaptor<Base, Pattern>>
{
private:
    struct cursor_type {
        cursor_t<Base> cur;
        bounds_t<Base> next;
        bool trailing_empty = false;

        friend bool operator==(cursor_type const&, cursor_type const&) = default;
    };

    static constexpr auto find_next(auto& self, auto const& from)
    {
        if constexpr (detail::is_single_seq<decltype(self.pattern_)>) {
            // auto cur = self.base_[{cur, last}].find(self.pattern_.value());
            auto cur = flux::find(flux::slice(self.base_, from, flux::last),
                                  self.pattern_.value());
            if (flux::is_last(self.base_, cur)) {
                return bounds{cur, cur};
            } else {
                return bounds{cur, flux::next(self.base_, cur)};
            }
        } else {
            return flux::search(flux::slice(self.base_, from, flux::last),
                                self.pattern_);
        }
    }

public:

    static constexpr bool is_infinite = infinite_sequence<Base>;

    static constexpr auto first(auto& self) -> cursor_type
    {
        auto bounds = flux::search(self.base_, self.pattern_);
        return cursor_type(flux::first(self.base_), std::move(bounds));
    }

    static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
    {
        return flux::is_last(self.base_, cur.cur) && !cur.trailing_empty;
    }

    static constexpr auto read_at(auto& self, cursor_type const& cur)
        requires sequence<decltype((self.base_))>
    {
        return flux::slice(self.base_, cur.cur, cur.next.from);
    }

    static constexpr auto inc(auto& self, cursor_type& cur)
    {
        cur.cur = cur.next.from;
        if (!flux::is_last(self.base_, cur.cur)) {
            cur.cur = cur.next.to;
            if (flux::is_last(self.base_, cur.cur)) {
                cur.trailing_empty = true;
                cur.next = {cur.cur, cur.cur};
            } else {
                cur.next = find_next(self, cur.cur);
            }
        } else {
            cur.trailing_empty = false;
        }
    }
};

inline constexpr auto split = detail::split_fn{};

template <typename Derived>
template <multipass_sequence Pattern>
    requires std::equality_comparable_with<element_t<Derived>, element_t<Pattern>>
constexpr auto inline_sequence_base<Derived>::split(Pattern&& pattern) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(pattern));
}

template <typename Derived>
template <typename ValueType>
    requires decays_to<ValueType, value_t<Derived>>
constexpr auto inline_sequence_base<Derived>::split(ValueType&& delim) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(delim));
}


} // namespace flux

#endif
