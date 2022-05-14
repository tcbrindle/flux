
#ifndef FLUX_RANGES_VIEW_HPP_INCLUDED
#define FLUX_RANGES_VIEW_HPP_INCLUDED

#include <flux/core/concepts.hpp>
#include <flux/op/from.hpp>

#include <ranges>

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

template <lens Base>
struct view_adaptor : std::ranges::view_interface<view_adaptor<Base>>
{
private:
    [[no_unique_address]] Base base_;

    template <bool IsConst>
    class iterator {
        using S = std::conditional_t<IsConst, Base const, Base>;
        S* base_ = nullptr;
        cursor_t<S> cur_{};

    public:
        using value_type = value_t<S>;
        using difference_type = distance_t<S>;
        using element_type = value_t<S>;
        using iterator_concept = decltype(get_iterator_tag<S>());

        iterator() requires std::default_initializable<cursor_t<S>> = default;

        constexpr iterator(S& base, cursor_t<S> cur)
            : base_(std::addressof(base)),
              cur_(std::move(cur))
        {}

        constexpr iterator(iterator<!IsConst> other)
            requires IsConst && std::convertible_to<cursor_t<Base>,
                            cursor_t<S>>
            : base_(other.base_),
              cur_(std::move(other.cur_))
        {}

        constexpr auto operator*() const -> element_t<S>
        {
            return flux::read_at(*base_, cur_);
        }

        constexpr auto operator++() -> iterator&
        {
            flux::inc(*base_, cur_);
            return *this;
        }

        constexpr void operator++(int) { flux::inc(*base_, cur_); }

        constexpr auto operator++(int) -> iterator
            requires multipass_sequence<S>
        {
            auto temp = *this;
            ++*this;
            return temp;
        }

        constexpr auto operator--() -> iterator&
            requires bidirectional_sequence<S>
        {
            flux::dec(*base_, cur_);
            return *this;
        }

        constexpr auto operator--(int) -> iterator
            requires bidirectional_sequence<S>
        {
            auto temp = *this;
            --*this;
            return temp;
        }

        constexpr auto operator+=(difference_type n) -> iterator&
            requires random_access_sequence<S>
        {
            flux::inc(*base_, cur_, n);
            return *this;
        }

        constexpr auto operator-=(difference_type n) -> iterator&
            requires random_access_sequence<S>
        {
            flux::inc(*base_, cur_, -n);
            return *this;
        }

        constexpr auto operator[](difference_type n) const -> element_t<S>
            requires random_access_sequence<S>
        {
            auto i = flux::first(*base_);
            flux::inc(*base_, i, n);
            return flux::read_at(*base_, i);
        }

        constexpr auto operator->() const -> std::add_pointer_t<element_t<S>>
            requires contiguous_sequence<S>
        {
            return flux::data(*base_) + flux::distance(*base_, flux::first(*base_), cur_);
        }

        friend constexpr bool operator==(iterator const& self, std::default_sentinel_t)
        {
            return flux::is_last(*self.base_, self.cur_);
        }

        friend bool operator==(iterator const&, iterator const&)
            requires multipass_sequence<S>
            = default;

        friend std::strong_ordering operator<=>(iterator const&, iterator const&)
            requires random_access_sequence<S>
            = default;

        friend constexpr auto operator+(iterator self, difference_type n) -> iterator
            requires random_access_sequence<S>
        {
            flux::inc(*self.base_, self.cur_, n);
            return self;
        }

        friend constexpr auto operator+(difference_type n, iterator self) -> iterator
            requires random_access_sequence<S>
        {
            flux::inc(*self.base_, self.cur_, n);
            return self;
        }

        friend constexpr auto operator-(iterator self, difference_type n) -> iterator
            requires random_access_sequence<S>
        {
            flux::inc(*self.base_, self.cur_, -n);
            return self;
        }

        friend constexpr auto operator-(iterator const& lhs, iterator const& rhs)
            -> difference_type
            requires random_access_sequence<S>
        {
            return flux::distance(*lhs.base_, rhs.cur_, lhs.cur_);
        }

        friend constexpr auto iter_move(iterator const& self)
            -> rvalue_element_t<S>
        {
            return flux::move_at(*self.base_, self.cur_);
        }

        friend constexpr void iter_swap(iterator const& lhs, iterator const& rhs)
            requires element_swappable_with<S, S>
        {
            flux::swap_with(*lhs.base_, lhs.cur_, *rhs.base_, rhs.cur_);
        }
    };

public:
    constexpr view_adaptor() requires std::default_initializable<Base> = default;

    constexpr explicit view_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    constexpr auto begin()
    {
        return iterator<false>(base_, flux::first(base_));
    }

    constexpr auto begin() const
        requires sequence<Base const>
    {
        return iterator<true>(base_, flux::first(base_));
    }

    constexpr auto end()
    {
        // Ranges requires sentinels to be copy-constructible
        if constexpr (bounded_sequence<Base> &&
                      std::copy_constructible<cursor_t<Base>>) {
            return iterator<false>(base_, flux::last(base_));
        } else {
            return std::default_sentinel;
        }
    }

    constexpr auto end() const
        requires sequence<Base const>
    {
        if constexpr (bounded_sequence<Base const> &&
                      std::copy_constructible<cursor_t<Base const>>) {
            return iterator<true>(base_, flux::last(base_));
        } else {
            return std::default_sentinel;
        }
    }

    constexpr auto size() requires sized_sequence<Base>
    {
        return flux::size(base_);
    }

    constexpr auto size() const requires sized_sequence<Base const>
    {
        return flux::size(base_);
    }

    constexpr auto data() requires contiguous_sequence<Base>
    {
        return flux::data(base_);
    }

    constexpr auto data() const requires contiguous_sequence<Base const>
    {
        return flux::data(base_);
    }
};

struct view_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (std::ranges::viewable_range<Seq>) {
            return std::views::all(FLUX_FWD(seq));
        } else {
            return view_adaptor(flux::from(FLUX_FWD(seq)));
        }
    }
};

} // namespace detail

inline constexpr auto view = detail::view_fn{};

template <typename D>
constexpr auto lens_base<D>::view() &
{
    return flux::view(derived());
}

template <typename D>
constexpr auto lens_base<D>::view() const& requires sequence<D const>
{
    return flux::view(derived());
}

template <typename D>
constexpr auto lens_base<D>::view() &&
{
    return flux::view(std::move(derived()));
}

template <typename D>
constexpr auto lens_base<D>::view() const&& requires sequence<D const>
{
    return flux::view(std::move(derived()));
}

} // namespace flux

#endif // FLUX_RANGE_VIEW_HPP_INCLUDED