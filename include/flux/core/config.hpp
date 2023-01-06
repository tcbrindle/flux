
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONFIG_HPP_INCLUDED
#define FLUX_CORE_CONFIG_HPP_INCLUDED

#include <concepts>
#include <cstddef>
#include <type_traits>

// Default int_t is ptrdiff_t
#define FLUX_DEFAULT_INT_TYPE std::ptrdiff_t

// Select which int type to use
#ifndef FLUX_INT_TYPE
#define FLUX_INT_TYPE FLUX_DEFAULT_INT_TYPE
#endif

namespace flux {

namespace config {

using int_type = FLUX_INT_TYPE;
static_assert(std::signed_integral<int_type> && (sizeof(int_type) >= sizeof(int)),
              "Custom FLUX_INT_TYPE must be a signed integer type at least as large as int");

} // namespace config

} // namespace flux

#endif
