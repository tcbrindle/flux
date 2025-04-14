
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_TO_HPP_INCLUDED
#define FLUX_ALGORITHM_TO_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/adaptor/map.hpp>
#include <flux/algorithm/output_to.hpp>

namespace flux {

FLUX_EXPORT
struct from_iterable_t {
    explicit from_iterable_t() = default;
};

FLUX_EXPORT inline constexpr auto from_iterable = from_iterable_t{};

namespace detail {

template <typename C, typename It, typename... Args>
concept direct_iterable_constructible =
    std::constructible_from<C, It, Args...>;

template <typename C, typename It, typename... Args>
concept from_iterable_constructible =
    std::constructible_from<C, from_iterable_t, It, Args...>;

template <typename C>
using container_value_t = typename C::value_type; // Let's just assume it exists

template <std::ranges::input_range Rng>
using common_iterator_t =
    std::ranges::iterator_t<decltype(std::views::common(FLUX_DECLVAL(Rng)))>;

template <typename C, typename Rng, typename... Args>
concept cpp17_range_constructible =
    std::constructible_from<C, common_iterator_t<Rng>, common_iterator_t<Rng>, Args...>;

template <typename C, typename Elem>
concept container_insertable =
    requires (C& c, Elem&& elem) {
        requires (requires { c.push_back(FLUX_FWD(elem)); } ||
                  requires { c.insert(c.end(), FLUX_FWD(elem)); });
    };

template <typename C, typename Seq, typename... Args>
concept container_convertible =
    direct_iterable_constructible<C, Seq, Args...> ||
    from_iterable_constructible<C, Seq, Args...> ||
    cpp17_range_constructible<C, Seq, Args...> ||
    (std::constructible_from<C, Args...> && container_insertable<C, element_t<Seq>>);

template <typename C>
concept reservable_container =
    std::ranges::sized_range<C> &&
    requires (C& c, std::ranges::range_size_t<C> sz) {
        c.reserve(sz);
        { c.max_size() } -> std::same_as<std::ranges::range_size_t<C>>;
        { c.capacity() } -> std::same_as<std::ranges::range_size_t<C>>;
    };

template <typename Elem, typename C>
constexpr auto make_inserter(C& c)
{
    if constexpr (requires { c.push_back(FLUX_DECLVAL(Elem)); }) {
        return std::back_inserter(c);
    } else {
        return std::inserter(c, c.end());
    }
}

template <template <typename...> typename C, typename It, typename... Args>
using ctad_direct_iterable = decltype(C(FLUX_DECLVAL(It), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename It, typename... Args>
using ctad_from_iterable = decltype((C(from_iterable, FLUX_DECLVAL(It), FLUX_DECLVAL(Args)...)));

template <template <typename...> typename C, typename Rng, typename... Args>
using ctad_from_iters = decltype(C(FLUX_DECLVAL(common_iterator_t<Rng>), FLUX_DECLVAL(common_iterator_t<Rng>), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename It, typename... Args>
concept can_deduce_container_type =
    requires { typename ctad_direct_iterable<C, It, Args...>; } ||
    requires { typename ctad_from_iterable<C, It, Args...>; } ||
    requires { typename ctad_from_iters<C, It, Args...>; } ||
    ( sizeof...(Args) == 0 && requires { typename C<value_t<It>>; });

template <typename T>
struct type_t { using type = T; };

template <template <typename...> typename C, typename It, typename... Args>
    requires can_deduce_container_type<C, It, Args...>
consteval auto deduce_container_type()
{
    if constexpr (requires { typename ctad_direct_iterable<C, It, Args...>; }) {
        return type_t<decltype(C(FLUX_DECLVAL(It), FLUX_DECLVAL(Args)...))>{};
    } else if constexpr (requires { typename ctad_from_iterable<C, It, Args...>; }) {
        return type_t<decltype((C(from_iterable, FLUX_DECLVAL(It), FLUX_DECLVAL(Args)...)))>{};
    } else if constexpr (requires { typename ctad_from_iters<C, It, Args...>; }) {
        using CI = common_iterator_t<It>;
        return type_t<decltype(C(FLUX_DECLVAL(CI), FLUX_DECLVAL(CI), FLUX_DECLVAL(Args)...))>{};
    } else {
        static_assert(requires { typename C<value_t<It>>; });
        return type_t<C<value_t<It>>>{};
    }
}

template <template <typename...> typename C, typename It, typename... Args>
    requires can_deduce_container_type<C, It, Args...>
using deduced_container_t = typename decltype(deduce_container_type<C, It, Args...>())::type;


} // namespace detail

FLUX_EXPORT
template <typename Container, iterable It, typename... Args>
    requires (std::convertible_to<element_t<It>, detail::container_value_t<Container>> &&
                 detail::container_convertible<Container, It, Args...>) ||
             iterable<element_t<It>>
constexpr auto to(It&& it, Args&&... args) -> Container
{
    if constexpr (std::convertible_to<element_t<It>, detail::container_value_t<Container>>) {
        if constexpr (detail::direct_iterable_constructible<Container, It, Args...>) {
            return Container(FLUX_FWD(it), FLUX_FWD(args)...);
        } else if constexpr (detail::from_iterable_constructible<Container, It, Args...>) {
            return Container(from_iterable, FLUX_FWD(it), FLUX_FWD(args)...);
        } else if constexpr (std::ranges::input_range<It> && detail::cpp17_range_constructible<Container, It, Args...>) {
            auto view_ = std::views::common(FLUX_FWD(it));
            return Container(view_.begin(), view_.end(), FLUX_FWD(args)...);
        } else {
            auto c = Container(FLUX_FWD(args)...);
            if constexpr (sized_iterable<It> && detail::reservable_container<Container>) {
                c.reserve(flux::usize(it));
            }
            flux::output_to(it, detail::make_inserter<element_t<It>>(c));
            return c;
        }
    } else {
        static_assert(iterable<element_t<It>>);
        return flux::to<Container>(flux::map(flux::from_fwd_ref(FLUX_FWD(it)), [](auto&& elem) {
            return flux::to<detail::container_value_t<Container>>(FLUX_FWD(elem));
        }), FLUX_FWD(args)...);
    }
}

FLUX_EXPORT
template <template <typename...> typename Container, iterable It, typename... Args>
    requires detail::can_deduce_container_type<Container, It, Args...> &&
             detail::container_convertible<
                 detail::deduced_container_t<Container, It, Args...>, It, Args...>
constexpr auto to(It&& it, Args&&... args)
{
    using C_ = detail::deduced_container_t<Container, It, Args...>;
    return flux::to<C_>(FLUX_FWD(it), FLUX_FWD(args)...);
}


template <typename D>
template <typename Container, typename... Args>
constexpr auto inline_iter_base<D>::to(Args&&... args) -> Container
{
    return flux::to<Container>(derived(), FLUX_FWD(args)...);
}

template <typename D>
template <template <typename...> typename Container, typename... Args>
constexpr auto inline_iter_base<D>::to(Args&&... args)
{
    return flux::to<Container>(derived(), FLUX_FWD(args)...);
}

} // namespace flux

#endif // FLUX_ALGORITHM_TO_HPP_INCLUDED
