
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_ASSERT_HPP_INCLUDED
#define FLUX_CORE_ASSERT_HPP_INCLUDED

#include <flux/core/config.hpp>

#include <cstdio>
#include <exception>
#include <source_location>
#include <stdexcept>
#include <type_traits>

namespace flux {

struct unrecoverable_error : std::logic_error {
    explicit unrecoverable_error(char const* msg) : std::logic_error(msg) {}
};

namespace detail {

struct assertion_failure_fn {
    [[noreturn]]
    inline void operator()(char const* msg, std::source_location loc) const
    {
        if constexpr (config::on_error == error_policy::unwind) {
            char buf[1024];
            std::snprintf(buf, 1024, "%s:%u: Fatal error: %s",
                          loc.file_name(), loc.line(), msg);
            throw unrecoverable_error(buf);
        } else {
            if constexpr (config::print_error_on_terminate) {
                std::fprintf(stderr, "%s:%u: Fatal error: %s\n",
                             loc.file_name(), loc.line(), msg);
            }
            std::terminate();
        }
    }
};

inline constexpr auto assertion_failure = assertion_failure_fn{};

struct assert_fn {
    constexpr void operator()(bool cond, char const* msg,
                              std::source_location loc = std::source_location::current()) const
    {
        if (cond) [[likely]] {
            return;
        } else [[unlikely]] {
            assertion_failure(msg, std::move(loc));
        }
    }
};

struct debug_assert_fn {
    constexpr void operator()(bool cond, char const* msg,
                              std::source_location loc = std::source_location::current()) const
    {
        if constexpr (config::enable_debug_asserts) {
            assert_fn{}(cond, msg, std::move(loc));
        }
    }
};

struct bounds_check_fn {
    constexpr void operator()(bool cond, std::source_location loc = std::source_location::current()) const
    {
        if (!std::is_constant_evaluated()) {
            assert_fn{}(cond, "out of bounds sequence access", std::move(loc));
        }
    }
};

} // namespace detail

inline constexpr auto assert_ = detail::assert_fn{};
inline constexpr auto debug_assert = detail::debug_assert_fn{};
inline constexpr auto bounds_check = detail::bounds_check_fn{};

} // namespace flux

#endif // FLUX_CORE_ASSERT_HPP_INCLUDED
