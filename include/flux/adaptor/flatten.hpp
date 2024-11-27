
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_FLATTEN_HPP_INCLUDED
#define FLUX_ADAPTOR_FLATTEN_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

template <iterable Base>
struct flatten_adaptor : inline_iter_base<flatten_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit flatten_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_iter_traits {
        template <typename Self>
        static consteval auto element_type(Self& self)
            -> element_t<element_t<decltype((self.base_))>>;

        static constexpr auto iterate(auto& self, auto&& pred) -> bool
        {
            return flux::iterate(self.base_, [&pred](auto&& inner) {
                return flux::iterate(inner, pred);
            });
        }
    };
};

template <sequence Base>
struct flatten_adaptor<Base> : inline_iter_base<flatten_adaptor<Base>> {
private:
    using InnerSeq = element_t<Base>;

    Base base_;
    optional<InnerSeq> inner_ = nullopt;

public:
    constexpr explicit flatten_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_iter_traits : default_iter_traits {
    private:
        using self_t = flatten_adaptor;

        struct cursor_type {
            constexpr explicit cursor_type(cursor_t<Base>&& outer_cur)
                : outer_cur(std::move(outer_cur))
            {}

            cursor_type() = default;
            cursor_type(cursor_type&&) = default;
            cursor_type& operator=(cursor_type&&) = default;

            cursor_t<Base> outer_cur;
            optional<cursor_t<InnerSeq>> inner_cur = nullopt;
        };

        static constexpr auto satisfy(auto& self, cursor_type& cur) -> void
        {
            while (!flux::is_last(self.base_, cur.outer_cur)) {
                self.inner_.emplace(flux::read_at(self.base_, cur.outer_cur));
                cur.inner_cur.emplace(flux::first(*self.inner_));
                if (!flux::is_last(*self.inner_, *cur.inner_cur)) {
                    return;
                }
                flux::inc(self.base_, cur.outer_cur);
            }
        }

    public:
        using value_type = value_t<InnerSeq>;

        static constexpr auto first(self_t& self) -> cursor_type
        {
            cursor_type cur(flux::first(self.base_));
            satisfy(self, cur);
            return cur;
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.outer_cur);
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            flux::inc(*self.inner_, *cur.inner_cur);
            if (flux::is_last(*self.inner_, *cur.inner_cur)) {
                flux::inc(self.base_, cur.outer_cur);
                satisfy(self, cur);
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const& cur) -> decltype(auto)
        {
            FLUX_ASSERT(self.inner_.has_value());
            FLUX_ASSERT(cur.inner_cur.has_value());
            return flux::read_at(*self.inner_, *cur.inner_cur);
        }

        static constexpr auto last(self_t& self) -> cursor_type
            requires bounded_sequence<Base>
        {
            return cursor_type(flux::last(self.base_));
        }
    };
};

template <multipass_sequence Base>
    requires std::is_reference_v<element_t<Base>> &&
             multipass_sequence<element_t<Base>>
struct flatten_adaptor<Base> : inline_iter_base<flatten_adaptor<Base>> {
private:
    Base base_;

public:
    constexpr explicit flatten_adaptor(decays_to<Base> auto&& base)
        : base_(FLUX_FWD(base))
    {}

    struct flux_iter_traits : default_iter_traits {
    private:
        using InnerSeq = element_t<Base>;

        template <typename Self>
        static constexpr bool can_flatten = [] () consteval {
            if constexpr (std::is_const_v<Self>) {
                return multipass_sequence<Base const> &&
                       std::same_as<element_t<Base const>, std::remove_reference_t<InnerSeq> const&> &&
                       multipass_sequence<InnerSeq const>;
            } else {
                return true;
            }
        }();

        struct cursor_type {
            cursor_t<Base> outer_cur{};
            cursor_t<InnerSeq> inner_cur{};

            friend auto operator==(cursor_type const&, cursor_type const&) -> bool = default;
        };

        static constexpr auto satisfy(auto& self, cursor_type& cur) -> void
        {
            while (true) {
                if (flux::is_last(self.base_, cur.outer_cur)) {
                    cur.inner_cur = cursor_t<InnerSeq>{};
                    return;
                }
                auto&& inner = flux::read_at(self.base_, cur.outer_cur);
                cur.inner_cur = flux::first(inner);
                if (!flux::is_last(inner, cur.inner_cur)) {
                    return;
                }
                flux::inc(self.base_, cur.outer_cur);
            }
        }

    public:
        using value_type = value_t<InnerSeq>;

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto first(Self& self) -> cursor_type
        {
            cursor_type cur{.outer_cur = flux::first(self.base_) };
            satisfy(self, cur);
            return cur;
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto is_last(Self& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.outer_cur);
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto read_at(Self& self, cursor_type const& cur) -> decltype(auto)
        {
            return flux::read_at(flux::read_at(self.base_, cur.outer_cur),
                                 cur.inner_cur);
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto inc(Self& self, cursor_type& cur) -> void
        {
            auto&& inner = flux::read_at(self.base_, cur.outer_cur);
            flux::inc(inner, cur.inner_cur);
            if (flux::is_last(inner, cur.inner_cur)) {
                flux::inc(self.base_, cur.outer_cur);
                satisfy(self, cur);
            }
        }

        template <typename Self>
            requires can_flatten<Self>
        static constexpr auto for_each_while(Self& self, auto&& pred) -> cursor_type
        {
            auto inner_cur = cursor_t<InnerSeq>{};
            auto outer_cur = flux::for_each_while(self.base_, [&](auto&& inner_seq) {
                inner_cur = flux::for_each_while(inner_seq, pred);
                return flux::is_last(inner_seq, inner_cur);
            });
            return cursor_type{.outer_cur = std::move(outer_cur),
                               .inner_cur = std::move(inner_cur)};
        }

        template <typename Self>
            requires can_flatten<Self> && bounded_sequence<Base>
        static constexpr auto last(Self& self) -> cursor_type
        {
            return cursor_type{.outer_cur = flux::last(self.base_)};
        }

        template <typename Self>
            requires can_flatten<Self> &&
                     bidirectional_sequence<Base> &&
                     bidirectional_sequence<InnerSeq> &&
                     bounded_sequence<InnerSeq>
        static constexpr auto dec(Self& self, cursor_type& cur) -> void
        {
            if (flux::is_last(self.base_, cur.outer_cur)) {
                flux::dec(self.base_, cur.outer_cur);
                auto&& inner = flux::read_at(self.base_, cur.outer_cur);
                cur.inner_cur = flux::last(inner);
            }
            while (true) {
                auto&& inner = flux::read_at(self.base_, cur.outer_cur);
                if (cur.inner_cur != flux::first(inner)) {
                    flux::dec(inner, cur.inner_cur);
                    return;
                } else {
                    flux::dec(self.base_, cur.outer_cur);
                    auto&& next_inner = flux::read_at(self.base_, cur.outer_cur);
                    cur.inner_cur = flux::last(next_inner);
                }
            }
        }
    };

};

struct flatten_fn {
    template <sink_iterable It>
        requires iterable<element_t<It>>
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> iterable auto
    {
        return flatten_adaptor<std::decay_t<It>>(FLUX_FWD(it));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto flatten = detail::flatten_fn{};

template <typename Derived>
constexpr auto inline_iter_base<Derived>::flatten() &&
        requires iterable<element_t<Derived>>
{
    return flux::flatten(std::move(derived()));
}

} // namespace flux

#endif // FLUX_ADAPTOR_FLATTEN_HPP_INCLUDED
