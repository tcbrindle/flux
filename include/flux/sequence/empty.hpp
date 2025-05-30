
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_EMPTY_HPP_INCLUDED
#define FLUX_SEQUENCE_EMPTY_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename T>
struct empty_sequence : inline_sequence_base<empty_sequence<T>> {
    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            friend auto operator==(cursor_type, cursor_type) -> bool = default;
            friend auto operator<=>(cursor_type, cursor_type) = default;
        };

    public:
        static constexpr auto first(empty_sequence) -> cursor_type { return {}; }
        static constexpr auto last(empty_sequence) -> cursor_type { return {}; }
        static constexpr auto is_last(empty_sequence, cursor_type) -> bool { return true; }

        static constexpr auto inc(empty_sequence, cursor_type& cur, int_t = 0) -> cursor_type&
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
        static constexpr auto data(empty_sequence) -> std::add_pointer_t<T> requires std::is_object_v<T> { return nullptr; }

        [[noreturn]]
        static constexpr auto read_at(empty_sequence, cursor_type) -> T&
        {
            runtime_error("Attempted read of flux::empty");
        }
    };
};

} // namespace detail

FLUX_EXPORT
template <typename T>
inline constexpr auto empty = detail::empty_sequence<T>{};

} // namespace flux

#endif // FLUX_SEQUENCE_EMPTY_HPP_INCLUDED
