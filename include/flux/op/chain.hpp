
#ifndef FLUX_OP_CHAIN_HPP_INCLUDED
#define FLUX_OP_CHAIN_HPP_INCLUDED

#include <flux/core.hpp>

#include <cassert>
#include <tuple>
#include <variant>

namespace flux {

namespace detail {

template <lens... Bases>
struct chain_adaptor : lens_base<chain_adaptor<Bases...>> {
private:
    std::tuple<Bases...> bases_;

    friend struct sequence_iface<chain_adaptor>;

public:
    explicit constexpr chain_adaptor(Bases&&... bases)
        : bases_(std::move(bases)...)
    {}
};

template <typename... Ts>
concept all_have_common_ref =
    requires { typename std::common_reference_t<Ts...>; } &&
    (std::convertible_to<Ts, std::common_reference_t<Ts...>> && ...);

template <typename... Seqs>
concept chainable =
    all_have_common_ref<element_t<Seqs>...> &&
    all_have_common_ref<rvalue_element_t<Seqs>...> &&
    requires { typename std::common_type_t<value_t<Seqs>...>; };

struct chain_fn {
    template <sequence... Seqs>
        requires (sizeof...(Seqs) >= 1) &&
                 chainable<Seqs...>
    constexpr auto operator()(Seqs&&... seqs) const
    {
        if constexpr (sizeof...(Seqs) == 1) {
            return flux::from(FLUX_FWD(seqs)...);
        } else {
            return chain_adaptor(flux::from(FLUX_FWD(seqs))...);
        }
    }
};

} // namespace detail

template <typename... Bases>
struct sequence_iface<detail::chain_adaptor<Bases...>> {

    using value_type = std::common_type_t<value_t<Bases>...>;
    using distance_type = std::common_type_t<distance_t<Bases>...>;

    static constexpr bool disable_multipass = !(multipass_sequence<Bases> && ...);
    static constexpr bool is_infinite = (infinite_sequence<Bases> || ...);

private:
    static constexpr std::size_t End = sizeof...(Bases) - 1;

    template <typename From, typename To>
    using const_like_t = std::conditional_t<std::is_const_v<From>, To const, To>;

    template <typename Self>
    using index_type = std::variant<index_t<const_like_t<Self, Bases>>...>;

    template <std::size_t N, typename Self, std::size_t... I>
    static constexpr auto first_impl(Self& self, std::index_sequence<I...> iseq)
        -> index_type<Self>
    {
        auto& base = std::get<N>(self.bases_);
        auto idx = flux::first(base);

        if constexpr (N < End) {
            if (!flux::is_last(base, idx)) {
                return index_type<Self>(std::in_place_index<N>, std::move(idx));
            } else {
                return first_impl<N+1>(self, iseq);
            }
        } else {
            return index_type<Self>(std::in_place_index<N>, std::move(idx));
        }
    }

    template <std::size_t N, typename Self, std::size_t... Idx>
    static constexpr auto inc_impl(Self& self, index_type<Self>& idx,
                                   std::index_sequence<Idx...> iseq)
            -> index_type<Self>&
    {
        if constexpr (N < End) {
            if (idx.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_idx = std::get<N>(idx);
                flux::inc(base, base_idx);
                if (flux::is_last(base, base_idx)) {
                    idx = first_impl<N + 1>(self, iseq);
                }
                return idx;
            } else {
                return inc_impl<N+1>(self, idx, iseq);
            }
        } else {
            flux::inc(std::get<N>(self.bases_), std::get<N>(idx));
            return idx;
        }
    }

    template <std::size_t N, typename Self, std::size_t... Idx>
    static constexpr auto dec_impl(Self& self, index_type<Self>& idx,
                                   std::index_sequence<Idx...> iseq)
        -> index_type<Self>&
    {
        if constexpr (N > 0) {
            if (idx.index() == N) {
                auto& base = std::get<N>(self.bases_);
                auto& base_idx = std::get<N>(idx);

                if (base_idx == flux::first(base)) {
                    idx = index_type<Self>(std::in_place_index<N-1>,
                                           flux::last(std::get<N-1>(self.bases_)));
                    return dec_impl<N-1>(self, idx, iseq);
                } else {
                    flux::dec(base, base_idx);
                    return idx;
                }
            } else {
                return dec_impl<N-1>(self, idx, iseq);
            }
        } else {
            flux::dec(std::get<0>(self.bases_), std::get<0>(idx));
            return idx;
        }
    }

    template <std::size_t N, typename Self, std::size_t... Idx>
    static constexpr auto read_impl(Self& self,
                                    index_type<Self> const& idx,
                                    std::index_sequence<Idx...> iseq)
        -> std::common_reference_t<element_t<const_like_t<Self, Bases>>...>
    {
        if constexpr (N < End) {
            if (idx.index() == N) {
                return flux::read_at(std::get<N>(self.bases_), std::get<N>(idx));
            } else {
                return read_impl<N+1>(self, idx, iseq);
            }
        } else {
            return flux::read_at(std::get<N>(self.bases_), std::get<N>(idx));
        }
    }

    template <std::size_t N, typename Self, std::size_t... Idx>
    static constexpr auto move_impl(Self& self,
                                    index_type<Self> const& idx,
                                    std::index_sequence<Idx...> iseq)
        -> std::common_reference_t<rvalue_element_t<const_like_t<Self, Bases>>...>
    {
        if constexpr (N < End) {
            if (idx.index() == N) {
                return flux::move_at(std::get<N>(self.bases_), std::get<N>(idx));
            } else {
                return move_impl<N+1>(self, idx, iseq);
            }
        } else {
            return flux::move_at(std::get<N>(self.bases_), std::get<N>(idx));
        }
    }


