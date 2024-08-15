
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_MACROS_HPP_INCLUDED
#define FLUX_CORE_MACROS_HPP_INCLUDED

#include <version>

#define FLUX_VERSION_MAJOR 0
#define FLUX_VERSION_MINOR 4
#define FLUX_VERSION_PATCH 0
#define FLUX_VERSION_DEVEL 1 // 0 => Release, 1 => development post Major.Minor.Patch

#define FLUX_VERSION \
    (FLUX_VERSION_MAJOR * 100'000 \
    + FLUX_VERSION_MINOR * 1'000  \
    + FLUX_VERSION_PATCH * 10   \
    + FLUX_VERSION_DEVEL)

#define FLUX_FWD(x) static_cast<decltype(x)&&>(x)

#define FLUX_DECLVAL(...)  ((static_cast<__VA_ARGS__(*)()noexcept>(nullptr))())

#ifdef __GNUC__
#define FLUX_ALWAYS_INLINE [[gnu::always_inline]]
#else
#define FLUX_ALWAYS_INLINE
#endif

#define FLUX_NO_UNIQUE_ADDRESS [[no_unique_address]]

#define FLUX_FOR(_flux_var_decl_, ...)                    \
    if (auto&& _flux_seq_ = __VA_ARGS__; true)           \
        for (auto _flux_cur_ = ::flux::first(_flux_seq_);   \
             !::flux::is_last(_flux_seq_, _flux_cur_);     \
              ::flux::inc(_flux_seq_, _flux_cur_))         \
            if (_flux_var_decl_ = ::flux::read_at(_flux_seq_, _flux_cur_); true)

#define FLUX_ASSERT(cond) (::flux::assert_(cond, "assertion '" #cond "' failed"))

#define FLUX_DEBUG_ASSERT(cond) (::flux::assert_(!::flux::config::enable_debug_asserts || (cond), "assertion '" #cond "' failed"));

#ifdef FLUX_MODULE_INTERFACE
#define FLUX_EXPORT export
#else
#define FLUX_EXPORT
#endif

#endif // FLUX_CORE_MACROS_HPP_INCLUDED
