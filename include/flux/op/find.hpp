
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_FIND_HPP_INCLUDED
#define FLUX_OP_FIND_HPP_INCLUDED

#include <flux/core/utils.hpp>
#include <flux/op/for_each_while.hpp>

#include <cstring>
#include <type_traits>

namespace flux {

namespace detail {

struct find_fn {
private:
    template <typename Seq, typename Value>
    static constexpr auto impl(Seq&& seq, Value const& value) -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return FLUX_FWD(elem) != value;
        });
    }

public:
    template <sequence Seq, typename Value>
        requires std::equality_comparable_with<element_t<Seq>, Value const&>
    constexpr auto operator()(Seq&& seq, Value const& value) const -> cursor_t<Seq>
    {
        constexpr auto can_memchr = 
            contiguous_sequence<Seq> && sized_sequence<Seq> && 
            std::same_as<Value, value_t<Seq>> &&
            flux::detail::any_of<value_t<Seq>, char, signed char, unsigned char, char8_t, std::byte>;

        if constexpr (can_memchr) {
            if (std::is_constant_evaluated()) {
                return impl(seq, value);
            } else {
                auto size = flux::usize(seq);
                if(size == 0) return flux::last(seq);
                FLUX_ASSERT(flux::data(seq) != nullptr);
                auto location = std::memchr(flux::data(seq), static_cast<unsigned char>(value),
                    flux::usize(seq) * sizeof(value_t<Seq>));
                if (location == nullptr) {
                    return flux::last(seq);
                } else {
                    auto offset = static_cast<value_t<Seq> const*>(location) - flux::data(seq);
                    return flux::next(seq, flux::first(seq), offset);
                }
            }
        } else {
            return impl(seq, value);
        }
    }
};

struct find_if_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return !std::invoke(pred, FLUX_FWD(elem));
        });
    }
};

struct find_if_not_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
        -> cursor_t<Seq>
    {
        return for_each_while(seq, [&](auto&& elem) {
            return std::invoke(pred, FLUX_FWD(elem));
        });
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto find = detail::find_fn{};
FLUX_EXPORT inline constexpr auto find_if = detail::find_if_fn{};
FLUX_EXPORT inline constexpr auto find_if_not = detail::find_if_not_fn{};

template <typename D>
template <typename Value>
    requires std::equality_comparable_with<element_t<D>, Value const&>
constexpr auto inline_sequence_base<D>::find(Value const& val)
{
    return flux::find(derived(), val);
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::find_if(Pred pred)
{
    return flux::find_if(derived(), std::ref(pred));
}

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto inline_sequence_base<D>::find_if_not(Pred pred)
{
    return flux::find_if_not(derived(), std::ref(pred));
}

} // namespace flux

#endif // FLUX_OP_FIND_HPP_INCLUDED