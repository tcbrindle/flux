
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_SINGLE_HPP_INCLUDED
#define FLUX_SOURCE_SINGLE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <std::movable T>
struct single_sequence : inline_sequence_base<single_sequence<T>> {
private:
    T obj_;

    friend struct sequence_traits<single_sequence>;

public:
    constexpr single_sequence()
        requires std::default_initializable<T>
    = default;

    constexpr explicit single_sequence(T const& obj)
        requires std::copy_constructible<T>
    : obj_(obj)
    {}

    constexpr explicit single_sequence(T&& obj)
        requires std::move_constructible<T>
    : obj_(std::move(obj))
    {}

    template <typename... Args>
    constexpr explicit single_sequence(std::in_place_t, Args&&... args)
        requires std::constructible_from<T, Args...>
    : obj_(FLUX_FWD(args)...)
    {}

    constexpr auto value() -> T& { return obj_; }
    constexpr auto value() const -> T const& { return obj_; }
};

struct single_fn {
    template <typename T>
    constexpr auto operator()(T&& t) const -> single_sequence<std::decay_t<T>>
    {
        return single_sequence<std::decay_t<T>>(FLUX_FWD(t));
    }
};

} // namespace detail

template <typename T>
struct sequence_traits<detail::single_sequence<T>>
{
private:
    using self_t = detail::single_sequence<T>;

    enum class cursor_type : bool { valid, done };

public:

    static constexpr auto first(self_t const&) { return cursor_type::valid; }

    static constexpr auto last(self_t const&) { return cursor_type::done; }

    static constexpr bool is_last(self_t const&, cursor_type cur)
    {
        return cur == cursor_type::done;
    }

    static constexpr auto read_at(auto& self, [[maybe_unused]] cursor_type cur) -> auto&
    {
        FLUX_DEBUG_ASSERT(cur == cursor_type::valid);
        return self.obj_;
    }

    static constexpr auto inc(self_t const&, cursor_type& cur) -> cursor_type&
    {
        FLUX_DEBUG_ASSERT(cur == cursor_type::valid);
        cur = cursor_type::done;
        return cur;
    }

    static constexpr auto dec(self_t const&, cursor_type& cur) -> cursor_type&
    {
        FLUX_DEBUG_ASSERT(cur == cursor_type::done);
        cur = cursor_type::valid;
        return cur;
    }

    static constexpr auto inc(self_t const&, cursor_type& cur, distance_t off)
        -> cursor_type&
    {
        if (off > 0) {
            FLUX_DEBUG_ASSERT(cur == cursor_type::valid && off == 1);
            cur = cursor_type::done;
        } else if (off < 0) {
            FLUX_DEBUG_ASSERT(cur == cursor_type::done && off == -1);
            cur = cursor_type::valid;
        }
        return cur;
    }

    static constexpr auto distance(self_t const&, cursor_type from,
                                   cursor_type to)
        -> std::ptrdiff_t
    {
        return static_cast<int>(to) - static_cast<int>(from);
    }

    static constexpr auto size(self_t const&) { return 1; }

    static constexpr auto data(auto& self)
    {
        return std::addressof(self.obj_);
    }

    static constexpr auto for_each_while(auto& self, auto&& pred)
    {
        return std::invoke(pred, self.obj_) ? cursor_type::done : cursor_type::valid;
    }

};

inline constexpr auto single = detail::single_fn{};

} // namespace flux

#endif // FLUX_SOURCE_SINGLE_HPP_INCLUDED
