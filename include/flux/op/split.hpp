
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_SPLIT_HPP_INCLUDED
#define FLUX_OP_SPLIT_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/find.hpp>
#include <flux/op/from.hpp>
#include <flux/op/search.hpp>
#include <flux/op/slice.hpp>
#include <flux/source/single.hpp>

namespace flux {

namespace detail {

template <typename Splitter, typename Seq>
concept splitter_for = requires(Splitter& splitter, Seq& seq, cursor_t<Seq> const& cur) {
    { splitter(flux::slice(seq, cur, flux::last)) } -> std::same_as<bounds_t<Seq>>;
};

template <multipass_sequence Base, splitter_for<Base> Splitter>
struct split_adaptor : inline_sequence_base<split_adaptor<Base, Splitter>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Splitter splitter_;

public:
    constexpr split_adaptor(decays_to<Base> auto&& base, decays_to<Splitter> auto&& splitter)
        : base_(FLUX_FWD(base)),
          splitter_(FLUX_FWD(splitter))
    {}

    struct flux_sequence_traits {
    private:
        struct cursor_type {
            cursor_t<Base> cur{};
            bounds_t<Base> next{};
            bool trailing_empty = false;

            friend constexpr bool operator==(cursor_type const& lhs, cursor_type const& rhs)
            {
                return lhs.cur == rhs.cur && lhs.trailing_empty == rhs.trailing_empty;
            }
        };

    public:
        static constexpr bool is_infinite = infinite_sequence<Base>;

        static constexpr auto first(auto& self) -> cursor_type
            requires sequence<decltype((self.base_))> &&
                     splitter_for<decltype((self.splitter_)), decltype((self.base_))>
        {
            auto fst = flux::first(self.base_);
            auto bounds = self.splitter_(flux::slice(self.base_, fst, flux::last));
            return cursor_type{.cur = std::move(fst),
                               .next = std::move(bounds)};
        }

        static constexpr auto is_last(auto& self, cursor_type const& cur)
            -> bool
        {
            return flux::is_last(self.base_, cur.cur) && !cur.trailing_empty;
        }

        static constexpr auto read_at(auto& self, cursor_type const& cur)
        {
            return flux::slice(self.base_, cur.cur, cur.next.from);
        }

        static constexpr auto inc(auto& self, cursor_type& cur) -> void
        {
            cur.cur = cur.next.from;
            if (!flux::is_last(self.base_, cur.cur)) {
                cur.cur = cur.next.to;
                if (flux::is_last(self.base_, cur.cur)) {
                    cur.trailing_empty = true;
                    cur.next = {cur.cur, cur.cur};
                } else {
                    cur.next = self.splitter_(flux::slice(self.base_, cur.cur, flux::last));
                }
            } else {
                cur.trailing_empty = false;
            }
        }

        static constexpr auto last(auto& self) -> cursor_type
            requires bounded_sequence<decltype(self.base_)>
        {
            return cursor_type{.cur = flux::last(self.base_)};
        }
    };
};

template <multipass_sequence Pattern>
struct pattern_splitter {
private:
    FLUX_NO_UNIQUE_ADDRESS Pattern pattern_;

public:
    constexpr explicit pattern_splitter(decays_to<Pattern> auto&& pattern)
        : pattern_(FLUX_FWD(pattern))
    {}

    template <multipass_sequence Seq>
        requires std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    constexpr auto operator()(Seq&& seq) -> bounds_t<Seq>
    {
        return flux::search(seq, pattern_);
    }

    template <multipass_sequence Seq>
        requires multipass_sequence<Pattern const> &&
                 std::equality_comparable_with<element_t<Seq>, element_t<Pattern const>>
    constexpr auto operator()(Seq&& seq) const -> bounds_t<Seq>
    {
        return flux::search(seq, pattern_);
    }
};

template <typename Delim>
struct delim_splitter {
private:
    FLUX_NO_UNIQUE_ADDRESS Delim delim_;

public:
    constexpr explicit delim_splitter(decays_to<Delim> auto&& delim)
        : delim_(FLUX_FWD(delim))
    {}

    template <multipass_sequence Seq>
        requires std::equality_comparable_with<element_t<Seq>, Delim const&>
    constexpr auto operator()(Seq&& seq) const -> bounds_t<Seq>
    {
        auto nxt = flux::find(seq, delim_);
        if (!flux::is_last(seq, nxt)) {
            return bounds{nxt, flux::next(seq, nxt)};
        } else {
            return bounds{nxt, nxt};
        }
    }
};

template <typename Pred>
struct predicate_splitter {
private:
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;

public:
    constexpr explicit predicate_splitter(decays_to<Pred> auto&& pred)
        : pred_(FLUX_FWD(pred))
    {}

    template <multipass_sequence Seq>
        requires std::predicate<Pred const&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq) const -> bounds_t<Seq>
    {
        auto nxt = flux::find_if(seq, pred_);
        if (!flux::is_last(seq, nxt)) {
            return bounds{nxt, flux::next(seq, nxt)};
        } else {
            return bounds{nxt, nxt};
        }
    }
};

struct split_fn {
    template <adaptable_sequence Seq, adaptable_sequence Pattern>
        requires multipass_sequence<Seq> &&
                 multipass_sequence<Pattern> &&
                 std::equality_comparable_with<element_t<Seq>, element_t<Pattern>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pattern&& pattern) const
    {
        using splitter_t = pattern_splitter<std::decay_t<Pattern>>;
        return split_adaptor<std::decay_t<Seq>, splitter_t>(
            FLUX_FWD(seq), splitter_t(FLUX_FWD(pattern)));
    }

    template <adaptable_sequence Seq, typename Delim>
        requires multipass_sequence<Seq> &&
                 std::equality_comparable_with<element_t<Seq>, Delim const&>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Delim&& delim) const
    {
        using splitter_t = delim_splitter<std::decay_t<Delim>>;
        return split_adaptor<std::decay_t<Seq>, splitter_t>(
            FLUX_FWD(seq), splitter_t(FLUX_FWD(delim)));
    }

    template <adaptable_sequence Seq, typename Pred>
        requires multipass_sequence<Seq> &&
                 std::predicate<Pred const&, element_t<Seq>>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        using splitter_t = predicate_splitter<Pred>;
        return split_adaptor<std::decay_t<Seq>, splitter_t>(
            FLUX_FWD(seq), splitter_t(std::move(pred)));
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto split = detail::split_fn{};

template <typename Derived>
template <typename Pattern>
    requires multipass_sequence<Derived> &&
             multipass_sequence<Pattern> &&
             std::equality_comparable_with<element_t<Derived>, element_t<Pattern>>
constexpr auto inline_sequence_base<Derived>::split(Pattern&& pattern) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(pattern));
}

template <typename Derived>
template <typename Delim>
    requires multipass_sequence<Derived> &&
             std::equality_comparable_with<element_t<Derived>, Delim const&>
constexpr auto inline_sequence_base<Derived>::split(Delim&& delim) &&
{
    return flux::split(std::move(derived()), FLUX_FWD(delim));
}

template <typename Derived>
template <typename Pred>
    requires multipass_sequence<Derived> &&
             std::predicate<Pred const&, element_t<Derived>>
constexpr auto inline_sequence_base<Derived>::split(Pred pred) &&
{
    return flux::split(std::move(derived()), std::move(pred));
}


} // namespace flux

#endif
