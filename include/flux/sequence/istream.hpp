
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_SEQUENCE_FROM_ISTREAM_HPP_INCLUDED
#define FLUX_SEQUENCE_FROM_ISTREAM_HPP_INCLUDED

#include <flux/core.hpp>

#include <iosfwd>

namespace flux {

namespace detail {

template <typename T, typename CharT, typename Traits>
    requires std::default_initializable<T>
class istream_adaptor : public inline_iter_base<istream_adaptor<T, CharT, Traits>> {
    using istream_type = std::basic_istream<CharT, Traits>;
    istream_type* is_ = nullptr;
    T val_ = T();

    friend struct iter_traits<istream_adaptor>;

public:
    explicit istream_adaptor(istream_type& is)
        : is_(std::addressof(is))
    {}

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

template <typename T, typename CharT, typename Traits>
struct iter_traits<detail::istream_adaptor<T, CharT, Traits>> : default_iter_traits
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        friend struct iter_traits;
        explicit cursor_type() = default;
    };

    using self_t = detail::istream_adaptor<T, CharT, Traits>;

public:
    static auto first(self_t& self) -> cursor_type
    {
        cursor_type cur{};
        inc(self, cur);
        return cur;
    }

    static auto is_last(self_t& self, cursor_type const&) -> bool
    {
        return !(self.is_ && static_cast<bool>(*self.is_));
    }

    static auto read_at(self_t& self, cursor_type const&) -> T const&
    {
        return self.val_;
    }

    static auto inc(self_t& self, cursor_type& cur) -> cursor_type&
    {
        if (!(self.is_ && (*self.is_ >> self.val_))) {
            self.is_ = nullptr;
        }
        return cur;
    }
};

FLUX_EXPORT
template <std::default_initializable T>
inline constexpr auto from_istream = detail::from_istream_fn<T>{};

} // namespace flux

#endif // FLUX_SEQUENCE_ISTREAM_HPP_INCLUDED
