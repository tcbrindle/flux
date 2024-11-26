
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED
#define FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED

#include <flux/core/concepts.hpp>
#include <flux/core/optional.hpp>
#include <flux/core/numeric.hpp>

namespace flux {

namespace detail {

struct iterate_fn {
    template <iterable It, typename Pred>
        requires std::invocable<Pred&, element_t<It>> &&
                 boolean_testable<std::invoke_result_t<Pred&, element_t<It>>>
    constexpr auto operator()(It&& iter, Pred pred) const -> bool
    {
        return traits_t<It>::iterate(iter, pred);
    }
};

struct first_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(traits_t<Seq>::first(seq))) -> cursor_t<Seq>
    {
        return traits_t<Seq>::first(seq);
    }
};

struct is_last_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        noexcept(noexcept(traits_t<Seq>::is_last(seq, cur))) -> bool
    {
        return traits_t<Seq>::is_last(seq, cur);
    }
};

struct read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        noexcept(noexcept(traits_t<Seq>::read_at(seq, cur))) -> element_t<Seq>
    {
        return traits_t<Seq>::read_at(seq, cur);
    }
};

struct inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(traits_t<Seq>::inc(seq, cur))) -> cursor_t<Seq>&
    {
        (void) traits_t<Seq>::inc(seq, cur);
        return cur;
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const
        noexcept(noexcept(traits_t<Seq>::inc(seq, cur, offset))) -> cursor_t<Seq>&
    {
        (void) traits_t<Seq>::inc(seq, cur, offset);
        return cur;
    }
};

struct dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(traits_t<Seq>::dec(seq, cur))) -> cursor_t<Seq>&
    {
        (void) traits_t<Seq>::dec(seq, cur);
        return cur;
    }
};

struct distance_fn {
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& from,
                                            cursor_t<Seq> const& to) const
        -> distance_t
    {
        if constexpr (random_access_sequence<Seq>) {
            return traits_t<Seq>::distance(seq, from, to);
        } else {
            distance_t n = 0;
            auto from_ = from;
            while (from_ != to) {
                inc_fn{}(seq, from_);
                ++n;
            }
            return n;
        }
    }
};

struct data_fn {
    template <contiguous_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(traits_t<Seq>::data(seq)))
    {
        return traits_t<Seq>::data(seq);
    }
};

struct last_fn {
    template <bounded_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(traits_t<Seq>::last(seq))) -> cursor_t<Seq>
    {
        return traits_t<Seq>::last(seq);
    }
};

struct size_fn {
    template <sized_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> distance_t
    {
        return traits_t<Seq>::size(seq);
    }
};

struct usize_fn {
    template <sized_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> std::size_t
    {
        return num::unchecked_cast<std::size_t>(size_fn{}(seq));
    }
};

struct move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> rvalue_element_t<Seq>
    {
        return traits_t<Seq>::move_at(seq, cur);
    }
};

struct read_at_unchecked_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> element_t<Seq>
    {
        return traits_t<Seq>::read_at_unchecked(seq, cur);
    }
};

struct move_at_unchecked_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> rvalue_element_t<Seq>
    {
        return traits_t<Seq>::move_at_unchecked(seq, cur);
    }
};

struct for_each_while_fn {
    template <sequence Seq, typename Pred>
        requires std::invocable<Pred&, element_t<Seq>> &&
        boolean_testable<std::invoke_result_t<Pred&, element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, Pred pred) const -> cursor_t<Seq>
    {
        return traits_t<Seq>::for_each_while(seq, std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto iterate = detail::iterate_fn{};
FLUX_EXPORT inline constexpr auto first = detail::first_fn{};
FLUX_EXPORT inline constexpr auto is_last = detail::is_last_fn{};
FLUX_EXPORT inline constexpr auto read_at = detail::read_at_fn{};
FLUX_EXPORT inline constexpr auto move_at = detail::move_at_fn{};
FLUX_EXPORT inline constexpr auto read_at_unchecked = detail::read_at_unchecked_fn{};
FLUX_EXPORT inline constexpr auto move_at_unchecked = detail::move_at_unchecked_fn{};
FLUX_EXPORT inline constexpr auto inc = detail::inc_fn{};
FLUX_EXPORT inline constexpr auto dec = detail::dec_fn{};
FLUX_EXPORT inline constexpr auto distance = detail::distance_fn{};
FLUX_EXPORT inline constexpr auto data = detail::data_fn{};
FLUX_EXPORT inline constexpr auto last = detail::last_fn{};
FLUX_EXPORT inline constexpr auto size = detail::size_fn{};
FLUX_EXPORT inline constexpr auto usize = detail::usize_fn{};
FLUX_EXPORT inline constexpr auto for_each_while = detail::for_each_while_fn{};

namespace detail {

struct next_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> cur) const
        noexcept(noexcept(inc(seq, cur)))
        -> cursor_t<Seq>
    {
        return inc(seq, cur);
    }

    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> cur, distance_t offset) const
        -> cursor_t<Seq>
    {
        if constexpr (random_access_sequence<Seq>) {
            return inc(seq, cur, offset);
        } else if constexpr (bidirectional_sequence<Seq>) {
            auto const zero = distance_t{0};
            if (offset > zero) {
                while (offset-- > zero) {
                    inc(seq, cur);
                }
            } else {
                while (offset++ < zero) {
                    dec(seq, cur);
                }
            }
            return cur;
        } else {
            while (offset-- > distance_t{0}) {
                inc(seq, cur);
            }
            return cur;
        }
    }
};

