
#pragma once

#include <flux/core.hpp>

#include <coroutine>
#include <iostream>

namespace flux {

namespace experimental {

template <typename ElemT>
struct generator : lens_base<generator<ElemT>> {

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

    friend struct sequence_iface<generator>;

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

} // namespace experimental

template <typename T>
struct sequence_iface<experimental::generator<T>>
{
private:
    struct index_type {
        index_type(index_type&&) = default;
        index_type& operator=(index_type&&) = default;
    private:
        index_type() = default;
        friend struct sequence_iface;
    };

    using self_t = experimental::generator<T>;

public:
    static auto first(self_t& self) {
        self.coro_.resume();
        return index_type{};
    }

    static auto is_last(self_t& self, index_type const&) -> bool
    {
        return self.coro_.done();
    }

    static auto inc(self_t& self, index_type& idx) -> index_type&
    {
        self.coro_.resume();
        return idx;
    }

    static auto read_at(self_t& self, index_type const&) -> decltype(auto)
    {
        return static_cast<typename self_t::yielded_type>(*self.coro_.promise().ptr_);
    }
};

} // namespace flux
