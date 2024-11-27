
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_STRIDE_HPP_INCLUDED
#define FLUX_ADAPTOR_STRIDE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

// This is a Flux-ified version of ranges::advance.
inline constexpr struct advance_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const -> distance_t
    {
        if (offset > 0) {
            distance_t counter = 0;
            while (offset-- > 0 && !flux::is_last(seq, cur)) {
                flux::inc(seq, cur);
                ++counter;
            }
            return counter;
        } else if (offset < 0) {
            if constexpr (bidirectional_sequence<Seq>) {
                auto const fst = flux::first(seq);
                while (offset < 0) {
                    if (flux::dec(seq, cur) == fst) {
                        break;
                    }
                    ++offset;
                }
                return offset;
            } else {
                runtime_error(
                    "advance() called with negative offset and non-bidirectional sequence");
            }
        } else {
            return 0;
        }
    }

    template <random_access_sequence Seq>
        requires bounded_sequence<Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const -> distance_t
    {
        if (offset > 0) {
            auto dist = (cmp::min)(flux::distance(seq, cur, flux::last(seq)), offset);
            flux::inc(seq, cur, dist);
            return num::sub(offset, dist);
        } else if (offset < 0) {
            auto dist = num::neg(
                (cmp::min)(flux::distance(seq, flux::first(seq), cur), num::neg(offset)));
            flux::inc(seq, cur, dist);
            return num::sub(offset, dist);
        } else {
            return 0;
        }
    }
} advance;

template <typename Base>
struct stride_adaptor : inline_sequence_base<stride_adaptor<Base>> {
private:
    Base base_;
    distance_t stride_;

public:
    constexpr stride_adaptor(decays_to<Base> auto&& base, distance_t stride)
        : base_(FLUX_FWD(base)),
          stride_(stride)
    {}

    constexpr auto base() & -> Base& { return base_; }
    constexpr auto base() const& -> Base const& { return base_; }

    constexpr auto stride() const -> distance_t { return stride_; }
};

template <typename Base>
struct stride_iterable_traits {
    using value_type = value_t<Base>;

    template <typename Self>
    static consteval auto element_type(Self& self) -> element_t<decltype(self.base())>;

    static constexpr auto iterate(auto& self, auto&& pred) -> bool
    {
        distance_t n = self.stride();
        distance_t s = num::sub(n, distance_t{1});
        return flux::iterate(self.base(), [&n, &pred, s](auto&& elem) {
            if (n < s) {
                n = num::add(n, distance_t{1});
                return true;
            } else {
                n = 0;
                return std::invoke(pred, FLUX_FWD(elem));
            }
        });
    }

    static constexpr auto size(auto& self) -> distance_t
        requires sized_iterable<Base>
    {
        auto s = flux::size(self.base());
        return num::add(num::div(s, self.stride()),
            (num::mod(s, self.stride()) == 0 ? distance_t{0} : distance_t{1}));
    }
};

template <typename Base>
struct stride_sequence_traits : stride_iterable_traits<Base>, passthrough_traits_base {
    using stride_iterable_traits<Base>::element_type;
    using stride_iterable_traits<Base>::iterate;
    using stride_iterable_traits<Base>::size;

    static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
    {
        advance(self.base(), cur, self.stride());
    }

    // This version of stride is never bidir
    static void dec(...) = delete;

    static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_t<Base>
        requires sequence<decltype(self.base())>
    {
        distance_t n = self.stride();
        return flux::for_each_while(self.base(), [&n, &pred, s = self.stride()](auto&& elem) {
            if (++n < s) {
                return true;
            } else {
                n = 0;
                return std::invoke(pred, FLUX_FWD(elem));
            }
        });
    }
};

template <typename Base>
struct stride_bidir_traits : stride_iterable_traits<Base> {
private:
    struct cursor_type {
        cursor_t<Base> cur{};
        distance_t missing = 0;

        friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs) -> bool
        {
            return lhs.cur == rhs.cur;
        }

