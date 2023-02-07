
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_CHUNK_HPP_INCLUDED
#define FLUX_OP_CHUNK_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/op/slice.hpp>
#include <flux/op/stride.hpp>
#include <flux/op/take.hpp>

namespace flux {

namespace detail {

template <typename Base>
struct chunk_adaptor;

template <multipass_sequence Base>
struct chunk_adaptor<Base> : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    distance_t chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, distance_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {}

    struct flux_sequence_traits {
        static constexpr auto first(auto& self) -> cursor_t<Base>
        {
            return flux::first(self.base_);
        }

        static constexpr auto is_last(auto& self, cursor_t<Base> const& cur) -> bool
        {
            return flux::is_last(self.base_, cur);
        }

        static constexpr auto inc(auto& self, cursor_t<Base>& cur) -> void
        {
            advance(self.base_, cur, self.chunk_sz_);
        }

        static constexpr auto read_at(auto& self, cursor_t<Base> const& cur)
            -> decltype(flux::take(flux::slice(self.base_, cur, flux::last), self.chunk_sz_))
            requires multipass_sequence<decltype((self.base_))>
        {
            return flux::take(flux::slice(self.base_, cur, flux::last), self.chunk_sz_);
        }

        static constexpr auto last(auto& self) -> cursor_t<Base>
            requires bounded_sequence<Base>
        {
            return flux::last(self.base_);
        }

        static constexpr auto size(auto& self) -> distance_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.chunk_sz_ + (s % self.chunk_sz_ == 0 ? 0 : 1);
        }
    };
};

struct chunk_fn {
    template <adaptable_sequence Seq>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, std::integral auto chunk_sz) const
        -> sequence auto
    {
        FLUX_ASSERT(chunk_sz > 0);
        return chunk_adaptor<std::decay_t<Seq>>(FLUX_FWD(seq),
                                                checked_cast<distance_t>(chunk_sz));
    }
};

} // namespace detail

inline constexpr auto chunk = detail::chunk_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::chunk(std::integral auto chunk_sz) &&
{
    return flux::chunk(std::move(derived()), chunk_sz);
}

} // namespace flux

#endif // FLUX_OP_CHUNK_HPP_INCLUDED
