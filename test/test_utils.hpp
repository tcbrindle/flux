
#pragma once

#include <flux/core.hpp>
#include <flux/op/from.hpp>
#include <flux/op/ref.hpp>

#define STATIC_CHECK(...) if (!(__VA_ARGS__)) return false

inline namespace test_utils {

inline constexpr struct {
private:
    static constexpr bool impl(flux::lens auto seq1, flux::lens auto seq2)
    {
        auto idx1 = seq1.first();
        auto idx2 = seq2.first();

        while (!seq1.is_last(idx1) && !seq2.is_last(idx2)) {
            if (seq1[idx1] != seq2[idx2]) { return false; }

            seq1.inc(idx1);
            seq2.inc(idx2);
        }

        return seq1.is_last(idx1) == seq2.is_last(idx2);
    }

public:
    template <typename T>
    constexpr bool operator()(flux::sequence auto&& seq,
                              std::initializer_list<T> ilist) const
    {
        return impl(flux::from(FLUX_FWD(seq)), flux::from(ilist));
    }

    constexpr bool operator()(flux::sequence auto&& seq1,
                              flux::sequence auto&& seq2) const
    {
        return impl(flux::from(FLUX_FWD(seq1)), flux::from(FLUX_FWD(seq2)));
    }

} check_equal;

template <flux::lens Base>
struct single_pass_only : flux::lens_base<single_pass_only<Base>> {
private:
    Base base_;

    struct index_type {
        flux::index_t<Base> base_idx;

        constexpr explicit(false) index_type(flux::index_t<Base>&& base_idx)
            : base_idx(std::move(base_idx))
        {
        }

        index_type(index_type&&) = default;
        index_type& operator=(index_type&&) = default;
    };

    friend struct flux::sequence_iface<single_pass_only>;

public:
    constexpr explicit single_pass_only(Base&& base)
        : base_(std::move(base)) {}

    // Move-only sequence is useful for testing
    single_pass_only(single_pass_only&&) = default;
    single_pass_only&  operator=(single_pass_only&&) = default;
};

}

template <typename Base>
struct flux::sequence_iface<single_pass_only<Base>>
{
    using self_t = single_pass_only<Base>;
    using index_t = typename single_pass_only<Base>::index_type;

    static constexpr bool disable_multipass = true;

    static constexpr auto first(self_t& self)
    {
        return index_t(self.base_.first());
    }

    static constexpr auto is_last(self_t& self, index_t const& idx)
    {
        return self.base_.is_last(idx.base_idx);
    }

    static constexpr auto& inc(self_t& self, index_t& idx)
    {
        self.base_.inc(idx.base_idx);
        return idx;
    }

    static constexpr decltype(auto) read_at(self_t& self, index_t const& idx)
    {
        return self.base_.read_at(idx.base_idx);
    }

    static constexpr auto last(self_t& self)
        requires bounded_sequence<Base>
    {
        return index_t(self.base_.last());
    }

    static constexpr auto size(self_t& self)
        requires sized_sequence<Base>
    {
        return self.base_.size();
    }
};

template <typename Reqd, typename Expr>
constexpr void assert_has_type(Expr&&)
{
    static_assert(std::same_as<Reqd, Expr>);
}
