
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED
#define FLUX_CORE_SEQUENCE_ACCESS_HPP_INCLUDED

#include <flux/core/concepts.hpp>

#include <stdexcept>

namespace flux {

namespace detail {

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

struct unchecked_read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        noexcept(noexcept(traits_t<Seq>::read_at(seq, cur))) -> element_t<Seq>
    {
        return traits_t<Seq>::read_at(seq, cur);
    }
};

struct unchecked_inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(traits_t<Seq>::inc(seq, cur))) -> cursor_t<Seq>&
    {
        return traits_t<Seq>::inc(seq, cur);
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur,
                              distance_t offset) const
        noexcept(noexcept(traits_t<Seq>::inc(seq, cur, offset))) -> cursor_t<Seq>&
    {
        return traits_t<Seq>::inc(seq, cur, offset);
    }
};

struct unchecked_dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        noexcept(noexcept(traits_t<Seq>::dec(seq, cur))) -> cursor_t<Seq>&
    {
        return traits_t<Seq>::dec(seq, cur);
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
                unchecked_inc_fn{}(seq, from_);
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
    constexpr auto operator()(Seq& seq) const -> distance_t
    {
        if constexpr (requires { traits_t<Seq>::size(seq); }) {
            return traits_t<Seq>::size(seq);
        } else {
            static_assert(bounded_sequence<Seq> && random_access_sequence<Seq>);
            return distance_fn{}(seq, first_fn{}(seq), last_fn{}(seq));
        }
    }
};

struct usize_fn {
    template <sized_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> std::size_t
    {
        return narrow_cast<std::size_t>(size_fn{}(seq));
    }
};

struct unchecked_move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> rvalue_element_t<Seq>
    {
        if constexpr (requires { traits_t<Seq>::move_at(seq, cur); }) {
            return traits_t<Seq>::move_at(seq, cur);
        } else {
            if constexpr (std::is_lvalue_reference_v<element_t<Seq>>) {
                return std::move(unchecked_read_at_fn{}(seq, cur));
            } else {
                return unchecked_read_at_fn{}(seq, cur);
            }
        }
    }
};

struct check_bounds_fn {
    template <typename Seq>
    [[nodiscard]]
    constexpr bool operator()(Seq& seq, cursor_t<Seq> const& cur) const
    {
        if constexpr (random_access_sequence<Seq>) {
            distance_t dist = distance_fn{}(seq, first_fn{}(seq), cur);
            if (dist < distance_t{0}) {
                return false;
            }
            if constexpr (sized_sequence<Seq>) {
                return dist < size_fn{}(seq);
            }
        }
        return !is_last_fn{}(seq, cur);
    }
};

inline constexpr auto check_bounds = check_bounds_fn{};

struct checked_read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
        -> decltype(unchecked_read_at_fn{}(seq, cur))
    {
        if (!check_bounds(seq, cur)) {
            throw std::out_of_range("Read via an out-of-bounds cursor");
        }
        return unchecked_read_at_fn{}(seq, cur);
    }
};

struct checked_move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, cursor_t<Seq> const& cur) const
            -> decltype(unchecked_move_at_fn{}(seq, cur))
    {
        if (!check_bounds(seq, cur)) {
            throw std::out_of_range("Read via an out-of-bounds cursor");
        }
        return unchecked_move_at_fn{}(seq, cur);
    }
};

struct checked_inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        -> cursor_t<Seq>&
    {
        if (!check_bounds(seq, cur)) {
            throw std::out_of_range("Increment would result in an out-of-bounds cursor");
        }
        return unchecked_inc_fn{}(seq, cur);
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const
        -> cursor_t<Seq>&
    {
        const auto dist = distance_fn{}(seq, first_fn{}(seq), cur);
        if (dist + offset < 0) {
            throw std::out_of_range("Increment with offset would result in an out-of-bounds cursor");
        }
        if constexpr (sized_sequence<Seq>) {
            if (dist + offset > size_fn{}(seq)) {
                throw std::out_of_range("Increment with offset would result in an out-of-bounds cursor");
            }
        }

        return unchecked_inc_fn{}(seq, cur, offset);
    }
};

struct checked_dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur) const
        -> cursor_t<Seq>&
    {
        if (cur == first_fn{}(seq)) {
            throw std::out_of_range("Decrement would result in a before-the-start cursor");
        }
        return unchecked_dec_fn{}(seq, cur);
    }
};

} // namespace detail

inline constexpr auto first = detail::first_fn{};
inline constexpr auto is_last = detail::is_last_fn{};
inline constexpr auto unchecked_read_at = detail::unchecked_read_at_fn{};
inline constexpr auto unchecked_move_at = detail::unchecked_move_at_fn{};
inline constexpr auto unchecked_inc = detail::unchecked_inc_fn{};
inline constexpr auto unchecked_dec = detail::unchecked_dec_fn{};
inline constexpr auto distance = detail::distance_fn{};
inline constexpr auto data = detail::data_fn{};
inline constexpr auto last = detail::last_fn{};
inline constexpr auto size = detail::size_fn{};
inline constexpr auto usize = detail::usize_fn{};
inline constexpr auto checked_read_at = detail::checked_read_at_fn{};
inline constexpr auto checked_move_at = detail::checked_move_at_fn{};
inline constexpr auto checked_inc = detail::checked_inc_fn{};
inline constexpr auto checked_dec = detail::checked_dec_fn{};

#ifdef FLUX_ENABLE_BOUNDS_CHECKING
inline constexpr auto read_at = checked_read_at;
inline constexpr auto move_at = checked_move_at;
inline constexpr auto inc = checked_inc;
inline constexpr auto dec = checked_dec;
#else
inline constexpr auto read_at = unchecked_read_at;
inline constexpr auto move_at = unchecked_move_at;
inline constexpr auto inc = unchecked_inc;
inline constexpr auto dec = unchecked_dec;
#endif

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
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> bool
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
    writable_sequence_of<Seq1, rvalue_element_t<Seq2>> &&
    writable_sequence_of<Seq2, value_t<Seq1>&&>;

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

} // namespace detail


inline constexpr auto next = detail::next_fn{};
inline constexpr auto prev = detail::prev_fn{};
inline constexpr auto is_empty = detail::is_empty_fn{};
inline constexpr auto swap_with = detail::swap_with_fn{};
inline constexpr auto swap_at = detail::swap_at_fn{};

} // namespace flux

#endif
