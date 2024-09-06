
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
struct getlines_sequence : inline_sequence_base<getlines_sequence<CharT, Traits>> {
private:
    using istream_type = std::basic_istream<CharT, Traits>;
    using string_type = std::basic_string<CharT, Traits>;
    using char_type = CharT;

    istream_type* is_ = nullptr;
    string_type str_;
    char_type delim_{};

public:
    getlines_sequence() = default;

    getlines_sequence(istream_type& is, char_type delim)
        : is_(std::addressof(is)),
          delim_(delim)
    {}

    getlines_sequence(getlines_sequence&&) = default;
    getlines_sequence& operator=(getlines_sequence&&) = default;

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            explicit cursor_type() = default;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;
        };

        using self_t = getlines_sequence;

    public:
        static constexpr auto first(self_t& self) -> cursor_type
        {
            cursor_type cur{};
            inc(self, cur);
            return cur;
        }

        static constexpr auto is_last(self_t& self, cursor_type const&) -> bool
        {
            return !(self.is_ && static_cast<bool>(*self.is_));
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> cursor_type&
        {
            flux::assert_(self.is_ != nullptr,
                         "flux::getlines::inc(): attempt to iterate after stream EOF");
            if (!std::getline(*self.is_, self.str_, self.delim_)) {
                self.is_ = nullptr;
            }
            return cur;
        }

        static constexpr auto read_at(self_t& self, cursor_type const&) -> string_type const&
        {
            return self.str_;
        }
    };
};

struct getlines_fn {
    template <typename CharT, typename Traits>
    constexpr auto operator()(std::basic_istream<CharT, Traits>& istream, CharT delim) const
    {
        return getlines_sequence<CharT, Traits>(istream, delim);
    }

    template <typename CharT, typename Traits>
    constexpr auto operator()(std::basic_istream<CharT, Traits>& istream) const
    {
        return getlines_sequence<CharT, Traits>(istream, istream.widen('\n'));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto getlines = detail::getlines_fn{};

} // namespace flux

#endif // FLUX_SEQUENCE_GETLINES_HPP_INCLUDED
