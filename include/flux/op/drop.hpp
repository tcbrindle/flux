
#ifndef FLUX_OP_DROP_HPP_INCLUDED
#define FLUX_OP_DROP_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/from.hpp>

#include <optional>

namespace flux {

namespace detail {

template <lens Base>
struct drop_adaptor : lens_base<drop_adaptor<Base>> {
private:
    FLUX_NO_UNIQUE_ADDRESS Base base_;
    distance_t<Base> count_;
    std::optional<cursor_t<Base>> cached_first_;

    friend struct sequence_iface<drop_adaptor>;

public:
    constexpr drop_adaptor(Base&& base, distance_t<Base> count)
        : base_(std::move(base)),
          count_(count)
    {}

    constexpr Base& base() & { return base_; }
    constexpr Base const& base() const& { return base_; }
};

struct drop_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq&& seq, distance_t<Seq> count) const
    {
        return drop_adaptor(flux::from(FLUX_FWD(seq)), count);
    }

};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::drop_adaptor<Base>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = detail::drop_adaptor<Base>;

    using value_type = value_t<Base>;
    using distance_type = distance_t<Base>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;

    static constexpr auto first(self_t& self)
    {
        if constexpr (std::copy_constructible<cursor_t<Base>>) {
            if (!self.cached_first_) {
                self.cached_first_ = flux::next(self.base_, flux::first(self.base()), self.count_);
            }

            return *self.cached_first_;
        } else {
            return flux::next(self.base_, flux::first(self.base()), self.count_);
        }
    }

    static constexpr auto size(self_t& self)
        requires sized_sequence<Base>
    {
        return flux::size(self.base()) - self.count_;
    }

    static constexpr auto data(self_t& self)
        requires contiguous_sequence<Base>
    {
        return flux::data(self.base()) + self.count_;
    }

    void for_each_while(...) = delete;
};

inline constexpr auto drop = detail::drop_fn{};

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::drop(distance_t<D> count) &&
{
    return detail::drop_adaptor<Derived>(std::move(derived()), count);
}

} // namespace flux

#endif
