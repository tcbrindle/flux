
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_ALGORITHM_FOLD_HPP_INCLUDED
#define FLUX_ALGORITHM_FOLD_HPP_INCLUDED

#include <flux/algorithm/for_each_while.hpp>

namespace flux {

FLUX_EXPORT
struct fold_t {
    template <iterable It, typename Func, std::movable Init = iterable_value_t<It>,
              typename R = fold_result_t<It, Func, Init>>
        requires std::invocable<Func&, Init, element_t<It>>
        && std::invocable<Func&, R, element_t<It>> && std::convertible_to<Init, R>
        && std::assignable_from<Init&, std::invoke_result_t<Func&, R, element_t<It>>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Func func, Init init = Init{}) const -> R
    {
        R init_ = R(std::move(init));
        for_each_while(it, [&func, &init_](auto&& elem) {
            init_ = std::invoke(func, std::move(init_), FLUX_FWD(elem));
            return true;
        });
        return init_;
    }
};

FLUX_EXPORT inline constexpr auto fold = fold_t{};

FLUX_EXPORT
struct fold_first_t {
    template <iterable It, typename Func, typename V = iterable_value_t<It>>
        requires std::invocable<Func&, V, iterable_element_t<It>>
        && std::assignable_from<iterable_value_t<It>&,
                                std::invoke_result_t<Func&, V&&, iterable_element_t<It>>>
    [[nodiscard]]
    constexpr auto operator()(It&& it, Func func) const -> flux::optional<V>
    {
        iteration_context auto ctx = iterate(it);

        auto opt = next_element(ctx);

        if (!opt.has_value()) {
            return flux::nullopt;
        }

        V init = std::move(opt).value();
        run_while(ctx, [&](auto&& elem) {
            init = std::invoke(func, std::move(init), FLUX_FWD(elem));
            return loop_continue;
        });

        return flux::optional<V>(std::in_place, std::move(init));
    }
};

FLUX_EXPORT inline constexpr fold_first_t fold_first{};

namespace detail {

// Workaround libc++18 invoke() bug: https://github.com/llvm/llvm-project/issues/106428
consteval bool libcpp_fold_invoke_workaround_required()
{
#if defined(_LIBCPP_VERSION)
    return _LIBCPP_VERSION >= 180000 && _LIBCPP_VERSION < 190000;
#else
    return false;
#endif
}

} // namespace detail

FLUX_EXPORT
struct sum_t {
    template <iterable It>
        requires std::invocable<fold_t, It, std::plus<>> && requires { iterable_value_t<It>(0); }
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> iterable_value_t<It>
    {
        if constexpr (num::integral<iterable_value_t<It>>) {
            if constexpr (detail::libcpp_fold_invoke_workaround_required()) {
                auto add = []<typename T>(T lhs, T rhs) -> T { return num::add(lhs, rhs); };
                return fold(FLUX_FWD(it), add, iterable_value_t<It>(0));
            } else {
                return fold(FLUX_FWD(it), num::add, iterable_value_t<It>(0));
            }
        } else {
            return fold(FLUX_FWD(it), std::plus<>{}, iterable_value_t<It>(0));
        }
    }
};

FLUX_EXPORT inline constexpr auto sum = sum_t{};

FLUX_EXPORT
struct product_t {
    template <iterable It>
        requires std::invocable<fold_t, It, std::multiplies<>>
        && requires { iterable_value_t<It>(1); }
    [[nodiscard]]
    constexpr auto operator()(It&& it) const -> iterable_value_t<It>
    {
        if constexpr (num::integral<iterable_value_t<It>>) {
            if constexpr (detail::libcpp_fold_invoke_workaround_required()) {
                auto mul = []<typename T>(T lhs, T rhs) -> T { return num::mul(lhs, rhs); };
                return fold(FLUX_FWD(it), mul, iterable_value_t<It>(1));
            } else {
                return fold(FLUX_FWD(it), num::mul, iterable_value_t<It>(1));
            }
        } else {
            return fold(FLUX_FWD(it), std::multiplies<>{}, iterable_value_t<It>(1));
        }
    }
};

FLUX_EXPORT inline constexpr auto product = product_t{};

template <typename Derived>
template <typename D, typename Func, typename Init>
    requires foldable<Derived, Func, Init>
[[nodiscard]]
constexpr auto inline_sequence_base<Derived>::fold(Func func, Init init)
    -> fold_result_t<D, Func, Init>
{
    return flux::fold(derived(), std::move(func), std::move(init));
}

template <typename Derived>
template <typename D, typename Func>
    requires std::invocable<Func&, value_t<D>, element_t<D>>
    && std::assignable_from<value_t<D>&, std::invoke_result_t<Func&, value_t<D>, element_t<D>>>
constexpr auto inline_sequence_base<Derived>::fold_first(Func func)
{
    return flux::fold_first(derived(), std::move(func));
}

template <typename D>
constexpr auto inline_sequence_base<D>::sum()
    requires foldable<D, std::plus<>, value_t<D>> && std::default_initializable<value_t<D>>
{
    return flux::sum(derived());
}

template <typename D>
constexpr auto inline_sequence_base<D>::product()
    requires foldable<D, std::multiplies<>, value_t<D>> && requires { value_t<D>(1); }
{
    return flux::product(derived());
}

} // namespace flux

#endif // FLUX_ALGORITHM_FOLD_HPP_INCLUDED
