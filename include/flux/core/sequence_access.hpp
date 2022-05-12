
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
        noexcept(noexcept(iface_t<Seq>::first(seq))) -> index_t<Seq>
    {
        return iface_t<Seq>::first(seq);
    }
};

struct is_last_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> const& idx) const
        noexcept(noexcept(iface_t<Seq>::is_last(seq, idx))) -> bool
    {
        return iface_t<Seq>::is_last(seq, idx);
    }
};

struct unchecked_read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> const& idx) const
        noexcept(noexcept(iface_t<Seq>::read_at(seq, idx))) -> element_t<Seq>
    {
        return iface_t<Seq>::read_at(seq, idx);
    }
};

struct unchecked_inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq>& idx) const
        noexcept(noexcept(iface_t<Seq>::inc(seq, idx))) -> index_t<Seq>&
    {
        return iface_t<Seq>::inc(seq, idx);
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq>& idx,
                              distance_t<Seq> offset) const
        noexcept(noexcept(iface_t<Seq>::inc(seq, idx, offset))) -> index_t<Seq>&
    {
        return iface_t<Seq>::inc(seq, idx, offset);
    }
};

struct unchecked_dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq>& idx) const
        noexcept(noexcept(iface_t<Seq>::dec(seq, idx))) -> index_t<Seq>&
    {
        return iface_t<Seq>::dec(seq, idx);
    }
};

struct distance_fn {
    template <multipass_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq,
                              index_t<Seq> const& from,
                              index_t<Seq> const& to) const
        -> distance_t<Seq>
    {
        if constexpr (random_access_sequence<Seq>) {
            return iface_t<Seq>::distance(seq, from, to);
        } else {
            distance_t<Seq> n = 0;
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
        noexcept(noexcept(iface_t<Seq>::data(seq)))
    {
        return iface_t<Seq>::data(seq);
    }
};

struct last_fn {
    template <bounded_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
        noexcept(noexcept(iface_t<Seq>::last(seq))) -> index_t<Seq>
    {
        return iface_t<Seq>::last(seq);
    }
};

struct size_fn {
    template <sized_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const -> distance_t<Seq>
    {
        if constexpr (requires { iface_t<Seq>::size(seq); }) {
            return iface_t<Seq>::size(seq);
        } else {
            static_assert(bounded_sequence<Seq> && random_access_sequence<Seq>);
            return distance_fn{}(seq, first_fn{}(seq), last_fn{}(seq));
        }
    }
};

struct unchecked_move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> const& idx) const
        -> rvalue_element_t<Seq>
    {
        if constexpr (requires { iface_t<Seq>::move_at(seq, idx); }) {
            return iface_t<Seq>::move_at(seq, idx);
        } else {
            if constexpr (std::is_lvalue_reference_v<element_t<Seq>>) {
                return std::move(unchecked_read_at_fn{}(seq, idx));
            } else {
                return unchecked_read_at_fn{}(seq, idx);
            }
        }
    }
};

struct check_bounds_fn {
    template <typename Seq>
    [[nodiscard]]
    constexpr bool operator()(Seq& seq, index_t<Seq> const& idx) const
    {
        if constexpr (random_access_sequence<Seq>) {
            distance_t<Seq> dist = distance_fn{}(seq, first_fn{}(seq), idx);
            if (dist < distance_t<Seq>{0}) {
                return false;
            }
            if constexpr (sized_sequence<Seq>) {
                if (dist >= size_fn{}(seq)) {
                    return false;
                }
            }
        }
        return !is_last_fn{}(seq, idx);
    }
};

inline constexpr auto check_bounds = check_bounds_fn{};

struct checked_read_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> const& idx) const
        -> decltype(unchecked_read_at_fn{}(seq, idx))
    {
        if (!check_bounds(seq, idx)) {
            throw std::out_of_range("Read via an out-of-bounds index");
        }
        return unchecked_read_at_fn{}(seq, idx);
    }
};

