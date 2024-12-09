
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_CHUNK_BY_HPP_INCLUDED
#define FLUX_ADAPTOR_CHUNK_BY_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <multipass_sequence Base, typename Pred>
struct chunk_by_adaptor : inline_iter_base<chunk_by_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

public:
    constexpr explicit chunk_by_adaptor(decays_to<Base> auto&& base, Pred&& pred)
        : base_(FLUX_FWD(base)),
          pred_(std::move(pred))
    {}

    struct flux_iter_traits : default_iter_traits {
    private:
        struct cursor_type {
            cursor_t<Base> from;
            cursor_t<Base> to;

            friend constexpr auto operator==(cursor_type const& lhs, cursor_type const& rhs) -> bool
            {
                return lhs.from == rhs.from;
            }
        };

        static constexpr auto find_next(auto& self, cursor_t<Base> cur) -> cursor_t<Base>
        {
            if (flux::is_last(self.base_, cur)) {
                return cur;
            }

            auto nxt = flux::next(self.base_, cur);

            while (!flux::is_last(self.base_, nxt)) {
                if (!std::invoke(self.pred_, flux::read_at(self.base_, cur), flux::read_at(self.base_, nxt))) {
                    break;
                }
                cur = nxt;
                flux::inc(self.base_, nxt);
            }

            return nxt;
        }

        static constexpr auto find_prev(auto& self, cursor_t<Base> cur) -> cursor_t<Base>
        {
            auto const fst = flux::first(self.base_);

            if (cur == fst || flux::dec(self.base_, cur) == fst) {
                return cur;
            }

            do {
                auto prv = flux::prev(self.base_, cur);
                if (!std::invoke(self.pred_, flux::read_at(self.base_, prv), flux::read_at(self.base_, cur))) {
                    break;
                }
                cur = std::move(prv);
            } while (cur != fst);

            return cur;
        }

    public:
        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{
                .from = flux::first(self.base_),
                .to = find_next(self, flux::first(self.base_))
            };
        }

        static constexpr auto is_last(auto&, cursor_type const& cur) -> bool
        {
            return cur.from == cur.to;
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            cur = cursor_type{
                .from = cur.to,
                .to = find_next(self, cur.to)
            };
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::slice(self.base_, cur.from, cur.to))
        {
            return flux::slice(self.base_, cur.from, cur.to);
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type{flux::last(self.base_), flux::last(self.base_)};
        }

        static constexpr auto dec(auto& self, cursor_type& cur) -> void
            requires bidirectional_sequence<Base>
        {
            cur = cursor_type{
                .from = find_prev(self, cur.from),
                .to = cur.from
            };
        }
    };
};

struct chunk_by_fn {
    template <adaptable_sequence Seq, std::move_constructible Pred>
        requires multipass_sequence<Seq> &&
                 std::predicate<Pred&, element_t<Seq>, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const -> multipass_sequence auto
    {
        return chunk_by_adaptor<std::decay_t<Seq>, Pred>(FLUX_FWD(seq), std::move(pred));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto chunk_by = detail::chunk_by_fn{};

template <typename Derived>
template <typename Pred>
    requires multipass_sequence<Derived> &&
             std::predicate<Pred&, element_t<Derived>, element_t<Derived>>
constexpr auto inline_iter_base<Derived>::chunk_by(Pred pred) &&
{
    return flux::chunk_by(std::move(derived()), std::move(pred));
}

} // namespace flux

#endif // FLUX_ADAPTOR_CHUNK_BY_HPP_INCLUDED
