
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_ISTREAMBUF_HPP_INCLUDED
#define FLUX_SEQUENCE_ISTREAMBUF_HPP_INCLUDED

#include <flux/core.hpp>

#include <iosfwd>

namespace flux {

namespace detail {

template <typename CharT, typename Traits>
void derives_from_streambuf_test(std::basic_streambuf<CharT, Traits>&);

template <typename T>
concept derives_from_streambuf = requires (T& t) { derives_from_streambuf_test(t); };

} // namespace detail

template <detail::derives_from_streambuf Streambuf>
struct iterable_traits<Streambuf> {

    template <typename CharT, typename Traits>
    struct context_type : immovable {
    private:
        using streambuf_type = std::basic_streambuf<CharT, Traits>;
        using char_type = CharT;
        using traits_type = Traits;

        streambuf_type* streambuf_;

    public:
        using element_type = char_type;

        explicit context_type(streambuf_type& streambuf) : streambuf_(std::addressof(streambuf)) { }

        auto run_while(auto&& pred) -> iteration_result
        {
            while (true) {
                auto c = streambuf_->sbumpc();
                if (c == traits_type::eof()) {
                    return iteration_result::complete;
                }
                if (!std::invoke(pred, traits_type::to_char_type(c))) {
                    return iteration_result::incomplete;
                }
            }
        }
    };

    template <typename CharT, typename Traits>
    static auto iterate(std::basic_streambuf<CharT, Traits>& streambuf)
        -> context_type<CharT, Traits>
    {
        return context_type<CharT, Traits>(streambuf);
    }
};

} // namespace flux

#endif // FLUX_SEQUENCE_ISTREAMBUF_HPP_INCLUDED
