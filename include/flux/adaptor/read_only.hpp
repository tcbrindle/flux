
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_READ_ONLY_HPP_INCLUDED
#define FLUX_ADAPTOR_READ_ONLY_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/adaptor/map.hpp>

namespace flux {

namespace detail {

template <typename T>
struct cast_to_const {
    constexpr auto operator()(auto&& elem) const -> T { return FLUX_FWD(elem); }
};

template <iterable Base>
    requires (not read_only_iterable<Base>)
struct read_only_adaptor : map_adaptor<Base, cast_to_const<const_element_t<Base>>> {
private:
    using map = map_adaptor<Base, cast_to_const<const_element_t<Base>>>;

public:
    constexpr explicit read_only_adaptor(decays_to<Base> auto&& base)
        : map(FLUX_FWD(base), cast_to_const<const_element_t<Base>>{})
    {}

    struct flux_sequence_traits : map::flux_sequence_traits {
    private:
        template <typename B = Base>
        using const_rvalue_element_t = std::common_reference_t<
            value_t<B> const&&, rvalue_element_t<B>>;

    public:
        using value_type = value_t<Base>;

        static constexpr auto move_at(auto& self, auto const& cur)
            -> decltype(auto)
        {
            return static_cast<const_rvalue_element_t<>>(flux::move_at(self.base(), cur));
        }

        static constexpr auto move_at_unchecked(auto& self, auto const& cur)
            -> decltype(auto)
        {
            return static_cast<const_rvalue_element_t<>>(flux::move_at_unchecked(self.base(), cur));
        }

        static constexpr auto data(auto& self)
            requires contiguous_sequence<Base>
        {
            using P = std::add_pointer_t<std::remove_reference_t<const_element_t<Base>>>;
            return static_cast<P>(flux::data(self.base()));
        }
    };
};

struct read_only_fn {
    template <sink_iterable It>
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> read_only_iterable auto
    {
        if constexpr (read_only_iterable<It>) {
            return FLUX_FWD(it);
        } else {
            return read_only_adaptor<std::decay_t<It>>(FLUX_FWD(it));
        }
    }
};


} // namespace detail

FLUX_EXPORT inline constexpr auto read_only = detail::read_only_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::read_only() &&
{
    return flux::read_only(std::move(derived()));
}

} // namespace flux

#endif // FLUX_ADAPTOR_READ_ONlY_HPP_INCLUDED
