
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ADAPTOR_CHUNK_HPP_INCLUDED
#define FLUX_ADAPTOR_CHUNK_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/adaptor/stride.hpp>
#include <flux/adaptor/take.hpp>

namespace flux {

namespace detail {

template <typename BaseCtx>
struct chunk_iteration_context : immovable {
    BaseCtx base_ctx;

    using OptElem = decltype(next_element(base_ctx));

    int_t chunk_sz;
    OptElem elem = nullopt;
    int_t remaining = 0;
    bool base_done = false;

    struct inner_iterable {
        chunk_iteration_context* outer_ctx;

        struct inner_context_type : immovable {
            chunk_iteration_context* outer_ctx;

            using element_type = context_element_t<BaseCtx>;

            constexpr auto run_while(auto&& pred) -> iteration_result
            {
                while (true) {
                    if (outer_ctx->remaining == 0) {
                        return iteration_result::complete;
                    }

                    if (!outer_ctx->elem) {
                        if (!outer_ctx->read_next()) {
                            return iteration_result::complete;
                        }
                    }

                    --outer_ctx->remaining;

                    auto r = pred(*std::move(outer_ctx->elem));
                    outer_ctx->elem.reset();
                    if (!r) {
                        return iteration_result::incomplete;
                    }
                }
            }
        };

        constexpr auto iterate() const -> inner_context_type
        {
            return inner_context_type{.outer_ctx = this->outer_ctx};
        }
    };

    constexpr bool read_next()
    {
        elem = flux::next_element(base_ctx);
        return elem.has_value();
    }

    using element_type = inner_iterable;

    constexpr auto run_while(auto&& outer_pred) -> iteration_result
    {
        while (true) {
            while (remaining-- > 0) {
                if (!read_next()) {
                    return iteration_result::complete;
                }
            }

            if (!read_next()) {
                return iteration_result::complete;
            }

            remaining = chunk_sz;

            if (!outer_pred(inner_iterable{.outer_ctx = this})) {
                return iteration_result::incomplete;
            }
        }
    }
};

template <typename Base>
struct chunk_adaptor : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    int_t chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, int_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {
    }

    constexpr auto iterate()
    {
        return detail::chunk_iteration_context<iteration_context_t<Base>>{
            .base_ctx = flux::iterate(base_), .chunk_sz = chunk_sz_};
    }

    constexpr auto iterate() const
        requires iterable<Base const>
    {
        return detail::chunk_iteration_context<iteration_context_t<Base const>>{
            .base_ctx = flux::iterate(base_), .chunk_sz = chunk_sz_};
    }

    constexpr auto size() -> int_t
        requires sized_iterable<Base>
    {
        auto sz = flux::iterable_size(base_);
        return sz / chunk_sz_ + (sz % chunk_sz_ == 0 ? 0 : 1);
    }

    constexpr auto size() const -> int_t
        requires sized_iterable<Base const>
    {
        auto sz = flux::iterable_size(base_);
        return sz / chunk_sz_ + (sz % chunk_sz_ == 0 ? 0 : 1);
    }
};

template <sequence Base>
struct chunk_adaptor<Base> : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    int_t chunk_sz_;
    optional<cursor_t<Base>> cur_ = nullopt;
    int_t rem_ = chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, int_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {}

    chunk_adaptor(chunk_adaptor&&) = default;
    chunk_adaptor& operator=(chunk_adaptor&&) = default;

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct outer_cursor {
            outer_cursor(outer_cursor&&) = default;
            outer_cursor& operator=(outer_cursor&&) = default;

            friend struct flux_sequence_traits;

        private:
            explicit outer_cursor() = default;
        };

        using self_t = chunk_adaptor;

    public:
        struct value_type : inline_sequence_base<value_type> {
        private:
            chunk_adaptor* parent_;
            constexpr explicit value_type(chunk_adaptor& parent)
                : parent_(std::addressof(parent))
            {}

            friend struct self_t::flux_sequence_traits;

        public:
            value_type(value_type&&) = default;
            value_type& operator=(value_type&&) = default;

            struct flux_sequence_traits : default_sequence_traits {
            private:
                struct inner_cursor {
                    inner_cursor(inner_cursor&&) = default;
                    inner_cursor& operator=(inner_cursor&&) = default;

                    friend struct value_type::flux_sequence_traits;

                private:
                    explicit inner_cursor() = default;
                };

            public:
                static constexpr auto first(value_type&) -> inner_cursor
                {
                    return inner_cursor{};
                }

                static constexpr auto is_last(value_type& self, inner_cursor const&) -> bool
                {
                    return self.parent_->rem_ == 0;
                }

                static constexpr auto inc(value_type& self, inner_cursor&) -> void
                {
                    flux::inc(self.parent_->base_, *self.parent_->cur_);
                    if (flux::is_last(self.parent_->base_, *self.parent_->cur_)) {
                        self.parent_->rem_ = 0;
                    } else {
                        --self.parent_->rem_;
                    }
                }

