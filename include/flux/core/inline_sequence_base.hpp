
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_INLINE_SEQUENCE_BASE_HPP_INCLUDED
#define FLUX_CORE_INLINE_SEQUENCE_BASE_HPP_INCLUDED

#include <flux/core/sequence_access.hpp>

#include <flux/op/requirements.hpp>

namespace flux {

template <cursor Cur>
struct bounds {
    FLUX_NO_UNIQUE_ADDRESS Cur from;
    FLUX_NO_UNIQUE_ADDRESS Cur to;

    friend bool operator==(bounds const&, bounds const&) = default;
};

template <sequence Seq>
using bounds_t = bounds<cursor_t<Seq>>;

template <typename Derived>
struct inline_sequence_base {
private:
    constexpr auto derived() -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto derived() const -> Derived const& { return static_cast<Derived const&>(*this); }

public:
    /*
     * Basic iteration functions
     */

    /// Returns a cursor pointing to the first element of the sequence
    [[nodiscard]]
    constexpr auto first() { return flux::first(derived()); }

    /// Returns true if `cur` points to the end of the sequence
    ///
    /// @param cur The cursor to test
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr bool is_last(cursor_t<D> const& cur) { return flux::is_last(derived(), cur); }

    /// Increments the given cursor
    ///
    /// @param cur the cursor to increment
    template <std::same_as<Derived> D = Derived>
    constexpr auto& inc(cursor_t<D>& cur) { return flux::inc(derived(), cur); }

    /// Returns the element at the given cursor
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) read_at(cursor_t<D> const& cur) { return flux::read_at(derived(), cur); }

