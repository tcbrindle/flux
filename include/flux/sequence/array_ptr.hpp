
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_ARRAY_PTR_HPP_INCLUDED
#define FLUX_SEQUENCE_ARRAY_PTR_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename From, typename To>
concept non_slicing_ptr_convertible = std::convertible_to<From (*)[], To (*)[]>;

struct make_array_ptr_unchecked_fn;

}

FLUX_EXPORT
template <typename T>
    requires (std::is_object_v<T> && !std::is_abstract_v<T>)
struct array_ptr : inline_iter_base<array_ptr<T>> {
private:
    T* data_ = nullptr;
    distance_t sz_ = 0;

    friend struct detail::make_array_ptr_unchecked_fn;

    constexpr array_ptr(T* ptr, distance_t sz) noexcept
        : data_(ptr),
          sz_(sz)
    {}

public:
    array_ptr() = default;

    template <typename U>
        requires (!std::same_as<U, T> &&
                  detail::non_slicing_ptr_convertible<U, T>)
    constexpr explicit(false) array_ptr(array_ptr<U> const& other) noexcept
        : data_(flux::data(other)),
          sz_(flux::size(other))
    {}

    template <typename Seq>
        requires (!decays_to<Seq, array_ptr> &&
                  contiguous_sequence<Seq> &&
                  sized_iterable<Seq> &&
                  detail::non_slicing_ptr_convertible<std::remove_reference_t<element_t<Seq>>, T>)
    constexpr explicit array_ptr(Seq& seq)
        : data_(flux::data(seq)),
          sz_(flux::size(seq))
    {}

    template <typename Seq>
        requires (contiguous_sequence<Seq> &&
                  sized_iterable<Seq> &&
                  detail::non_slicing_ptr_convertible<std::remove_reference_t<element_t<Seq>>, T>)
    constexpr array_ptr(detail::ref_adaptor<Seq> ref)
        : data_(flux::data(ref)),
          sz_(flux::size(ref))
    {}

    array_ptr(array_ptr&&) = default;
    array_ptr& operator=(array_ptr&&) = default;
    ~array_ptr() = default;

    array_ptr(array_ptr const&) requires std::is_const_v<T> = default;
    array_ptr& operator=(array_ptr const&) requires std::is_const_v<T> = default;

    friend constexpr auto operator==(array_ptr const& lhs, array_ptr const&  rhs) -> bool
    {
        return std::ranges::equal_to{}(lhs.data_, rhs.data_) && lhs.sz_ == rhs.sz_;
    }

    struct flux_iter_traits : default_iter_traits {

        static constexpr auto first(array_ptr const&) -> index_t { return 0; }

        static constexpr auto is_last(array_ptr const& self, index_t idx) -> bool
        {
            return idx >= self.sz_;
        }

        static constexpr auto inc(array_ptr const& self, index_t& idx) -> void
        {
            FLUX_DEBUG_ASSERT(idx < self.sz_);
            idx = num::add(idx, distance_t{1});
        }

        static constexpr auto read_at(array_ptr const& self, index_t idx) -> T&
        {
            indexed_bounds_check(idx, self.sz_);
            return self.data_[idx];
        }

        static constexpr auto read_at_unchecked(array_ptr const& self, index_t idx) -> T&
        {
            return self.data_[idx];
        }

        static constexpr auto dec(array_ptr const& , index_t& idx) -> void
        {
            FLUX_DEBUG_ASSERT(idx > 0);
            --idx;
        }

        static constexpr auto last(array_ptr const& self) -> index_t { return self.sz_; }

        static constexpr auto inc(array_ptr const& self, index_t& idx, distance_t offset)
            -> void
        {
            index_t nxt = num::add(idx, offset);
            FLUX_DEBUG_ASSERT(nxt >= 0);
            FLUX_DEBUG_ASSERT(nxt <= self.sz_);
            idx = nxt;
        }

        static constexpr auto distance(array_ptr const&, index_t from, index_t to)
            -> distance_t
        {
            return num::sub(to, from);
        }

        static constexpr auto size(array_ptr const& self) -> distance_t
        {
            return self.sz_;
        }

        static constexpr auto data(array_ptr const& self) -> T* { return self.data_; }

        static constexpr auto for_each_while(array_ptr const& self, auto&& pred)
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

template <contiguous_sequence Seq>
array_ptr(detail::ref_adaptor<Seq>) -> array_ptr<std::remove_reference_t<element_t<Seq>>>;

namespace detail {

struct make_array_ptr_unchecked_fn {
    template <typename T>
        requires (std::is_object_v<T> && !std::is_abstract_v<T>)
    constexpr auto operator()(T* ptr, num::integral auto size) const -> array_ptr<T>
    {
        return array_ptr<T>(ptr, num::checked_cast<distance_t>(size));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto make_array_ptr_unchecked =
    detail::make_array_ptr_unchecked_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_ARRAY_PTR_HPP_INCLUDED
