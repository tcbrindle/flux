
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_OPTIONAL_HPP_INCLUDED
#define FLUX_CORE_OPTIONAL_HPP_INCLUDED

#include <flux/core/assert.hpp>
#include <flux/core/utils.hpp>

#include <functional>
#include <optional>

namespace flux {

FLUX_EXPORT using nullopt_t = std::nullopt_t;
FLUX_EXPORT inline constexpr nullopt_t const& nullopt = std::nullopt;

namespace detail {

template <typename T>
concept can_optional =
    (std::is_object_v<T> || std::is_lvalue_reference_v<T>) &&
    !decays_to<T, nullopt_t> &&
    !decays_to<T, std::in_place_t>;

}

FLUX_EXPORT
template <typename T>
class optional;

template <detail::can_optional T>
    requires std::is_object_v<T>
class optional<T> {

    struct dummy {};

    union {
        dummy dummy_{};
        T item_;
    };

    bool engaged_ = false;

    template <typename... Args>
    constexpr auto construct(Args&&... args) {
        std::construct_at(std::addressof(item_), FLUX_FWD(args)...);
        engaged_ = true;
    }

public:

    constexpr optional() noexcept {}

    constexpr explicit(false) optional(nullopt_t) noexcept {}

    template <decays_to<T> U = T>
    constexpr explicit optional(U&& item)
        noexcept(std::is_nothrow_constructible_v<T, U>)
        : item_(FLUX_FWD(item)),
          engaged_(true)
    {}

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr explicit optional(std::in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : item_(FLUX_FWD(args)...),
          engaged_(true)
    {}

    /*
     * Destructors
     */
    constexpr ~optional()
    {
        if (engaged_) {
            item_.T::~T();
        }
    }

    ~optional() requires std::is_trivially_destructible_v<T> = default;

    /*
     * Copy constructors
     */
    constexpr optional(optional const& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires std::copy_constructible<T>
    {
        if (other.engaged_) {
            construct(other.item_);
        }
    }

    optional(optional const&)
        requires std::copy_constructible<T> &&
                 std::is_trivially_constructible_v<T>
        = default;

    /*
     * Copy-assignment operators
     */
    constexpr optional& operator=(optional const& other)
        noexcept(std::is_nothrow_copy_assignable_v<T> &&
                 std::is_nothrow_copy_constructible_v<T>)
        requires std::copyable<T>
    {
        if (engaged_) {
            if (other.engaged_) {
                item_ = other.item_;
            } else {
                reset();
            }
        } else {
            if (other.engaged_) {
                construct(other.item_);
            }
        }
        return *this;
    }

    optional& operator=(optional const&)
        requires std::copyable<T> && std::is_trivially_copy_assignable_v<T>
        = default;

    /*
     * Move constructors
     */
    constexpr optional(optional&& other)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::move_constructible<T>
    {
        if (other.engaged_) {
            construct(std::move(other).item_);
        }
    }

    optional(optional&&)
        requires std::move_constructible<T> &&
                 std::is_trivially_move_constructible_v<T>
        = default;

    /*
     * Move-assignment operators
     */
    constexpr optional& operator=(optional&& other)
        noexcept(std::is_nothrow_move_constructible_v<T> &&
                 std::is_nothrow_move_assignable_v<T>)
        requires std::movable<T>
    {
        if (engaged_) {
            if (other.engaged_) {
                item_ = std::move(other.item_);
            } else {
                reset();
            }
        } else {
            if (other.engaged_) {
                construct(std::move(other).item_);
            }
        }
        return *this;
    }

    constexpr optional& operator=(optional&&)
        requires std::movable<T> &&
                 std::is_trivially_move_assignable_v<T>
        = default;

    /*
     * Observers
     */
    [[nodiscard]]
    constexpr auto has_value() const -> bool { return engaged_; }

    constexpr explicit operator bool() const { return engaged_; }

    template <decays_to<optional> Opt>
    [[nodiscard]]
    friend constexpr auto operator*(Opt&& self) -> decltype(auto)
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(self.has_value());
        }
        return (FLUX_FWD(self).item_);
    }

    constexpr auto operator->() -> T*
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(this->has_value());
        }
        return std::addressof(item_);
    }

    constexpr auto operator->() const -> T const*
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(this->has_value());
        }
        return std::addressof(item_);
    }

    [[nodiscard]] constexpr auto value() & -> T& { return **this; }
    [[nodiscard]] constexpr auto value() const& -> T const& { return **this; }
    [[nodiscard]] constexpr auto value() && -> T&& { return *std::move(*this); }
    [[nodiscard]] constexpr auto value() const&& -> T const&& { return *std::move(*this); }

    [[nodiscard]] constexpr auto value_unchecked() & noexcept -> T& { return item_; }
    [[nodiscard]] constexpr auto value_unchecked() const& noexcept -> T const& { return item_; }
    [[nodiscard]] constexpr auto value_unchecked() && noexcept -> T&& { return std::move(item_); }
    [[nodiscard]] constexpr auto value_unchecked() const&& noexcept -> T const&& { return std::move(item_); }

    [[nodiscard]] constexpr auto value_or(auto&& alt) &
        -> decltype(has_value() ? value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]] constexpr auto value_or(auto&& alt) const&
        -> decltype(has_value() ? value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]] constexpr auto value_or(auto&& alt) &&
        -> decltype(has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]] constexpr auto value_or(auto&& alt) const&&
        -> decltype(has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? std::move(*this).value_unchecked() : FLUX_FWD(alt);
    }

