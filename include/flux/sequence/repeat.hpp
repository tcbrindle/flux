
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_REPEAT_HPP_INCLUDED
#define FLUX_SEQUENCE_REPEAT_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename T>
struct repeat_iterable : inline_sequence_base<repeat_iterable<T>> {
private:
    T obj_;

    struct context_type : immovable {
    private:
        T const* ptr_;

    public:
        constexpr explicit context_type(T const& obj) : ptr_(std::addressof(obj)) { }

        using element_type = T const&;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            while (true) {
                if (!pred(*ptr_)) {
                    return iteration_result::incomplete;
                }
            }
        }
    };

public:
    constexpr explicit repeat_iterable(decays_to<T> auto&& value) : obj_(FLUX_FWD(value)) { }

    constexpr auto iterate() const { return context_type(obj_); }
};

template <typename T>
struct repeat_sequence : inline_sequence_base<repeat_sequence<T>> {
private:
    T obj_;
    int_t count_;

public:
    constexpr repeat_sequence(decays_to<T> auto&& obj, int_t count)
        : obj_(FLUX_FWD(obj)),
          count_(count)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        using self_t = repeat_sequence;

    public:
        static constexpr auto first(self_t const&) -> int_t { return 0; }

        static constexpr auto is_last(self_t const& self, int_t cur) -> bool
        {
            return cur >= self.count_;
        }

        static constexpr auto inc(self_t const&, int_t& cur) -> void { ++cur; }

        static constexpr auto read_at(self_t const& self, int_t) -> T const& { return self.obj_; }

        static constexpr auto dec(self_t const&, int_t& cur) -> void { --cur; }

        static constexpr auto inc(self_t const&, int_t& cur, int_t offset) -> void
        {
            cur = num::add(cur, offset);
        }

        static constexpr auto distance(self_t const&, int_t from, int_t to) -> int_t
        {
            return to - from;
        }

        static constexpr auto for_each_while(self_t const& self, auto&& pred) -> int_t
        {
            int_t idx = 0;
            for (; idx < self.count_; ++idx) {
                if (!std::invoke(pred, std::as_const(self.obj_))) {
                    break;
                }
            }
            return idx;
        }

        static constexpr auto last(self_t const& self) -> int_t { return self.count_; }

        static constexpr auto size(self_t const& self) -> int_t { return self.count_; }
    };
};

struct repeat_fn {
    template <typename T>
        requires std::movable<std::decay_t<T>>
    constexpr auto operator()(T&& obj) const
    {
        return repeat_iterable<std::decay_t<T>>(FLUX_FWD(obj));
    }

    template <typename T>
        requires std::movable<std::decay_t<T>>
    constexpr auto operator()(T&& obj, num::integral auto count) const
    {
        auto c = num::checked_cast<int_t>(count);
        if (c < 0) {
            runtime_error("Negative count passed to repeat()");
        }
        return repeat_sequence<std::decay_t<T>>(FLUX_FWD(obj), c);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto repeat = detail::repeat_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_REPEAT_HPP_INCLUDED
