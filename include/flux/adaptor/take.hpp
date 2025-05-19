
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_TAKE_HPP_INCLUDED
#define FLUX_ADAPTOR_TAKE_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <typename Base>
struct take_adaptor : inline_sequence_base<take_adaptor<Base>> {
private:
    Base base_;
    int_t count_;

    template <typename BaseCtx>
    struct context_type : immovable {
        BaseCtx base_ctx;
        int_t remaining;

        using element_type = context_element_t<BaseCtx>;

        constexpr auto run_while(auto&& pred) -> iteration_result
        {
            if (remaining > 0) {
                auto res = base_ctx.run_while([&](auto&& elem) {
                    --remaining;
                    return pred(FLUX_FWD(elem)) && (remaining > 0);
                });
                return static_cast<iteration_result>(static_cast<bool>(res) || (remaining == 0));
            } else {
                return iteration_result::complete;
            }
        }
    };

public:
    constexpr take_adaptor(decays_to<Base> auto&& base, int_t count)
        : base_(FLUX_FWD(base)),
          count_(count)
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }

    constexpr auto iterate()
    {
        return context_type{.base_ctx = flux::iterate(base_), .remaining = count_};
    }
    constexpr auto iterate() const
        requires iterable<Base const>
    {
        return context_type{.base_ctx = flux::iterate(base_), .remaining = count_};
    }

    constexpr auto size() -> int_t
        requires sized_iterable<Base>
    {
        return (cmp::min)(flux::iterable_size(base_), count_);
    }

    constexpr auto size() const -> int_t
        requires sized_iterable<Base const>
    {
        return (cmp::min)(flux::iterable_size(base_), count_);
    }

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> base_cur;
            int_t length;

            friend bool operator==(cursor_type const&, cursor_type const&) = default;
            friend auto operator<=>(cursor_type const& lhs, cursor_type const& rhs) = default;
        };

    public:
        using value_type = value_t<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{.base_cur = flux::first(self.base_),
                               .length = self.count_};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return cur.length <= 0 || flux::is_last(self.base_, cur.base_cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur)
        {
            flux::inc(self.base_, cur.base_cur);
            cur.length = num::sub(cur.length, int_t {1});
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at(self.base_, cur.base_cur))
        {
            return flux::read_at(self.base_, cur.base_cur);
        }

        static constexpr auto move_at(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at(self.base_, cur.base_cur))
        {
            return flux::move_at(self.base_, cur.base_cur);
        }

        static constexpr auto read_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::read_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::read_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto move_at_unchecked(auto& self, cursor_type const& cur)
            -> decltype(flux::move_at_unchecked(self.base_, cur.base_cur))
        {
            return flux::move_at_unchecked(self.base_, cur.base_cur);
        }

        static constexpr auto dec(auto& self, cursor_type& cur)
            requires bidirectional_sequence<Base>
        {
            flux::dec(self.base_, cur.base_cur);
            cur.length = num::add(cur.length, int_t {1});
        }

        static constexpr auto inc(auto& self, cursor_type& cur, int_t offset)
            requires random_access_sequence<Base>
        {
            flux::inc(self.base_, cur.base_cur, offset);
            cur.length = num::sub(cur.length, offset);
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> int_t
            requires random_access_sequence<Base>
        {
            return (cmp::min)(flux::distance(self.base_, from.base_cur, to.base_cur),
                              num::sub(from.length, to.length));
        }

        static constexpr auto data(auto& self)
            -> decltype(flux::data(self.base_))
            requires contiguous_sequence<Base>
        {
            return flux::data(self.base_);
        }

        static constexpr auto size(auto& self)
            requires sized_sequence<Base> || infinite_sequence<Base>
        {
            if constexpr (infinite_sequence<Base>) {
                return self.count_;
            } else {
                return (cmp::min)(flux::size(self.base_), self.count_);
            }
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires (random_access_sequence<Base> && sized_sequence<Base>) ||
                      infinite_sequence<Base>
        {
            return cursor_type{
                .base_cur = flux::next(self.base_, flux::first(self.base_), size(self)),
                .length = 0
            };
        }

        static constexpr auto for_each_while(auto& self, auto&& pred) -> cursor_type
        {
            int_t len = self.count_;
            auto cur = flux::seq_for_each_while(self.base_, [&](auto&& elem) {
                return (len-- > 0) && std::invoke(pred, FLUX_FWD(elem));
            });

            return cursor_type{.base_cur = std::move(cur), .length = ++len};
        }
    };
};

struct take_fn {
    template <adaptable_iterable It>
    [[nodiscard]]
    constexpr auto operator()(It&& it, num::integral auto count) const
    {
        auto count_ = num::checked_cast<int_t>(count);
        if (count_ < 0) {
            runtime_error("Negative argument passed to take()");
        }

        return take_adaptor<std::decay_t<It>>(FLUX_FWD(it), count_);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto take = detail::take_fn{};

template <typename Derived>
constexpr auto inline_sequence_base<Derived>::take(num::integral auto count) &&
{
    return flux::take(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_ADAPTOR_TAKE_HPP_INCLUDED
