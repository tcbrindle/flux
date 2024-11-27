
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_REPEAT_HPP_INCLUDED
#define FLUX_SEQUENCE_REPEAT_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <bool>
struct repeat_data {};

template <>
struct repeat_data<false> { std::size_t count; };

template <std::movable T, bool IsInfinite>
struct repeat_sequence : inline_sequence_base<repeat_sequence<T, IsInfinite>>
{
private:
    T obj_;
    FLUX_NO_UNIQUE_ADDRESS repeat_data<IsInfinite> data_;

public:
    constexpr explicit repeat_sequence(decays_to<T> auto&& obj)
        requires IsInfinite
        : obj_(FLUX_FWD(obj))
    {}

    constexpr repeat_sequence(decays_to<T> auto&& obj, std::size_t count)
        requires (!IsInfinite)
        : obj_(FLUX_FWD(obj)),
          data_{count}
    {}

    struct flux_iter_traits : default_iter_traits {
    private:
        using self_t = repeat_sequence;

    public:
        static inline constexpr bool is_infinite = IsInfinite;

        static constexpr auto first(self_t const&) -> std::size_t { return 0; }

        static constexpr auto is_last(self_t const& self, std::size_t cur) -> bool
        {
            if constexpr (IsInfinite) {
                return false;
            } else {
                return cur >= self.data_.count;
            }
        }

        static constexpr auto inc(self_t const&, std::size_t& cur) -> void
        {
            ++cur;
        }

        static constexpr auto read_at(self_t const& self, std::size_t) -> T const&
        {
            return self.obj_;
        }

        static constexpr auto dec(self_t const&, std::size_t& cur) -> void
        {
            --cur;
        }

        static constexpr auto inc(self_t const&, std::size_t& cur, distance_t offset) -> void
        {
            cur += static_cast<std::size_t>(offset);
        }

        static constexpr auto distance(self_t const&, std::size_t from, std::size_t to) -> distance_t
        {
            return num::cast<distance_t>(to) - num::cast<distance_t>(from);
        }

        static constexpr auto for_each_while(self_t const& self, auto&& pred) -> std::size_t
        {
            if constexpr (IsInfinite) {
                std::size_t idx = 0;
                while (true) {
                    if (!std::invoke(pred, std::as_const(self.obj_))) {
                        return idx;
                    }
                    ++idx;
                }
            } else {
                std::size_t idx = 0;
                for ( ; idx < self.data_.count; ++idx) {
                    if (!std::invoke(pred, std::as_const(self.obj_))) {
                        break;
                    }
                }
                return idx;
            }
        }

        static constexpr auto last(self_t const& self) -> std::size_t
            requires (!IsInfinite)
        {
            return self.data_.count;
        }

        static constexpr auto size(self_t const& self) -> distance_t
            requires (!IsInfinite)
        {
            return num::cast<distance_t>(self.data_.count);
        }
    };
};

struct repeat_fn {
    template <typename T>
        requires std::movable<std::decay_t<T>>
    constexpr auto operator()(T&& obj) const
    {
        return repeat_sequence<std::decay_t<T>, true>(FLUX_FWD(obj));
    }

    template <typename T>
        requires std::movable<std::decay_t<T>>
    constexpr auto operator()(T&& obj, num::integral auto count) const
    {
        auto c = num::checked_cast<distance_t>(count);
        if (c < 0) {
            runtime_error("Negative count passed to repeat()");
        }
        return repeat_sequence<std::decay_t<T>, false>(
            FLUX_FWD(obj), num::checked_cast<std::size_t>(c));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto repeat = detail::repeat_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_REPEAT_HPP_INCLUDED
