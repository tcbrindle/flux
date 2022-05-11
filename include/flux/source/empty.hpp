
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
    struct index_type {
        friend constexpr bool operator==(index_type, index_type) = default;
        friend constexpr auto operator<=>(index_type, index_type) = default;
    };

public:
    static constexpr auto first() -> index_type { return {}; }
    static constexpr auto last() -> index_type { return {}; }
    static constexpr auto is_last(index_type) -> bool { return true; }
    static constexpr auto inc(index_type& idx, std::ptrdiff_t = 0) -> index_type& { return idx; }
    static constexpr auto dec(index_type& idx) -> index_type& { return idx; }
    static constexpr auto distance(index_type, index_type) -> std::ptrdiff_t { return 0; }
    static constexpr auto size() -> std::ptrdiff_t { return 0; }
    static constexpr auto data() -> T* { return nullptr; }

    static constexpr auto read_at(index_type) -> T&
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