        friend constexpr auto operator<=>(cursor_type const& lhs, cursor_type const& rhs)
            -> std::strong_ordering
            requires ordered_cursor<cursor_t<Base>>
        {
            return lhs.cur <=> rhs.cur;
        }
    };

public:
    using value_type = value_t<Base>;
    static constexpr bool is_infinite = infinite_sequence<Base>;

    static constexpr auto first(auto& self) -> cursor_type
    {
        return cursor_type{.cur = flux::first(self.base()), .missing = 0};
    }

    static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
    {
        return flux::is_last(self.base(), cur.cur);
    }

    static constexpr auto inc(auto& self, cursor_type& cur) -> void
    {
        cur.missing = advance(self.base(), cur.cur, self.stride());
    }

    static constexpr auto read_at(auto& self, cursor_type const& cur)
        -> decltype(flux::read_at(self.base(), cur.cur))
    {
        return flux::read_at(self.base(), cur.cur);
    }

    static constexpr auto move_at(auto& self, cursor_type const& cur)
        -> decltype(flux::move_at(self.base(), cur.cur))
    {
        return flux::move_at(self.base(), cur.cur);
    }

    static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
        -> decltype(flux::read_at_unchecked(self.base(), cur.cur))
    {
        return flux::read_at_unchecked(self.base(), cur.cur);
    }

    static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
        -> decltype(flux::move_at_unchecked(self.base(), cur.cur))
    {
        return flux::move_at_unchecked(self.base(), cur.cur);
    }

    static constexpr auto last(auto& self) -> cursor_type
        requires bounded_sequence<Base> && sized_iterable<Base>
    {
        distance_t missing = (self.stride() - flux::size(self.base()) % self.stride()) % self.stride();
        return cursor_type{.cur = flux::last(self.base()), .missing = missing};
    }

    static constexpr auto dec(auto& self, cursor_type& cur) -> void
        requires bidirectional_sequence<Base>
    {
        advance(self.base(), cur.cur, cur.missing - self.stride());
        cur.missing = 0;
    }

    static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
        -> distance_t
        requires random_access_sequence<Base>
    {
        return (flux::distance(self.base(), from.cur, to.cur) - from.missing + to.missing)
            / self.stride();
    }

    static constexpr auto inc(auto& self, cursor_type& cur, distance_t offset) -> void
        requires random_access_sequence<Base>
    {
        if (offset > 0) {
            cur.missing = num::mod(advance(self.base(), cur.cur, num::mul(offset, self.stride())),
                                   self.stride());
        } else if (offset < 0) {
            advance(self.base(), cur.cur, num::add(num::mul(offset, self.stride()), cur.missing));
            cur.missing = 0;
        }
    }

    static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
        requires sequence<decltype(self.base())>
    {
        distance_t n = self.stride();
        auto c = flux::for_each_while(self.base(), [&n, &pred, s = self.stride()](auto&& elem) {
            if (++n < s) {
                return true;
            } else {
                n = 0;
                return std::invoke(pred, FLUX_FWD(elem));
            }
        });
        return cursor_type{std::move(c), (n + 1) % self.stride()};
    }
};

struct stride_fn {
    template <sink_iterable It>
    [[nodiscard]]
    constexpr auto operator()(It&& it, num::integral auto by) const
    {
        FLUX_ASSERT(by > 0);
        return stride_adaptor<std::decay_t<It>>(FLUX_FWD(it), num::checked_cast<distance_t>(by));
    }
};

} // namespace detail

template <typename Base>
struct iter_traits<detail::stride_adaptor<Base>>
    : detail::stride_iterable_traits<Base> {};

template <sequence Base>
struct iter_traits<detail::stride_adaptor<Base>>
    : detail::stride_sequence_traits<Base> {};

template <bidirectional_sequence Base>
struct iter_traits<detail::stride_adaptor<Base>>
    : detail::stride_bidir_traits<Base> {};

FLUX_EXPORT inline constexpr auto stride = detail::stride_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::stride(num::integral auto by) &&
{
    return flux::stride(std::move(derived()), by);
}

} // namespace flux

#endif // FLUX_ADAPTOR_STRIDE_HPP_INCLUDED
