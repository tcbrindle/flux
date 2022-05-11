
#pragma once

#include <flux/core/sequence_access.hpp>

namespace flux {

template <index Idx>
struct bounds {
    FLUX_NO_UNIQUE_ADDRESS Idx from;
    FLUX_NO_UNIQUE_ADDRESS Idx to;

    friend bool operator==(bounds const&, bounds const&) = default;
};

template <sequence Seq>
using bounds_t = bounds<index_t<Seq>>;

template <typename Derived>
    /*requires std::is_class_v<Derived>&&
             std::same_as<Derived, std::remove_cv_t<Derived>>*/
struct lens_base {
private:
    constexpr auto derived() -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto derived() const -> Derived const& { return static_cast<Derived const&>(*this); }

protected:
    ~lens_base() = default;

public:
    /*
     * Basic iteration functions
     */

    /// Returns an index pointing to the first element of the sequence
    [[nodiscard]]
    constexpr auto first() { return flux::first(derived()); }

    /// Returns true if `idx` points to the end of the sequence
    ///
    /// @param idx The index to test
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr bool is_last(index_t<D> const& idx) { return flux::is_last(derived(), idx); }

    /// Increments the given index
    ///
    /// @param idx the index to increment
    template <std::same_as<Derived> D = Derived>
    constexpr auto& inc(index_t<D>& idx) { return flux::inc(derived(), idx); }

    /// Returns the element at the given index
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) read_at(index_t<D> const& idx) { return flux::read_at(derived(), idx); }

    /// Returns an rvalue version of the element at the given index
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) move_at(index_t<D> const& idx) { return flux::move_at(derived(), idx); }

    /// Returns the element at the given index
    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr decltype(auto) operator[](index_t<D> const& idx) { return flux::read_at(derived(), idx); }

    /// Returns an index pointing to one past the last element of the sequence
    [[nodiscard]]
    constexpr auto last() requires bounded_sequence<Derived> { return flux::last(derived()); }

    /// Decrements the given index
    template <std::same_as<Derived> D = Derived>
        requires bidirectional_sequence<Derived>
    constexpr auto& dec(index_t<D>& idx) { return flux::dec(derived(), idx); }

    /// Increments the given index by `offset` places
    template <std::same_as<Derived> D = Derived>
        requires random_access_sequence<Derived>
    constexpr auto& inc(index_t<D>& idx, distance_t<D> offset) { return flux::inc(derived(), idx, offset); }

    /// Returns the number of times `from` must be incremented to reach `to`
    ///
    /// For a random-access sequence, returns the result in constant time
    template <std::same_as<Derived> D = Derived>
        requires multipass_sequence<Derived>
    [[nodiscard]]
    constexpr auto distance(index_t<D> const& from, index_t<D> const& to)
    {
        return flux::distance(derived(), from, to);
    }

    [[nodiscard]]
    constexpr auto data() requires contiguous_sequence<Derived>
    {
        return flux::data(derived());
    }

    /// Returns the number of elements in the sequence
    [[nodiscard]]
    constexpr auto size() requires sized_sequence<Derived> { return flux::size(derived()); }

    /// Returns true of the sequence contains no elements
    [[nodiscard]]
    constexpr auto is_empty() requires multipass_sequence<Derived> { return flux::is_empty(derived()); }

    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr auto next(index_t<D> idx) { return flux::next(derived(), idx); }

    template <std::same_as<Derived> D = Derived>
        requires bidirectional_sequence<Derived>
    [[nodiscard]]
    constexpr auto prev(index_t<D> idx) { return flux::prev(derived(), idx); }

    /*
     * Adaptors
     */

    template <std::same_as<Derived> D = Derived>
    constexpr auto drop(distance_t<D> count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    constexpr auto drop_while(Pred pred) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>&>
    [[nodiscard]]
    constexpr auto filter(Pred pred) &&;

    template <typename Func>
        requires std::invocable<Func&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto map(Func func) &&;

    [[nodiscard]]
    constexpr auto reverse() &&
            requires bidirectional_sequence<Derived> && bounded_sequence<Derived>;

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

    template <std::same_as<Derived> D = Derived>
    [[nodiscard]]
    constexpr auto take(distance_t<D> count) &&;

    template <typename Pred>
        requires std::predicate<Pred&, element_t<Derived>>
    [[nodiscard]]
    constexpr auto take_while(Pred pred) &&;

    [[nodiscard]]
    constexpr auto view() &;

    [[nodiscard]]
    constexpr auto view() const& requires sequence<Derived const>;

    [[nodiscard]]
    constexpr auto view() &&;

    [[nodiscard]]
    constexpr auto view() const&& requires sequence<Derived const>;

    /*
     * Folds of various kinds
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

    template <typename Pred, typename Proj = std::identity>
        requires predicate_for<Pred, Derived, Proj>
    [[nodiscard]]
    constexpr auto none(Pred pred, Proj proj = {});

    /// Returns the index of `value` in the sequence
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
};


} // namespace flux