    template <std::size_t N, typename Self, std::size_t... Idx>
    static constexpr auto for_each_while_impl(Self& self, auto& pred,
                                              std::index_sequence<Idx...> iseq)
        -> index_type<Self>
    {
        if constexpr (N < End) {
            auto& base = std::get<N>(self.bases_);
            auto base_idx = flux::for_each_while(base, pred);
            if (!flux::is_last(base, base_idx)) {
                return index_type<Self>(std::in_place_index<N>, std::move(base_idx));
            } else {
                return for_each_while_impl<N+1>(self, pred, iseq);
            }
        } else {
            return index_type<Self>(std::in_place_index<N>,
                                    flux::for_each_while(std::get<N>(self.bases_), pred));
        }
    }

    template <std::size_t N, typename Self, std::size_t... Idx>
    static constexpr auto distance_impl(Self& self,
                                        index_type<Self> const& from,
                                        index_type<Self> const& to,
                                        std::index_sequence<Idx...> iseq)
    {
        if constexpr (N < End) {
            if (N < from.index()) {
                return distance_impl<N+1>(self, from, to, iseq);
            }

            assert(N == from.index());
            if (N == to.index()) {
                return flux::distance(std::get<N>(self.bases_),
                                      std::get<N>(from), std::get<N>(to));
            } else {
                auto dist_to_end = flux::distance(std::get<N>(self.bases_),
                                                  std::get<N>(from),
                                                  flux::last(std::get<N>(self.bases_)));
                auto remaining = distance_impl<N+1>(self, first_impl<N+1>(self, iseq), to, iseq);
                return dist_to_end + remaining;
            }
        } else {
            assert(N == from.index() && N == to.index());
            return flux::distance(std::get<N>(self.bases_), std::get<N>(from), std::get<N>(to));
        }
    }

    template <std::size_t N, typename Self, std::size_t... Idx>
    static constexpr auto inc_ra_impl(Self& self, index_type<Self>& idx,
                                      distance_type offset, std::index_sequence<Idx...> iseq)
        -> index_type<Self>&
    {
        if constexpr (N < End) {
            if (N < idx.index()) {
                return inc_ra_impl<N+1>(self, idx, offset, iseq);
            }

            assert(idx.index() == N);
            auto& base = std::get<N>(self.bases_);
            auto& base_idx = std::get<N>(idx);
            auto dist = flux::distance(base, base_idx, flux::last(base));
            if (offset < dist) {
                flux::inc(base, base_idx, offset);
                return idx;
            } else {
                idx = first_impl<N+1>(self, iseq);
                offset -= dist;
                return inc_ra_impl<N+1>(self, idx, offset, iseq);
            }
        } else {
            assert(idx.index() == N);
            flux::inc(std::get<N>(self.bases_), std::get<N>(idx), offset);
            return idx;
        }
    }

public:
    template <typename Self>
    static constexpr auto first(Self& self) -> index_type<Self>
    {
        return first_impl<0>(self, std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto is_last(Self& self, index_type<Self> const& idx)
    {
        return idx.index() == End &&
               flux::is_last(std::get<End>(self.bases_), std::get<End>(idx));
    }

    template <typename Self>
    static constexpr auto read_at(Self& self, index_type<Self> const& idx)
        -> std::common_reference_t<element_t<const_like_t<Self, Bases>>...>
    {
        return read_impl<0>(self, idx, std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto move_at(Self& self, index_type<Self> const& idx)
        -> std::common_reference_t<rvalue_element_t<const_like_t<Self, Bases>>...>
    {
        return move_impl<0>(self, idx, std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto inc(Self& self, index_type<Self>& idx)
        -> index_type<Self>&
    {
        return inc_impl<0>(self, idx, std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto dec(Self& self, index_type<Self>& idx)
        -> index_type<Self>&
        requires (bidirectional_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self,Bases>> &&...)
    {
        return dec_impl<sizeof...(Bases) - 1>(self, idx, std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto last(Self& self) -> index_type<Self>
        requires bounded_sequence<decltype(std::get<End>(self.bases_))>
    {
        constexpr auto Last = sizeof...(Bases) - 1;
        return index_type<Self>(std::in_place_index<Last>, flux::last(std::get<Last>(self.bases_)));
    }

    template <typename Self>
    static constexpr auto size(Self& self)
        requires (sized_sequence<const_like_t<Self, Bases>> && ...)
    {
        return std::apply([](auto&... bases) { return (flux::size(bases) + ...); },
                          self.bases_);
    }

    template <typename Self>
    static constexpr auto for_each_while(Self& self, auto&& pred)
    {
        return for_each_while_impl<0>(self, pred, std::index_sequence_for<Bases...>{});
    }

    template <typename Self>
    static constexpr auto distance(Self& self,
                                   index_type<Self> const& from,
                                   index_type<Self> const& to)
        -> distance_type
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    {
        if (from.index() <= to.index()) {
            return distance_impl<0>(self, from, to, std::index_sequence_for<Bases...>{});
        } else {
            return -distance_impl<0>(self, to, from, std::index_sequence_for<Bases...>{});
        }
    }

    template <typename Self>
    static constexpr auto inc(Self& self, index_type<Self>& idx, distance_type offset)
        -> index_type<Self>&
        requires (random_access_sequence<const_like_t<Self, Bases>> && ...) &&
                 (bounded_sequence<const_like_t<Self, Bases>> && ...)
    {
        return inc_ra_impl<0>(self, idx, offset, std::index_sequence_for<Bases...>{});
    }

};

inline constexpr auto chain = detail::chain_fn{};

} // namespace flux

#endif
