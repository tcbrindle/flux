// flux/op/detail/pqdsort.hpp
//
// Copyright Orson Peters 2017.
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Modified from Boost.Sort by Orson Peters
// https://github.com/boostorg/sort/blob/develop/include/boost/sort/pdqsort/pdqsort.hpp

#ifndef FLUX_ALGORITHM_DETAIL_PDQSORT_HPP_INCLUDED
#define FLUX_ALGORITHM_DETAIL_PDQSORT_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/algorithm/detail/heap_ops.hpp>

namespace flux::detail {

// Partitions below this size are sorted using insertion sort.
inline constexpr int pdqsort_insertion_sort_threshold = 24;

// Partitions above this size use Tukey's ninther to select the pivot.
inline constexpr int pdqsort_ninther_threshold = 128;

// When we detect an already sorted partition, attempt an insertion sort that
// allows this amount of element moves before giving up.
inline constexpr int pqdsort_partial_insertion_sort_limit = 8;

// Must be multiple of 8 due to loop unrolling, and < 256 to fit in unsigned
// char.
inline constexpr int pdqsort_block_size = 64;

// Cacheline size, assumes power of two.
inline constexpr int pdqsort_cacheline_size = 64;

template <typename T>
inline constexpr bool is_default_compare_v = false;

template <>
inline constexpr bool is_default_compare_v<std::compare_three_way> = true;
template <>
inline constexpr bool is_default_compare_v<decltype(flux::cmp::reverse_compare)> = true;
template <>
inline constexpr bool is_default_compare_v<decltype(flux::cmp::compare_floating_point_unchecked)> = true;

// Returns floor(log2(n)), assumes n > 0.
template <class T>
constexpr int log2(T n)
{
    int log = 0;
    while (n >>= 1)
        ++log;
    return log;
}

// Sorts [begin, end) using insertion sort with the given comparison function.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr void insertion_sort(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    if (begin == end) {
        return;
    }

    for (auto cur = next(seq, begin); cur != end; inc(seq, cur)) {
        cursor_t<Seq> sift = cur;
        cursor_t<Seq> sift_1 = prev(seq, cur);

        // Compare first so we can avoid 2 moves for an element already
        // positioned correctly.
        if (comp(read_at(seq, sift), read_at(seq, sift_1))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (sift != begin && comp(tmp, read_at(seq, dec(seq, sift_1))));

            read_at(seq, sift) = std::move(tmp);
        }
    }
}

// Sorts [begin, end) using insertion sort with the given comparison function.
// Assumes
// *(begin - 1) is an element smaller than or equal to any element in [begin,
// end).
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr void unguarded_insertion_sort(Seq& seq, Cur const begin, Cur const end,
                                        Comp& comp)
{
    using T = value_t<Seq>;

    if (begin == end) {
        return;
    }

    for (auto cur = next(seq, begin); cur != end; inc(seq, cur)) {
        cursor_t<Seq> sift = cur;
        cursor_t<Seq> sift_1 = prev(seq, cur);

        // Compare first so we can avoid 2 moves for an element already
        // positioned correctly.
        if (comp(read_at(seq, sift), read_at(seq, sift_1))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (comp(tmp, read_at(seq, dec(seq, sift_1))));

            read_at(seq, sift) = std::move(tmp);
        }
    }
}

// Attempts to use insertion sort on [begin, end). Will return false if more
// than partial_insertion_sort_limit elements were moved, and abort sorting.
// Otherwise it will successfully sort and return true.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr bool partial_insertion_sort(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    if (begin == end) {
        return true;
    }

    distance_t limit = 0;

    for (auto cur = next(seq, begin); cur != end; inc(seq, cur)) {
        if (limit > pqdsort_partial_insertion_sort_limit) {
            return false;
        }

        cursor_t<Seq> sift = cur;
        cursor_t<Seq> sift_1 = prev(seq, cur);

        // Compare first so we can avoid 2 moves for an element already
        // positioned correctly.
        if (comp(read_at(seq, sift), read_at(seq, sift_1))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (sift != begin && comp(tmp, read_at(seq, dec(seq, sift_1))));

            read_at(seq, sift) = std::move(tmp);
            limit += distance(seq, sift, cur);
        }
    }

    return true;
}

