
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SOURCE_ARRAY_PTR_HPP_INCLUDED
#define FLUX_SOURCE_ARRAY_PTR_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename From, typename To>
concept non_slicing_ptr_convertible = std::convertible_to<From (*)[], To (*)[]>;

}

template <typename T>
    requires (std::is_object_v<T> && !std::is_abstract_v<T>)
struct array_ptr : inline_sequence_base<array_ptr<T>> {
private:
    T* data_ = nullptr;
    distance_t sz_ = 0;

public:
    array_ptr() = default;

    template <typename U>
        requires detail::non_slicing_ptr_convertible<U, T>
    constexpr explicit(false) array_ptr(array_ptr<U> const& other) noexcept
        : data_(flux::data(other)),
          sz_(flux::size(other))
    {}

    template <typename Seq>
        requires (!decays_to<Seq, array_ptr> &&
                  contiguous_sequence<Seq> &&
                  sized_sequence<Seq> &&
                  detail::non_slicing_ptr_convertible<std::remove_reference_t<element_t<Seq>>, T>)
    constexpr explicit array_ptr(Seq& seq)
        : data_(flux::data(seq)),
          sz_(flux::size(seq))
    {}

    template <typename U, std::integral S>
        requires detail::non_slicing_ptr_convertible<U, T>
    constexpr array_ptr(U* ptr, S size)
        : data_(ptr),
          sz_(checked_cast<distance_t>(size))
    {
        FLUX_DEBUG_ASSERT(size >= 0);
    }

    friend constexpr auto operator==(array_ptr lhs, array_ptr rhs) -> bool
    {
        return std::ranges::equal_to{}(lhs.data_, rhs.data_) && lhs.sz_ == rhs.sz_;
    }

    struct flux_sequence_traits {

        static constexpr auto first(array_ptr) -> index_t { return 0; }

        static constexpr auto is_last(array_ptr self, index_t idx) -> bool
        {
            return idx >= self.sz_;
        }

        static constexpr auto inc(array_ptr self, index_t& idx) -> void
        {
            FLUX_DEBUG_ASSERT(idx < self.sz_);
            idx = num::checked_add(idx, distance_t{1});
        }

        static constexpr auto read_at(array_ptr self, index_t idx) -> T&
        {
            bounds_check(idx >= 0);
            bounds_check(idx < self.sz_);
            return self.data_[idx];
        }

        static constexpr auto read_at_unchecked(array_ptr self, index_t idx) -> T&
        {
            return self.data_[idx];
        }

        static constexpr auto dec(array_ptr, index_t& idx) -> void
        {
            FLUX_DEBUG_ASSERT(idx >= 0);
            --idx;
        }

        static constexpr auto last(array_ptr self) -> index_t { return self.sz_; }

        static constexpr auto inc(array_ptr self, index_t& idx, distance_t offset)
            -> void
        {
            index_t nxt = num::checked_add(idx, offset);
            FLUX_DEBUG_ASSERT(nxt >= 0);
            FLUX_DEBUG_ASSERT(nxt <= self.sz_);
            idx = nxt;
        }

        static constexpr auto distance(array_ptr, index_t from, index_t to)
            -> distance_t
        {
            return num::checked_sub(to, from);
        }

        static constexpr auto size(array_ptr self) -> distance_t
        {
            return self.sz_;
        }

        static constexpr auto data(array_ptr self) -> T* { return self.data_; }

        static constexpr auto for_each_while(array_ptr self, auto&& pred)
        {
            index_t idx = 0;
            for (; idx < self.sz_; idx++) {
                if (!std::invoke(pred, self.data_[idx])) {
                    break;
                }
            }
            return idx;
        }
    };
};

template <contiguous_sequence Seq>
array_ptr(Seq&) -> array_ptr<std::remove_reference_t<element_t<Seq>>>;

template <typename T, std::integral Sz>
array_ptr(T*, Sz) -> array_ptr<T>;

} // namespace flux

#endif