    /// Returns an rvalue version of the element at the given cursor
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) move_at(cursor_t<D> const& cur) { return flux::move_at(derived(), cur); }

    /// Returns the element at the given cursor
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) operator[](cursor_t<D> const& cur) { return flux::read_at(derived(), cur); }

    /// Returns an cursor pointing to one past the last element of the sequence
    [[nodiscard]]
    constexpr auto last() requires bounded_sequence<Derived> { return flux::last(derived()); }

    /// Decrements the given cursor
    template <std::same_as<Derived> D = Derived>
        requires bidirectional_sequence<Derived>
    constexpr auto& dec(cursor_t<D>& cur) { return flux::dec(derived(), cur); }

    /// Increments the given cursor by `offset` places
    template <std::same_as<Derived> D = Derived>
        requires random_access_sequence<Derived>
    constexpr auto& inc(cursor_t<D>& cur, distance_t offset) { return flux::inc(derived(), cur, offset); }

    /// Returns the number of times `from` must be incremented to reach `to`
    ///
    /// For a random-access sequence, returns the result in constant time
    template <std::same_as<Derived> D = Derived>
        requires multipass_sequence<Derived>
    [[nodiscard]]
    constexpr auto distance(cursor_t<D> const& from, cursor_t<D> const& to)
    {
        return flux::distance(derived(), from, to);
    }

    [[nodiscard]]
    constexpr auto data() requires contiguous_sequence<Derived>
    {
        return flux::data(derived());
    }

    [[nodiscard]]
    constexpr auto data() const requires contiguous_sequence<Derived const>
    {
        return flux::data(derived());
    }

    /// Returns the number of elements in the sequence
    [[nodiscard]]
    constexpr auto size() requires sized_sequence<Derived> { return flux::size(derived()); }

    [[nodiscard]]
    constexpr auto size() const requires sized_sequence<Derived const> { return flux::size(derived()); }

    /// Returns the number of elements in the sequence as a size_t
    [[nodiscard]]
    constexpr auto usize() requires sized_sequence<Derived> { return flux::usize(derived()); }

    [[nodiscard]]
    constexpr auto usize() const requires sized_sequence<Derived const> { return flux::usize(derived()); }

    /// Returns true if the sequence contains no elements
    [[nodiscard]]
    constexpr auto is_empty()
        requires (multipass_sequence<Derived> || sized_sequence<Derived>)
    {
        return flux::is_empty(derived());
    }

    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr auto next(cursor_t<D> cur) { return flux::next(derived(), cur); }

    template <std::same_as<Derived> D = Derived>
        requires bidirectional_sequence<Derived>
    [[nodiscard]]
    constexpr auto prev(cursor_t<D> cur) { return flux::prev(derived(), cur); }

    [[nodiscard]]
    constexpr auto front() requires multipass_sequence<Derived>
    {
        return flux::front(derived());
    }

    [[nodiscard]]
    constexpr auto front() const requires multipass_sequence<Derived const>
    {
        return flux::front(derived());
    }

    [[nodiscard]]
    constexpr auto back()
        requires bidirectional_sequence<Derived> && bounded_sequence<Derived>
    {
        return flux::back(derived());
    }

    [[nodiscard]]
    constexpr auto back() const
        requires bidirectional_sequence<Derived const> && bounded_sequence<Derived const>
    {
        return flux::back(derived());
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived&, Args...>
    constexpr auto _(Func&& func, Args&&... args) & -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), derived(), FLUX_FWD(args)...);
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived const&, Args...>
    constexpr auto _(Func&& func, Args&&... args) const& -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), derived(), FLUX_FWD(args)...);
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived, Args...>
    constexpr auto _(Func&& func, Args&&... args) && -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), std::move(derived()), FLUX_FWD(args)...);
    }

    template <typename Func, typename... Args>
        requires std::invocable<Func, Derived const, Args...>
    constexpr auto _(Func&& func, Args&&... args) const&& -> decltype(auto)
    {
        return std::invoke(FLUX_FWD(func), std::move(derived()), FLUX_FWD(args)...);
    }

    /*
     * Iterator support
     */
    constexpr auto begin() &;

    constexpr auto begin() const& requires sequence<Derived const>;

    constexpr auto end() &;

    constexpr auto end() const& requires sequence<Derived const>;

    /*
     * Adaptors
     */

    [[nodiscard]]
    constexpr auto cache_last() &&
            requires bounded_sequence<Derived> ||
                     (multipass_sequence<Derived> && not infinite_sequence<Derived>);

    [[nodiscard]]
    constexpr auto chunk(std::integral auto chunk_sz) &&;

    template <typename Pred>
        requires multipass_sequence<Derived> &&
                 std::predicate<Pred&, element_t<Derived>, element_t<Derived>>
    [[nodiscard]]
    constexpr auto chunk_by(Pred pred) &&;

    [[nodiscard]]
    constexpr auto drop(distance_t count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto drop_while(Pred pred) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>&>
    [[nodiscard]]
    constexpr auto filter(Pred pred) &&;

    [[nodiscard]]
    constexpr auto flatten() && requires sequence<element_t<Derived>>;

    template <typename Func>
        requires std::invocable<Func&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto map(Func func) &&;

    [[nodiscard]]
    constexpr auto reverse() &&
            requires bidirectional_sequence<Derived> && bounded_sequence<Derived>;

    [[nodiscard]]
    constexpr auto slide(std::integral auto win_sz) && requires multipass_sequence<Derived>;

    template <multipass_sequence Pattern>
        requires std::equality_comparable_with<element_t<Derived>, element_t<Pattern>>
    [[nodiscard]]
    constexpr auto split(Pattern&& pattern) &&;

    template <typename ValueType>
        requires decays_to<ValueType, value_t<Derived>>
    [[nodiscard]]
    constexpr auto split(ValueType&& delim) &&;

    template <typename Pattern>
    [[nodiscard]]
    constexpr auto split_string(Pattern&& pattern) &&;

    [[nodiscard]]
    constexpr auto stride(std::integral auto by) &&;

    [[nodiscard]]
    constexpr auto take(distance_t count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto take_while(Pred pred) &&;

    /*
     * Algorithms
     */

    /// Returns `true` if every element of the sequence satisfies the predicate
    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto all(Pred pred, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto any(Pred pred, Proj proj = {});

    template <typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Derived>, Value const&>
    constexpr auto contains(Value const& value, Proj proj = {}) -> bool;

    /// Returns the number of elements in the sequence
    constexpr auto count();

    /// Returns the number of elements in the sequence which are equal to `value`
    template <typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Derived>, Value const&>
    constexpr auto count_eq(Value const& value, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    constexpr auto count_if(Pred pred, Proj proj = {});

    template <typename Value>
        requires writable_sequence_of<Derived, Value const&>
    constexpr auto fill(Value const& value) -> void;

    /// Returns a cursor pointing to the first occurrence of `value` in the sequence
    template <typename Value, typename Proj = std::identity>
        requires std::equality_comparable_with<projected_t<Proj, Derived>, Value const&>
    [[nodiscard]]
    constexpr auto find(Value const&, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto find_if(Pred pred, Proj proj = {});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto find_if_not(Pred pred, Proj proj = {});

    template <typename D = Derived, typename Func, typename Init>
        requires foldable<Derived, Func, Init>
    [[nodiscard]]
    constexpr auto fold(Func func, Init init) -> fold_result_t<D, Func, Init>;

    template <typename D = Derived, typename Func>
        requires std::invocable<Func&, value_t<D>, element_t<D>> &&
                 std::assignable_from<value_t<D>&, std::invoke_result_t<Func&, value_t<D>, element_t<D>>>
    [[nodiscard]]
    constexpr auto fold_first(Func func);

    template <typename Func, typename Proj = std::identity>
        requires std::invocable<Func&, projected_t<Proj, Derived>>
    constexpr auto for_each(Func func, Proj proj = {}) -> Func;

    template <typename Pred>
        requires std::invocable<Pred&, element_t<Derived>> &&
                 detail::boolean_testable<std::invoke_result_t<Pred&, element_t<Derived>>>
    constexpr auto for_each_while(Pred pred);

    constexpr auto inplace_reverse()
        requires bounded_sequence<Derived> &&
                 detail::element_swappable_with<Derived, Derived>;

    template <typename Cmp = std::ranges::less, typename Proj = std::identity>
    constexpr auto max(Cmp cmp = Cmp{}, Proj proj = Proj{});

    template <typename Cmp = std::ranges::less, typename Proj = std::identity>
    constexpr auto min(Cmp cmp = Cmp{}, Proj proj = Proj{});

    template <typename Cmp = std::ranges::less, typename Proj = std::identity>
    constexpr auto minmax(Cmp cmp = Cmp{}, Proj proj = Proj{});

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto none(Pred pred, Proj proj = {});

    template <typename Iter>
        requires std::weakly_incrementable<Iter> &&
                 std::indirectly_writable<Iter, element_t<Derived>>
    constexpr auto output_to(Iter iter) -> Iter;

    constexpr auto sum()
        requires foldable<Derived, std::plus<>, value_t<Derived>> &&
                 std::default_initializable<value_t<Derived>>;

    template <typename Cmp = std::less<>, typename Proj = std::identity>
        requires random_access_sequence<Derived> &&
                 bounded_sequence<Derived> &&
                 detail::element_swappable_with<Derived, Derived> &&
                 std::predicate<Cmp&, projected_t<Proj, Derived>, projected_t<Proj, Derived>>
    constexpr void sort(Cmp cmp = {}, Proj proj = {});

    constexpr auto product()
        requires foldable<Derived, std::multiplies<>, value_t<Derived>> &&
                 requires { value_t<Derived>(1); };

    template <typename Container, typename... Args>
    constexpr auto to(Args&&... args) -> Container;

    template <template <typename...> typename Container, typename... Args>
    constexpr auto to(Args&&... args);

    auto write_to(std::ostream& os) -> std::ostream&;
};


} // namespace flux

#endif // FLUX_CORE_SEQUENCE_IFACE_HPP_INCLUDED
