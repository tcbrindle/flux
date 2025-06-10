
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_TO_HPP_INCLUDED
#define FLUX_ALGORITHM_TO_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/adaptor/map.hpp>
#include <flux/algorithm/output_to.hpp>

#if defined(__cpp_lib_ranges_to_container) && (__cpp_lib_ranges_to_container >= 202202L)
#    define FLUX_HAVE_FROM_RANGE_CONSTRUCTORS
#endif

namespace flux {

FLUX_EXPORT
struct from_iterable_t {
    explicit from_iterable_t() = default;
};

FLUX_EXPORT inline constexpr auto from_iterable = from_iterable_t{};

namespace detail {

template <typename C, typename It, typename... Args>
concept direct_iterable_constructible = std::constructible_from<C, It, Args...>;

template <typename C, typename It, typename... Args>
concept from_iterable_constructible = std::constructible_from<C, from_iterable_t, It, Args...>;

#ifdef FLUX_HAVE_FROM_RANGE_CONSTRUCTORS
template <typename C, typename It, typename... Args>
concept from_range_constructible
    = std::constructible_from<C, std::from_range_t, decltype(as_range(std::declval<It>())),
                              Args...>;
#else
template <typename...>
concept from_range_constructible = false;
#endif

template <typename C>
using container_value_t = typename C::value_type; // Let's just assume it exists

template <iterable It>
using common_iterator_t
    = std::ranges::iterator_t<decltype(std::views::common(as_range(std::declval<It>())))>;

template <typename C, typename It, typename... Args>
concept cpp17_range_constructible
    = std::constructible_from<C, common_iterator_t<It>, common_iterator_t<It>, Args...>;

template <typename C, typename Elem>
concept container_appendable = requires(C& c, Elem&& elem) {
    requires(
        requires { c.emplace_back(FLUX_FWD(elem)); } || requires { c.push_back(FLUX_FWD(elem)); }
        || requires { c.emplace(c.end(), FLUX_FWD(elem)); }
        || requires { c.insert(c.end(), FLUX_FWD(elem)); });
};

template <typename C, typename It, typename... Args>
concept container_convertible
    = direct_iterable_constructible<C, It, Args...> || from_iterable_constructible<C, It, Args...>
    || from_range_constructible<C, It, Args...> || cpp17_range_constructible<C, It, Args...>
    || (std::constructible_from<C, Args...> && container_appendable<C, iterable_element_t<It>>);

template <typename C>
concept reservable_container =
    std::ranges::sized_range<C> &&
    requires (C& c, std::ranges::range_size_t<C> sz) {
        c.reserve(sz);
        { c.max_size() } -> std::same_as<std::ranges::range_size_t<C>>;
        { c.capacity() } -> std::same_as<std::ranges::range_size_t<C>>;
    };

template <typename Container>
constexpr auto container_appender(Container& c)
{
    return [&c](auto&& elem) {
        if constexpr (requires { c.emplace_back(FLUX_FWD(elem)); }) {
            c.emplace_back(FLUX_FWD(elem));
        } else if constexpr (requires { c.push_back(FLUX_FWD(elem)); }) {
            c.push_back(FLUX_FWD(elem));
            // } else if constexpr (requires { c.emplace(c.end(), FLUX_FWD(elem)); }) {
            //     c.emplace(c.end(), FLUX_FWD(elem));
        } else {
            c.insert(c.end(), FLUX_FWD(elem));
        }
    };
}

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_direct_iterable = decltype(C(FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_from_iterable = decltype((C(from_iterable, FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...)));

#ifdef FLUX_HAVE_FROM_RANGE_CONSTRUCTORS
template <template <typename...> typename C, typename It, typename... Args>
using ctad_from_range
    = decltype((C(std::from_range, as_range(std::declval<It>()), std::declval<Args>()...)));
#else
template <typename...>
struct nonesuch { };

template <template <typename...> typename, typename... Args>
using ctad_from_range = typename nonesuch<Args...>::type;
#endif

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_from_iters = decltype(C(FLUX_DECLVAL(common_iterator_t<Seq>),
                                   FLUX_DECLVAL(common_iterator_t<Seq>), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename Seq, typename... Args>
concept can_deduce_container_type = requires { typename ctad_direct_iterable<C, Seq, Args...>; }
    || requires { typename ctad_from_iterable<C, Seq, Args...>; }
    || requires { typename ctad_from_range<C, Seq, Args...>; }
    || requires { typename ctad_from_iters<C, Seq, Args...>; }
    || (sizeof...(Args) == 0 && requires { typename C<iterable_value_t<Seq>>; });

template <typename T>
struct type_t {
    using type = T;
};

template <template <typename...> typename C, typename Seq, typename... Args>
    requires can_deduce_container_type<C, Seq, Args...>
consteval auto deduce_container_type()
{
    if constexpr (requires { typename ctad_direct_iterable<C, Seq, Args...>; }) {
        return type_t<decltype(C(FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...))>{};
    } else if constexpr (requires { typename ctad_from_iterable<C, Seq, Args...>; }) {
        return type_t<decltype((C(from_iterable, FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...)))>{};
#ifdef FLUX_HAVE_FROM_RANGE_CONSTRUCTORS
    } else if constexpr (requires { typename ctad_from_range<C, Seq, Args...>; }) {
        return type_t<decltype((
            C(std::from_range, as_range(std::declval<Seq>()), std::declval<Args>()...)))>{};
#endif
    } else if constexpr (requires { typename ctad_from_iters<C, Seq, Args...>; }) {
        using I = common_iterator_t<Seq>;
        return type_t<decltype(C(FLUX_DECLVAL(I), FLUX_DECLVAL(I), FLUX_DECLVAL(Args)...))>{};
    } else {
        static_assert(requires { typename C<value_t<Seq>>; });
        return type_t<C<value_t<Seq>>>{};
    }
}

template <template <typename...> typename C, typename Seq, typename... Args>
    requires can_deduce_container_type<C, Seq, Args...>
using deduced_container_t = typename decltype(deduce_container_type<C, Seq, Args...>())::type;


} // namespace detail

FLUX_EXPORT
template <typename Container, iterable It, typename... Args>
    requires(std::convertible_to<iterable_element_t<It>, detail::container_value_t<Container>>
             && detail::container_convertible<Container, It, Args...>)
    || iterable<iterable_element_t<It>>
constexpr auto to(It&& it, Args&&... args) -> Container
{
    if constexpr (std::convertible_to<element_t<It>, detail::container_value_t<Container>>) {
        if constexpr (detail::direct_iterable_constructible<Container, It, Args...>) {
            return Container(FLUX_FWD(it), FLUX_FWD(args)...);
        } else if constexpr (detail::from_iterable_constructible<Container, It, Args...>) {
            return Container(from_iterable, FLUX_FWD(it), FLUX_FWD(args)...);
#ifdef FLUX_HAVE_FROM_RANGE_CONSTRUCTORS
        } else if constexpr (detail::from_range_constructible<Container, It, Args...>) {
            return Container(std::from_range, as_range(FLUX_FWD(it)), FLUX_FWD(args)...);
#endif
        } else if constexpr (detail::cpp17_range_constructible<Container, It, Args...>) {
            auto view_ = std::views::common(as_range(FLUX_FWD(it)));
            return Container(view_.begin(), view_.end(), FLUX_FWD(args)...);
        } else {
            auto c = Container(FLUX_FWD(args)...);
            if constexpr (sized_iterable<It> && detail::reservable_container<Container>) {
                c.reserve(num::cast<std::size_t>(flux::iterable_size(it)));
            }
            for_each(it, detail::container_appender(c));
            return c;
        }
    } else {
        static_assert(iterable<iterable_element_t<It>>);
        return flux::to<Container>(
            flux::map(flux::from_fwd_ref(FLUX_FWD(it)),
                      [](auto&& elem) {
                          return flux::to<detail::container_value_t<Container>>(FLUX_FWD(elem));
                      }),
            FLUX_FWD(args)...);
    }
}

FLUX_EXPORT
template <template <typename...> typename Container, sequence Seq, typename... Args>
    requires detail::can_deduce_container_type<Container, Seq, Args...> &&
             detail::container_convertible<
                 detail::deduced_container_t<Container, Seq, Args...>, Seq, Args...>
constexpr auto to(Seq&& seq, Args&&... args)
{
    using C_ = detail::deduced_container_t<Container, Seq, Args...>;
    return flux::to<C_>(FLUX_FWD(seq), FLUX_FWD(args)...);
}


template <typename D>
template <typename Container, typename... Args>
constexpr auto inline_sequence_base<D>::to(Args&&... args) -> Container
{
    return flux::to<Container>(derived(), FLUX_FWD(args)...);
}

template <typename D>
template <template <typename...> typename Container, typename... Args>
constexpr auto inline_sequence_base<D>::to(Args&&... args)
{
    return flux::to<Container>(derived(), FLUX_FWD(args)...);
}

} // namespace flux

#endif // FLUX_ALGORITHM_TO_HPP_INCLUDED
