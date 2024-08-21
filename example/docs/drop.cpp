
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include "assert.hpp"
#include <vector>

int main()
{
    std::vector vec{1, 2, 3, 4, 5};

    auto dropped = flux::drop(std::move(vec), 3);

    assert(flux::size(dropped) == 2);
    assert(flux::equal(dropped, std::vector{4, 5}));
}