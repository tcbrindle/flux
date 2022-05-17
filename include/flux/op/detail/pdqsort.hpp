// flux/op/detail/pqdsort.hpp
//
// Copyright Orson Peters 2017.
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Modified from Boost.Sort by Orson Peters
// https://github.com/boostorg/sort/blob/develop/include/boost/sort/pdqsort/pdqsort.hpp

#ifndef FLUX_OP_DETAIL_PDQSORT_HPP_INCLUDED
#define FLUX_OP_DETAIL_PDQSORT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/slice.hpp>
#include <flux/ranges/view.hpp>

namespace flux {

namespace detail {

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

template <typename T>
inline constexpr bool is_default_compare_v<std::less<T>> = true;
template <typename T>
inline constexpr bool is_default_compare_v<std::greater<T>> = true;
template <>
inline constexpr bool is_default_compare_v<std::ranges::less> = true;
template <>
inline constexpr bool is_default_compare_v<std::ranges::greater> = true;


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
template <sequence Seq, typename Comp, typename Proj, typename Cur = cursor_t<Seq>>
constexpr void insertion_sort(Seq& seq, Cur const begin, Cur const end, Comp& comp, Proj& proj)
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
        if (std::invoke(comp, std::invoke(proj, read_at(seq, sift)),
                         std::invoke(proj, read_at(seq, sift_1)))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (sift != begin &&
                     std::invoke(comp, std::invoke(proj, tmp),
                                  std::invoke(proj, read_at(seq, dec(seq, sift_1)))));

              read_at(seq, sift) = std::move(tmp);
        }
    }
}

// Sorts [begin, end) using insertion sort with the given comparison function.
// Assumes
// *(begin - 1) is an element smaller than or equal to any element in [begin,
// end).
template <sequence Seq, typename Comp, typename Proj, typename Cur = cursor_t<Seq>>
constexpr void unguarded_insertion_sort(Seq& seq, Cur const begin, Cur const end,
                                        Comp& comp, Proj& proj)
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
        if (std::invoke(comp, std::invoke(proj, read_at(seq, sift)),
                         std::invoke(proj, read_at(seq, sift_1)))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (std::invoke(comp, std::invoke(proj, tmp),
                                  std::invoke(proj, read_at(seq, dec(seq, sift_1)))));

            read_at(seq, sift) = std::move(tmp);
        }
    }
}

// Attempts to use insertion sort on [begin, end). Will return false if more
// than partial_insertion_sort_limit elements were moved, and abort sorting.
// Otherwise it will successfully sort and return true.
template <sequence Seq, typename Comp, typename Proj, typename Cur = cursor_t<Seq>>
constexpr bool partial_insertion_sort(Seq& seq, Cur const begin, Cur const end, Comp& comp, Proj& proj)
{
    using T = value_t<Seq>;

    if (begin == end) {
        return true;
    }

    distance_t<Seq> limit = 0;

    for (auto cur = next(seq, begin); cur != end; inc(seq, cur)) {
        if (limit > pqdsort_partial_insertion_sort_limit) {
            return false;
        }

        cursor_t<Seq> sift = cur;
        cursor_t<Seq> sift_1 = prev(seq, cur);

        // Compare first so we can avoid 2 moves for an element already
        // positioned correctly.
        if (std::invoke(comp, std::invoke(proj, read_at(seq, sift)),
                         std::invoke(proj, read_at(seq, sift_1)))) {
            T tmp = move_at(seq, sift);

            do {
                read_at(seq, sift) = move_at(seq, sift_1);
                dec(seq, sift);
            } while (sift != begin &&
                     std::invoke(comp, std::invoke(proj, tmp),
                                  std::invoke(proj, read_at(seq, dec(seq, sift_1)))));

            read_at(seq, sift) = std::move(tmp);
            limit += distance(seq, sift, cur);
        }
    }

    return true;
}

template <sequence Seq, typename Comp, typename Proj>
constexpr void sort2(Seq& seq, cursor_t<Seq> a, cursor_t<Seq> b, Comp& comp, Proj& proj)
{
    if (std::invoke(comp, std::invoke(proj, read_at(seq, b)), std::invoke(proj, read_at(seq, a)))) {
        swap_at(seq, a, b);
    }
}

