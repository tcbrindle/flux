
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_STRING_SPLIT_HPP_INCLUDED
#define FLUX_STRING_SPLIT_HPP_INCLUDED

#include <flux/op/split.hpp>

#include <string_view>

namespace flux {

namespace detail {

template <typename T, typename... U>
concept any_of = (std::same_as<T, U> || ...);

template <typename C>
concept character = any_of<C, char, wchar_t, char8_t, char16_t, char32_t>;

struct to_string_view_fn {
    template <contiguous_sequence Seq>
        requires sized_sequence<Seq> && character<value_t<Seq>>
    constexpr auto operator()(Seq&& seq) const
    {
        return std::basic_string_view<value_t<Seq>>(flux::data(seq), flux::usize(seq));
    }
};

inline constexpr auto to_string_view = to_string_view_fn{};

struct split_string_fn {

    template <contiguous_sequence Seq, multipass_sequence Pattern>
        requires character<value_t<Seq>> &&
                std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    constexpr auto operator()(Seq&& seq, Pattern&& pattern) const
    {
        return flux::split(FLUX_FWD(seq), FLUX_FWD(pattern)).map(to_string_view);
    }

    // Attempt to hijack string literal patterns to do the right thing
    template <contiguous_sequence Seq, std::size_t N>
        requires character<value_t<Seq>>
    constexpr auto operator()(Seq&& seq, value_t<Seq> const (&pattern)[N]) const
    {
        return flux::split(FLUX_FWD(seq), std::basic_string_view(pattern))
                    .map(to_string_view);
    }

    template <contiguous_sequence Seq>
        requires character<value_t<Seq>>
    constexpr auto operator()(Seq&& seq, value_t<Seq> delim) const
    {
        return flux::split(FLUX_FWD(seq), delim).map(to_string_view);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto split_string = detail::split_string_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::split_string(auto&& pattern) &&
{
    return flux::split_string(std::move(derived()), FLUX_FWD(pattern));
}


} // namespace flux

#endif
