
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_GETLINES_HPP_INCLUDED
#define FLUX_SEQUENCE_GETLINES_HPP_INCLUDED

#include <flux/core.hpp>

#include <iosfwd>
#include <string>

namespace flux {

namespace detail {

template <typename CharT, typename Traits>
struct getlines_iterable : inline_sequence_base<getlines_iterable<CharT, Traits>> {
private:
    using istream_type = std::basic_istream<CharT, Traits>;
    using string_type = std::basic_string<CharT, Traits>;
    using char_type = CharT;

    istream_type* is_ = nullptr;
    char_type delim_{};

    struct context_type : immovable {
        istream_type* is_;
        string_type str_;
        char_type delim_;

        using element_type = string_type const&;

        explicit context_type(getlines_iterable& iterable)
            : is_(iterable.is_),
              delim_(iterable.delim_)
        {
        }

        auto run_while(auto&& pred) -> iteration_result
        {
            while (std::getline(*is_, str_, delim_)) {
                if (!std::invoke(pred, static_cast<element_type>(str_))) {
                    return iteration_result::incomplete;
                }
            }
            return iteration_result::complete;
        }
    };

public:
    getlines_iterable(istream_type& is, char_type delim)
        : is_(std::addressof(is)),
          delim_(delim) { }

    auto iterate() { return context_type{*this}; }
};

struct getlines_fn {
    template <typename CharT, typename Traits>
    constexpr auto operator()(std::basic_istream<CharT, Traits>& istream, CharT delim) const
    {
        return getlines_iterable<CharT, Traits>(istream, delim);
    }

    template <typename CharT, typename Traits>
    constexpr auto operator()(std::basic_istream<CharT, Traits>& istream) const
    {
        return getlines_iterable<CharT, Traits>(istream, istream.widen('\n'));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto getlines = detail::getlines_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_GETLINES_HPP_INCLUDED