template <sequence Seq, typename Comp>
constexpr void sort2(Seq& seq, cursor_t<Seq> a, cursor_t<Seq> b, Comp& comp)
{
    if (comp(read_at(seq, b), read_at(seq, a))) {
        swap_at(seq, a, b);
    }
}

// Sorts the elements *a, *b and *c using comparison function comp.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr void sort3(Seq& seq, Cur a, Cur b, Cur c, Comp& comp)
{
    sort2(seq, a, b, comp);
    sort2(seq, b, c, comp);
    sort2(seq, a, b, comp);
}


template <typename Seq, typename Cur = cursor_t<Seq>>
constexpr void swap_offsets(Seq& seq, Cur const first, Cur const last,
                            unsigned char* offsets_l,
                            unsigned char* offsets_r, int num, bool use_swaps)
{
    using T = value_t<Seq>;
    if (use_swaps) {
        // This case is needed for the descending distribution, where we need
        // to have proper swapping for pdqsort to remain O(n).
        for (int i = 0; i < num; ++i) {
            swap_at(seq, next(seq, first, offsets_l[i]), next(seq, last, -offsets_r[i]));
        }
    } else if (num > 0) {
        Cur l = next(seq, first, offsets_l[0]);
        Cur r = next(seq, last, -offsets_r[0]);
        T tmp(move_at(seq, l));
        read_at(seq, l) = move_at(seq, r);

        for (int i = 1; i < num; ++i) {
            l = next(seq, first, offsets_l[i]);
            read_at(seq, r) = move_at(seq, l);
            r = next(seq, last, -offsets_r[i]);
            read_at(seq, l) = move_at(seq, r);
        }
        read_at(seq, r) = std::move(tmp);
    }
}

