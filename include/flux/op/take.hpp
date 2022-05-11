
#ifndef FLUX_OP_TAKE_HPP_INCLUDED
#define FLUX_OP_TAKE_HPP_INCLUDED

#include <flux/core.hpp>
#include <flux/op/for_each_while.hpp>
#include <flux/op/from.hpp>

namespace flux {

namespace detail {

template <typename Base>
struct take_adaptor : lens_base<take_adaptor<Base>>
{
private:
    Base base_;
    distance_t<Base> count_;

    template <bool IsConst>
    struct index_type {
    private:
        using base_t = std::conditional_t<IsConst, Base const, Base>;

    public:
        index_t<base_t> base_idx;
        distance_t<base_t> length;

        friend bool operator==(index_type const&, index_type const&) = default;
        friend auto operator<=>(index_type const& lhs, index_type const& rhs) = default;
    };

    friend struct sequence_iface<take_adaptor>;

public:
    constexpr take_adaptor(Base&& base, distance_t<Base> count)
        : base_(std::move(base)),
          count_(count)
    {}

    [[nodiscard]] constexpr auto base() const& -> Base const& { return base_; }
    [[nodiscard]] constexpr auto base() && -> Base&& { return std::move(base_); }
};

struct take_fn {

    template <sequence Seq>
    constexpr auto operator()(Seq&& seq, distance_t<Seq> count) const
    {
        if constexpr (random_access_sequence<Seq> && std::is_lvalue_reference_v<Seq>) {
            auto first = flux::first(seq);
            auto last = flux::next(seq, first, count);
            return flux::slice(FLUX_FWD(seq), std::move(first), std::move(last));
        } else {
            return take_adaptor(flux::from(FLUX_FWD(seq)), count);
        }
    }
};

} // namespace detail

template <typename Base>
struct sequence_iface<detail::take_adaptor<Base>> {

    template <typename Self>
    using index_t =
        typename std::remove_const_t<Self>::template index_type<std::is_const_v<Self>>;

    using value_type = value_t<Base>;
    using distance_type = distance_t<Base>;

    static constexpr bool disable_multipass = !multipass_sequence<Base>;

    template <typename Self>
    static constexpr auto first(Self& self)
    {
        return index_t<Self>{
            .base_idx = flux::first(self.base_),
            .length = self.count_
        };
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, index_t<Self> const& idx) -> bool
    {
        return idx.length <= 0 || flux::is_last(self.base_, idx.base_idx);
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, index_t<Self> const& idx)
        -> decltype(auto)
    {
        return flux::read_at(self.base_, idx.base_idx);
    }

    template <typename Self>
    static constexpr auto inc(Self& self, index_t<Self>& idx) -> index_t<Self>&
    {
        flux::inc(self.base_, idx.base_idx);
        --idx.length;
        return idx;
    }

    template <typename Self>
    static constexpr auto dec(Self& self, index_t<Self>& idx) -> index_t<Self>&
        requires bidirectional_sequence<Base>
    {
        flux::dec(self.base_, idx.base_idx);
        ++idx.length;
        return idx;
    }

    template <typename Self>
    static constexpr auto inc(Self& self, index_t<Self>& idx, distance_t<Base> offset)
        -> index_t<Self>&
        requires random_access_sequence<Base>
    {
        flux::inc(self.base_, idx.base_idx, offset);
        idx.length -= offset;
        return idx;
    }

    template <typename Self>
    static constexpr auto distance(Self& self, index_t<Self> const& from, index_t<Self> const& to)
        requires random_access_sequence<Base>
    {
        return std::min(flux::distance(self.base_, from.base_idx, to.base_idx),
                        to.length - from.length);
    }

    template <typename Self>
    static constexpr auto data(Self& self)
    {
        return flux::data(self.base_);
    }

    template <typename Self>
    static constexpr auto last(Self& self)
        requires random_access_sequence<Base> && sized_sequence<Base>
    {
        return index_t<Self>{
            .base_idx = flux::next(self.base_, flux::first(self.base_), size(self)),
            .length = 0
        };
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires sized_sequence<Base>
    {
        return std::min(flux::size(self.base_), self.count_);
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, index_t<Self> const& idx)
        -> decltype(auto)
    {
        return flux::move_at(self.base_, idx.base_idx);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        auto count = self.count_;
        auto idx = flux::for_each_while(self.base_, [&pred, &count](auto&& elem) {
            if (count > 0) {
                --count;
                return std::invoke(pred, FLUX_FWD(elem));
            } else {
                return false;
            }
        });
        return index_t<Self>{.base_idx = std::move(idx), .length = count};
    }
};

inline constexpr auto take = detail::take_fn{};

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::take(distance_t<D> count) &&
{
    return detail::take_adaptor<D>(std::move(derived()), count);
}

} // namespace flux

#endif // FLUX_OP_TAKE_HPP_INCLUDED