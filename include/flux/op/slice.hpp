
#ifndef FLUX_OP_SLICE_HPP_INCLUDED
#define FLUX_OP_SLICE_HPP_INCLUDED

#include <flux/core/lens_base.hpp>

namespace flux {

namespace detail {

template <index Idx, bool Bounded>
struct slice_data {
    Idx first;
    Idx last;
};

template <index Idx>
struct slice_data<Idx, false> {
    Idx first;
};

template <lens Base, bool Bounded>
    requires (!Bounded || regular_index<index_t<Base>>)
struct subsequence : lens_base<subsequence<Base, Bounded>>
{
private:
    Base* base_;
    FLUX_NO_UNIQUE_ADDRESS slice_data<index_t<Base>, Bounded> data_;

    friend struct sequence_iface<subsequence>;

public:
    constexpr subsequence(Base& base, index_t<Base>&& from, index_t<Base>&& to)
        requires Bounded
        : base_(std::addressof(base)),
          data_{std::move(from), std::move(to)}
    {}

    constexpr subsequence(Base& base, index_t<Base>&& from)
        requires (!Bounded)
        : base_(std::addressof(base)),
          data_{std::move(from)}
    {}

    constexpr auto base() const -> Base& { return *base_; };
};

template <sequence Seq>
subsequence(Seq&, index_t<Seq>, index_t<Seq>) -> subsequence<Seq, true>;

template <sequence Seq>
subsequence(Seq&, index_t<Seq>) -> subsequence<Seq, false>;

template <typename Seq>
concept has_overloaded_slice =
    requires (Seq& seq, index_t<Seq> idx) {
        { iface_t<Seq>::slice(seq, std::move(idx)) } -> sequence;
        { iface_t<Seq>::slice(seq, std::move(idx), std::move(idx)) } -> sequence;
    };

struct slice_fn {
    template <sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq> from, index_t<Seq> to) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return iface_t<Seq>::slice(seq, std::move(from), std::move(to));
        } else {
            return subsequence(seq, std::move(from), std::move(to));
        }
    }

    template <sequence Seq>
    constexpr auto operator()(Seq& seq, index_t<Seq> from, last_fn) const
        -> sequence auto
    {
        if constexpr (has_overloaded_slice<Seq>) {
            return iface_t<Seq>::slice(seq, std::move(from));
        } else {
            return subsequence(seq, std::move(from));
        }
    }
};

} // namespace detail

using detail::subsequence;

template <typename Base, bool Bounded>
struct sequence_iface<subsequence<Base, Bounded>>
    : detail::passthrough_iface_base<Base>
{
    using self_t = subsequence<Base, Bounded>;

    static constexpr auto first(self_t& self) -> index_t<Base>
    {
        if constexpr (std::copy_constructible<decltype(self.data_.first)>) {
            return self.data_.first;
        } else {
            return std::move(self.data_.first);
        }
    }

    static constexpr bool is_last(self_t& self, index_t<Base> const& idx) {
        if constexpr (Bounded) {
            return idx == self.data_.last;
        } else {
            return flux::is_last(*self.base_, idx);
        }
    }

    static constexpr auto last(self_t& self) -> index_t<Base>
        requires (Bounded || bounded_sequence<Base>)
    {
        if constexpr (Bounded) {
            return self.data_.last;
        } else {
            return flux::last(*self.base_);
        }
    }

    static constexpr auto data(self_t& self)
        requires contiguous_sequence<Base>
    {
        return flux::data(*self.base_) +
               flux::distance(*self.base_, flux::first(*self.base_), self.data_.first);
    }

    void size() = delete;
    void for_each_while() = delete;
};

inline constexpr auto slice = detail::slice_fn{};

#if 0
template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::slice(index_t<D> from, index_t<D> to) &
{
    return flux::slice(derived(), std::move(from), std::move(to));
}

template <typename Derived>
template <std::same_as<Derived> D>
constexpr auto lens_base<Derived>::slice_from(index_t<D> from) &
{
    return flux::slice(derived(), std::move(from));
}
#endif

} // namespace flux

#endif // namespace FLUX_OP_SLICE_HPP_INCLUDED
