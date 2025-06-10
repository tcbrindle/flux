// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_AS_RANGE_HPP_INCLUDED
#define FLUX_CORE_AS_RANGE_HPP_INCLUDED

#include <flux/core/iterable_concepts.hpp>

#include <ranges>

namespace flux {

namespace detail {

template <typename It>
struct iterable_range : std::ranges::view_interface<iterable_range<It>> {
private:
    It iterable_;
    iteration_context_t<It> ctx_ = iterate(iterable_);
    using opt_t = decltype(next_element(ctx_));
    opt_t next_ = next_element(ctx_);

    struct iterator {
    private:
        iterable_range* parent_ = nullptr;

    public:
        using reference = iterable_element_t<It>;
        using value_type = iterable_value_t<It>;
        using difference_type = int_t;

        iterator() = default;

        explicit iterator(iterable_range& parent) : parent_(std::addressof(parent)) { }

        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        constexpr auto operator*() const -> reference
        {
            return static_cast<reference>(parent_->next_.value());
        }

        constexpr auto operator++() -> iterator&
        {
            parent_->next_ = next_element(parent_->ctx_);
            return *this;
        }

        constexpr auto operator++(int) -> void { ++*this; }

        constexpr auto operator==(std::default_sentinel_t) const -> bool
        {
            return !static_cast<bool>(parent_->next_);
        }
    };

public:
    explicit constexpr iterable_range(decays_to<It> auto&& it) : iterable_(FLUX_FWD(it)) { }

    constexpr auto begin() -> iterator { return iterator(*this); }

    static constexpr auto end() -> std::default_sentinel_t { return {}; }
};

} // namespace detail

FLUX_EXPORT
struct as_range_t {
    template <iterable It>
    constexpr auto operator()(It&& it) const -> std::ranges::input_range decltype(auto)
    {
        if constexpr (std::ranges::input_range<It>) {
            return FLUX_FWD(it);
        } else {
            if constexpr (std::is_lvalue_reference_v<It>) {
                return detail::iterable_range<std::reference_wrapper<It>>(std::ref(it));
            } else {
                return detail::iterable_range<It>(it);
            }
        }
    }
};

FLUX_EXPORT inline constexpr as_range_t as_range{};

} // namespace flux

#endif // FLUX_CORE_AS_RANGE_HPP_INCLUDED