// Partitions [begin, end) around pivot *begin using comparison function comp.
// Elements equal to the pivot are put in the right-hand partition. Returns the
// position of the pivot after partitioning and whether the passed sequence
// already was correctly partitioned. Assumes the pivot is a median of at least
// 3 elements and that [begin, end) is at least insertion_sort_threshold long.
// Uses branchless partitioning.
template <typename Seq, typename Cur = cursor_t<Seq>, typename Comp>
constexpr std::pair<Cur, bool>
partition_right_branchless(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    // Move pivot into local for speed.
    T pivot(move_at(seq, begin));
    Cur first = begin;
    Cur last = end;

    // Find the first element greater than or equal than the pivot (the median
    // of 3 guarantees this exists).
    while (comp(read_at(seq, inc(seq, first)), pivot))
        ;

    // Find the first element strictly smaller than the pivot. We have to guard
    // this search if there was no element before *first.
    if (prev(seq, first) == begin) {
        while (first < last && !comp(read_at(seq, dec(seq, last)), pivot))
            ;
    } else {
        while (!comp(read_at(seq, dec(seq, last)), pivot))
            ;
    }

    // If the first pair of elements that should be swapped to partition are the
    // same element, the passed in sequence already was correctly partitioned.
    bool already_partitioned = first >= last;
    if (!already_partitioned) {
        swap_at(seq, first, last);
        inc(seq, first);
    }

    // The following branchless partitioning is derived from "BlockQuicksort:
    // How Branch Mispredictions don't affect Quicksort" by Stefan Edelkamp and
    // Armin Weiss.
    alignas(pdqsort_cacheline_size) unsigned char
        offsets_l_storage[pdqsort_block_size] = {};
    alignas(pdqsort_cacheline_size) unsigned char
        offsets_r_storage[pdqsort_block_size] = {};
    unsigned char* offsets_l = offsets_l_storage;
    unsigned char* offsets_r = offsets_r_storage;
    int num_l = 0, num_r = 0, start_l = 0, start_r = 0;

    while (distance(seq, first, last) > 2 * pdqsort_block_size) {
        // Fill up offset blocks with elements that are on the wrong side.
        if (num_l == 0) {
            start_l = 0;
            Cur cur = first;
            for (unsigned char i = 0; i < pdqsort_block_size;) {
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
                offsets_l[num_l] = i++;
                num_l += !comp(read_at(seq, cur), pivot);
                inc(seq, cur);
            }
        }
        if (num_r == 0) {
            start_r = 0;
            Cur cur = last;
            for (unsigned char i = 0; i < pdqsort_block_size;) {
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
                offsets_r[num_r] = ++i;
                num_r += comp(read_at(seq, dec(seq, cur)), pivot);
            }
        }

        // Swap elements and update block sizes and first/last boundaries.
        int num = (cmp::min)(num_l, num_r);
        swap_offsets(seq, first, last, offsets_l + start_l, offsets_r + start_r, num,
                     num_l == num_r);
        num_l -= num;
        num_r -= num;
        start_l += num;
        start_r += num;
        if (num_l == 0)
            inc(seq, first, pdqsort_block_size);
        if (num_r == 0)
            inc(seq, last, -pdqsort_block_size);
    }

    distance_t l_size = 0, r_size = 0;
    distance_t unknown_left =
        distance(seq, first, last) - ((num_r || num_l) ? pdqsort_block_size : 0);
    if (num_r) {
        // Handle leftover block by assigning the unknown elements to the other
        // block.
        l_size = unknown_left;
        r_size = pdqsort_block_size;
    } else if (num_l) {
        l_size = pdqsort_block_size;
        r_size = unknown_left;
    } else {
        // No leftover block, split the unknown elements in two blocks.
        l_size = unknown_left / 2;
        r_size = unknown_left - l_size;
    }

    // Fill offset buffers if needed.
    if (unknown_left && !num_l) {
        start_l = 0;
        Cur cur = first;
        for (unsigned char i = 0; static_cast<distance_t>(i) < l_size;) {
            offsets_l[num_l] = i++;
            num_l += !comp(read_at(seq, cur), pivot);
            inc(seq, cur);
        }
    }
    if (unknown_left && !num_r) {
        start_r = 0;
        Cur cur = last;
        for (unsigned char i = 0; static_cast<distance_t>(i) < r_size;) {
            offsets_r[num_r] = ++i;
            num_r += comp(read_at(seq, dec(seq, cur)), pivot);
        }
    }

    int num = (cmp::min)(num_l, num_r);
    swap_offsets(seq, first, last, offsets_l + start_l, offsets_r + start_r, num,
                 num_l == num_r);
    num_l -= num;
    num_r -= num;
    start_l += num;
    start_r += num;
    if (num_l == 0)
        inc(seq, first, l_size);
    if (num_r == 0)
        inc(seq, last, -r_size);

    // We have now fully identified [first, last)'s proper position. Swap the
    // last elements.
    if (num_l) {
        offsets_l += start_l;
        while (num_l--) {
            swap_at(seq, next(seq, first, offsets_l[num_l]), dec(seq, last));
        }
        first = last;
    }
    if (num_r) {
        offsets_r += start_r;
        while (num_r--) {
            swap_at(seq, next(seq, last, -offsets_r[num_r]), first);
            inc(seq, first);
        }
        last = first;
    }

    // Put the pivot in the right place.
    Cur pivot_pos = prev(seq, first);
    read_at(seq, begin) = move_at(seq, pivot_pos);
    read_at(seq, pivot_pos) = std::move(pivot);

    return std::make_pair(std::move(pivot_pos), already_partitioned);
}