struct checked_move_at_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> const& idx) const
            -> decltype(unchecked_move_at_fn{}(seq, idx))
    {
        if (!check_bounds(seq, idx)) {
            throw std::out_of_range("Read via an out-of-bounds index");
        }
        return unchecked_move_at_fn{}(seq, idx);
    }
};

struct checked_inc_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq>& idx) const
        -> index_t<Seq>&
    {
        if (!check_bounds(seq, idx)) {
            throw std::out_of_range("Increment would result in an out-of-bounds index");
        }
        return unchecked_inc_fn{}(seq, idx);
    }

    template <random_access_sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq>& idx, distance_t<Seq> offset) const
        -> index_t<Seq>&
    {
        const auto dist = distance_fn{}(seq, first_fn{}(seq), idx);
        if (dist + offset < 0) {
            throw std::out_of_range("Increment with offset would result in an out-of-bounds index");
        }
        if constexpr (sized_sequence<Seq>) {
            if (dist + offset > size_fn{}(seq)) {
                throw std::out_of_range("Increment with offset would result in an out-of-bounds index");
            }
        }

        return unchecked_inc_fn{}(seq, idx, offset);
    }
};

struct checked_dec_fn {
    template <bidirectional_sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq>& idx) const
        -> index_t<Seq>&
    {
        if (idx == first_fn{}(seq)) {
            throw std::out_of_range("Decrement would result in a before-the-start index");
        }
        return unchecked_dec_fn{}(seq, idx);
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
inline constexpr auto checked_read_at = detail::checked_read_at_fn{};
inline constexpr auto checked_move_at = detail::checked_read_at_fn{};
inline constexpr auto checked_inc = detail::checked_inc_fn{};
inline constexpr auto checked_dec = detail::checked_dec_fn{};

inline constexpr auto read_at = unchecked_read_at;
inline constexpr auto move_at = unchecked_move_at;
inline constexpr auto inc = unchecked_inc;
inline constexpr auto dec = unchecked_dec;


namespace detail {

struct next_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> idx) const
        noexcept(noexcept(inc(seq, idx)))
        -> index_t<Seq>
    {
        return inc(seq, idx);
    }

    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> idx, distance_t<Seq> offset) const
        -> index_t<Seq>
    {
        if constexpr (random_access_sequence<Seq>) {
            return inc(seq, idx, offset);
        } else if constexpr (bidirectional_sequence<Seq>) {
            const auto zero = distance_t<Seq>{0};
            if (offset > zero) {
                while (offset-- > zero) {
                    inc(seq, idx);
                }
            } else {
                while (offset++ < zero) {
                    dec(seq, idx);
                }
            }
            return idx;
        } else {
            while (offset-- > distance_t<Seq>{0}) {
                inc(seq, idx);
            }
            return idx;
        }
    }
};

struct prev_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq, index_t<Seq> idx) const
        noexcept(noexcept(dec(seq, idx)))
        -> index_t<Seq>
    {
        return dec(seq, idx);
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
    constexpr void operator()(Seq1& seq1, index_t<Seq1> const& idx1,
                              Seq2& seq2, index_t<Seq2> const& idx2) const
        requires (std::swappable_with<element_t<Seq1>, element_t<Seq2>> ||
                  element_swappable_with<Seq1, Seq2>)
    {
        if constexpr (std::swappable_with<element_t<Seq1>, element_t<Seq2>>) {
            return std::ranges::swap(read_at(seq1, idx1), read_at(seq2, idx2));
        } else {
            value_t<Seq1> temp(move_at(seq1, idx1));
            read_at(seq1, idx1) = move_at(seq2, idx2);
            read_at(seq2, idx2) = std::move(temp);
        }
    }
};

struct swap_at_fn {
    template <sequence Seq>
    constexpr void operator()(Seq& seq, index_t<Seq> const& first, index_t<Seq> const& second) const
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
