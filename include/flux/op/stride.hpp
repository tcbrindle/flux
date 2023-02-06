
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_STRIDE_HPP_INCLUDED
#define FLUX_OP_STRIDE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

// This is a Flux-ified version of ranges::advance.
inline constexpr struct advance_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, cursor_t<Seq>& cur, distance_t offset) const -> distance_t
    {
        if (offset > 0) {
            while (--offset >= 0)  {
                if (flux::is_last(seq, flux::inc(seq, cur))) {
                    break;
                }
            }
            return offset;
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
                runtime_error("advance() called with negative offset and non-bidirectional sequence");
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
            auto dist = std::min(flux::distance(seq, cur, flux::last(seq)), offset);
            flux::inc(seq, cur, dist);
            return offset - dist;
        } else if (offset < 0) {
            auto dist = -std::min(flux::distance(seq, flux::first(seq), cur), -offset);
            flux::inc(seq, cur, dist);
            return offset - dist;
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

    struct flux_sequence_traits : passthrough_traits_base<Base> {

        using value_type = value_t<Base>;
        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
        {
            advance(self.base(), cur, self.stride_);
        }

        // This version of stride is never bidir
        static void dec(...) = delete;

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.stride_ + (s % self.stride_ == 0 ? 0 : 1);
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_t<Base>
            requires sequence<decltype((self.base_))>
        {
            distance_t n = self.stride_;
            return flux::for_each_while(self.base_, [&n, &pred, s = self.stride_](auto&& elem) {
                if (++n < s) {
                    return true;
                } else {
                    n = 0;
                    return std::invoke(pred, FLUX_FWD(elem));
                }
            });
        }

    };
};

struct stride_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto by) const
    {
        FLUX_ASSERT(by > 0);
        return stride_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq), checked_cast<distance_t>(by));
    }
};

} // namespace detail

inline constexpr auto stride = detail::stride_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::stride(std::integral auto by) &&
{
    return flux::stride(std::move(derived()), by);
}

} // namespace flux

#endif // FLUX_OP_STRIDE_HPP_INCLUDED
