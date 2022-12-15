
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_EMPTY_HPP_INCLUDED
#define FLUX_SOURCE_EMPTY_HPP_INCLUDED

#include <flux/core.hpp>

#include <cassert>

namespace flux {

namespace detail {

template <typename T>
    requires std::is_object_v<T>
struct empty_sequence : inline_sequence_base<empty_sequence<T>> {
    struct flux_sequence_traits {
    private:
        struct cursor_type {
            friend auto operator==(cursor_type, cursor_type) -> bool = default;
            friend auto operator<=>(cursor_type, cursor_type) = default;
        };

    public:
        static constexpr auto first(empty_sequence) -> cursor_type { return {}; }
        static constexpr auto last(empty_sequence) -> cursor_type { return {}; }
        static constexpr auto is_last(empty_sequence, cursor_type) -> bool { return true; }

        static constexpr auto inc(empty_sequence, cursor_type& cur, distance_t = 0)
            -> cursor_type&
        {
            return cur;
        }

        static constexpr auto dec(empty_sequence, cursor_type& cur) -> cursor_type&
        {
            return cur;
        }

        static constexpr auto distance(empty_sequence, cursor_type, cursor_type)
            -> std::ptrdiff_t
        {
            return 0;
        }

        static constexpr auto size(empty_sequence) -> std::ptrdiff_t { return 0; }
        static constexpr auto data(empty_sequence) -> T* { return nullptr; }

        static constexpr auto read_at(empty_sequence, cursor_type) -> T&
        {
            assert(false && "Attempted read of flux::empty");
            /* Guaranteed UB... */
            return *static_cast<T*>(nullptr);
        }
    };
};

} // namespace detail

template <typename T>
    requires std::is_object_v<T>
inline constexpr auto empty = detail::empty_sequence<T>{};

} // namespace flux

#endif // FLUX_SOURCE_EMPTY_HPP_INCLUDED