// Partitions [begin, end) around pivot *begin using comparison function comp.
// Elements equal to the pivot are put in the right-hand partition. Returns the
// position of the pivot after partitioning and whether the passed sequence
// already was correctly partitioned. Assumes the pivot is a median of at least
// 3 elements and that [begin, end) is at least insertion_sort_threshold long.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr std::pair<cursor_t<Seq>, bool>
partition_right(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    // Move pivot into local for speed.
    T pivot(move_at(seq, begin));

    cursor_t<Seq> first = begin;
    cursor_t<Seq> last = end;

    // Find the first element greater than or equal than the pivot (the median
    // of 3 guarantees this exists).
    while (comp(read_at(seq, inc(seq, first)), pivot)) {
    }

    // Find the first element strictly smaller than the pivot. We have to guard
    // this search if there was no element before *first.
    if (prev(seq, first) == begin) {
        while (first < last && !comp(read_at(seq, dec(seq, last)), pivot)) {
        }
    } else {
        while (!comp(read_at(seq, dec(seq, last)), pivot)) {
        }
    }

    // If the first pair of elements that should be swapped to partition are the
    // same element, the passed in sequence already was correctly partitioned.
    bool already_partitioned = first >= last;

    // Keep swapping pairs of elements that are on the wrong side of the pivot.
    // Previously swapped pairs guard the searches, which is why the first
    // iteration is special-cased above.
    while (first < last) {
        swap_at(seq, first, last);
        while (comp(read_at(seq, inc(seq, first)), pivot))
            ;
        while (!comp(read_at(seq, dec(seq, last)), pivot))
            ;
    }

    // Put the pivot in the right place.
    cursor_t<Seq> pivot_pos = prev(seq, first);
    read_at(seq, begin) = move_at(seq, pivot_pos);
    read_at(seq, pivot_pos) = std::move(pivot);

    return std::make_pair(std::move(pivot_pos), already_partitioned);
}

// Similar function to the one above, except elements equal to the pivot are put
// to the left of the pivot and it doesn't check or return if the passed
// sequence already was partitioned. Since this is rarely used (the many equal
// case), and in that case pdqsort already has O(n) performance, no block
// quicksort is applied here for simplicity.
template <sequence Seq, typename Comp, typename Cur = cursor_t<Seq>>
constexpr cursor_t<Seq> partition_left(Seq& seq, Cur const begin, Cur const end, Comp& comp)
{
    using T = value_t<Seq>;

    T pivot(move_at(seq, begin));
    cursor_t<Seq> first = begin;
    cursor_t<Seq> last = end;

    while (comp(pivot, read_at(seq, dec(seq, last))))
        ;

    if (next(seq, last) == end) {
        while (first < last && !comp(pivot, read_at(seq, inc(seq, first))))
            ;
    } else {
        while (!comp(pivot, read_at(seq, inc(seq, first))))
            ;
    }

    while (first < last) {
        swap_at(seq, first, last);
        while (comp(pivot, read_at(seq, dec(seq, last))))
            ;
        while (!comp(pivot, read_at(seq, inc(seq, first))))
            ;
    }

    cursor_t<Seq> pivot_pos = last;
    read_at(seq, begin) = move_at(seq, pivot_pos);
    read_at(seq, pivot_pos) = std::move(pivot);

    return pivot_pos;
}

template <bool Branchless, typename Seq, typename Comp,
          typename Cur = cursor_t<Seq>>
