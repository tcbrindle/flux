
#pragma once

#include <flux/core.hpp>
#include <flux/op/for_each_while.hpp>
#include <flux/op/from.hpp>

#include <optional>

namespace flux {

namespace detail {

template <lens Base, typename Pred>
    requires std::predicate<Pred&, element_t<Base>&>
class filter_adaptor : public lens_base<filter_adaptor<Base, Pred>>
{
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    FLUX_NO_UNIQUE_ADDRESS Pred pred_;
    std::optional<index_t<Base>> cached_first_{};

public:
    constexpr filter_adaptor(Base&& base, Pred&& pred)
        : base_(std::move(base)),
          pred_(std::move(pred))
    {}

    [[nodiscard]]
    constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]]
    constexpr auto base() && -> Base { return std::move(base_); }

    friend struct sequence_iface<filter_adaptor>;
};


struct filter_fn {
    template <sequence Seq, typename Pred>
        requires std::predicate<Pred&, element_t<Seq>&>
    constexpr auto operator()(Seq&& seq, Pred pred) const
    {
        return filter_adaptor(flux::from(FLUX_FWD(seq)), std::move(pred));
    }
};

} // namespace detail

template <typename Base, typename Pred>
struct sequence_iface<detail::filter_adaptor<Base, Pred>>
{
    using self_t = detail::filter_adaptor<Base, Pred>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;

    static constexpr auto first(self_t& self) -> index_t<Base>
    {
        // If the index_type is not copy-constructible, we can't cache it
        // and hand out copies. But this only applies to single-pass sequences
        // where we probably can't call first() more than once anyway
        if constexpr (!std::copy_constructible<index_t<Base>>) {
            return flux::for_each_while(self.base_, [&](auto&& elem) {
                return !std::invoke(self.pred_, elem);
            });
        } else {
            if (!self.cached_first_) {
                self.cached_first_ =
                    flux::for_each_while(self.base_, [&](auto&& elem) {
                        return !std::invoke(self.pred_, elem);
                    });
            }

            return *self.cached_first_;
        }
    }

    static constexpr auto is_last(self_t& self, index_t<Base> const& idx) -> bool
    {
        return flux::is_last(self.base_, idx);
    }

    static constexpr auto read_at(self_t& self, index_t<Base> const& idx)
        -> element_t<Base>
    {
        return flux::read_at(self.base_, idx);
    }

    static constexpr auto inc(self_t& self, index_t<Base>& idx) -> index_t<Base>&
    {
        // base_[{next(base_, idx), _}].for_each_while(!pred)?
        while (!flux::is_last(self.base_, flux::inc(self.base_, idx))) {
            if (std::invoke(self.pred_, flux::read_at(self.base_, idx))) {
                break;
            }
        }

        return idx;
    }

    static constexpr auto dec(self_t& self, index_t<Base>& idx) -> index_t<Base>&
        requires bidirectional_sequence<Base>
    {
        do {
            flux::dec(self.base_, idx);
        } while(!std::invoke(self.pred_, flux::read_at(self.base_, idx)));

        return idx;
    }

    static constexpr auto last(self_t& self) -> index_t<Base>
        requires bounded_sequence<Base>
    {
        return flux::last(self.base_);
    }

    static constexpr auto for_each_while(self_t& self, auto&& func) -> index_t<Base>
    {
        return flux::for_each_while(self.base_, [&](auto&& elem) {
            if (std::invoke(self.pred_, elem)) {
                return std::invoke(func, FLUX_FWD(elem));
            } else {
                return true;
            }
        });
    }
};

inline constexpr auto filter = detail::filter_fn{};

template <typename D>
template <typename Pred>
    requires std::predicate<Pred&, element_t<D>&>
constexpr auto lens_base<D>::filter(Pred pred) &&
{
    return detail::filter_adaptor<D, Pred>(std::move(derived()), std::move(pred));
}


}
