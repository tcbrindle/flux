
#ifndef FLUX_SOURCE_FROM_ISTREAM_HPP_INCLUDED
#define FLUX_SOURCE_FROM_ISTREAM_HPP_INCLUDED

#include <flux/core.hpp>

#include <iosfwd>

namespace flux {

namespace detail {

template <typename T, typename CharT, typename Traits>
    requires std::default_initializable<T>
class istream_adaptor : public lens_base<istream_adaptor<T, CharT, Traits>> {
    using istream_type = std::basic_istream<CharT, Traits>;
    istream_type* is_ = nullptr;
    T val_ = T();

    friend struct sequence_iface<istream_adaptor>;

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
struct sequence_iface<detail::istream_adaptor<T, CharT, Traits>>
{
private:
    struct cursor_type {
        cursor_type(cursor_type&&) = default;
        cursor_type& operator=(cursor_type&&) = default;
    private:
        friend struct sequence_iface;
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

template <std::default_initializable T>
inline constexpr auto from_istream = detail::from_istream_fn<T>{};

} // namespace flux

#endif
