
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_REF_HPP_INCLUDED
#define FLUX_CORE_REF_HPP_INCLUDED

#include <flux/core/concepts.hpp>
#include <flux/core/inline_sequence_base.hpp>
#include <flux/core/sequence_access.hpp>

namespace flux {

namespace detail {

struct passthrough_traits_base : default_sequence_traits {

    static constexpr auto first(auto& self)
        -> decltype(flux::first(self.base()))
    {
        return flux::first(self.base());
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, auto const& cur)
        -> decltype(flux::is_last(self.base(), cur))
    {
        return flux::is_last(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, auto const& cur)
        -> decltype(flux::read_at(self.base(), cur))
    {
        return flux::read_at(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, auto& cur)
        -> decltype(flux::inc(self.base(), cur))
    {
        return flux::inc(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto dec(Self& self, auto& cur)
        -> decltype(flux::dec(self.base(), cur))
    {
        return flux::dec(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, auto& cur, distance_t dist)
        -> decltype(flux::inc(self.base(), cur, dist))
    {
        return flux::inc(self.base(), cur, dist);
    }

    template <typename Self>
    static constexpr auto distance(Self& self, auto const& from, auto const& to)
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
    static constexpr auto size(Self& self) -> decltype(flux::size(self.base()))
    {
        return flux::size(self.base());
    }

    static constexpr auto last(auto& self) -> decltype(flux::last(self.base()))
    {
        return flux::last(self.base());
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, auto const& cur)
        -> decltype(flux::move_at(self.base(), cur))
    {
        return flux::move_at(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto read_at_unchecked(Self& self, auto const& cur)
        -> decltype(flux::read_at_unchecked(self.base(), cur))
    {
        return flux::read_at_unchecked(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto move_at_unchecked(Self& self, auto const& cur)
        -> decltype(flux::move_at_unchecked(self.base(), cur))
    {
        return flux::move_at_unchecked(self.base(), cur);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
        -> decltype(flux::for_each_while(self.base(), FLUX_FWD(pred)))
    {
        return flux::for_each_while(self.base(), FLUX_FWD(pred));
    }
};

template <iterable Base>
struct ref_adaptor : inline_sequence_base<ref_adaptor<Base>> {
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

    // We are always movable
    ref_adaptor(ref_adaptor&&) = default;
    ref_adaptor& operator=(ref_adaptor&&) = default;
    ~ref_adaptor() = default;

    // ...but only copyable when `Base` is const
    ref_adaptor(ref_adaptor const&) requires std::is_const_v<Base> = default;
    ref_adaptor& operator=(ref_adaptor const&) requires std::is_const_v<Base> = default;

    constexpr Base& base() const noexcept { return *base_; }

    struct flux_sequence_traits : passthrough_traits_base {
        using value_type = value_t<Base>;
    };
};

template <typename>
inline constexpr bool is_ref_adaptor = false;

template <typename T>
inline constexpr bool is_ref_adaptor<ref_adaptor<T>> = true;

struct mut_ref_fn {
    template <iterable Seq>
        requires (!std::is_const_v<Seq>)
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

struct ref_fn {
    template <const_iterable Seq>
        requires (!is_ref_adaptor<Seq>)
    [[nodiscard]]
    constexpr auto operator()(Seq const& seq) const
    {
        return ref_adaptor<Seq const>(seq);
    }

    template <const_iterable Seq>
    [[nodiscard]]
    constexpr auto operator()(ref_adaptor<Seq> ref) const
    {
        return ref_adaptor<Seq const>(ref.base());
    }

    template <typename T>
    auto operator()(T const&&) const -> void = delete;
};

template <sequence Base>
    requires std::movable<Base>
struct owning_adaptor : inline_sequence_base<owning_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit owning_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    constexpr Base& base() & noexcept { return base_; }
    constexpr Base const& base() const& noexcept { return base_; }
    constexpr Base&& base() && noexcept { return std::move(base_); }
    constexpr Base const&& base() const&& noexcept { return std::move(base_); }

    struct flux_sequence_traits : passthrough_traits_base {
        using value_type = value_t<Base>;
    };
};

struct from_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (derived_from_inline_sequence_base<Seq>) {
            return FLUX_FWD(seq);
        } else {
            return owning_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq));
        }
    }
};

struct from_fwd_ref_fn {
    template <sequence Seq>
        requires adaptable_sequence<Seq> || std::is_lvalue_reference_v<Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq) const
    {
        if constexpr (std::is_lvalue_reference_v<Seq>) {
            if constexpr (std::is_const_v<std::remove_reference_t<Seq>>) {
                return ref_fn{}(seq);
            } else {
                return mut_ref_fn{}(seq);
            }
        } else {
            return from_fn{}(seq);
        }
    }
};


} // namespace detail

FLUX_EXPORT inline constexpr auto mut_ref = detail::mut_ref_fn{};
FLUX_EXPORT inline constexpr auto ref = detail::ref_fn{};
FLUX_EXPORT inline constexpr auto from = detail::from_fn{};
FLUX_EXPORT inline constexpr auto from_fwd_ref = detail::from_fwd_ref_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::ref() const&
    requires const_iterable<D>
{
    return flux::ref(derived());
}

template <typename D>
constexpr auto inline_sequence_base<D>::mut_ref() &
{
    return flux::mut_ref(derived());
}

} // namespace flux

#endif // FLUX_CORE_REF_HPP_INCLUDED
