
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
struct from_sequence_t {
    explicit from_sequence_t() = default;
};

FLUX_EXPORT inline constexpr auto from_sequence = from_sequence_t{};

namespace detail {

template <typename C, typename Seq, typename... Args>
concept direct_sequence_constructible =
    std::constructible_from<C, Seq, Args...>;

template <typename C, typename Seq, typename... Args>
concept from_sequence_constructible =
    std::constructible_from<C, from_sequence_t, Seq, Args...>;

template <typename C>
using container_value_t = typename C::value_type; // Let's just assume it exists

template <sequence Seq>
using common_iterator_t =
    std::ranges::iterator_t<decltype(std::views::common(FLUX_DECLVAL(Seq)))>;


template <typename C, typename Seq, typename... Args>
concept cpp17_range_constructible =
    std::constructible_from<C, common_iterator_t<Seq>, common_iterator_t<Seq>, Args...>;

template <typename C, typename Elem>
concept container_insertable =
    requires (C& c, Elem&& elem) {
        requires (requires { c.push_back(FLUX_FWD(elem)); } ||
                  requires { c.insert(c.end(), FLUX_FWD(elem)); });
    };

template <typename C, typename Seq, typename... Args>
concept container_convertible =
    direct_sequence_constructible<C, Seq, Args...> ||
    from_sequence_constructible<C, Seq, Args...> ||
    cpp17_range_constructible<C, Seq, Args...> ||
    (  std::constructible_from<C, Args...> &&
        container_insertable<C, element_t<Seq>>);

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

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_direct_seq = decltype(C(FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_from_seq = decltype((C(from_sequence, FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...)));

template <template <typename...> typename C, typename Seq, typename... Args>
using ctad_from_iters = decltype(C(FLUX_DECLVAL(common_iterator_t<Seq>), FLUX_DECLVAL(common_iterator_t<Seq>), FLUX_DECLVAL(Args)...));

template <template <typename...> typename C, typename Seq, typename... Args>
concept can_deduce_container_type =
    requires { typename ctad_direct_seq<C, Seq, Args...>; } ||
    requires { typename ctad_from_seq<C, Seq, Args...>; } ||
    requires { typename ctad_from_iters<C, Seq, Args...>; } ||
    ( sizeof...(Args) == 0 && requires { typename C<value_t<Seq>>; });

template <typename T>
struct type_t { using type = T; };

template <template <typename...> typename C, typename Seq, typename... Args>
    requires can_deduce_container_type<C, Seq, Args...>
consteval auto deduce_container_type()
{
    if constexpr (requires { typename ctad_direct_seq<C, Seq, Args...>; }) {
        return type_t<decltype(C(FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...))>{};
    } else if constexpr (requires { typename ctad_from_seq<C, Seq, Args...>; }) {
        return type_t<decltype((C(from_sequence, FLUX_DECLVAL(Seq), FLUX_DECLVAL(Args)...)))>{};
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
template <typename Container, sequence Seq, typename... Args>
    requires (std::convertible_to<element_t<Seq>, detail::container_value_t<Container>> &&
                 detail::container_convertible<Container, Seq, Args...>) ||
             sequence<element_t<Seq>>
constexpr auto to(Seq&& seq, Args&&... args) -> Container
{
    if constexpr (std::convertible_to<element_t<Seq>, detail::container_value_t<Container>>) {
        if constexpr (detail::direct_sequence_constructible<Container, Seq, Args...>) {
            return Container(FLUX_FWD(seq), FLUX_FWD(args)...);
        } else if constexpr (detail::from_sequence_constructible<Container, Seq, Args...>) {
            return Container(from_sequence, FLUX_FWD(seq), FLUX_FWD(args)...);
        } else if constexpr (detail::cpp17_range_constructible<Container, Seq, Args...>) {
            auto view_ = std::views::common(FLUX_FWD(seq));
            return Container(view_.begin(), view_.end(), FLUX_FWD(args)...);
        } else {
            auto c = Container(FLUX_FWD(args)...);
            if constexpr (sized_sequence<Seq> && detail::reservable_container<Container>) {
                c.reserve(flux::usize(seq));
            }
            flux::output_to(seq, detail::make_inserter<element_t<Seq>>(c));
            return c;
        }
    } else {
        static_assert(sequence<element_t<Seq>>);
        return flux::to<Container>(flux::map(flux::from_fwd_ref(FLUX_FWD(seq)), [](auto&& elem) {
            return flux::to<detail::container_value_t<Container>>(FLUX_FWD(elem));
        }), FLUX_FWD(args)...);
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
