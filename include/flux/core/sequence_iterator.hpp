// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_SEQUENCE_ITERATOR_HPP_INCLUDED
#define FLUX_CORE_SEQUENCE_ITERATOR_HPP_INCLUDED

#include <flux/core/concepts.hpp>
#include <flux/core/sequence_access.hpp>

namespace flux {

namespace detail {

template <sequence Base>
consteval auto get_iterator_tag()
{
    if constexpr (contiguous_sequence<Base>) {
        return std::contiguous_iterator_tag{};
    } else if constexpr (random_access_sequence<Base>) {
        return std::random_access_iterator_tag{};
    } else if constexpr (bidirectional_sequence<Base>) {
        return std::bidirectional_iterator_tag{};
    } else if constexpr (multipass_sequence<Base>) {
        return std::forward_iterator_tag{};
    } else {
        return std::input_iterator_tag{};
    }
}

template <sequence S>
struct sequence_iterator {
private:
    S* seq_ = nullptr;
    cursor_t<S> cur_{};

    template <sequence SS>
    friend struct sequence_iterator;

public:
    using value_type = value_t<S>;
    using difference_type = distance_t;
    using element_type = value_t<S>; // Yes, really
    using iterator_concept = decltype(get_iterator_tag<S>());

    sequence_iterator() requires std::default_initializable<cursor_t<S>> = default;

    constexpr sequence_iterator(S& base, cursor_t<S> cur)
        : seq_(std::addressof(base)),
          cur_(std::move(cur))
    {}

    template <typename SS = S>
        requires std::is_const_v<SS>
    constexpr sequence_iterator(sequence_iterator<std::remove_const_t<SS>> other)
        : seq_(other.seq_),
          cur_(std::move(other.cur_))
    {}

    constexpr auto operator*() const -> element_t<S>
    {
        return flux::read_at(*seq_, cur_);
    }

    constexpr auto operator++() -> sequence_iterator&
    {
        flux::inc(*seq_, cur_);
        return *this;
    }

    constexpr void operator++(int) { flux::inc(*seq_, cur_); }

    constexpr auto operator++(int) -> sequence_iterator
        requires multipass_sequence<S>
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    constexpr auto operator--() -> sequence_iterator&
        requires bidirectional_sequence<S>
    {
        flux::dec(*seq_, cur_);
        return *this;
    }

    constexpr auto operator--(int) -> sequence_iterator
        requires bidirectional_sequence<S>
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    constexpr auto operator+=(difference_type n) -> sequence_iterator&
        requires random_access_sequence<S>
    {
        flux::inc(*seq_, cur_, n);
        return *this;
    }

    constexpr auto operator-=(difference_type n) -> sequence_iterator&
        requires random_access_sequence<S>
    {
        flux::inc(*seq_, cur_, num::neg(n));
        return *this;
    }

    constexpr auto operator[](difference_type n) const -> element_t<S>
    requires random_access_sequence<S>
    {
        auto i = flux::first(*seq_);
        flux::inc(*seq_, i, n);
        return flux::read_at(*seq_, i);
    }

    constexpr auto operator->() const -> std::add_pointer_t<element_t<S>>
        requires contiguous_sequence<S>
    {
        return flux::data(*seq_) + flux::distance(*seq_, flux::first(*seq_), cur_);
    }

    friend constexpr bool operator==(sequence_iterator const& self, std::default_sentinel_t)
    {
        return flux::is_last(*self.seq_, self.cur_);
    }

    friend bool operator==(sequence_iterator const&, sequence_iterator const&)
        requires multipass_sequence<S>
        = default;

    friend std::strong_ordering operator<=>(sequence_iterator const&, sequence_iterator const&)
        requires random_access_sequence<S>
        = default;

    friend constexpr auto operator+(sequence_iterator self, difference_type n)
        -> sequence_iterator
        requires random_access_sequence<S>
    {
        flux::inc(*self.seq_, self.cur_, n);
        return self;
    }

    friend constexpr auto operator+(difference_type n, sequence_iterator self)
        -> sequence_iterator
        requires random_access_sequence<S>
    {
        flux::inc(*self.seq_, self.cur_, n);
        return self;
    }

    friend constexpr auto operator-(sequence_iterator self, difference_type n)
        -> sequence_iterator
        requires random_access_sequence<S>
    {
        flux::inc(*self.seq_, self.cur_, num::neg(n));
        return self;
    }

    friend constexpr auto operator-(sequence_iterator const& lhs, sequence_iterator const& rhs)
        -> difference_type
        requires random_access_sequence<S>
    {
        FLUX_ASSERT(lhs.seq_ == rhs.seq_);
        return flux::distance(*lhs.seq_, rhs.cur_, lhs.cur_);
    }

    friend constexpr auto iter_move(sequence_iterator const& self)
        -> rvalue_element_t<S>
    {
        return flux::move_at(*self.seq_, self.cur_);
    }

    friend constexpr void iter_swap(sequence_iterator const& lhs, sequence_iterator const& rhs)
        requires element_swappable_with<S, S>
    {
        flux::swap_with(*lhs.seq_, lhs.cur_, *rhs.seq_, rhs.cur_);
    }
};

struct begin_fn {
    template <sequence S>
    constexpr auto operator()(S& seq) const
    {
        return sequence_iterator<S>(seq, flux::first(seq));
    }
};

struct end_fn {
    template <sequence S>
    constexpr auto operator()(S& seq) const
    {
        // Ranges requires sentinels to be copy-constructible
        if constexpr (bounded_sequence<S> && std::copy_constructible<cursor_t<S>>) {
            return sequence_iterator(seq, flux::last(seq));
        } else {
            return std::default_sentinel;
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto begin = detail::begin_fn{};
FLUX_EXPORT inline constexpr auto end = detail::end_fn{};

template <typename D>
constexpr auto inline_iter_base<D>::begin() &
    requires sequence<D>
{
    return flux::begin(derived());
}

template <typename D>
constexpr auto inline_iter_base<D>::begin() const&
    requires sequence<D const>
{
    return flux::begin(derived());
};

template <typename D>
constexpr auto inline_iter_base<D>::end() &
    requires sequence<D>
{
    return flux::end(derived());
}

template <typename D>
constexpr auto inline_iter_base<D>::end() const&
requires sequence<D const>
{
    return flux::end(derived());
};

} // namespace flux

// Every sequence is a range: furthermore, it is a view if it is either
// trivially copyable, or not copyable at all
// See P2415 for the logic behind this
template <flux::detail::derived_from_inline_sequence_base Seq>
inline constexpr bool std::ranges::enable_view<Seq> =
    std::is_trivially_copyable_v<Seq> || !std::copyable<Seq>;

#endif // FLUX_CORE_SEQUENCE_ITERATOR_HPP_INCLUDED