// Sorts the elements *a, *b and *c using comparison function comp.
template <sequence Seq, typename Comp, typename Proj, typename Cur = cursor_t<Seq>>
constexpr void sort3(Seq& seq, Cur a, Cur b, Cur c, Comp& comp, Proj& proj)
{
    sort2(seq, a, b, comp, proj);
    sort2(seq, b, c, comp, proj);
    sort2(seq, a, b, comp, proj);
}

#if 0
template <typename I>
constexpr void swap_offsets(I first, I last, unsigned char* offsets_l,
                            unsigned char* offsets_r, int num, bool use_swaps)
{
    using T = iter_value_t<I>;
    if (use_swaps) {
        // This case is needed for the descending distribution, where we need
        // to have proper swapping for pdqsort to remain O(n).
        for (int i = 0; i < num; ++i) {
            nano::iter_swap(first + offsets_l[i], last - offsets_r[i]);
        }
    } else if (num > 0) {
        I l = first + offsets_l[0];
        I r = last - offsets_r[0];
        T tmp(nano::iter_move(l));
        *l = nano::iter_move(r);

        for (int i = 1; i < num; ++i) {
            l = first + offsets_l[i];
            *r = nano::iter_move(l);
            r = last - offsets_r[i];
            *l = nano::iter_move(r);
        }
        *r = std::move(tmp);
    }
}

// Partitions [begin, end) around pivot *begin using comparison function comp.
// Elements equal to the pivot are put in the right-hand partition. Returns the
// position of the pivot after partitioning and whether the passed sequence
// already was correctly partitioned. Assumes the pivot is a median of at least
// 3 elements and that [begin, end) is at least insertion_sort_threshold long.
// Uses branchless partitioning.
template <typename I, typename Comp, typename Pred>
constexpr std::pair<I, bool> partition_right_branchless(I begin, I end,
                                                        Comp& comp, Pred& pred)
{
    using T = iter_value_t<I>;

    // Move pivot into local for speed.
    T pivot(nano::iter_move(begin));
    I first = begin;
    I last = end;

    // Find the first element greater than or equal than the pivot (the median
    // of 3 guarantees this exists).
    while (std::invoke(comp, std::invoke(pred, *++first),
                        std::invoke(pred, pivot)))
        ;

    // Find the first element strictly smaller than the pivot. We have to guard
    // this search if there was no element before *first.
    if (first - 1 == begin) {
        while (first < last && !std::invoke(comp, std::invoke(pred, *--last),
                                             std::invoke(pred, pivot)))
            ;
    } else {
        while (!std::invoke(comp, std::invoke(pred, *--last),
                             std::invoke(pred, pivot)))
            ;
    }

    // If the first pair of elements that should be swapped to partition are the
    // same element, the passed in sequence already was correctly partitioned.
    bool already_partitioned = first >= last;
    if (!already_partitioned) {
        nano::iter_swap(first, last);
        ++first;
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

    while (last - first > 2 * pdqsort_block_size) {
        // Fill up offset blocks with elements that are on the wrong side.
        if (num_l == 0) {
            start_l = 0;
            I it = first;
            for (unsigned char i = 0; i < pdqsort_block_size;) {
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
                offsets_l[num_l] = i++;
                num_l += !std::invoke(comp, std::invoke(pred, *it),
                                       std::invoke(pred, pivot));
                ++it;
            }
        }
        if (num_r == 0) {
            start_r = 0;
            I it = last;
            for (unsigned char i = 0; i < pdqsort_block_size;) {
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
                offsets_r[num_r] = ++i;
                num_r += std::invoke(comp, std::invoke(pred, *--it),
                                      std::invoke(pred, pivot));
            }
        }

        // Swap elements and update block sizes and first/last boundaries.
        int num = (nano::min)(num_l, num_r);
        swap_offsets(first, last, offsets_l + start_l, offsets_r + start_r, num,
                     num_l == num_r);
        num_l -= num;
        num_r -= num;
        start_l += num;
        start_r += num;
        if (num_l == 0)
            first += pdqsort_block_size;
        if (num_r == 0)
            last -= pdqsort_block_size;
    }

    iter_difference_t<I> l_size = 0, r_size = 0;
    iter_difference_t<I> unknown_left =
        (last - first) - ((num_r || num_l) ? pdqsort_block_size : 0);
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
        I it = first;
        for (unsigned char i = 0; i < l_size;) {
            offsets_l[num_l] = i++;
            num_l += !std::invoke(comp, std::invoke(pred, *it),
                                   std::invoke(pred, pivot));
            ++it;
        }
    }
    if (unknown_left && !num_r) {
        start_r = 0;
        I it = last;
        for (unsigned char i = 0; i < r_size;) {
            offsets_r[num_r] = ++i;
            num_r += std::invoke(comp, std::invoke(pred, *--it),
                                  std::invoke(pred, pivot));
        }
    }

    int num = (nano::min)(num_l, num_r);
    swap_offsets(first, last, offsets_l + start_l, offsets_r + start_r, num,
                 num_l == num_r);
    num_l -= num;
    num_r -= num;
    start_l += num;
    start_r += num;
    if (num_l == 0)
        first += l_size;
    if (num_r == 0)
        last -= r_size;

    // We have now fully identified [first, last)'s proper position. Swap the
    // last elements.
    if (num_l) {
        offsets_l += start_l;
        while (num_l--)
            nano::iter_swap(first + offsets_l[num_l], --last);
        first = last;
    }
    if (num_r) {
        offsets_r += start_r;
        while (num_r--)
            nano::iter_swap(last - offsets_r[num_r], first), ++first;
        last = first;
    }

    // Put the pivot in the right place.
    I pivot_pos = first - 1;
    *begin = nano::iter_move(pivot_pos);
    *pivot_pos = std::move(pivot);

    return std::make_pair(std::move(pivot_pos), already_partitioned);
}
#endif