constexpr void pdqsort_loop(Seq& seq, Cur begin, Cur end, Comp& comp,
                            int bad_allowed, bool leftmost = true)
{
    using diff_t = distance_t;

    // Use a while loop for tail recursion elimination.
    while (true) {
        diff_t size = flux::distance(seq, begin, end);

        // Insertion sort is faster for small arrays.
        if (size < pdqsort_insertion_sort_threshold) {
            if (leftmost) {
                insertion_sort(seq, begin, end, comp);
            } else {
                unguarded_insertion_sort(seq, begin, end, comp);
            }
            return;
        }

        // Choose pivot as median of 3 or pseudomedian of 9.
        diff_t s2 = size / 2;
        if (size > pdqsort_ninther_threshold) {
            sort3(seq, begin, next(seq, begin, s2), prev(seq, end), comp);
            sort3(seq, next(seq, begin), next(seq, begin, s2 - 1), next(seq, end, -2), comp);
            sort3(seq, next(seq, begin, 2), next(seq, begin, s2 + 1), next(seq, end, -3), comp);
            sort3(seq, next(seq, begin, s2 - 1), next(seq, begin, s2), next(seq, begin, s2 + 1), comp);
            swap_at(seq, begin, next(seq, begin, s2));
        } else {
            sort3(seq, next(seq, begin, s2), begin, prev(seq, end), comp);
        }

        // If *(begin - 1) is the end of the right partition of a previous
        // partition operation there is no element in [begin, end) that is
        // smaller than *(begin - 1). Then if our pivot compares equal to
        // *(begin - 1) we change strategy, putting equal elements in the left
        // partition, greater elements in the right partition. We do not have to
        // recurse on the left partition, since it's sorted (all equal).
        if (!leftmost && !comp(read_at(seq, prev(seq, begin)), read_at(seq, begin))) {
            begin = next(seq, partition_left(seq, begin, end, comp));
            continue;
        }

        // Partition and get results.
        auto [pivot_pos, already_partitioned] = [&] {
            if constexpr (Branchless) {
                return partition_right_branchless(seq, begin, end, comp);
            } else {
                return partition_right(seq, begin, end, comp);
            }
        }();

        // Check for a highly unbalanced partition.
        diff_t l_size = distance(seq, begin, pivot_pos);
        diff_t r_size = distance(seq, next(seq, pivot_pos), end);
        bool highly_unbalanced = l_size < size / 8 || r_size < size / 8;

        // If we got a highly unbalanced partition we shuffle elements to break
        // many patterns.
        if (highly_unbalanced) {
            // If we had too many bad partitions, switch to heapsort to
            // guarantee O(n log n).
            if (--bad_allowed == 0) {
                auto subseq = flux::slice(seq, begin, end);
                detail::make_heap(subseq, comp);
                detail::sort_heap(subseq, comp);
                return;
            }

            if (l_size >= pdqsort_insertion_sort_threshold) {
                swap_at(seq, begin, next(seq, begin, l_size/4));
                swap_at(seq, prev(seq, pivot_pos), next(seq, pivot_pos, -l_size/4));

                if (l_size > pdqsort_ninther_threshold) {
                    swap_at(seq, next(seq, begin), next(seq, begin, l_size/4 + 1));
                    swap_at(seq, next(seq, begin, 2), next(seq, begin, l_size/4 + 2));
                    swap_at(seq, next(seq, pivot_pos, -2), next(seq, pivot_pos, -(l_size/4 + 1)));
                    swap_at(seq, next(seq, pivot_pos, -3), next(seq, pivot_pos, -(l_size/4 + 2)));
                }
            }

            if (r_size >= pdqsort_insertion_sort_threshold) {
                swap_at(seq, next(seq, pivot_pos), next(seq, pivot_pos, (1 + r_size/4)));
                swap_at(seq, prev(seq, end), next(seq, end, -r_size/4));

                if (r_size > pdqsort_ninther_threshold) {
                    swap_at(seq, next(seq, pivot_pos, 2), next(seq, pivot_pos, 2 + r_size/4));
                    swap_at(seq, next(seq, pivot_pos, 3), next(seq, pivot_pos, 3 + r_size/4));
                    swap_at(seq, next(seq, end, -2), next(seq, end, -(1 + r_size/4)));
                    swap_at(seq, next(seq, end, -3), next(seq, end, -(2 + r_size/4)));
                }
            }
        } else {
            // If we were decently balanced and we tried to sort an already
            // partitioned sequence try to use insertion sort.
            if (already_partitioned &&
                partial_insertion_sort(seq, begin, pivot_pos, comp) &&
                partial_insertion_sort(seq, flux::next(seq, pivot_pos), end, comp))
                return;
        }

        // Sort the left partition first using recursion and do tail recursion
        // elimination for the right-hand partition.
        detail::pdqsort_loop<Branchless>(seq, begin, pivot_pos, comp,
                                         bad_allowed, leftmost);
        begin = next(seq, pivot_pos);
        leftmost = false;
    }
}

template <typename Seq, typename Comp>
constexpr void pdqsort(Seq& seq, Comp& comp)
{
    if (is_empty(seq)) {
        return;
    }

    constexpr bool Branchless =
         is_default_compare_v<std::remove_const_t<Comp>> &&
         std::is_arithmetic_v<value_t<Seq>>;

    auto comp_wrapper = [&comp](auto&& lhs, auto&& rhs) -> bool {
        return std::is_lt(std::invoke(comp, FLUX_FWD(lhs), FLUX_FWD(rhs)));
    };

    detail::pdqsort_loop<Branchless>(seq,
                                     first(seq),
                                     last(seq),
                                     comp_wrapper,
                                     detail::log2(size(seq)));
}

} // namespace flux::detail

#endif // FLUX_ALGORITHM_DETAIL_PDQSORT_HPP_INCLUDED
