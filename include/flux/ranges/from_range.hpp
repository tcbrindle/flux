#ifndef FLUX_RANGES_FROM_RANGE_HPP_INCLUDED
#define FLUX_RANGES_FROM_RANGE_HPP_INCLUDED

#include <flux/core/concepts.hpp>

#include <ranges>

namespace flux {

template <typename R>
    requires (!detail::derived_from_lens_base<R> && std::ranges::input_range<R>)
struct sequence_iface<R> {

    using value_type = std::ranges::range_value_t<R>;
    using distance_type = std::ranges::range_difference_t<R>;

    static constexpr bool disable_multipass = !std::ranges::forward_range<R>;

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto first(Self& self)
    {
        return std::ranges::begin(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr bool is_last(Self& self, index_t<Self> const& idx)
    {
        return idx == std::ranges::end(self);
    }

    template <decays_to<R> Self>
    static constexpr auto read_at(Self&, index_t<Self> const& idx)
        -> decltype(auto)
    {
        return *idx;
    }

    template <decays_to<R> Self>
    static constexpr auto inc(Self&, index_t<Self>& idx)
        -> index_t<Self>&
    {
        return ++idx;
    }

    template <decays_to<R> Self>
        requires std::bidirectional_iterator<index_t<Self>>
    static constexpr auto dec(Self&, index_t<Self>& idx)
        -> index_t<Self>&
    {
        return --idx;
    }

    template <decays_to<R> Self>
        requires std::random_access_iterator<index_t<Self>>
    static constexpr auto inc(Self&, index_t<Self>& idx, distance_t<Self> offset)
        -> index_t<Self>&
    {
        return idx += offset;
    }

    template <decays_to<R> Self>
        requires std::random_access_iterator<index_t<Self>>
    static constexpr auto distance(Self&, index_t<Self> const& from, index_t<Self> const& to)
    {
        return to - from;
    }

    template <decays_to<R> Self>
        requires std::ranges::contiguous_range<Self>
    static constexpr auto data(Self& self)
    {
        return std::ranges::data(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::common_range<Self>
    static constexpr auto last(Self& self) -> index_t<Self>
    {
        return std::ranges::end(self);
    }

    template <decays_to<R> Self>
        requires std::ranges::sized_range<Self>
    static constexpr auto size(Self& self)
    {
        return std::ranges::size(self);
    }

    template <decays_to<R> Self>
    static constexpr auto move_at(Self&, index_t<Self> const& idx)
        -> decltype(auto)
    {
        return std::ranges::iter_move(idx);
    }

    template <decays_to<R> Self>
    static constexpr auto slice(Self& self, index_t<Self> from)
    {
        return std::ranges::subrange(std::move(from), std::ranges::end(self));
    }

    template <decays_to<R> Self>
    static constexpr auto slice(Self&, index_t<Self> from, index_t<Self> to)
    {
        return std::ranges::subrange(std::move(from), std::move(to));
    }

    template <decays_to<R> Self>
        requires std::ranges::range<Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        auto iter = std::ranges::begin(self);
        auto const last = std::ranges::end(self);

        for ( ; iter != last; ++iter) {
            if (!std::invoke(pred, *iter)) {
                break;
            }
        }

        return iter;
    }
};

} // namespace flux

#endif // FLUX_RANGE_FROM_RANGE_HPP_INCLUDED