private:
    template <typename Cmp>
    static constexpr auto do_compare(optional const& lhs, optional const& rhs, Cmp cmp)
        -> std::decay_t<std::invoke_result_t<Cmp&, T const&, T const&>>
    {
        if (lhs.has_value() && rhs.has_value()) {
            return cmp(lhs.value_unchecked(), rhs.value_unchecked());
        } else {
            return cmp(lhs.has_value(), rhs.has_value());
        }
    }

public:
    [[nodiscard]]
    friend constexpr auto operator==(optional const& lhs, optional const& rhs) -> bool
        requires std::equality_comparable<T>
    {
        return do_compare(lhs, rhs, std::equal_to{});
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& lhs, optional const& rhs)
        requires std::totally_ordered<T> && std::three_way_comparable<T>
    {
        return do_compare(lhs, rhs, std::compare_three_way{});
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& lhs, optional const& rhs)
        requires std::totally_ordered<T>
    {
        return do_compare(lhs, rhs, std::compare_partial_order_fallback);
    }

    [[nodiscard]]
    friend constexpr auto operator==(optional const& o, nullopt_t) -> bool
    {
        return !o.has_value();
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& o, nullopt_t)
        -> std::strong_ordering
    {
        return o.has_value() ? std::strong_ordering::greater
                             : std::strong_ordering::equivalent;
    }

    /*
     * Modifiers
     */

    constexpr auto reset() -> void
    {
        if (engaged_) {
            item_.T::~T();
            engaged_ = false;
        }
    }

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr auto emplace(Args&&... args) -> T&
    {
        reset();
        construct(FLUX_FWD(args)...);
        return item_;
    }

    /*
     * Monadic operations
     */
    template <typename F>
        requires std::invocable<F, T&> &&
                 detail::can_optional<std::invoke_result_t<F, T&>>
    [[nodiscard]]
    constexpr auto map(F&& func) & -> optional<std::invoke_result_t<F, T&>>
    {
        using R = optional<std::invoke_result_t<F, T&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), value_unchecked()));
        } else {
            return nullopt;
        }
    }

    template <typename F>
        requires std::invocable<F, T const&> &&
                 detail::can_optional<std::invoke_result_t<F, T const&>>
    [[nodiscard]]
    constexpr auto map(F&& func) const& -> optional<std::invoke_result_t<F, T const&>>
    {
        using R = optional<std::invoke_result_t<F, T const&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), value_unchecked()));
        } else {
            return nullopt;
        }
    }

    template <typename F>
        requires std::invocable<F, T&&> &&
                 detail::can_optional<std::invoke_result_t<F, T&&>>
    [[nodiscard]]
    constexpr auto map(F&& func) && -> optional<std::invoke_result_t<F, T&&>>
    {
        using R = optional<std::invoke_result_t<F, T&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), std::move(*this).value_unchecked()));
        } else {
            return nullopt;
        }
    }

    template <typename F>
        requires std::invocable<F, T const&&> &&
                 detail::can_optional<std::invoke_result_t<F, T const&&>>
    [[nodiscard]]
    constexpr auto map(F&& func) const&& -> optional<std::invoke_result_t<F, T const&&>>
    {
        using R = optional<std::invoke_result_t<F, T const&&>>;
        if (engaged_) {
            return R(std::invoke(FLUX_FWD(func), std::move(*this).value_unchecked()));
        } else {
            return nullopt;
        }
    }
};

template <typename T>
optional(T) -> optional<T>;

template <detail::can_optional T>
class optional<T&> {
    T* ptr_ = nullptr;

    static void test_fn(T&);

public:
    optional() = default;

    constexpr explicit(false) optional(nullopt_t) noexcept {};

    template <typename U = T>
        requires requires(U& u) { test_fn(u); }
    constexpr explicit optional(U& item) noexcept
        : ptr_(std::addressof(item))
    {}

    /*
     * Observers
     */
    [[nodiscard]]
    constexpr auto has_value() const noexcept { return ptr_ != nullptr; }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept { return ptr_ != nullptr; }

    [[nodiscard]]
    constexpr auto operator*() const -> T&
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(ptr_ != nullptr);
        }
        return *ptr_;
    }

    constexpr auto operator->() const -> T*
    {
        if (!std::is_constant_evaluated()) {
            FLUX_ASSERT(ptr_ != nullptr);
        }
        return ptr_;
    }

    [[nodiscard]]
    constexpr auto value() const -> T& { return **this; }

    [[nodiscard]]
    constexpr auto value_unchecked() const noexcept -> T& { return *ptr_; }

    [[nodiscard]]
    constexpr auto value_or(auto&& alt) const
        -> decltype(has_value() ? value_unchecked() : FLUX_FWD(alt))
    {
        return has_value() ? value_unchecked() : FLUX_FWD(alt);
    }

    [[nodiscard]]
    friend constexpr auto operator==(optional const& o, nullopt_t) -> bool
    {
        return !o.has_value();
    }

    [[nodiscard]]
    friend constexpr auto operator<=>(optional const& o, nullopt_t)
        -> std::strong_ordering
    {
        return o.has_value() ? std::strong_ordering::greater
                             : std::strong_ordering::equivalent;
    }

    /*
     * Modifiers
     */
    constexpr auto reset() -> void { ptr_ = nullptr; }

    /*
     * Monadic operations
     */
    template <typename F>
        requires std::invocable<F, T&> &&
                 detail::can_optional<std::invoke_result_t<F, T&>>
    [[nodiscard]]
    constexpr auto map(F&& func) const -> optional<std::invoke_result_t<F, T&>>
    {
        using R = optional<std::invoke_result_t<F, T&>>;
        if (ptr_) {
            return R(std::invoke(FLUX_FWD(func), *ptr_));
        } else {
            return nullopt;
        }
    }
};

} // namespace flux

#endif
