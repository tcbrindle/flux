
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FOLD_HPP_INCLUDED
#define FLUX_ALGORITHM_FOLD_HPP_INCLUDED

#include <flux/core.hpp>

namespace flux {

namespace detail {

struct fold_op {
    template <iterable It, typename Func, std::movable Init = value_t<It>,
              typename R = fold_result_t<It, Func, Init>>
        requires foldable<It, Func, Init>
    constexpr auto operator()(It&& it, Func func, Init init = Init{}) const -> R
    {
        R init_ = R(std::move(init));
        iterate(it, [&func, &init_](auto&& elem) {
            init_ = std::invoke(func, std::move(init_), FLUX_FWD(elem));
            return true;
        });
        return init_;
    }
};

struct fold_first_op {
    template <iterable It, typename Func, typename V = value_t<It>>
        requires std::invocable<Func&, V, element_t<It>> &&
                 std::assignable_from<V&, std::invoke_result_t<Func&, V&&, element_t<It>>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Func func) const -> flux::optional<V>
    {
        using Opt = optional<V>;

        return fold_op{}(it, [&func](Opt&& opt, auto&& elem) -> Opt {
            if (opt.has_value()) {
                auto& acc = opt.value_unchecked();
                acc = std::invoke(func, std::move(acc), FLUX_FWD(elem));
            } else {
                opt.emplace(FLUX_FWD(elem));
            }
            return opt;
        }, Opt{});
    }
};

// Workaround libc++18 invoke() bug: https://github.com/llvm/llvm-project/issues/106428
consteval bool libcpp_fold_invoke_workaround_required()
{
#if defined(_LIBCPP_VERSION)
    return _LIBCPP_VERSION >= 180000 && _LIBCPP_VERSION < 190000;
#else
    return false;
#endif
}

struct sum_op {
    template <iterable It>
        requires foldable<It, std::plus<>, value_t<It>> &&
                 requires { value_t<It>{0}; }
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> value_t<It>
    {
        if constexpr (num::integral<value_t<It>>) {
            if constexpr (libcpp_fold_invoke_workaround_required()) {
                auto add = []<typename T>(T lhs, T rhs) -> T { return num::add(lhs, rhs); };
                return fold_op{}(FLUX_FWD(it), add, value_t<It>(0));
            } else {
                return fold_op{}(FLUX_FWD(it), num::add, value_t<It>(0));
            }
        } else {
            return fold_op{}(FLUX_FWD(it), std::plus<>{}, value_t<It>(0));
        }
    }
};

struct product_op {
    template <iterable It>
        requires foldable<It, std::multiplies<>, value_t<It>> &&
                 requires { value_t<It>{1}; }
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> value_t<It>
    {
        if constexpr (num::integral<value_t<It>>) {
            if constexpr (libcpp_fold_invoke_workaround_required()) {
                auto mul = []<typename T>(T lhs, T rhs) -> T { return num::mul(lhs, rhs); };
                return fold_op{}(FLUX_FWD(it), mul, value_t<It>(1));
            } else {
                return fold_op{}(FLUX_FWD(it), num::mul, value_t<It>(1));
            }
        } else {
            return fold_op{}(FLUX_FWD(it), std::multiplies<>{}, value_t<It>(1));
        }
    }
};

} // namespace detail

FLUX_EXPORT inline constexpr auto fold = detail::fold_op{};
FLUX_EXPORT inline constexpr auto fold_first = detail::fold_first_op{};
FLUX_EXPORT inline constexpr auto sum = detail::sum_op{};
FLUX_EXPORT inline constexpr auto product = detail::product_op{};

template <typename Derived>
template <typename D, typename Func, typename Init>
    requires foldable<Derived, Func, Init>
[[nodiscard]]
constexpr auto inline_sequence_base<Derived>::fold(Func func, Init init) -> fold_result_t<D, Func, Init>
{
    return flux::fold(derived(), std::move(func), std::move(init));
}

template <typename Derived>
template <typename D, typename Func>
    requires std::invocable<Func&, value_t<D>, element_t<D>> &&
             std::assignable_from<value_t<D>&, std::invoke_result_t<Func&, value_t<D>, element_t<D>>>
constexpr auto inline_sequence_base<Derived>::fold_first(Func func)
{
    return flux::fold_first(derived(), std::move(func));
}

template <typename D>
constexpr auto inline_sequence_base<D>::sum()
    requires foldable<D, std::plus<>, value_t<D>> &&
             std::default_initializable<value_t<D>>
{
    return flux::sum(derived());
}

template <typename D>
constexpr auto inline_sequence_base<D>::product()
    requires foldable<D, std::multiplies<>, value_t<D>> &&
             requires { value_t<D>(1); }
{
    return flux::product(derived());
}

} // namespace flux

#endif // FLUX_ALGORITHM_FOLD_HPP_INCLUDED