// Partitions [begin, end) around pivot *begin using comparison function comp.
// Elements equal to the pivot are put in the right-hand partition. Returns the
// position of the pivot after partitioning and whether the passed sequence
// already was correctly partitioned. Assumes the pivot is a median of at least
// 3 elements and that [begin, end) is at least insertion_sort_threshold long.
template <sequence Seq, typename Comp, typename Proj, typename Cur = cursor_t<Seq>>
constexpr std::pair<cursor_t<Seq>, bool>
partition_right(Seq& seq, Cur const begin, Cur const end, Comp& comp, Proj& proj)
{
    using T = value_t<Seq>;

    // Move pivot into local for speed.
    T pivot(move_at(seq, begin));

    cursor_t<Seq> first = begin;
    cursor_t<Seq> last = end;

    // Find the first element greater than or equal than the pivot (the median
    // of 3 guarantees this exists).
    while (std::invoke(comp, std::invoke(proj, read_at(seq, inc(seq, first))),
                        std::invoke(proj, pivot))) {
    }

    // Find the first element strictly smaller than the pivot. We have to guard
    // this search if there was no element before *first.
    if (prev(seq, first) == begin) {
        while (first < last && !std::invoke(comp, std::invoke(proj, read_at(seq, dec(seq, last))),
                                             std::invoke(proj, pivot))) {
        }
    } else {
        while (!std::invoke(comp, std::invoke(proj, read_at(seq, dec(seq, last))),
                             std::invoke(proj, pivot))) {
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
        while (std::invoke(comp, std::invoke(proj, read_at(seq, inc(seq, first))),
                            std::invoke(proj, pivot)))
            ;
        while (!std::invoke(comp, std::invoke(proj, read_at(seq, dec(seq, last))),
                             std::invoke(proj, pivot)))
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
template <sequence Seq, typename Comp, typename Proj, typename Cur = cursor_t<Seq>>
constexpr cursor_t<Seq> partition_left(Seq& seq, Cur const begin, Cur const end, Comp& comp, Proj& proj)
{
    using T = value_t<Seq>;

    T pivot(move_at(seq, begin));
    cursor_t<Seq> first = begin;
    cursor_t<Seq> last = end;

    while (std::invoke(comp, std::invoke(proj, pivot),
                        std::invoke(proj, read_at(seq, dec(seq, last)))))
        ;

    if (next(seq, last) == end) {
        while (first < last && !std::invoke(comp, std::invoke(proj, pivot),
                                             std::invoke(proj, read_at(seq, inc(seq, first)))))
            ;
    } else {
        while (!std::invoke(comp, std::invoke(proj, pivot),
                             std::invoke(proj, read_at(seq, inc(seq, first)))))
            ;
    }

    while (first < last) {
        swap_at(seq, first, last);
        while (std::invoke(comp, std::invoke(proj, pivot),
                            std::invoke(proj, read_at(seq, dec(seq, last)))))
            ;
        while (!std::invoke(comp, std::invoke(proj, pivot),
                             std::invoke(proj, read_at(seq, inc(seq, first)))))
            ;
    }

    cursor_t<Seq> pivot_pos = last;
    read_at(seq, begin) = move_at(seq, pivot_pos);
    read_at(seq, pivot_pos) = std::move(pivot);

    return pivot_pos;
}

template <bool Branchless, typename Seq, typename Comp, typename Proj,
          typename Cur = cursor_t<Seq>>
constexpr void pdqsort_loop(Seq& seq, Cur begin, Cur end, Comp& comp, Proj& proj,
                            int bad_allowed, bool leftmost = true)
{
    //using diff_t = iter_difference_t<I>
    using diff_t = distance_t<Seq>;

    // Use a while loop for tail recursion elimination.
    while (true) {
        diff_t size = flux::distance(seq, begin, end);

        // Insertion sort is faster for small arrays.
        if (size < pdqsort_insertion_sort_threshold) {
            if (leftmost) {
                insertion_sort(seq, begin, end, comp, proj);
            } else {
                unguarded_insertion_sort(seq, begin, end, comp, proj);
            }
            return;
        }

        // Choose pivot as median of 3 or pseudomedian of 9.
        diff_t s2 = size / 2;
        if (size > pdqsort_ninther_threshold) {
            sort3(seq, begin, next(seq, begin, s2), prev(seq, end), comp, proj);
            sort3(seq, next(seq, begin), next(seq, begin, s2 - 1), next(seq, end, -2), comp, proj);
            sort3(seq, next(seq, begin, 2), next(seq, begin, s2 + 1), next(seq, end, -3), comp, proj);
            sort3(seq, next(seq, begin, s2 - 1), next(seq, begin, s2), next(seq, begin, s2 + 1), comp, proj);
            swap_at(seq, begin, next(seq, begin, s2));
        } else {
            sort3(seq, next(seq, begin, s2), begin, prev(seq, end), comp, proj);
        }

        // If *(begin - 1) is the end of the right partition of a previous
        // partition operation there is no element in [begin, end) that is
        // smaller than *(begin - 1). Then if our pivot compares equal to
        // *(begin - 1) we change strategy, putting equal elements in the left
        // partition, greater elements in the right partition. We do not have to
        // recurse on the left partition, since it's sorted (all equal).
        if (!leftmost && !std::invoke(comp, std::invoke(proj, read_at(seq, prev(seq, begin))),
                                       std::invoke(proj, read_at(seq, begin)))) {
            begin = next(seq, partition_left(seq, begin, end, comp, proj));
            continue;
        }

        // Partition and get results.
        /*
        std::pair<I, bool> part_result =
            Branchless ? partition_right_branchless(begin, end, comp, proj)
                       : partition_right(begin, end, comp, proj);
        I pivot_pos = part_result.first;
        bool already_partitioned = part_result.second;*/
        auto [pivot_pos, already_partitioned] = partition_right(seq, begin, end, comp, proj);

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
                auto view = flux::view(slice(seq, begin, end));
                std::ranges::make_heap(view, comp, proj);
                std::ranges::sort_heap(view, comp, proj);
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
                partial_insertion_sort(seq, begin, pivot_pos, comp, proj) &&
                partial_insertion_sort(seq, pivot_pos + 1, end, comp, proj))
                return;
        }

        // Sort the left partition first using recursion and do tail recursion
        // elimination for the right-hand partition.
        detail::pdqsort_loop<Branchless>(seq, begin, pivot_pos, comp, proj,
                                         bad_allowed, leftmost);
        begin = next(seq, pivot_pos);
        leftmost = false;
    }
}

template <typename Seq, typename Comp, typename Proj>
constexpr void pdqsort(Seq& seq, Comp& comp, Proj& proj)
{
    if (is_empty(seq)) {
        return;
    }
    /*
    constexpr bool Branchless =
         is_default_compare_v<std::remove_const_t<Comp>>&&
         std::same_as<Proj, identity> &&
         std::is_arithmetic_v<iter_value_t<I>>::value
*/
    constexpr bool Branchless = false;
    detail::pdqsort_loop<Branchless>(seq,
                                     first(seq),
                                     last(seq),
                                     comp,
                                     proj,
                                     detail::log2(size(seq)));
}

} // namespace detail

} // namespace flux

#endif