
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
concept advancable =
    decrementable<T> &&
    std::totally_ordered<T> &&
    std::weakly_incrementable<T> && // iter_difference_t exists
    requires (T t, T const u, std::iter_difference_t<T> o) {
        { t += o } -> std::same_as<T&>;
        { t -= o } -> std::same_as<T&>;
        T(u + o);
        T(o + u);
        T(u - o);
        { u - u } -> std::convertible_to<distance_t>;
    };

struct iota_traits {
    bool has_start;
    bool has_end;
};

template <incrementable T, iota_traits Traits>
struct iota_sequence_traits : default_iter_traits {
    using cursor_type = T;

    static constexpr bool is_infinite = !Traits.has_end;

    static constexpr auto first(auto& self) -> cursor_type
    {
        if constexpr (Traits.has_start) {
            return self.start_;
        } else {
            return cursor_type{};
        }
    }

    static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
    {
        if constexpr (Traits.has_end) {
            return cur == self.end_;
        } else {
            return false;
        }
    }

    static constexpr auto inc(auto&, cursor_type& cur) -> cursor_type&
    {
        return ++cur;
    }

    static constexpr auto read_at(auto&, cursor_type const& cur) -> T
    {
        return cur;
    }

    static constexpr auto last(auto& self) -> cursor_type
        requires (Traits.has_end)
    {
        return self.end_;
    }

    static constexpr auto dec(auto&, cursor_type& cur) -> cursor_type&
        requires decrementable<T>
    {
        return --cur;
    }

    static constexpr auto inc(auto&, cursor_type& cur, distance_t offset)
        -> cursor_type&
        requires advancable<T>
    {
        return cur += num::cast<std::iter_difference_t<T>>(offset);
    }

    static constexpr auto distance(auto&, cursor_type const& from, cursor_type const& to)
        requires advancable<T>
    {
        return from <= to ? num::cast<distance_t>(to - from) : -num::cast<distance_t>(from - to);
    }

    static constexpr auto size(auto& self) -> distance_t
        requires advancable<T> && (Traits.has_start && Traits.has_end)
    {
        return num::cast<distance_t>(self.end_ - self.start_);
    }
};

template <typename T>
struct basic_iota_sequence : inline_sequence_base<basic_iota_sequence<T>> {
    using flux_iter_traits = iota_sequence_traits<T, iota_traits{}>;
    friend flux_iter_traits;
};

template <typename T>
struct iota_sequence : inline_sequence_base<iota_sequence<T>> {
private:
    T start_;

    static constexpr iota_traits traits{.has_start = true, .has_end = false};

public:
    inline constexpr explicit iota_sequence(T from)
        : start_(std::move(from))
    {}

    using flux_iter_traits = iota_sequence_traits<T, traits>;
    friend flux_iter_traits;
};

template <typename T>
struct bounded_iota_sequence : inline_sequence_base<bounded_iota_sequence<T>> {
    T start_;
    T end_;

    static constexpr iota_traits traits{.has_start = true, .has_end = true};

public:
    inline constexpr bounded_iota_sequence(T from, T to)
        : start_(std::move(from)),
          end_(std::move(to))
    {}

    using flux_iter_traits = iota_sequence_traits<T, traits>;
    friend flux_iter_traits;
};

struct iota_fn {
    template <incrementable T>
    constexpr auto operator()(T from) const
    {
        return iota_sequence<T>(std::move(from));
    }

    template <incrementable T>
    constexpr auto operator()(T from, T to) const
    {
        return bounded_iota_sequence<T>(std::move(from), std::move(to));
    }
};

struct ints_fn {
    inline constexpr auto operator()() const
    {
        return basic_iota_sequence<distance_t>();
    }

    inline constexpr auto operator()(distance_t from) const
    {
        return iota_sequence<distance_t>(from);
    }

    inline constexpr auto operator()(distance_t from, distance_t to) const
    {
        return bounded_iota_sequence<distance_t>(from, to);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto iota = detail::iota_fn{};
FLUX_EXPORT inline constexpr auto ints = detail::ints_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_IOTA_HPP_INCLUDED
