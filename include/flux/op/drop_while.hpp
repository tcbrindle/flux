
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_DROP_WHILE_HPP_INCLUDED
#define FLUX_OP_DROP_WHILE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

#include <optional>

namespace flux {

namespace detail {

template <sequence Base, typename Pred>
struct drop_while_adaptor : lens_base<drop_while_adaptor<Base, Pred>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;
    std::optional<cursor_t<Base>> cached_first_{};

    friend struct sequence_iface<drop_while_adaptor>;
    friend struct passthrough_iface_base<Base>;

    constexpr auto base() & -> Base& { return base_; }

public:
    constexpr drop_while_adaptor(Base&& base, Pred&& pred)
        : base_(std::move(base)),
          pred_(std::move(pred))
    {}
};

struct drop_while_fn {
    template <adaptable_sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>>
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return drop_while_adaptor(FLUX_FWD(seq), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_iface<detail::drop_while_adaptor<Base, Pred>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = detail::drop_while_adaptor<Base, Pred>;

    static constexpr auto first(self_t& self)
    {
        if constexpr (std::copy_constructible<cursor_t<Base>>) {
            if (!self.cached_first_) {
                self.cached_first_ = flux::for_each_while(self.base_, self.pred_);
            }
            return *self.cached_first_;
        } else {
            return flux::for_each_while(self.base_, self.pred_);
        }
    }

    static constexpr auto data(self_t& self)
        requires contiguous_sequence<Base>
    {
        return flux::data(self.base_) +
                    flux::distance(self.base_, flux::first(self.base_), first(self));
    }

    void size(...) = delete;
    void for_each_while(...) = delete;
};

inline constexpr auto drop_while = detail::drop_while_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>>
constexpr auto lens_base<D>::drop_while(Pred pred) &&
{
    return flux::drop_while(std::move(derived()), std::move(pred));
};

} // namespace flux

#endif