struct prev_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> cur) const
        noexcept(noexcept(dec(seq, cur)))
        -> cursor_t<Seq>
    {
        return dec(seq, cur);
    }
};

struct is_empty_fn {
    template <sequence Seq>
        requires (multipass_sequence<Seq> || sized_sequence<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const -> bool
    {
        if constexpr (sized_sequence<Seq>) {
            return flux::size(seq) == 0;
        } else {
            return is_last(seq, first(seq));
        }
    }
};

template <typename Seq1, typename Seq2>
concept element_swappable_with_ =
    std::constructible_from<value_t<Seq1>, rvalue_element_t<Seq1>> &&
    writable_iterable_of<Seq1, rvalue_element_t<Seq2>> &&
    writable_iterable_of<Seq2, value_t<Seq1>&&>;

template <typename Seq1, typename Seq2>
concept element_swappable_with =
    element_swappable_with_<Seq1, Seq2> &&
    element_swappable_with_<Seq2, Seq1>;

struct swap_with_fn {
    template <sequence Seq1, sequence Seq2>
    constexpr void operator()(Seq1& seq1, cursor_t<Seq1> const& cur1,
                              Seq2& seq2, cursor_t<Seq2> const& cur2) const
        requires (std::swappable_with<element_t<Seq1>, element_t<Seq2>> ||
                  element_swappable_with<Seq1, Seq2>)
    {
        if constexpr (std::swappable_with<element_t<Seq1>, element_t<Seq2>>) {
            return std::ranges::swap(read_at(seq1, cur1), read_at(seq2, cur2));
        } else {
            value_t<Seq1> temp(move_at(seq1, cur1));
            read_at(seq1, cur1) = move_at(seq2, cur2);
            read_at(seq2, cur2) = std::move(temp);
        }
    }
};

struct swap_at_fn {
    template <sequence Seq>
    constexpr void operator()(Seq& seq, cursor_t<Seq> const& first,
                              cursor_t<Seq> const& second) const
        requires requires { swap_with_fn{}(seq, first, seq, second); }
    {
        return swap_with_fn{}(seq, first, seq, second);
    }
};

struct front_fn {
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> optional<element_t<Seq>>
    {
        auto cur = first(seq);
        if (!is_last(seq, cur)) {
            return optional<element_t<Seq>>(read_at(seq, cur));
        } else {
            return nullopt;
        }
    }
};

struct back_fn {
    template <bidirectional_sequence Seq>
        requires bounded_sequence<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> optional<element_t<Seq>>
    {
        auto cur = last(seq);
        if (cur != first(seq)) {
            return optional<element_t<Seq>>(read_at(seq, dec(seq, cur)));
        } else {
            return nullopt;
        }
    }
};

} // namespace detail


FLUX_EXPORT inline constexpr auto next = detail::next_fn{};
FLUX_EXPORT inline constexpr auto prev = detail::prev_fn{};
FLUX_EXPORT inline constexpr auto is_empty = detail::is_empty_fn{};
FLUX_EXPORT inline constexpr auto swap_with = detail::swap_with_fn{};
FLUX_EXPORT inline constexpr auto swap_at = detail::swap_at_fn{};
FLUX_EXPORT inline constexpr auto front = detail::front_fn{};
FLUX_EXPORT inline constexpr auto back = detail::back_fn{};

} // namespace flux

#endif