                static constexpr auto read_at(value_type& self, inner_cursor const&)
                    -> element_t<Base>
                {
                    return flux::read_at(self.parent_->base_, *self.parent_->cur_);
                }
            };
        };

        static constexpr auto first(self_t& self) -> outer_cursor
        {
            self.cur_ = optional<cursor_t<Base>>(flux::first(self.base_));
            self.rem_ = self.chunk_sz_;
            return outer_cursor{};
        }

        static constexpr auto is_last(self_t& self, outer_cursor const&) -> bool
        {
            return self.rem_ != 0 && flux::is_last(self.base_, *self.cur_);
        }

        static constexpr auto inc(self_t& self, outer_cursor&) -> void
        {
            advance(self.base_, *self.cur_, self.rem_);
            self.rem_ = self.chunk_sz_;
        }

        static constexpr auto read_at(self_t& self, outer_cursor const&) -> value_type
        {
            return value_type(self);
        }

        static constexpr auto size(self_t& self) -> int_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.chunk_sz_ + (s % self.chunk_sz_ == 0 ? 0 : 1);
        }
    };
};

template <multipass_sequence Base>
struct chunk_adaptor<Base> : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    int_t chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, int_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {}

    struct flux_sequence_traits : default_sequence_traits {
        static inline constexpr bool is_infinite = infinite_sequence<Base>;

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

        static constexpr auto size(auto& self) -> int_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.chunk_sz_ + (s % self.chunk_sz_ == 0 ? 0 : 1);
        }
    };
};

template <bidirectional_sequence Base>
struct chunk_adaptor<Base> : inline_sequence_base<chunk_adaptor<Base>> {
private:
    Base base_;
    int_t chunk_sz_;

public:
    constexpr chunk_adaptor(decays_to<Base> auto&& base, int_t chunk_sz)
        : base_(FLUX_FWD(base)),
          chunk_sz_(chunk_sz)
    {}

    struct flux_sequence_traits : default_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> cur{};
            int_t missing = 0;

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

        static inline constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_type
        {
            return cursor_type{
                .cur = flux::first(self.base_),
                .missing = 0
            };
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur) -> bool
        {
            return flux::is_last(self.base_, cur.cur);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            cur.missing = advance(self.base_, cur.cur, self.chunk_sz_);
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
            requires sequence<decltype((self.base_))>
        {
            if constexpr (random_access_sequence<Base>) {
                auto end_cur = cur.cur;
                advance(self.base_, end_cur, self.chunk_sz_);
                return flux::slice(self.base_, cur.cur, std::move(end_cur));
            } else {
                return flux::take(flux::slice(self.base_, cur.cur, flux::last), self.chunk_sz_);
            }
        }

        static constexpr auto dec(auto& self, cursor_type& cur)
        {
            advance(self.base_, cur.cur, cur.missing - self.chunk_sz_);
            cur.missing = 0;
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<Base> && sized_sequence<Base>
        {
            int_t missing
                = (self.chunk_sz_ - flux::size(self.base_) % self.chunk_sz_) % self.chunk_sz_;
            return cursor_type{
                .cur = flux::last(self.base_),
                .missing = missing
            };
        }

        static constexpr auto size(auto& self) -> int_t
            requires sized_sequence<Base>
        {
            auto s = flux::size(self.base_);
            return s/self.chunk_sz_ + (s % self.chunk_sz_ == 0 ? 0 : 1);
        }

        static constexpr auto distance(auto& self, cursor_type const& from, cursor_type const& to)
            -> int_t
            requires random_access_sequence<Base>
        {
            return (flux::distance(self.base_, from.cur, to.cur) - from.missing + to.missing)/self.chunk_sz_;
        }

        static constexpr auto inc(auto& self, cursor_type& cur, int_t offset) -> void
            requires random_access_sequence<Base>
        {
            if (offset > 0) {
                cur.missing = advance(self.base_, cur.cur, num::mul(offset, self.chunk_sz_)) % self.chunk_sz_;
            } else if (offset < 0) {
                advance(self.base_, cur.cur, num::add(num::mul(offset, self.chunk_sz_), cur.missing));
                cur.missing = 0;
            }
        }
    };
};

struct chunk_fn {
    template <adaptable_iterable It>
    [[nodiscard]]
    constexpr auto operator()(It&& it, num::integral auto chunk_sz) const //-> iterable auto
    {
        int_t chunk_sz_ = num::checked_cast<int_t>(chunk_sz);
        FLUX_ASSERT(chunk_sz_ > 0);
        return chunk_adaptor<std::decay_t<It>>(FLUX_FWD(it), chunk_sz_);
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto chunk = detail::chunk_fn{};

template <typename D>
constexpr auto inline_sequence_base<D>::chunk(num::integral auto chunk_sz) &&
{
    return flux::chunk(std::move(derived()), chunk_sz);
}

} // namespace flux

#endif // FLUX_ADAPTOR_CHUNK_HPP_INCLUDED
