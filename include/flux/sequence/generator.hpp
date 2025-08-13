
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
struct generator : inline_sequence_base<generator<ElemT>> {

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

    struct iteration_context_type : immovable {
        handle_type* coro_;

        explicit iteration_context_type(handle_type& coro) : coro_(std::addressof(coro)) { }

        using element_type = yielded_type;

        auto run_while(auto&& pred) -> iteration_result
        {
            coro_->resume();
            while (!coro_->done()) {
                if (!std::invoke(pred, static_cast<element_type>(*coro_->promise().ptr_))) {
                    return iteration_result::incomplete;
                }
                coro_->resume();
            }
            return iteration_result::complete;
        }
    };

private:
    handle_type coro_;

    explicit generator(handle_type&& handle) : coro_(std::move(handle)) {}

    friend struct sequence_traits<generator>;

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

    auto iterate() -> iteration_context_type { return iteration_context_type(coro_); }
};

} // namespace flux

#endif // FLUX_SEQUENCE_GENERATOR_HPP_INCLUDED