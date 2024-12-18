
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_GENERATOR_HPP_INCLUDED
#define FLUX_SEQUENCE_GENERATOR_HPP_INCLUDED

#include <flux/core.hpp>

#include <coroutine>
#include <utility>

namespace flux {

FLUX_EXPORT
template <typename ElemT>
struct generator : inline_iter_base<generator<ElemT>> {

    using yielded_type = std::conditional_t<std::is_reference_v<ElemT>,
                                            ElemT,
                                            ElemT const&>;

    struct promise_type;

    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        auto initial_suspend() { return std::suspend_always{}; }

        auto final_suspend() noexcept { return std::suspend_always{}; }

        auto get_return_object()
        {
            return generator(handle_type::from_promise(*this));
        }

        auto yield_value(yielded_type elem)
        {
            ptr_ = std::addressof(elem);
            return std::suspend_always{};
        }

        auto unhandled_exception() { throw; }

        void return_void() noexcept {}

        std::add_pointer_t<yielded_type> ptr_;
    };

private:
    handle_type coro_;

    explicit generator(handle_type&& handle) : coro_(std::move(handle)) {}

    friend struct iter_traits<generator>;

public:
    generator(generator&& other) noexcept
        : coro_(std::exchange(other.coro_, {}))
    {}

    generator& operator=(generator&& other) noexcept
    {
        std::swap(coro_, other.coro_);
        return *this;
    }

    ~generator()
    {
        if (coro_) { coro_.destroy(); }
    }
};

template <typename T>
struct iter_traits<generator<T>> : default_iter_traits
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        cursor_type() = default;
        friend struct iter_traits;
    };

    using self_t = generator<T>;

public:
    static auto first(self_t& self) {
        self.coro_.resume();
        return cursor_type{};
    }

    static auto is_last(self_t& self, cursor_type const&) -> bool
    {
        return self.coro_.done();
    }

    static auto inc(self_t& self, cursor_type& cur) -> cursor_type&
    {
        self.coro_.resume();
        return cur;
    }

    static auto read_at(self_t& self, cursor_type const&) -> decltype(auto)
    {
        return static_cast<typename self_t::yielded_type>(*self.coro_.promise().ptr_);
    }
};

} // namespace flux

#endif // FLUX_SEQUENCE_GENERATOR_HPP_INCLUDED