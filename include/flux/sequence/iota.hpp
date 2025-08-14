
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_IOTA_HPP_INCLUDED
#define FLUX_SEQUENCE_IOTA_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

// These concepts mirror the standard ones, except that iter_difference_t is not required
template <typename T>
concept incrementable =
    std::regular<T> &&
    requires (T t) {
        { ++t } -> std::same_as<T&>;
        { t++ } -> std::same_as<T>;
    };

template <typename T>
concept decrementable =
    incrementable<T> &&
    requires (T t) {
        { --t } -> std::same_as<T&>;
        { t-- } -> std::same_as<T>;
    };

template <typename T>
concept advancable = decrementable<T> && std::weakly_incrementable<T> && // iter_difference_t exists
    requires(T t, T const u, std::iter_difference_t<T> o) {
        { t += o } -> std::same_as<T&>;
        { t -= o } -> std::same_as<T&>;
        T(u + o);
        T(o + u);
        T(u - o);
        { u - u } -> std::convertible_to<int_t>;
    };

template <incrementable T>
struct iota_iterable : inline_sequence_base<iota_iterable<T>> {
    T from;

    struct iteration_context : immovable {
        T value;

        using element_type = T;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            while (true) {
                if (!pred(value++)) {
                    return iteration_result::incomplete;
                }
            }
        }
    };

    constexpr auto iterate() const -> iteration_context { return iteration_context{.value = from}; }
};

template <incrementable T>
struct bounded_iota_iterable : inline_sequence_base<bounded_iota_iterable<T>> {
    T from;
    T to;

    struct iteration_context : immovable {
        T value;
        T last;

        using element_type = T;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            while (value != last) {
                if (!pred(value++)) {
                    return iteration_result::incomplete;
                }
            }
            return iteration_result::complete;
        }
    };

    struct reverse_iteration_context : immovable {
        T value;
        T first;

        using element_type = T;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            while (value != first) {
                if (!pred(static_cast<T>(--value))) {
                    return iteration_result::incomplete;
                }
            }
            return iteration_result::complete;
        }
    };

    constexpr auto iterate() const -> iteration_context
    {
        return iteration_context{.value = from, .last = to};
    }

    constexpr auto reverse_iterate() const -> reverse_iteration_context
        requires decrementable<T>
    {
        return reverse_iteration_context{.value = to, .first = from};
    }

    constexpr auto size() const -> int_t
        requires advancable<T>
    {
        return num::cast<int_t>(to - from);
    }
};

template <incrementable T>
    requires std::totally_ordered<T>
struct iota_sequence : inline_sequence_base<iota_sequence<T>> {
    T from;
    T to;

    struct flux_sequence_traits : default_sequence_traits {
        using self_t = iota_sequence;

        struct cursor_type {
            T current;

            friend auto operator<=>(cursor_type const&, cursor_type const&) -> std::strong_ordering
                = default;
        };

        static constexpr auto first(self_t const& self) -> cursor_type { return {self.from}; }

        static constexpr auto last(self_t const& self) -> cursor_type { return {self.to}; }

        static constexpr auto is_last(self_t const& self, cursor_type const& cur) -> bool
        {
            return cur.current == self.to;
        }

        static constexpr void inc(self_t const&, cursor_type& cur) { ++cur.current; }

        static constexpr void dec(self_t const&, cursor_type& cur)
            requires decrementable<T>
        {
            --cur.current;
        }

        static constexpr auto read_at(self_t const&, cursor_type const& cur) -> T
        {
            return cur.current;
        }

        static constexpr void inc(self_t const&, cursor_type& cur, int_t offset)
            requires advancable<T>
        {
            cur.current += num::cast<std::iter_difference_t<T>>(offset);
        }

        static constexpr auto distance(self_t const&, cursor_type const& from,
                                       cursor_type const& to) -> int_t
            requires advancable<T>
        {
            return num::cast<int_t>(to.current - from.current);
        }
    };
};

struct iota_t {
    template <incrementable T>
    constexpr auto operator()(T from, T to) const
    {
        if constexpr (std::three_way_comparable<T, std::strong_ordering>) {
            return iota_sequence<T>{.from = from, .to = to};
        } else {
            return bounded_iota_iterable<T>{.from = from, .to = to};
        }
    }

    template <incrementable T>
    constexpr auto operator()(T from) const
    {
        if constexpr (std::is_arithmetic_v<T>) {
            return (*this)(from, std::numeric_limits<T>::max());
        } else {
            return iota_iterable<T>{.from = from};
        }
    }
};

struct ints_t {
    inline constexpr auto operator()() const { return iota_t{}(int_t{}); }

    inline constexpr auto operator()(int_t from) const { return iota_t{}(from); }

    inline constexpr auto operator()(int_t from, int_t to) const { return iota_t{}(from, to); }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto iota = detail::iota_t{};
FLUX_EXPORT inline constexpr auto ints = detail::ints_t{};

} // namespace flux

#endif // FLUX_SEQUENCE_IOTA_HPP_INCLUDED
