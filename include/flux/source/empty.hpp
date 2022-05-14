
#ifndef FLUX_SOURCE_EMPTY_HPP_INCLUDED
#define FLUX_SOURCE_EMPTY_HPP_INCLUDED

#include <flux/core/lens_base.hpp>

#include <cassert>

namespace flux {

namespace detail {

template <typename T>
    requires std::is_object_v<T>
struct empty_sequence {
private:
    struct cursor_type {
        friend constexpr bool operator==(cursor_type, cursor_type) = default;
        friend constexpr auto operator<=>(cursor_type, cursor_type) = default;
    };

public:
    static constexpr auto first() -> cursor_type { return {}; }
    static constexpr auto last() -> cursor_type { return {}; }
    static constexpr auto is_last(cursor_type) -> bool { return true; }
    static constexpr auto inc(cursor_type& cur, std::ptrdiff_t = 0) -> cursor_type& { return cur; }
    static constexpr auto dec(cursor_type& cur) -> cursor_type& { return cur; }
    static constexpr auto distance(cursor_type, cursor_type) -> std::ptrdiff_t { return 0; }
    static constexpr auto size() -> std::ptrdiff_t { return 0; }
    static constexpr auto data() -> T* { return nullptr; }

    static constexpr auto read_at(cursor_type) -> T&
    {
        assert(false && "Attempted read of flux::empty");
        /* Guaranteed UB... */
        return *static_cast<T*>(nullptr);
    }
};

} // namespace detail

template <typename T>
    requires std::is_object_v<T>
inline constexpr auto empty = detail::empty_sequence<T>{};

} // namespace flux

#endif // FLUX_SOURCE_EMPTY_HPP_INCLUDED
