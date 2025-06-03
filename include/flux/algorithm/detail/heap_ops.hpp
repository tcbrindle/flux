// flux/op/detail/heap_sift.hpp
//
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Eric Niebler 2014
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef FLUX_ALGORITHM_DETAIL_HEAP_OPS_HPP_INCLUDED
#define FLUX_ALGORITHM_DETAIL_HEAP_OPS_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux::detail {

template <typename Seq, typename Comp>
constexpr void sift_up_n(Seq& seq, int_t n, Comp& comp)
{
    cursor_t<Seq> first = flux::first(seq);

    if (n > 1) {
        cursor_t<Seq> last = flux::next(seq, first, n);
        n = (n - 2) / 2;
        cursor_t<Seq> i = first + n;
        if (std::invoke(comp, read_at(seq, i), read_at(seq, dec(seq, last)))) {
            value_t<Seq> v = move_at(seq, last);
            do {
                read_at(seq, last) = move_at(seq, i);
                last = i;
                if (n == 0) {
                    break;
                }
                n = (n - 1) / 2;
                i = next(seq, first, n);
            } while (std::invoke(comp, read_at(seq, i), v));
            read_at(seq, last) = std::move(v);
        }
    }
}

template <typename Seq, typename Comp>
constexpr void sift_down_n(Seq& seq, int_t n, cursor_t<Seq> start, Comp& comp)
{
    cursor_t<Seq> first = flux::first(seq);

    // left-child of start is at 2 * start + 1
    // right-child of start is at 2 * start + 2
    //auto child = start - first;
    auto child = flux::distance(seq, first, start);

    if (n < 2 || (n - 2) / 2 < child) {
        return;
    }

    child = 2 * child + 1;
    cursor_t<Seq> child_i = flux::next(seq, first, child);

    if ((child + 1) < n && std::invoke(comp, read_at(seq, child_i),
                                       read_at(seq, next(seq, child_i)))) {
        // right-child exists and is greater than left-child
        flux::inc(seq, child_i);
        ++child;
    }

    // check if we are in heap-order
    if (std::invoke(comp, read_at(seq, child_i),
                     read_at(seq, start))) {
        // we are, start is larger than its largest child
        return;
    }

    value_t<Seq> top = move_at(seq, start);
    do {
        // we are not in heap-order, swap the parent with it's largest child
        read_at(seq, start) = move_at(seq, child_i);
        //*start = nano::iter_move(child_i);
        start = child_i;

        if ((n - 2) / 2 < child) {
            break;
        }

        // recompute the child based off of the updated parent
        child = 2 * child + 1;
        child_i = next(seq, first, child); //child_i = first + child;

        if ((child + 1) < n &&
            std::invoke(comp, read_at(seq, child_i),
                         read_at(seq, next(seq, child_i)))) {
            // right-child exists and is greater than left-child
            inc(seq, child_i);
            ++child;
        }

        // check if we are in heap-order
    } while (!std::invoke(comp, read_at(seq, child_i), top));
    read_at(seq, start) = std::move(top);
}

template <sequence Seq, typename Comp>
constexpr void make_heap(Seq& seq, Comp& comp)
{
    int_t n = flux::size(seq);
    auto first = flux::first(seq);

    if (n > 1) {
        for (auto start = (n - 2) / 2; start >= 0; --start) {
            detail::sift_down_n(seq, n, flux::next(seq, first, start), comp);
        }
    }
}

template <sequence Seq, typename Comp>
constexpr void pop_heap(Seq& seq, int_t n, Comp& comp)
{
    auto first = flux::first(seq);
    if (n > 1) {
        swap_at(seq, first, next(seq, first, n - 1));
        detail::sift_down_n(seq, n - 1, first, comp);
    }
}

template <sequence Seq, typename Comp>
constexpr void sort_heap(Seq& seq, Comp& comp)
{
    auto n = flux::size(seq);

    if (n < 2) {
        return;
    }

    for (auto i = n; i > 1; --i) {
        pop_heap(seq, i, comp);
    }
}

}

#endif // FLUX_ALGORITHM_DETAIL_HEAP_OPS_INCLUDED_HPP
