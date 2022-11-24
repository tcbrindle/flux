
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_REF_HPP_INCLUDED
#define FLUX_OP_REF_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/op/for_each_while.hpp>

namespace flux {

namespace detail {

template <sequence Base>
struct ref_adaptor : lens_base<ref_adaptor<Base>> {
private:
    Base* base_;

    static void test_func(Base&) noexcept;
    static void test_func(Base&&) = delete;

public:
    // This seems thoroughly overcomplicated, but it's the formulation
    // std::reference_wrapper and ranges::ref_view use to avoid binding rvalues
    // when Base is a const type, while also avoiding implicit conversions
    template <typename Seq>
        requires (!std::same_as<std::decay_t<Seq>, ref_adaptor> &&
                  std::convertible_to<Seq, Base&> &&
                  requires { test_func(std::declval<Seq>()); })
    constexpr ref_adaptor(Seq&& seq)
        noexcept(noexcept(test_func(std::declval<Seq>())))
        : base_(std::addressof(static_cast<Base&>(FLUX_FWD(seq))))
    {}

    constexpr Base& base() const noexcept { return *base_; }

    friend struct sequence_iface<ref_adaptor>;
};

template <typename>
inline constexpr bool is_ref_adaptor = false;

template <typename T>
inline constexpr bool is_ref_adaptor<ref_adaptor<T>> = true;

struct ref_fn {
    template <sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq& seq) const
    {
        if constexpr (is_ref_adaptor<Seq>) {
            return seq;
        } else {
            return ref_adaptor<Seq>(seq);
        }
    }
};

template <sequence Base>
    requires std::movable<Base>
struct owning_adaptor : lens_base<owning_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit owning_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    owning_adaptor(owning_adaptor&&) = default;
    owning_adaptor& operator=(owning_adaptor&&) = default;

    constexpr Base& base() & noexcept { return base_; }
    constexpr Base const& base() const& noexcept { return base_; }
    constexpr Base&& base() && noexcept { return std::move(base_); }
    constexpr Base const&& base() const&& noexcept { return std::move(base_); }

    friend struct sequence_iface<owning_adaptor>;
};

template <typename Base>
struct passthrough_iface_base {

    template <typename Self>
    using cursor_t = decltype(flux::first(FLUX_DECLVAL(Self&).base()));

    static constexpr auto first(auto& self)
        -> decltype(flux::first(self.base()))
    {
        return flux::first(self.base());
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, cursor_t<Self> const& cur)
        -> decltype(flux::is_last(self.base(), cur))
    {
        return flux::is_last(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, cursor_t<Self> const& cur)
        -> decltype(flux::read_at(self.base(), cur))
    {
        return flux::unchecked_read_at(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur)
        -> decltype(flux::inc(self.base(), cur))
    {
        return flux::unchecked_inc(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto dec(Self& self, cursor_t<Self>& cur)
        -> decltype(flux::dec(self.base(), cur))
    {
        return flux::unchecked_dec(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, cursor_t<Self>& cur, distance_t dist)
        -> decltype(flux::inc(self.base(), cur, dist))
    {
        return flux::unchecked_inc(self.base(), cur, dist);
    }

    template <typename Self>
    static constexpr auto distance(Self& self, cursor_t<Self> const& from, cursor_t<Self> const& to)
        -> decltype(flux::distance(self.base(), from, to))
        requires random_access_sequence<decltype(self.base())>
    {
        return flux::distance(self.base(), from, to);
    }

    static constexpr auto data(auto& self)
        -> decltype(flux::data(self.base()))
    {
        return flux::data(self.base());
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        -> decltype(flux::size(self.base()))
        requires sized_sequence<decltype(self.base())>
    {
        return flux::size(self.base());
    }

    static constexpr auto last(auto& self)
        -> decltype(flux::last(self.base()))
    {
        return flux::last(self.base());
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, auto const& cur)
        -> decltype(flux::move_at(self.base(), cur))
    {
        return flux::unchecked_move_at(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
        -> decltype(flux::for_each_while(self.base(), FLUX_FWD(pred)))
    {
        return flux::for_each_while(self.base(), FLUX_FWD(pred));
    }

    template <typename Self>
    static constexpr auto slice(Self& self, cursor_t<Self> from, cursor_t<Self> to)
        -> decltype(iface_t<decltype(self.base())>::slice(self.base(), std::move(from), std::move(to)))
    {
        return iface_t<decltype(self.base())>::slice(self.base(), std::move(from), std::move(to));
    }

    template <typename Self>
    static constexpr auto slice(Self& self, cursor_t<Self> from)
        -> decltype(iface_t<decltype(self.base())>::slice(self.base(), std::move(from)))
    {
        return iface_t<decltype(self.base())>::slice(self.base(), std::move(from));
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::ref_adaptor<Base>>
    : detail::passthrough_iface_base<Base> {
    using value_type = value_t<Base>;
};

template <typename Base>
struct sequence_iface<detail::owning_adaptor<Base>>
    : detail::passthrough_iface_base<Base> {
    using value_type = value_t<Base>;
};

inline constexpr auto ref = detail::ref_fn{};

/*
template <typename D>
constexpr auto lens_base<D>::ref() & -> lens auto
{
    return detail::ref_adaptor<D>(derived());
}
*/

} // namespace flux

#endif // FLUX_OP_REF_HPP_INCLUDED
