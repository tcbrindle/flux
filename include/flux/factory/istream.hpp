
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_FACTORY_FROM_ISTREAM_HPP_INCLUDED
#define FLUX_FACTORY_FROM_ISTREAM_HPP_INCLUDED

#include <flux/core.hpp>

#include <iosfwd>

namespace flux {

namespace detail {

template <typename T, typename CharT, typename Traits>
struct istream_adaptor : inline_sequence_base<istream_adaptor<T, CharT, Traits>> {
    std::basic_istream<CharT, Traits>* stream;

    struct context_type : immovable {
        std::basic_istream<CharT, Traits>* stream_;
        T elem_ = T{};

        using element_type = T const&;

        explicit context_type(istream_adaptor& adaptor) : stream_(adaptor.stream) { }

        auto run_while(auto&& pred) -> iteration_result
        {
            while (*stream_ >> elem_) {
                if (!std::invoke(pred, static_cast<element_type>(elem_))) {
                    return iteration_result::incomplete;
                }
            }
            return iteration_result::complete;
        }
    };

    explicit istream_adaptor(std::basic_istream<CharT, Traits>& is) : stream(std::addressof(is)) { }

    auto iterate() { return context_type{*this}; }
};

template <std::default_initializable T>
struct from_istream_fn {
    template <typename CharT, typename Traits>
    [[nodiscard]]
    auto operator()(std::basic_istream<CharT, Traits>& is) const
    {
        return istream_adaptor<T, CharT, Traits>(is);
    }
};

} // namespace detail

FLUX_EXPORT
template <std::default_initializable T>
inline constexpr auto from_istream = detail::from_istream_fn<T>{};

} // namespace flux

#endif // FLUX_FACTORY_ISTREAM_HPP_INCLUDED
