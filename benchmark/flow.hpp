
#ifndef FLOW_HPP_INCLUDED
#define FLOW_HPP_INCLUDED


#ifndef FLOW_CORE_FLOW_BASE_HPP_INCLUDED
#define FLOW_CORE_FLOW_BASE_HPP_INCLUDED


#ifndef FLOW_CORE_INVOKE_HPP_INCLUDED
#define FLOW_CORE_INVOKE_HPP_INCLUDED


#ifndef FLOW_CORE_MACROS_HPP_INCLUDED
#define FLOW_CORE_MACROS_HPP_INCLUDED

#include <ciso646>

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(no_unique_address)
#define FLOW_HAS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
#endif

#ifdef FLOW_HAS_NO_UNIQUE_ADDRESS
#define FLOW_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define FLOW_NO_UNIQUE_ADDRESS
#endif

#if defined(__cpp_coroutines)
#  if defined(__has_include) // If we can, check for <experimental/coroutine>
#    if __has_include(<experimental/coroutine>)
#      define FLOW_HAVE_COROUTINES
#    endif // __cpp_coroutines
#  else // can't check for it, let's hope for the best
#    define FLOW_HAVE_COROUTINES
#  endif // __has_include
#endif // __cpp_coroutines

#define FLOW_FWD(x) (static_cast<decltype(x)&&>(x))

#define FLOW_COPY(x) (static_cast<flow::remove_cvref_t<decltype(x)>>(x))

#define FLOW_FOR(VAR_DECL, ...) \
    if (auto&& _flow = (__VA_ARGS__); true) \
        while (auto _a = _flow.next()) \
            if (VAR_DECL = *std::move(_a); true)

#endif


#include <memory>       // for std::addressof
#include <type_traits>

namespace flow {

namespace detail {

struct invoke_fn {
private:
    template <typename T, typename Type, typename T1, typename... Args>
    static constexpr auto impl(Type T::* f, T1&& t1, Args&&... args) -> decltype(auto)
    {
        if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
            if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
                return (FLOW_FWD(t1).*f)(FLOW_FWD(args)...);
            } else {
                return ((*FLOW_FWD(t1)).*f)(FLOW_FWD(args)...);
            }
        } else {
            static_assert(std::is_member_object_pointer_v<decltype(f)>);
            static_assert(sizeof...(args) == 0);
            if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
                return FLOW_FWD(t1).*f;
            } else {
                return (*FLOW_FWD(t1)).*f;
            }
        }
    }

    template <typename F, typename... Args>
    static constexpr auto impl(F&& f, Args&&... args) -> decltype(auto)
    {
        return FLOW_FWD(f)(FLOW_FWD(args)...);
    }

public:
    template <typename F, typename... Args>
    constexpr auto operator()(F&& func, Args&&... args) const
        noexcept(std::is_nothrow_invocable_v<F, Args...>)
        -> std::invoke_result_t<F, Args...>
    {
        return impl(FLOW_FWD(func), FLOW_FWD(args)...);
    }
};

}

inline constexpr auto invoke = detail::invoke_fn{};

namespace detail {

// This is basically a cut-down, constexpr version of std::reference_wrapper
template <typename F>
struct function_ref {

    constexpr function_ref(F& func) : f_(std::addressof(func)) {}

    function_ref(F&&) = delete;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::invoke_result_t<F&, Args...>
    {
        return invoke(*f_, FLOW_FWD(args)...);
    }

private:
    F* f_;
};

}

struct equal_to {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(static_cast<bool>(FLOW_FWD(t) == FLOW_FWD(u)))
    {
        return static_cast<bool>(FLOW_FWD(t) == FLOW_FWD(u));
    }
};

struct not_equal_to {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(!equal_to{}(FLOW_FWD(t), FLOW_FWD(u)))
    {
        return !equal_to{}(FLOW_FWD(t), FLOW_FWD(u));
    }
};

struct less {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(static_cast<bool>(FLOW_FWD(t) < FLOW_FWD(u)))
    {
        return static_cast<bool>(FLOW_FWD(t) < FLOW_FWD(u));
    }
};

struct greater {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(less{}(FLOW_FWD(u), FLOW_FWD(t)))
    {
        return less{}(FLOW_FWD(u), FLOW_FWD(t));
    }
};

struct less_equal {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(!less{}(FLOW_FWD(u), FLOW_FWD(t)))
    {
        return !less{}(FLOW_FWD(u), FLOW_FWD(t));
    }
};

struct greater_equal {
    template <typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const
        -> decltype(!less{}(FLOW_FWD(t), FLOW_FWD(u)))
    {
        return !less{}(FLOW_FWD(t), FLOW_FWD(u));
    }
};

namespace detail {

// Reimplementations of std::min and std::max so we don't need to drag in <algorithm>
inline constexpr struct min_fn {
    template <typename T>
    constexpr auto operator()(const T& lhs, const T& rhs) const -> decltype(auto)
    {
        return lhs < rhs ? lhs : rhs;
    }

} min;

inline constexpr struct max_fn {
    template <typename T>
    constexpr auto operator()(const T& lhs, const T& rhs) const -> const T&
    {
        return rhs < lhs ? lhs : rhs;
    }
} max;

}

}

#endif


#ifndef FLOW_CORE_MAYBE_HPP_INCLUDED
#define FLOW_CORE_MAYBE_HPP_INCLUDED



#include <exception>

namespace flow {

struct bad_maybe_access : std::exception {
    bad_maybe_access() = default;

    [[nodiscard]] const char* what() const noexcept override
    {
        return "value() called on an empty maybe";
    }
};

namespace detail {

struct in_place_t {
    constexpr explicit in_place_t() = default;
};

// Storage for the non-trivial case
template <typename T, bool = std::is_trivially_destructible_v<T>>
struct maybe_storage_base {

    constexpr maybe_storage_base() noexcept
        : dummy_{},
          has_value_(false)
    {}

    template <typename... Args>
    constexpr maybe_storage_base(in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
        : value_(FLOW_FWD(args)...),
          has_value_(true)
    {}

    ~maybe_storage_base()
    {
        if (has_value_) {
            value_.~T();
        }
    }

    struct dummy {};

    union {
        dummy dummy_{};
        T value_;
    };

    bool has_value_ = false;
};

// Storage for the trivially-destructible case
template <typename T>
struct maybe_storage_base<T, /*is_trivially_destructible =*/ true>
{
    constexpr maybe_storage_base() noexcept
        : dummy_{},
          has_value_(false)
    {}

    template <typename... Args>
    constexpr maybe_storage_base(in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
        : value_(FLOW_FWD(args)...),
          has_value_(true)
    {}

    struct dummy {};

    union {
        dummy dummy_{};
        T value_;
    };

    bool has_value_ = false;
};

template <typename T>
struct maybe_ops_base : maybe_storage_base<T>
{
    using maybe_storage_base<T>::maybe_storage_base;

    constexpr void hard_reset()
    {
        get().~T();
        this->has_value_ = false;
    }

    template <typename... Args>
    void construct(Args&&... args)
    {
        new (std::addressof(this->value_)) T(FLOW_FWD(args)...);
        this->has_value_ = true;
    }

    template <typename Maybe>
    constexpr void assign(Maybe&& other)
    {
        if (this->has_value()) {
            if (other.has_value()) {
                this->value_ = FLOW_FWD(other).get();
            } else {
                hard_reset();
            }
        } else if (other.has_value()){
            construct(FLOW_FWD(other).get());
        }
    }

    constexpr bool has_value() const
    {
        return this->has_value_;
    }

    constexpr T& get() & { return this->value_; }
    constexpr T const& get() const& { return this->value_; }
    constexpr T&& get() && { return std::move(this->value_); }
    constexpr T&& get() const&& { return std::move(this->value); }
};

template <typename T, bool = std::is_trivially_copy_constructible_v<T>>
struct maybe_copy_base : maybe_ops_base<T>
{
    using maybe_ops_base<T>::maybe_ops_base;

    maybe_copy_base() = default;

    constexpr maybe_copy_base(maybe_copy_base const& other)
    {
        if (other.has_value()) {
            this->construct(other.get());
        } else {
            this->has_value_ = false;
        }
    }

    maybe_copy_base(maybe_copy_base&&) = default;
    maybe_copy_base& operator=(maybe_copy_base const&) = default;
    maybe_copy_base& operator=(maybe_copy_base&&) = default;
};

template <typename T>
struct maybe_copy_base<T, /*std::is_trivially_copy_constructible =*/ true>
    : maybe_ops_base<T>
{
    using maybe_ops_base<T>::maybe_ops_base;
};


template <typename T, bool = std::is_trivially_move_constructible_v<T>>
struct maybe_move_base : maybe_copy_base<T>
{
    using maybe_copy_base<T>::maybe_copy_base;

    maybe_move_base() = default;

    maybe_move_base(maybe_move_base const&) = default;

    constexpr maybe_move_base(maybe_move_base&& other)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        if (other.has_value()) {
            this->construct(std::move(other).get());
        } else {
            this->has_value_ = false;
        }
    }

    maybe_move_base& operator=(maybe_move_base const&) = default;
    maybe_move_base& operator=(maybe_move_base&&) = default;
};

template <typename T>
struct maybe_move_base<T, /*is_trivially_move_constructible = */true>
    : maybe_copy_base<T>
{
    using maybe_copy_base<T>::maybe_copy_base;
};

template <typename T, bool = std::is_trivially_copy_assignable_v<T>>
struct maybe_copy_assign_base : maybe_move_base<T>
{
    using maybe_move_base<T>::maybe_move_base;

    maybe_copy_assign_base() = default;

    maybe_copy_assign_base(maybe_copy_assign_base const&) = default;

    maybe_copy_assign_base(maybe_copy_assign_base&&) = default;

    constexpr maybe_copy_assign_base&
    operator=(maybe_copy_assign_base const& other)
    {
        this->assign(other);
        return *this;
    }

    maybe_copy_assign_base& operator=(maybe_copy_assign_base&&) = default;
};

template <typename T>
struct maybe_copy_assign_base<T, /*is_trivially_copy_assignable = */true>
    : maybe_move_base<T>
{
    using maybe_move_base<T>::maybe_move_base;
};

template <typename T, bool = std::is_trivially_move_assignable_v<T>>
struct maybe_move_assign_base : maybe_copy_assign_base<T>
{
    using maybe_copy_assign_base<T>::maybe_copy_assign_base;

    maybe_move_assign_base() = default;

    maybe_move_assign_base(maybe_move_assign_base const&) = default;

    maybe_move_assign_base(maybe_move_assign_base&&) = default;

    maybe_move_assign_base& operator=(maybe_move_assign_base const&) = default;

    constexpr maybe_move_assign_base& operator=(maybe_move_assign_base&& other)
    noexcept(std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_move_assignable_v<T>)
    {
        this->assign(std::move(other));
        return *this;
    }
};

template <typename T>
struct maybe_move_assign_base<T, /*is_trivially_move_assignable = */ true>
    : maybe_copy_assign_base<T>
{
    using maybe_copy_assign_base<T>::maybe_copy_assign_base;
};

template <typename T,
    bool EnableCopy = std::is_copy_constructible_v<T>,
    bool EnableMove = std::is_move_constructible_v<T>>
struct maybe_delete_ctor_base {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T>
struct maybe_delete_ctor_base<T, false, true> {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = delete;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T>
struct maybe_delete_ctor_base<T, true, false> {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = delete;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T>
struct maybe_delete_ctor_base<T, false, false> {
    maybe_delete_ctor_base() = default;
    maybe_delete_ctor_base(maybe_delete_ctor_base const&) = delete;
    maybe_delete_ctor_base(maybe_delete_ctor_base&&) = delete;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base const&) = default;
    maybe_delete_ctor_base& operator=(maybe_delete_ctor_base&&) = default;
};

template <typename T,
    bool EnableCopy = std::is_copy_assignable_v<T>,
    bool EnableMove = std::is_move_assignable_v<T>>
struct maybe_delete_assign_base {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = default;
};

template <typename T>
struct maybe_delete_assign_base<T, false, true> {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = delete;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = default;
};

template <typename T>
struct maybe_delete_assign_base<T, true, false> {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = delete;
};

template <typename T>
struct maybe_delete_assign_base<T, false, false> {
    maybe_delete_assign_base() = default;
    maybe_delete_assign_base(maybe_delete_assign_base const&) = default;
    maybe_delete_assign_base(maybe_delete_assign_base&&) = default;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base const&) = delete;
    maybe_delete_assign_base& operator=(maybe_delete_assign_base&&) = delete;
};

} // namespace detail

template <typename T>
class maybe : detail::maybe_move_assign_base<T>,
              detail::maybe_delete_ctor_base<T>,
              detail::maybe_delete_assign_base<T>
{
    using base = detail::maybe_move_assign_base<T>;

public:
    using value_type = T;

    maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U const& other)
        : base(detail::in_place_t{}, other)
    {}

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U&& other)
        : base(detail::in_place_t{}, FLOW_FWD(other))
    {}

    constexpr T& operator*() & { return this->get(); }
    constexpr T const& operator*() const& { return this->get(); }
    constexpr T&& operator*() && { return std::move(this->get()); }
    constexpr T const&& operator*() const&& { return std::move(this->get()); }

    constexpr T* operator->() { return std::addressof(**this); }
    constexpr T const* operator->() const { return std::addressof(**this); }

    constexpr T& value() & { return value_impl(*this); }
    constexpr T const& value() const& { return value_impl(*this); }
    constexpr T&& value() && { return value_impl(std::move(*this)); }
    constexpr T const&& value() const&& { return value_impl(std::move(*this)); }

    using base::has_value;

    constexpr explicit operator bool() const { return has_value(); }

    constexpr void reset() { this->hard_reset(); }

    // I really want "deducing this"...
    template <typename U>
    constexpr auto value_or(U&& arg) &
        -> decltype(has_value() ? std::declval<T&>() : FLOW_FWD(arg))
    {
        return has_value() ? **this : FLOW_FWD(arg);
    }

    template <typename U>
    constexpr auto value_or(U&& arg) const&
        -> decltype(has_value() ? std::declval<T const&>() : FLOW_FWD(arg))
    {
        return has_value() ? **this : FLOW_FWD(arg);
    }

    template <typename U>
    constexpr auto value_or(U&& arg) &&
        -> decltype(has_value() ? std::declval<T&&>() : FLOW_FWD(arg))
    {
        return has_value() ? *std::move(*this) : FLOW_FWD(arg);
    }

    template <typename U>
    constexpr auto value_or(U&& arg) const&&
        -> decltype(has_value() ? std::declval<T const&&>() : FLOW_FWD(arg))
    {
        return has_value() ? *std::move(*this) : FLOW_FWD(arg);
    }

    template <typename Func>
    constexpr auto map(Func&& func) & { return map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const& { return map_impl(*this, FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) && { return map_impl(std::move(*this),  FLOW_FWD(func)); }
    template <typename Func>
    constexpr auto map(Func&& func) const&& { return map_impl(std::move(*this), FLOW_FWD(func)); }

private:
    template <typename Self>
    constexpr static auto value_impl(Self&& self) -> decltype(auto)
    {
        if (self.has_value()) {
            return *FLOW_FWD(self);
        } else {
            throw bad_maybe_access{};
        }
    }

    template <typename Self, typename Func>
    constexpr static auto map_impl(Self&& self, Func&& func)
        -> maybe<std::invoke_result_t<Func, decltype(*FLOW_FWD(self))>>
    {
        if (self.has_value()) {
            return {FLOW_FWD(func)(*FLOW_FWD(self))};
        } else {
            return {};
        }
    }
};


template <typename T>
class maybe<T&> {
    T* ptr_ = nullptr;

public:
    using value_type = T&;

    maybe() = default;

    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    constexpr maybe(U& item) : ptr_(std::addressof(item)) {}

    maybe(T&&) = delete;

    constexpr T& operator*() const { return *ptr_; }
    constexpr T* operator->() const { return ptr_; }

    constexpr T& value() const
    {
        if (!ptr_) {
            throw bad_maybe_access{};
        }
        return *ptr_;
    }

    template <typename U>
    constexpr auto value_or(U&& arg) const
    -> decltype(ptr_ ? *ptr_ : FLOW_FWD(arg))
    {
        return ptr_ ? *ptr_ : FLOW_FWD(arg);
    }

    constexpr auto reset() { ptr_ = nullptr; }

    constexpr auto has_value() const -> bool { return ptr_ != nullptr; }
    explicit constexpr operator bool() const { return has_value(); }

    template <typename Func>
    constexpr auto map(Func&& func) const -> maybe<std::invoke_result_t<Func, T&>>
    {
        if (ptr_) {
            return {invoke(FLOW_FWD(func), *ptr_)};
        }
        return {};
    }
};

} // namespace flow

#endif


#ifndef FLOW_CORE_COMPARATORS_HPP_INCLUDED
#define FLOW_CORE_COMPARATORS_HPP_INCLUDED




#include <type_traits>

namespace flow::pred {

namespace detail {

template <typename Lambda>
struct predicate : Lambda {};

template <typename Lambda>
predicate(Lambda&&) -> predicate<Lambda>;

template <typename Lambda>
constexpr auto make_predicate(Lambda&& lambda)
{
    return predicate<Lambda>{FLOW_FWD(lambda)};
}

// This could/should be a lambda, but it confuses MSVC
template <typename Op>
struct cmp {
    template <typename T>
    constexpr auto operator()(T&& val) const
    {
        return predicate{[val = FLOW_FWD(val)](const auto& other) {
            return Op{}(other, val);
        }};
    }
};

} // namespace detail

/// Given a predicate, returns a new predicate with the condition reversed
inline constexpr auto not_ = [](auto&& pred) {
    return detail::make_predicate([p = FLOW_FWD(pred)] (auto const&... args) {
        return !invoke(p, FLOW_FWD(args)...);
    });
};

/// Returns a new predicate which is satisifed only if both the given predicates
/// return `true`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `false`, the second will not be evaluated.
inline constexpr auto both = [](auto&& p, auto&& and_) {
    return detail::predicate{[p1 = FLOW_FWD(p), p2 = FLOW_FWD(and_)] (auto const&... args) {
        return invoke(p1, args...) && invoke(p2, args...);
    }};
};

/// Returns a new predicate which is satisifed only if either of the given
/// predicates return `true`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `true`, the second will not be evaluated
inline constexpr auto either = [](auto&& p, auto&& or_) {
     return detail::predicate{[p1 = FLOW_FWD(p), p2 = FLOW_FWD(or_)] (auto const&... args) {
        return invoke(p1, args...) || invoke(p2, args...);
     }};
};

namespace detail {

template <typename P>
constexpr auto operator!(detail::predicate<P> pred)
{
    return not_(std::move(pred));
}

template <typename L, typename R>
constexpr auto operator&&(detail::predicate<L> lhs, detail::predicate<R> rhs)
{
    return both(std::move(lhs), std::move(rhs));
}

template <typename L, typename R>
constexpr auto operator||(detail::predicate<L> lhs, detail::predicate<R> rhs)
{
    return either(std::move(lhs), std::move(rhs));
}

}

/// Returns a new predicate with is satified only if both of the given
/// predicates return `false`.
///
/// The returned predicate is short-circuiting: if the first predicate returns
/// `true`, the second will not be evaluated.
inline constexpr auto neither = [](auto&& p1, auto&& nor) {
    return not_(either(FLOW_FWD(p1), FLOW_FWD(nor)));
};

inline constexpr auto eq = detail::cmp<flow::equal_to>{};
inline constexpr auto neq = detail::cmp<flow::not_equal_to>{};
inline constexpr auto lt = detail::cmp<flow::less>{};
inline constexpr auto gt = detail::cmp<flow::greater>{};
inline constexpr auto leq = detail::cmp<flow::less_equal>{};
inline constexpr auto geq = detail::cmp<flow::greater_equal>{};

/// Returns true if the given value is greater than a zero of the same type.
inline constexpr auto positive = detail::make_predicate([](auto const& val) -> bool {
    return val > decltype(val){0};
});

/// Returns true if the given value is less than a zero of the same type.
inline constexpr auto negative = detail::make_predicate([](auto const& val) -> bool {
    return val < decltype(val){0};
});

/// Returns true if the given value is not equal to a zero of the same type.
inline constexpr auto nonzero = detail::make_predicate([](auto const& val) -> bool {
    return val != decltype(val){0};
});

/// Given a sequence of values, constructs a predicate which returns true
/// if its argument compares equal to one of the values
inline constexpr auto in = [](auto const&... vals) {
    static_assert(sizeof...(vals) > 0);
    return detail::predicate{[vals...](auto const& arg) -> decltype(auto) {
        return (equal_to{}(arg, vals) || ...);
    }};
};

inline constexpr auto even = detail::make_predicate([](auto const& val) -> bool {
    return val % decltype(val){2} == decltype(val){0};
});

inline constexpr auto odd = detail::make_predicate([](auto const& val) -> bool {
  return val % decltype(val){2} != decltype(val){0};
});

} // namespaces

#endif


#ifndef FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED
#define FLOW_CORE_TYPE_TRAITS_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace flow {

template <typename T>
using remove_cvref_t = std::remove_const_t<std::remove_reference_t<T>>;

template <typename T>
using remove_rref_t = std::conditional_t<
    std::is_rvalue_reference_v<T>, std::remove_reference_t<T>, T>;

template <typename F>
using next_t = decltype(std::declval<F>().next());

template <typename F>
using item_t = typename next_t<F>::value_type;

template <typename F>
using value_t = remove_cvref_t<item_t<F>>;

template <typename F>
using subflow_t = decltype(std::declval<F&>().subflow());

using dist_t = std::make_signed_t<std::size_t>;

template <typename Derived>
struct flow_base;

template <typename>
class maybe;

namespace detail {

template <typename>
inline constexpr bool is_maybe = false;

template <typename T>
inline constexpr bool is_maybe<maybe<T>> = true;

template <typename, typename = void>
inline constexpr bool has_next = false;

template <typename T>
inline constexpr bool has_next<T, std::enable_if_t<is_maybe<next_t<T>>>> = true;

} // namespace detail

template <typename I, typename R = std::remove_reference_t<I>>
inline constexpr bool is_flow
    = std::is_move_constructible_v<R> &&
      std::is_base_of_v<flow_base<R>, R> &&
      std::is_convertible_v<std::add_lvalue_reference_t<I>, flow_base<R>&> &&
      detail::has_next<I>;

namespace detail {

template <typename, typename = void>
inline constexpr bool has_subflow = false;

template <typename T>
inline constexpr bool has_subflow<
    T, std::enable_if_t<is_flow<subflow_t<T>>>> = true;

} // namespace detail

template <typename F>
inline constexpr bool is_multipass_flow = is_flow<F> && detail::has_subflow<F>;

namespace detail {

template <typename, typename = void>
inline constexpr bool has_size = false;

template <typename T>
inline constexpr bool has_size<
    T, std::void_t<decltype(std::declval<T const&>().size())>> = true;

template <typename, typename = void>
inline constexpr bool is_infinite = false;

template <typename T>
inline constexpr bool is_infinite<T, std::enable_if_t<T::is_infinite>> = true;

}

template <typename F>
inline constexpr bool is_sized_flow = is_flow<F> && detail::has_size<F>;

template <typename F>
inline constexpr bool is_infinite_flow = is_flow<F> && detail::is_infinite<F>;

namespace detail {

template <typename, typename = void>
inline constexpr bool has_next_back = false;

template <typename T>
inline constexpr bool has_next_back<T, std::enable_if_t<
    std::is_same_v<next_t<T>, decltype(std::declval<T>().next_back())>>> = true;

}

template <typename F>
inline constexpr bool is_reversible_flow = is_flow<F> && detail::has_next_back<F>;

} // namespace flow

#endif



#ifndef FLOW_SOURCE_FROM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_HPP_INCLUDED

#include <cassert>
#include <iterator>

namespace flow {

namespace detail {

template <typename R>
using iterator_t = decltype(std::begin(std::declval<R&>()));

template <typename R>
using sentinel_t = decltype(std::end(std::declval<R&>()));

template <typename R>
using iter_reference_t = decltype(*std::declval<iterator_t<R>&>());

template <typename R>
using iter_value_t = typename std::iterator_traits<iterator_t<R>>::value_type;

template <typename R>
using iter_category_t =
    typename std::iterator_traits<iterator_t<R>>::iterator_category;

template <typename, typename = void>
inline constexpr bool is_stl_range = false;

template <typename R>
inline constexpr bool is_stl_range<
    R, std::void_t<iterator_t<R>, sentinel_t<R>, iter_category_t<R>>> = true;

template <typename R>
inline constexpr bool is_fwd_stl_range =
    std::is_base_of_v<std::forward_iterator_tag, iter_category_t<R>>;

template <typename R>
inline constexpr bool is_bidir_stl_range =
    std::is_base_of_v<std::bidirectional_iterator_tag, iter_category_t<R>>;

template <typename R>
inline constexpr bool is_random_access_stl_range =
    std::is_base_of_v<std::random_access_iterator_tag, iter_category_t<R>>;

template <typename R>
struct range_ref {
    constexpr range_ref(R& rng) : ptr_(std::addressof(rng)) {}

    constexpr auto begin() const { return std::begin(*ptr_); }
    constexpr auto end() const { return std::end(*ptr_); }

private:
    R* ptr_;
};

template <typename>
inline constexpr bool is_range_ref = false;

template <typename R>
inline constexpr bool is_range_ref<range_ref<R>> = true;

template <typename R>
struct stl_input_range_adaptor : flow_base<stl_input_range_adaptor<R>> {
private:
    R rng_;
    iterator_t<R> cur_ = std::begin(rng_);

public:
    constexpr explicit stl_input_range_adaptor(R&& range)
        : rng_(std::move(range))
    {}

    // We are move-only
    constexpr stl_input_range_adaptor(stl_input_range_adaptor&&) = default;
    constexpr stl_input_range_adaptor&
    operator=(stl_input_range_adaptor&&) = default;

    constexpr auto next() -> maybe<iter_value_t<R>>
    {
        if (cur_ != std::end(rng_)) {
            auto val = *cur_;
            ++cur_;
            return {std::move(val)};
        }
        return {};
    }

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(cur_, std::end(rng_));
        }
    }
};

template <typename R>
struct stl_fwd_range_adaptor : flow_base<stl_fwd_range_adaptor<R>> {
public:
    constexpr explicit stl_fwd_range_adaptor(R&& range) : rng_(std::move(range))
    {}

    constexpr auto next() -> maybe<detail::iter_reference_t<R>>
    {
        if (cur_ != std::end(rng_)) {
            return {*cur_++};
        }
        return {};
    }

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(cur_, std::end(rng_));
        }
    }

    constexpr auto subflow() &
    {
        if  constexpr (is_range_ref<R>) {
            return *this;
        } else {
            auto s = stl_fwd_range_adaptor<range_ref<R>>(rng_);
            s.cur_ = cur_;
            return s;
        }
    }

private:
    template <typename>
    friend struct stl_fwd_range_adaptor;

    R rng_;
    iterator_t<R> cur_ = std::begin(rng_);
};

template <typename R>
struct stl_bidir_range_adaptor : flow_base<stl_bidir_range_adaptor<R>> {

    constexpr explicit stl_bidir_range_adaptor(R&& rng)
        : rng_(std::move(rng))
    {}

    constexpr auto next() -> maybe<iter_reference_t<R>>
    {
        if (first_ != last_) {
            return {*first_++};
        }
        return {};
    }

    constexpr auto next_back() -> maybe<iter_reference_t<R>>
    {
        if (first_ != last_) {
            return {*--last_};
        }
        return {};
    }

    template <typename C>
    constexpr auto to() &&
    {
        return C(first_, last_);
    }

    constexpr auto subflow() &
    {
        if  constexpr (is_range_ref<R>) {
            return *this;
        } else {
            auto s = stl_bidir_range_adaptor<range_ref<R>>(rng_);
            s.first_ = first_;
            s.last_ = last_;
            return s;
        }
    }

private:
    R rng_;
    iterator_t<R> first_ = std::begin(rng_);
    iterator_t<R> last_ = std::end(rng_);
};

template <typename R>
struct stl_ra_range_adaptor : flow_base<stl_ra_range_adaptor<R>> {

    template <typename RR = R, std::enable_if_t<!std::is_array_v<RR>, int> = 0>
    constexpr explicit stl_ra_range_adaptor(R&& rng) : rng_(FLOW_FWD(rng))
    {}

    // Why would anyone ever try to use a raw language array with flow::from?
    // I don't know, but let's try to support them in their foolish quest anyway
    template <typename T, std::size_t N>
    constexpr explicit stl_ra_range_adaptor(T(&&arr)[N])
        : idx_back_(N)
    {
        for (std::size_t i = 0; i < N; i++) {
            rng_[i] = std::move(arr)[i];
        }
    }

    constexpr auto next() -> maybe<iter_reference_t<R>>
    {
        if (idx_ < idx_back_) {
            return {std::begin(rng_)[idx_++]};
        }
        return {};
    }

    constexpr auto next_back() -> maybe<iter_reference_t<R>>
    {
        if (idx_back_ > idx_ ) {
            return {std::begin(rng_)[--idx_back_]};
        }
        return {};
    }

    constexpr auto advance(dist_t dist)
    {
        assert(dist > 0);
        idx_ += dist - 1;
        return next();
    }

    constexpr auto to_range() &&
    {
        return std::move(rng_);
    }

    template <typename C>
    constexpr auto to() &&
    {
        if constexpr (std::is_same_v<C, R>) {
            return std::move(rng_);
        } else {
            return C(std::begin(rng_) + idx_, std::end(rng_));
        }
    }

    constexpr auto subflow() &
    {
        if constexpr (is_range_ref<R>) {
            return *this; // just copy
        } else {
            auto f = stl_ra_range_adaptor<range_ref<R>>(rng_);
            f.idx_ = idx_;
            return f;
        }
    }

    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return idx_back_ - idx_;
    }

private:
    template <typename>
    friend struct stl_ra_range_adaptor;

    R rng_{};
    dist_t idx_ = 0;
    dist_t idx_back_ = std::distance(std::begin(rng_), std::end(rng_));
};

template <typename R, typename = std::enable_if_t<detail::is_stl_range<R>>>
constexpr auto to_flow(R&& range)
{
    using rng_t =
        std::conditional_t<std::is_lvalue_reference_v<R>,
                           detail::range_ref<std::remove_reference_t<R>>, R>;

    if constexpr (detail::is_random_access_stl_range<R>) {
        return detail::stl_ra_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else if constexpr (detail::is_bidir_stl_range<R>) {
        return detail::stl_bidir_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else if constexpr (detail::is_fwd_stl_range<R>) {
        return detail::stl_fwd_range_adaptor<rng_t>{FLOW_FWD(range)};
    } else {
        static_assert(
            !std::is_lvalue_reference_v<R>,
            "Input ranges must be passed as rvalues -- use std::move()");
        return detail::stl_input_range_adaptor<R>{FLOW_FWD(range)};
    }
}

template <typename, typename = void>
inline constexpr bool has_member_to_flow = false;

template <typename T>
inline constexpr bool has_member_to_flow<
    T, std::enable_if_t<is_flow<decltype(std::declval<T>().to_flow())>>> = true;

template <typename, typename = void>
inline constexpr bool has_adl_to_flow = false;

template <typename T>
inline constexpr bool has_adl_to_flow<
    T, std::enable_if_t<is_flow<decltype(to_flow(std::declval<T>()))>>> = true;

} // namespace detail

template <typename T>
inline constexpr bool is_flowable =
    detail::has_member_to_flow<T> || detail::has_adl_to_flow<T>;

namespace detail {

struct from_fn {
    template <typename T, typename = std::enable_if_t<is_flowable<T>>>
    constexpr auto operator()(T&& t) const  -> decltype(auto)
    {
        if constexpr (detail::has_member_to_flow<T>) {
            return FLOW_FWD(t).to_flow();
        } else if constexpr (detail::has_adl_to_flow<T>) {
            return to_flow(FLOW_FWD(t));
        }
    }
};

} // namespace detail

inline constexpr detail::from_fn from{};

template <typename T>
using flow_t = decltype(from(std::declval<T>()));

template <typename T>
using flow_item_t = item_t<flow_t<T>>;

template <typename T>
using flow_value_t = value_t<flow_t<T>>;

} // namespace flow

#endif


#include <cassert>
#include <iosfwd>  // for stream_to()
#include <string>  // for to_string()
#include <vector>  // for to_vector()

namespace flow {

template <typename Derived>
struct flow_base {

private:
    constexpr auto derived() & -> Derived& { return static_cast<Derived&>(*this); }
    constexpr auto consume() -> Derived&& { return static_cast<Derived&&>(*this); }

    friend constexpr auto to_flow(flow_base& self) -> Derived& { return static_cast<Derived&>(self); }
    friend constexpr auto to_flow(flow_base const& self) -> Derived const& { return static_cast<Derived const&>(self); }
    friend constexpr auto to_flow(flow_base&& self) { return self.consume(); }

protected:
    ~flow_base() = default;

public:
    template <typename Adaptor, typename... Args>
    constexpr auto apply(Adaptor&& adaptor, Args&&... args) && -> decltype(auto)
    {
        return invoke(FLOW_FWD(adaptor), consume(), FLOW_FWD(args)...);
    }

    template <typename D = Derived>
    constexpr auto advance(dist_t dist) -> next_t<D>
    {
        assert(dist > 0);
        for (dist_t i = 0; i < dist - 1; i++) {
            derived().next();
        }

        return derived().next();
    }

    template <typename D = Derived,
              typename = std::enable_if_t<std::is_copy_constructible_v<D>>>
    constexpr auto subflow() & -> D
    {
        return derived();
    }

    /// Short-circuiting left fold operation.
    ///
    /// Given a function `func` and an initial value `init`, repeatedly calls
    /// `func(std::move(init), next())` and assigns the result to `init`. If
    /// `init` then evaluates to `false`, it immediately exits, returning
    /// the accumulated value. For an empty flow, it simply returns the initial
    /// value.
    ///
    /// Note that the type of the second parameter to `func` differs from plain
    /// fold(): this version takes a `maybe`, which fold() unwraps for
    /// convenience.
    ///
    /// Because this function is short-circuiting, the flow may be restarted
    /// after it has returned and the remaining items (if any) may be processed.
    ///
    /// This is the lowest-level reduction operation, on which all the other
    /// reductions are (eventually) based. For most user code, one of the
    /// higher-level operations is usually more convenient.
    ///
    /// @param func Callable with signature compatible with `(Init, maybe<next_t>) -> Init`
    /// @param init Initial value for the reduction. Must be contextually convertible to `bool`.
    /// @return The value accumulated at the point that the flow was exhausted or `(bool) init`
    ///         evaluated to `false`.
    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init;

    /// Short-circuiting version of for_each().
    ///
    /// Given a unary function `func`, repeatedly calls `func(next())`. If the
    /// return value of the function evaluates to `false`, it immediately exits,
    /// providing the last function return value.
    ///
    /// try_for_each() requires that the return type of `func` is "bool-ish":
    /// that it is default-constructible, move-assignable, and contextually
    /// convertible to `bool`.
    ///
    /// Note that the type of the parameter to `func` differs from plain
    /// for_each(): this version takes a #flow::maybe, which for_each() unwraps for
    /// convenience.
    ///
    /// Because this function is short-circuiting, the flow may be restarted
    /// after it has returned and the remaining items (if any) may be processed.
    ///
    /// This is a low-level operation, effectively a stateless version of
    /// try_fold(). For most user code, higher-level functions such as
    /// for_each() will be more convenient.
    ///
    /// @param func Callable with signature compatible with `(next_t<Flow>) -> T`,
    ///             where `T` is "bool-ish"
    /// @returns A copy of `func`
    template <typename Func>
    constexpr auto try_for_each(Func func);

    /// Exhausts the flow, performing a functional left fold operation.
    ///
    /// Given a callable `func` and an initial value `init`, calls
    /// `init = func(std::move(init), i)` for each item `i` in the flow. Once
    /// the flow is exhausted, it returns the value accumulated in `init`.
    ///
    /// This is the same operation as `std::accumulate`.
    ///
    /// @param func Callable with signature compatible with `(Init, item_t<Flow>) -> Init`
    /// @param init Initial value of the reduction
    /// @returns The accumulated value of type `Init`
    template <typename Func, typename Init>
    constexpr auto fold(Func func, Init init) -> Init;

    /// Exhausts the flow, performing a functional left fold operation.
    ///
    /// This is a convenience overload, equivalent to `fold(func, value_t{})`
    /// where `value_t` is the value type of the flow.
    ///
    /// @param func Callable with a signature compatible with `(value_t<Flow>, item_t<Flow>) -> value_t<Flow>`
    /// @returns The accumulated value of type `value_t<Flow>`
    template <typename Func>
    constexpr auto fold(Func func);

    /// Performs a left fold using `func`, passing the first element of the
    /// flow as the initial value of the accumulator.
    ///
    /// If the flow is empty, returns an empty `maybe`. Otherwise, returns a
    /// `maybe` containing the accumulated result.
    ///
    /// @param func Callable with a signature compatible with `(value_t<Flow>, item_t<Flow>) -> value_t<Flow>`
    /// @returns The accumulated value of the fold, wrapped in a `flow::maybe`
    template <typename Func>
    constexpr auto fold_first(Func func);

    /// Exhausts the flow, applying the given function to each element
    ///
    /// @param func A unary callable accepting this flow's item type
    /// @returns A copy of `func`
    template <typename Func>
    constexpr auto for_each(Func func) -> Func;

    /// Exhausts the flow, returning the number of items for which `pred`
    /// returned true
    ///
    /// Equivalent to `filter(pred).count()`.
    ///
    /// @param pred A predicate accepting the flow's item type
    /// @returns The number of items for which the `pred(item)` returned true
    template <typename Pred>
    constexpr auto count_if(Pred pred) -> dist_t;

    /// Exhausts the flow, returning the number of items it contained
    constexpr auto count() -> dist_t;

    /// Exhausts the flow, returning a count of the number of items which
    /// compared equal to `value`, using comparator `cmp`
    ///
    /// @param value Value which each item in the flow should be compared to
    /// @param cmp The comparator to be used, with a signature compatible with
    ///      `(T, item_t<Flow>) -> bool` (default is `std::equal<>`)
    /// @returns The number of items for which `cmp(value, item)` returned `true`
    template <typename T, typename Cmp = equal_to>
    constexpr auto count(const T& value, Cmp cmp = {}) -> dist_t;

    /// Processes the flow until it finds an item that compares equal to
    /// `value`, using comparator `cmp`.
    ///
    /// If the item is not found, returns an empty `maybe`.
    ///
    /// Unlike most operations, this function is short-circuiting: it will
    /// return immediately after finding an item, after which the flow can be
    /// restarted and the remaining items (if any) can be processed.
    ///
    /// @param value Value to find
    /// @param cmp Comparator to use for the find operation, with a signature
    ///            compatible with `(T, item_t<Flow>) -> bool` (default: `std::equal<>`)
    /// @returns A flow::maybe containing the first item for which `cmp(value, item)`
    ///          returned `true`, or an empty `maybe` if the flow was exhausted without
    ///          finding such an item.
    template <typename T, typename Cmp = equal_to>
    constexpr auto find(const T& value, Cmp cmp = {});

    /// Returns `true` if the flow contains an item which compares equal to
    /// `value`, using comparator `cmp`.
    ///
    /// This is a convenience method, equivalent to
    /// `static_cast<bool>(my_flow.find(value, cmp))`.
    ///
    /// Unlike most operations, this function is short-circuiting: it will
    /// return immediately after finding an item, after which the flow can be
    /// restarted and the remaining items (if any) can be processed.
    ///
    /// @param value Value to find
    /// @param cmp Comparator to use for the find operation, with a signature
    ///           compatible with `(T, item_t<Flow>) -> bool` (default: `std::equal<>`)
    /// @returns `true` iff the flow contained an item for which `cmp(value, item)`
    ///          returned `true`.
    template <typename T, typename Cmp = equal_to>
    constexpr auto contains(const T& value, Cmp cmp = {}) -> bool;

    /// Exhausts the flow, returning the sum of items using `operator+`
    ///
    /// Requires that the flow's value type is default-constructible.
    ///
    /// This is a convenience method, equivalent to `fold(std::plus<>{})`
    constexpr auto sum();

    /// Exhausts the flow, returning the product of the elements using `operator*`
    ///
    /// @note The flow's value type must be constructible from a literal `1`
    constexpr auto product();

    /// Exhausts the flow, returning the smallest item according `cmp`
    ///
    /// If several items are equally minimal, returns the first. If the flow
    /// is empty, returns an empty `maybe`.
    template <typename Cmp = less>
    constexpr auto min(Cmp cmp = Cmp{});

    /// Exhausts the flow, returning the largest item according to `cmp`
    ///
    /// If several items are equal maximal, returns the last. If the flow
    /// is empty, returns an empty `maybe`.
    template <typename Cmp = less>
    constexpr auto max(Cmp cmp = Cmp{});

    /// Exhausts the flow, returning both the minimum and maximum values
    /// according to `cmp`.
    ///
    /// If several items are equal minimal, returns the first. If several items
    /// are equally maximal, returns the last. If the flow is empty, returns
    /// an empty `maybe`.
    template <typename Cmp = less>
    constexpr auto minmax(Cmp cmp = Cmp{});

    /// Processes the flow, returning true if all the items satisfy the predicate.
    /// Returns `true` for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when an item fails to satisfy the predicate.
    template <typename Pred>
    constexpr auto all(Pred pred) -> bool;

    /// Processes the flow, returning false if any element satisfies the predicate.
    /// Returns `true` for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when an item satisfies the predicate.
    template <typename Pred>
    constexpr auto none(Pred pred) -> bool;

    /// Processes the flow, returning true if any element satisfies the predicate.
    /// Returns `false` for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when an item satisfies the predicate.
    template <typename Pred>
    constexpr auto any(Pred pred) -> bool;

    /// Processes the flow, returning true if the elements are sorted according to
    /// the given comparator, defaulting to std::less<>.
    ///
    /// Returns true for an empty flow.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flow when it finds an item which is not in sorted order.
    template <typename Cmp = less>
    constexpr auto is_sorted(Cmp cmp = Cmp{}) -> bool;

    /// Processes the flows, returning true if both flows contain equal items
    /// (according to `cmp`), and both flows end at the same time.
    ///
    /// Unlike most operations, this function is short-circuiting. It will stop
    /// processing the flows when it finds the first non-equal pair of items.
    ///
    /// @param flowable
    /// @param cmp Comparator to use, defaulting to std::equal_to<>
    /// @return True if the flows contain equal items, and false otherwise
    template <typename Flowable, typename Cmp = equal_to>
    constexpr auto equal(Flowable&& flowable, Cmp cmp = Cmp{}) -> bool;

    /// Given a reversible flow, returns a new flow which processes items
    /// from back to front.
    ///
    /// The returned adaptor effectively swaps the actions of the `next()` and
    /// `next_back()` functions.
    ///
    /// @return A new flow which runs in the opposite direction
    constexpr auto reverse() &&;

    template <typename Func,
              typename D = Derived,
              typename Init = value_t<D>>
    constexpr auto scan(Func func, Init init = Init{}) &&;

    constexpr auto partial_sum() &&;

    /// Consumes the flow, returning a new flow which lazily invokes the given
    /// callable for each item as it is processed.
    ///
    /// Another way of thinking about it is that `map` takes a flow whose item
    /// type is `T` and turns it into a flow whose item type is `R`, where `R`
    /// is the result of applying `func` to each item.
    ///
    /// It is equivalent to C++20 `std::views::transform`.
    ///
    /// @param func A callable with signature compatible with `(item_t<F>) -> R`
    /// @return A new flow whose item type is `R`, the return type of `func`
    template <typename Func>
    constexpr auto map(Func func) &&;

    /// Consumes the flow, returning a new flow which casts each item to type `T`,
    /// using `static_cast`.
    ///
    /// Equivalent to:
    ///
    /// `map([](auto&& item) -> T { return static_cast<T>(forward(item)); })`
    ///
    /// @tparam T The type that items should be cast to. May not be `void`.
    /// @return A new flow whose item type is `T`.
    template <typename T>
    constexpr auto as() &&;

    /// Consumes the flow, returning an adaptor which dereferences each item
    /// using unary `operator*`.
    ///
    /// This is useful when you have a flow of a dereferenceable type (for
    /// example a pointer, a `flow::maybe` or a `std::optional`), and want a
    /// flow of (references to) the values they contain.
    ///
    /// Equivalent to:
    ///
    /// `map([](auto&& item) -> decltype(auto) { return *forward(item); })`
    ///
    /// @warning This function **does not** check whether the items are
    /// non-null (or equivalent) before dereferencing. See `deref()` for
    /// a safer, checking version.
    ///
    /// @return An adaptor that dereferences each item of the original flow
    constexpr auto unchecked_deref() &&;

    /// Consumes the flow, returning an adaptor which copies every item.
    ///
    /// If the flow's item type is an lvalue reference type `T&`, this returns
    /// a new flow whose item type is `remove_cvref<T>`, effectively copying each
    /// item as it is dereferenced.
    ///
    /// If the flow's item type is not an lvalue reference, this adaptor has
    /// no effect.
    ///
    /// @return A flow adaptor which copies each item of the original flow.
    constexpr auto copy() && -> decltype(auto);

    /// Consumes the flow, returning an adaptor which casts each item to
    /// an xvalue, allowing it to be moved from.
    ///
    /// If the flow's item type is an lvalue reference type `T&`, this returns
    /// a new flow whose item type is `T&&`, allowing the item to be moved from.
    ///
    /// If the flow's item type is not an lvalue reference, this adaptor has
    /// no effect.
    ///
    /// @return A flow adaptor which casts each item to an xvalue.
    constexpr auto move() && -> decltype(auto);

    /// Consumes the flow, returning an adaptor which converts each reference
    /// item to a const reference.
    ///
    /// If the flow's item type is an lvalue reference type `T&`, this returns
    /// a new flow whose item type is `const T&`.
    ///
    /// If the flow's item type is not an lvalue reference, this adaptor has
    /// no effect.
    ///
    /// @return A flow adaptor which converts each item to a const reference
    constexpr auto as_const() && -> decltype(auto);

    /// Consumes the flow, returning a new flow which yields the same items
    /// but calls @func at each call to `next()`, passing it a const reference
    /// to the item.
    ///
    /// `inspect()` is intended as an aid to debugging. It can be
    /// inserted at any point in a pipeline, and allows examining the items
    /// in the flow at that point (for example, to print them).
    template <typename Func>
    constexpr auto inspect(Func func) &&;

    /// Consumes the flow, returning a new flow containing only those items for
    /// which `pred(item)` returned `true`.
    ///
    /// @param pred Predicate with signature compatible with `(const item_t<F>&) -> bool`
    /// @return A new filter adaptor.
    template <typename Pred>
    constexpr auto filter(Pred pred) &&;

    /// Consumes the flow, returning an adaptor which dereferences each item
    /// using unary `operator*`.
    ///
    /// This is useful when you have a flow of a dereferenceable type (for
    /// example a pointer, a `flow::maybe` or a `std::optional`), and want a
    /// flow of (references to) the values they contain.
    ///
    /// Unlike `unchecked_deref()`, this adaptor first attempts to check whether
    /// item contains a value, using a static_cast to bool. If the check returns
    /// false, the item is skipped.
    ///
    /// Equivalent to:
    ///
    /// `filter([](const auto& i) { return static_cast<bool>(i); }.unchecked_deref()`
    ///
    /// @return An adaptor that dereferences each item of the original flow
    constexpr auto deref() &&;

    /// Consumes the flow, returning a new flow which skips the first `count` items.
    ///
    /// @param count The number of items to skip
    /// @return A new drop adaptor
    constexpr auto drop(dist_t count) &&;

    /// Consumes the flow, returning a new flow which skips items until `pred`
    /// returns `false`.
    ///
    /// One `pred` has returned false, `drop_while` has done its job and the
    /// rest of the items will be returned as normal.
    ///
    /// @param pred A predicate with signature compatible with `(const item_t<F>&) -> bool)`.
    /// @return A new drop_while adaptor.
    template <typename Pred>
    constexpr auto drop_while(Pred pred) &&;

    /// Consumes the flow, returning a new flow which yields at most `count` items.
    ///
    /// The returned flow may return fewer than `count` items if the underlying
    /// flow contained fewer items.
    ///
    /// The returned flow is sized if the original flow is itself sized or infinite.
    ///
    /// @param count Maximum number of items this flow should return.
    /// @return A new take adaptor
    constexpr auto take(dist_t count) &&;

    /// Consumes the flow, returning a new flow which takes items until the
    /// predicate returns `false`
    ///
    /// One `pred` has returned `false`, `take_while` has done its job and
    /// will not yield any more items.
    ///
    /// @param pred Callable with signature compatible with `(const value_t<Flow>&) -> bool`.
    /// @return A new take_while adaptor
    template <typename Pred>
    constexpr auto take_while(Pred pred) &&;

    /// Consume the flow, returning a new flow which starts at the same point and
    /// then advances by `step` positions at each call to `next()`.
    ///
    /// @param step The step size, must be >= 1
    /// @return A new stride adaptor
    constexpr auto stride(dist_t step) &&;

    /// Consumes the flow, returning a new flow which yields fixed-size flows
    /// of `window_size`, stepping by `step_size` ever iteration.
    ///
    /// If `partial_windows` is `false` (the default), then windows smaller than
    /// `window_size` at the end of the flow are ignored.
    ///
    /// If `window_size` is 1, then this is like `stride(step_size)`, except that
    /// the inner item type is a flow (whereas `stride()` flattens these).
    ///
    /// If `window_size` is equal to `step_size`, then this is equivalent to
    /// `chunk()`, except that the undersized chunk at the end may be omitted
    /// depending on the value of `partial_windows`.
    ///
    /// @param window_size The size of the windows to generate. Must be >= 1
    /// @param step_size The step size to use. Must be >= 1, default is 1
    /// @param partial_windows Whether windows of size less than `window_size`
    ///                        occurring at the end of the flow should be included.
    ///                        Default is `false`.
    /// @return A new slide adaptor
    constexpr auto slide(dist_t window_size,
                         dist_t step_size = 1,
                         bool partial_windows = false) &&;

    /// Consumes the flow, returning a new flow which endlessly repeats its items.
    ///
    /// @note Requires that the flow is copy-constructable and copy-assignable.
    ///
    /// @return A new cycle adaptor
    constexpr auto cycle() &&;

    /// Takes a set of flows and creates a new flow which iterates over each of
    /// them, one after the other.
    ///
    /// @note All flows passed to `chain` must have the exact same item type
    ///
    /// @param flowables A list of Flowable objects to chain
    /// @return A new chain adaptor
    template <typename... Flowables>
    constexpr auto chain(Flowables&&... flowables) &&;

    /// Returns an adaptor which alternates an item from the first flow, followed
    /// by an item from the second flow, followed by the the next item from
    /// the first flow, and so on.
    ///
    /// The adaptor is exhausted when either of the two flows is exhausted.
    ///
    /// @note Both flows must have the exact same item type
    ///
    /// @param with A Flowable object with which to interleave
    /// @return A new interleave adaptor
    template <typename Flowable>
    constexpr auto interleave(Flowable&& with) &&;

    /// Returns an adaptor which flattens a nested flow of Flowable objects.
    ///
    /// This is equivalent to `std::ranges::join`
    ///
    /// @return A new flatten adaptor
    constexpr auto flatten() &&;

    /// Given a callable which returns a Flowable type, applies the function
    /// to each item in the flow and then flattens the resulting flows.
    ///
    /// Equivalent to `map(func).flatten()`.
    ///
    /// @param func A callable with signature compatible with `(item_t<Flow>) -> R`,
    ///             where `R` is a Flowable type
    /// @return A new adaptor which maps then flattens
    template <typename Func>
    constexpr auto flat_map(Func func) &&;

    /// Given a set of Flowable objects, processes them in lockstep, returning
    /// a `std::pair` or `std::tuple` of their items.
    ///
    /// The resulting adaptor will be exhausted when the first of the component
    /// flows is exhausted.
    ///
    /// @param flowables A pack of flowable objects
    /// @return A new zip adaptor
    template <typename... Flowables>
    constexpr auto zip(Flowables&&... flowables) &&;

    /// Adapts the flow so that it returns (index, item) pairs.
    ///
    /// Equivalent to flow::ints().zip(*this)
    ///
    /// @return A new enumerate adaptor
    constexpr auto enumerate() &&;

    /// Given a set of flowable objects, processes them in lockstep and
    /// calls `func` passing the flows' items as arguments. The result of
    /// `func` is used as the item type of the resulting flow.
    ///
    /// Think of this an an n-ary version of `map()`.
    ///
    /// @param func Callable with signature compatible with `(item_t<Flows>...) -> R`,
    ///             where R may not be `void`
    /// @param flowables A pack of flowable objects
    /// @return A new flow whose item type is `R`.
    template <typename Func, typename... Flowables>
    constexpr auto zip_with(Func func, Flowables&&... flowables) &&;

    /// Given a callable and a pack of multipass-flowable objects, returns a new
    /// flow which calls `func` with all combinations of items from each of the
    /// flows. This can be used as an alternative to nested `for` loops.
    ///
    /// @param func Callable with signature compatible with `(item_t<Flows>...) -> R`,
    ///             where `R` may not be `void`
    /// @param flowables A pack of flowable objects, all of which must be multipass
    /// @return A new flow whose item type is `R`.
    template <typename Func, typename... Flowables>
    constexpr auto cartesian_product_with(Func func, Flowables&&... flowables) &&;

    /// Given a set of mutipass-flowable objects, returns a new flow whose item
    /// type is a `std::pair` or `std::tuple` with successive combinations of
    /// all the items of all the flows. This can be used as an alternative to
    /// nested `for` loops.
    ///
    /// @param flowables A pack of flowable objects, all of which must be multipass
    /// @return An adapted flow whose item type is a `std::tuple` or `std::pair`.
    template <typename... Flowables>
    constexpr auto cartesian_product(Flowables&&... flowables) &&;

    /// Turns a flow into a flow-of-flows, where each inner flow ("group") is
    /// delimited by the return value of `func`.
    ///
    /// The key function `func` must return a type that is equality comparable,
    /// and must always return the same output when given the same input, no
    /// matter how many times it is called.
    ///
    /// @note This adaptor requires a multipass flow
    ///
    /// \param func Callable with signature (item_t<Flow>) -> K, where K is
    ///             equality comparable
    /// \return A new group_by adaptor
    template <typename Key>
    constexpr auto group_by(Key func) &&;

    /// Consumes the flow, returning a new flow where each item is a flow
    /// containing `size` items. The last chunk may have fewer items.
    ///
    /// @note This adaptor requires a multipass flow
    ///
    /// @param size The size of each chunk. Must be greater than zero.
    /// @return A new chunk adaptor
    constexpr auto chunk(dist_t size) &&;

    /// Splits a flow into a flow-of-flows, using `delimiter`.
    ///
    /// @note This adaptor requires a multipass flow.
    ///
    /// @param delimiter The delimiter to use for the split operation
    /// @return A new split adaptor
    template <typename D = Derived>
    constexpr auto split(value_t<D> delimiter) &&;

    /// If the flow's item type is tuple-like (e.g. `std::tuple` or `std::pair`),
    /// returns an adaptor whose item type is the `N`th element of each tuple,
    /// where indexing is zero-based.
    ///
    /// @tparam N Tuple index which the adaptor should return
    /// @return A new tuple element adaptor
    template <std::size_t N>
    constexpr auto elements() &&;

    /// If the flow's item type is a "tuple-like" key-value pair, returns an
    /// adaptor whose item type is the "key" part.
    ///
    /// This is a convenience function equivalent to `elements<0>()`.
    constexpr auto keys() &&;

    /// If the flow's item type is a "tuple-like" key-value pair, returns an
    /// adaptor whose item type is the "value" part.
    ///
    /// This is a convenience function equivalent to `elements<1>()`.
    constexpr auto values() &&;

    /// Consumes the flow, returning an object that is compatible with the
    /// standard library's range protocol (that is, it has begin() and end()
    /// methods which return iterators).
    ///
    /// It is mostly useful to allow flows to be used with built-in range-for loops,
    /// or to pass flows to standard library algorithms.
    ///
    /// @return A new range object
    constexpr auto to_range() &&;

    /// Consumes the flow, converting it into a new object of type `C`.
    ///
    /// If `std::is_constructible_v<C, Flow&&>` is true, then this is equivalent
    /// to `return C(move(flow));`.
    ///
    /// Otherwise, the flow is first converted to a range, and this function
    /// returns `C(first, last);` where `first` and `last` are the iterator and
    /// sentinel types of the range
    ///
    /// @tparam C The container type to construct, for example `std::vector<int>`.
    /// @return A new object of type `C`
    template <typename C>
    constexpr auto to() && -> C;

    /// Consumes the flow, converting it into a specialisation of template `C`.
    ///
    /// This overload uses constructor template argument deduction (CTAD) to
    /// work out the container template arguments.
    ///
    /// @tparam C A class template name, for example `std::vector`.
    /// @return A new object which is a specialisation of type `C.
    template <template <typename...> typename C>
    constexpr auto to() &&;

    /// Consumes the flow, converting it into a `std::vector`.
    ///
    /// Equivalent to `to_vector<value_t<Flow>>()`.
    ///
    /// @returns: A new `std::vector<T>`, where `T` is the value type of the flow
    auto to_vector() &&;

    /// Consumes the flow, converting it into a `std::vector<T>`.
    ///
    /// Requires that the flow's value type is convertible to `T`.
    ///
    /// Equivalent to `to<std::vector<T>>()`.
    ///
    /// @tparam T The value type of the resulting vector
    /// @return A new `std::vector<T>` containing the items of this flow.
    template <typename T>
    auto to_vector() && -> std::vector<T>;

    /// Consumes the flow, converting it into a `std::string`.
    ///
    /// Requires that the flow's value type is convertible to `char`.
    ///
    /// Equivalent to `to<std::string>()`, but fractionally less typing.
    ///
    /// @return A new `std::string`.
    auto to_string() && -> std::string;

    /// Consumes the flow, collecting its items into an object which can in turn
    /// be converted to a standard library container.
    constexpr auto collect() &&;

    /// Exhausts the flow, writing each item to the given output iterator
    ///
    /// This is equivalent to the standard library's `std::copy()`.
    ///
    /// @param oiter An output iterator which is compatible with this flow's item
    /// type
    /// @return A copy of the provided iterator
    template <typename Iter>
    constexpr auto output_to(Iter oiter) -> Iter;

    /// Exhausts the flow, writing each item to the given output stream.
    ///
    /// Each item is written to the stream using `stream << separator << item`
    /// (except for the first item, which is not preceded by a separator).
    /// The separator may be any C++ type which is "output streamable". The
    /// default separator is `", "`.
    ///
    /// @param os A specialisation of `std::basic_ostream` to write to
    /// @param sep Separator to use, defaulting to `", "`.
    /// @returns The provided `basic_ostream` reference
    template <typename Sep = const char*, typename CharT, typename Traits>
    constexpr auto write_to(std::basic_ostream<CharT, Traits>& os, Sep sep = ", ")
        -> std::basic_ostream<CharT, Traits>&;
};

}

#endif





#ifndef FLOW_OP_ALL_ANY_NONE_HPP_INCLUDED
#define FLOW_OP_ALL_ANY_NONE_HPP_INCLUDED



namespace flow {

inline constexpr auto all = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::all() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).all(std::move(pred));
};

inline constexpr auto any = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::any() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).any(std::move(pred));
};

inline constexpr auto none = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::none() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).none(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::all(Pred pred) -> bool
{
    static_assert(std::is_invocable_r_v<bool, Pred&, item_t<D>>,
                  "Predicate must be callable with the Flow's item_type,"
                   " and must return bool");
    return derived().try_fold([&pred](bool /*unused*/, auto&& m) {
      return invoke(pred, *FLOW_FWD(m));
    }, true);
}

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::any(Pred pred) -> bool
{
    return !derived().none(std::move(pred));
}

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::none(Pred pred) -> bool
{
    return derived().all(pred::not_(std::move(pred)));
}

}

#endif


#ifndef FLOW_OP_CARTESIAN_PRODUCT_HPP_INCLUDED
#define FLOW_OP_CARTESIAN_PRODUCT_HPP_INCLUDED


#ifndef FLOW_OP_ZIP_HPP_INCLUDED
#define FLOW_OP_ZIP_HPP_INCLUDED



#ifndef FLOW_SOURCE_IOTA_HPP_INCLUDED
#define FLOW_SOURCE_IOTA_HPP_INCLUDED



namespace flow {

namespace detail {

inline constexpr auto pre_inc = [] (auto& val)
    -> decltype(val++) { return val++; };

inline constexpr auto iota_size_fn = [](auto const& val, auto const& bound)
    -> decltype(bound - val)
{
    return bound - val;
};

inline constexpr auto stepped_iota_size_fn =
    [](auto const& val, auto const& bound, auto const& step)
    -> decltype((bound - val)/step + ((bound - val) % step != 0))
{
    return (bound - val)/step + ((bound - val) % step != 0);
};

template <typename Val>
struct iota_flow : flow_base<iota_flow<Val>> {
    static constexpr bool is_infinite = true;

    constexpr iota_flow(Val&& val)
        : val_(std::move(val))
    {}

    constexpr auto next() -> maybe<Val>
    {
        return {val_++};
    }

private:
    Val val_;
};

template <typename Val, typename Bound>
struct bounded_iota_flow : flow_base<bounded_iota_flow<Val, Bound>> {

    constexpr bounded_iota_flow(Val&& val, Bound&& bound)
        : val_(std::move(val)),
          bound_(std::move(bound))
    {}

    constexpr auto next() -> maybe<Val>
    {
        if (val_ < bound_) {
            return {val_++};
        }
        return {};
    }

    template <bool B = std::is_same_v<Val, Bound>>
    constexpr auto next_back() -> std::enable_if_t<B, maybe<Val>>
    {
        if (val_ < bound_) {
            return {--bound_};
        }
        return {};
    }

    template <typename V = const Val&, typename B = const Bound&,
              typename = std::enable_if_t<
                  std::is_invocable_r_v<flow::dist_t, decltype(iota_size_fn), V, B>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return iota_size_fn(val_, bound_);
    }

private:
    Val val_;
    FLOW_NO_UNIQUE_ADDRESS Bound bound_;
};

template <typename Val, typename Bound, typename Step>
struct stepped_iota_flow : flow_base<stepped_iota_flow<Val, Bound, Step>> {
private:
    Val val_;
    Bound bound_;
    Step step_;
    bool step_positive_ = step_ > Step{};

public:
    constexpr stepped_iota_flow(Val val, Bound bound, Step step)
        : val_(std::move(val)),
          bound_(std::move(bound)),
          step_(std::move(step))
    {}

    constexpr auto next() -> maybe<Val>
    {
        if (step_positive_) {
            if (val_ >= bound_) {
                return {};
            }
        } else {
            if (val_ <= bound_) {
                return {};
            }
        }

        auto tmp = val_;
        val_ += step_;
        return {std::move(tmp)};
    }

    template <typename V = Val, typename B = Bound, typename S = Step,
              typename = std::enable_if_t<
                  std::is_invocable_r_v<dist_t, decltype(stepped_iota_size_fn), V&, B&, S&>
                  >>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return static_cast<dist_t>(stepped_iota_size_fn(val_, bound_, step_));
    }


};

struct iota_fn {

    template <typename Val>
    constexpr auto operator()(Val from) const
    {
        static_assert(std::is_invocable_v<decltype(pre_inc), Val&>,
                      "Type passed to flow::iota() must have a pre-increment operator++(int)");
        return iota_flow<Val>(std::move(from));
    }

    template <typename Val, typename Bound>
    constexpr auto operator()(Val from, Bound upto) const
    {
        static_assert(std::is_invocable_v<decltype(pre_inc), Val&>,
                      "Type passed to flow::iota() must have a pre-increment operator++(int)");
        return bounded_iota_flow<Val, Bound>(std::move(from), std::move(upto));
    }

    template <typename Val, typename Bound, typename Step>
    constexpr auto operator()(Val from, Bound upto, Step step) const
    {
        return stepped_iota_flow<Val, Bound, Step>(
            std::move(from), std::move(upto), std::move(step));
    }
};

struct ints_fn {

    constexpr auto operator()() const
    {
        return iota_fn{}(dist_t{0});
    }

    constexpr auto operator()(dist_t from) const
    {
        return iota_fn{}(from);
    }

    constexpr auto operator()(dist_t from, dist_t upto) const
    {
        return iota_fn{}(from, upto);
    }

    constexpr auto operator()(dist_t from, dist_t upto, dist_t step) const
    {
        return iota_fn{}(from, upto, step);
    }

};

} // namespace detail

constexpr auto iota = detail::iota_fn{};

constexpr auto ints = detail::ints_fn{};

}

#endif


namespace flow {

namespace detail {

template <typename... Flows>
struct zip_item_type {
    using type = std::tuple<item_t<Flows>...>;
};

template <typename F1, typename F2>
struct zip_item_type<F1, F2> {
    using type = std::pair<item_t<F1>, item_t<F2>>;
};

template <typename... Flows>
using zip_item_t = typename zip_item_type<Flows...>::type;

}

inline constexpr auto zip = [](auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)>,
                  "All arguments to zip() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable0))).zip(FLOW_FWD(flowables)...);
};

inline constexpr auto enumerate = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::enumerate() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).enumerate();
};

template <typename D>
template <typename... Flowables>
constexpr auto flow_base<D>::zip(Flowables&&... flowables) &&
{
    static_assert((is_flowable<Flowables> && ...),
                   "All arguments to zip() must be Flowable");

    return consume().zip_with([](auto&&... items) {
        return detail::zip_item_t<D, flow_t<Flowables>...>(FLOW_FWD(items)...);
    }, FLOW_FWD(flowables)...);
}

template <typename D>
constexpr auto flow_base<D>::enumerate() &&
{
    return flow::ints().zip(consume());
}

}

#endif


namespace flow {

inline constexpr auto cartesian_product = [](auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)>,
                  "All arguments to cartesian_product() must be flowable");

    return FLOW_COPY(flow::from(FLOW_FWD(flowable0))).cartesian_product(FLOW_FWD(flowables)...);
};

template <typename D>
template <typename... Fs>
constexpr auto flow_base<D>::cartesian_product(Fs&&... flowables) &&
{
    static_assert((is_flowable<Fs> && ...),
                  "All arguments to cartesian_product() must be flowable");
    static_assert((is_multipass_flow<flow_t<Fs>> && ...),
                  "All flows passed to cartesian_product() must be multipass");

    return consume().cartesian_product_with(
            [](auto&&... items) { return detail::zip_item_t<D, flow_t<Fs>...>(FLOW_FWD(items)...); },
            FLOW_FWD(flowables)...);
}

}

#endif


#ifndef FLOW_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED
#define FLOW_OP_CARTESIAN_PRODUCT_WITH_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Func, typename Flow0, typename... Flows>
struct cartesian_product_with_adaptor
    : flow_base<cartesian_product_with_adaptor<Func, Flow0, Flows...>>
{
private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    std::tuple<Flow0, Flows...> flows_;
    std::tuple<subflow_t<Flows>...> subs_;
    std::tuple<next_t<Flow0>, next_t<Flows>...> maybes_{};

    inline static constexpr std::size_t N = 1 + sizeof...(Flows);
    using item_type = std::invoke_result_t<Func, item_t<Flow0>&, item_t<subflow_t<Flows>>&...>;

    template <std::size_t I>
    constexpr bool advance_()
    {
        if constexpr (I == 0) {
            if (!std::get<I>(maybes_)) {
                std::get<I>(maybes_) = std::get<I>(flows_).next();

                if (!std::get<I>(maybes_)) {
                    return false; // We're done
                }

                std::get<I>(subs_) = std::get<I + 1>(flows_).subflow();
            }
        } else {
            if (!std::get<I>(maybes_)) {
                std::get<I>(maybes_) = std::get<I - 1>(subs_).next();
                if (!std::get<I>(maybes_)) {
                    std::get<I - 1>(maybes_) = {};
                    return advance_<I - 1>();
                }

                if constexpr (I < N - 1) {
                    std::get<I>(subs_) = std::get<I + 1>(flows_).subflow();
                }
            }
        }

        if constexpr (I < N - 1) {
            return advance_<I + 1>();
        } else {
            return true;
        }
    }

    template <std::size_t... I>
    constexpr auto apply_(std::index_sequence<I...>) -> decltype(auto)
    {
        return invoke(func_, *std::get<I>(maybes_)..., *std::get<N - 1>(std::move(maybes_)));
    }

public:

    static constexpr bool is_infinite =
        is_infinite_flow<Flow0> || (is_infinite_flow<Flows> || ...);

    constexpr cartesian_product_with_adaptor(Func func, Flow0&& flow0, Flows&&... flows)
        : func_(std::move(func)),
          flows_(std::move(flow0), std::move(flows)...),
          subs_(flows.subflow()...)
    {}

    constexpr auto next() -> maybe<item_type>
    {
        std::get<N - 1>(maybes_) = {};

        if (!advance_<0>()) {
            return {};
        } else {
            return {apply_(std::make_index_sequence<N - 1>{})};
        }
    }

    template <bool B = is_sized_flow<Flow0> && (is_sized_flow<Flows> && ...)>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return std::apply([](auto const&... args) { return (args.size() * ...); }, flows_);
    }
};

// Specialisation for the common case of twp flows
template <typename Func, typename Flow1, typename Flow2>
struct cartesian_product_with_adaptor<Func, Flow1, Flow2>
    : flow_base<cartesian_product_with_adaptor<Func, Flow1, Flow2>>
{
private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    Flow1 f1_;
    Flow2 f2_;
    subflow_t<Flow2> s2_ = f2_.subflow();
    next_t<Flow1> m1_{};

    using item_type = std::invoke_result_t<Func, item_t<Flow1>&, item_t<Flow2>>;

public:
    static constexpr bool is_infinite = is_infinite_flow<Flow1> || is_infinite_flow<Flow2>;

    constexpr cartesian_product_with_adaptor(Func func, Flow1&& flow1, Flow2&& flow2)
        : func_(std::move(func)),
          f1_(std::move(flow1)),
          f2_(std::move(flow2))
    {}

    constexpr auto next() -> maybe<item_type>
    {
        while (true) {
            if (!m1_) {
                m1_ = f1_.next();
                if (!m1_) {
                    return {}; // we're done
                }

                s2_ = f2_.subflow();
            }

            auto m2 = s2_.next();
            if (!m2) {
                m1_.reset();
                continue;
            }

            return {invoke(func_, *m1_, *std::move(m2))};
        }
    }

    template <bool B = is_sized_flow<Flow1> && is_sized_flow<Flow2>>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return f1_.size() * f2_.size();
    }
};

}

inline constexpr auto cartesian_product_with =
    [](auto func, auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)>,
                  "All arguments to cartesian_product_with() must be Flowable");
    return flow::from(FLOW_FWD(flowable0))
      .cartesian_product_with(std::move(func), FLOW_FWD(flowables)...);
};

template <typename Derived>
template <typename Func, typename... Flowables>
constexpr auto flow_base<Derived>::cartesian_product_with(Func func, Flowables&&... flowables) &&
{
    static_assert((is_flowable<Flowables> && ...),
                  "All arguments to cartesian_product_with() must be Flowable");
    static_assert((is_multipass_flow<flow_t<Flowables>> && ...),
                  "To use cartesian_product_with(), all flows except the "
                  "first must be multipass");
    static_assert(std::is_invocable_v<Func&, item_t<Derived>&, flow_item_t<Flowables>&...>,
                  "Incompatible callable passed to cartesian_product_with()");

    return detail::cartesian_product_with_adaptor<Func, Derived, flow_t<Flowables>...>
        (std::move(func), consume(), flow::from(FLOW_FWD(flowables))...);
}

}

#endif


#ifndef FLOW_OP_CHAIN_HPP_INCLUDED
#define FLOW_OP_CHAIN_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename... Flows>
struct chain_adaptor : flow_base<chain_adaptor<Flows...>>
{
private:
    template <typename...>
    friend struct chain_adaptor;

    std::tuple<Flows...> flows_;
    std::uint8_t idx_ = 0; // Assume we never chain more than 255 flows...

    using idx_seq = std::make_index_sequence<sizeof...(Flows)>;

    using item_type = item_t<std::tuple_element_t<0, decltype(flows_)>>;

    template <std::size_t N, std::size_t... I>
    constexpr auto get_next_impl(idx_seq) -> maybe<item_type>
    {
        if constexpr (N < sizeof...(Flows)) {
            if (N == idx_) {
                return std::get<N>(flows_).next();
            } else {
                return get_next_impl<N + 1>(idx_seq{});
            }
        } else {
            return {};
        }
    }

    constexpr auto get_next() -> maybe<item_type>
    {
        return get_next_impl<0>(idx_seq{});
    }

    template <std::size_t N, typename Func, typename Init, std::size_t... I>
    constexpr auto try_fold_impl(Func& func, Init init, idx_seq)
    {
        if constexpr (N < sizeof...(Flows)) {
            if (N == idx_) {
                init = std::get<N>(flows_).try_fold(func, std::move(init));
                if (!init) {
                    return init;
                } else {
                    ++idx_;
                }
            }
            return try_fold_impl<N+1>(func, std::move(init), idx_seq{});
        } else {
            return init;
        }
    }

public:
    static constexpr bool is_infinite = (is_infinite_flow<Flows> || ...);

    constexpr explicit chain_adaptor(Flows&&... flows)
        : flows_(std::move(flows)...)
    {}

    constexpr auto next() -> maybe<item_type>
    {
        if (idx_ < sizeof...(Flows)) {
            if (auto m = get_next()) {
                return m;
            }
            ++idx_;
            return next();
        }
        return {};
    }

    template <bool B = (flow::is_sized_flow<Flows> && ...)>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return std::apply([](auto const&... args) {
            return (args.size() + ...);
        }, flows_);
    }

    template <bool B = (flow::is_multipass_flow<Flows> && ...),
              typename = std::enable_if_t<B>>
    constexpr auto subflow() &
    {
        auto s = std::apply([](auto&... args) {
            return chain_adaptor<subflow_t<Flows>...>(args.subflow()...);
        }, flows_);
        s.idx_ = idx_;
        return s;
    }

    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init
    {
        return try_fold_impl<0>(func, std::move(init), idx_seq{});
    }
};

// Specialisation for the common case of chaining two flows
template <typename Flow1, typename Flow2>
struct chain_adaptor<Flow1, Flow2> : flow_base<chain_adaptor<Flow1, Flow2>> {

    static constexpr bool is_infinite = is_infinite_flow<Flow1> || is_infinite_flow<Flow2>;

    constexpr chain_adaptor(Flow1&& flow1, Flow2&& flow2)
        : flow1_(std::move(flow1)),
          flow2_(std::move(flow2))
    {}

    constexpr auto next() -> next_t<Flow1>
    {
        if (first_)  {
            if (auto m = flow1_.next()) {
                return m;
            }
            first_ = false;
        }
        return flow2_.next();
    }

    template <typename F1 = Flow1, typename F2 = Flow2,
              typename = std::enable_if_t<is_sized_flow<F1> && is_sized_flow<F2>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return flow1_.size() + flow2_.size();
    }

    template <typename F1 = Flow1, typename F2 = Flow2>
    constexpr auto subflow() & -> chain_adaptor<subflow_t<F1>, subflow_t<F2>>
    {
        auto s = chain_adaptor<subflow_t<F1>, subflow_t<F2>>{flow1_.subflow(), flow2_.subflow()};
        s.first_ = first_;
        return s;
    }

    template <typename Func, typename Init>
    constexpr auto try_fold(Func func, Init init) -> Init
    {
        if (first_) {
            init = flow1_.try_fold(function_ref{func}, std::move(init));
            if (!init) {
                return init;
            }
            first_ = false;
        }
        return flow2_.try_fold(std::move(func), std::move(init));
    }

private:
    template <typename...>
    friend struct chain_adaptor;

    Flow1 flow1_;
    Flow2 flow2_;
    bool first_ = true;
};

}

inline constexpr auto chain = [](auto&& flowable0, auto&&... flowables)
{
    static_assert(is_flowable<decltype(flowable0)> && (is_flowable<decltype(flowables)> && ...),
                  "All arguments to flow::chain() must be Flowable types");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable0))).chain(FLOW_FWD(flowables)...);
};

template <typename D>
template <typename... Flowables>
constexpr auto flow_base<D>::chain(Flowables&&... flowables) &&
{
    static_assert((is_flowable<Flowables> && ...),
                  "Arguments to chain() must be flowable types");
    static_assert((std::is_same_v<item_t<D>, item_t<flow_t<Flowables>>> && ...),
                  "All flows passed to chain() must have the exact same item type");

    return detail::chain_adaptor<D, flow_t<Flowables>...>(
        consume(), FLOW_COPY(flow::from(FLOW_FWD(flowables)))...);
}

}

#endif


#ifndef FLOW_OP_CHUNK_HPP_INCLUDED
#define FLOW_OP_CHUNK_HPP_INCLUDED



#ifndef FLOW_OP_TAKE_HPP_INCLUDED
#define FLOW_OP_TAKE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow>
struct take_adaptor : flow_base<take_adaptor<Flow>> {

    constexpr take_adaptor(Flow&& flow, dist_t count)
        : flow_(std::move(flow)),
          count_(count)
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (count_ > 0) {
            --count_;
            return flow_.next();
        }
        return {};
    }

    constexpr auto advance(dist_t dist) -> next_t<Flow>
    {
        if (count_ >= dist) {
            count_ -= dist;
            return flow_.advance(dist);
        }
        return {};
    }

    template <bool B = is_reversible_flow<Flow> && is_sized_flow<Flow>>
    constexpr auto next_back() -> std::enable_if_t<B, next_t<Flow>>
    {
        if (size() > 0) {
            return flow_.next_back();
        }
        return {};
    }

    template <typename F = Flow,
              typename = std::enable_if_t<is_sized_flow<F> || is_infinite_flow<F>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        if constexpr (is_infinite_flow<Flow>) {
            return count_;
        } else {
            return min(flow_.size(), count_);
        }
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> take_adaptor<subflow_t<F>>
    {
        return {flow_.subflow(), count_};
    }

private:
    Flow flow_;
    dist_t count_;
};

}

inline constexpr auto take = [](auto&& flowable, dist_t count)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::take() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).take(count);
};

template <typename D>
constexpr auto flow_base<D>::take(dist_t count) &&
{
    assert(count >= 0 && "Cannot take a negative number of items!");
    return detail::take_adaptor<D>(consume(), count);
}

}

#endif


namespace flow {

namespace detail {

struct chunk_counter {

    explicit constexpr chunk_counter(dist_t size)
        : size_(size)
    {}

    template <typename T>
    constexpr auto operator()(T&& /*unused*/) -> bool
    {
        if (++counter_ < size_) {
            return last_;
        }
        counter_ = 0;
        return (last_ = !last_);
    }

private:
    const dist_t size_;
    dist_t counter_ = -1; // Begin off-track
    bool last_ = true;
};

template <typename Flow>
struct chunk_adaptor : flow_base<chunk_adaptor<Flow>>
{
    constexpr chunk_adaptor(Flow&& flow, dist_t size)
        : flow_(std::move(flow)),
          size_(size)
    {}

    constexpr auto next() -> maybe<take_adaptor<subflow_t<Flow>>>
    {
        if (done_) {
            return {};
        }

        auto image = flow_.subflow().take(size_);
        done_ = !(flow_.advance(size_).has_value() && flow_.subflow().next().has_value());
        return image;
    }

private:
    Flow flow_;
    dist_t size_;
    bool done_ = false;
};

}

inline constexpr auto chunk = [](auto&& flowable, dist_t size)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::chunk() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).chunk(size);
};

template <typename D>
constexpr auto flow_base<D>::chunk(dist_t size) &&
{
    static_assert(is_multipass_flow<D>,
                  "chunk() can only be used on multipass flows");
    assert((size > 0) && "Chunk size must be greater than zero");
    return detail::chunk_adaptor<D>(consume(), size);
}

}

#endif


#ifndef FLOW_OP_COLLECT_HPP_INCLUDED
#define FLOW_OP_COLLECT_HPP_INCLUDED



#ifndef FLOW_OP_TO_RANGE_HPP_INCLUDED
#define FLOW_OP_TO_RANGE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename F>
struct flow_range {
private:
    struct iterator {
        using value_type = value_t<F>;
        using reference = item_t<F>;
        using difference_type = dist_t;
        using pointer = value_type*;
        using iterator_category = std::input_iterator_tag;

        constexpr iterator() = default;

        constexpr explicit iterator(flow_range& parent)
            : parent_(std::addressof(parent))
        {}

        constexpr iterator& operator++()
        {
            if (parent_) {
                if (!parent_->do_next()) {
                    parent_ = nullptr;
                }
            }
            return *this;
        }

        constexpr void operator++(int) { ++*this;}

        constexpr reference operator*() const
        {
            return *std::move(parent_->item_);
        }

        constexpr pointer operator->() const
        {
            return std::addressof(*parent_->item_);
        }

        friend constexpr bool operator==(const iterator& lhs,
                                         const iterator& rhs)
        {
            return lhs.parent_ == rhs.parent_;
        }

        friend constexpr bool operator!=(const iterator& lhs,
                                         const iterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        flow_range* parent_ = nullptr;
    };

public:
    constexpr explicit flow_range(F&& flow)
        : flow_(std::move(flow)), item_(flow_.next())
    {}

    constexpr auto begin() { return iterator{*this}; }
    constexpr auto end() { return iterator{}; }

private:
    constexpr bool do_next()
    {
        item_ = flow_.next();
        return (bool) item_;
    }

    F flow_;
    maybe<item_t<F>> item_;
};

}

inline constexpr auto to_range = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::to_range() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).to_range();
};

template <typename D>
constexpr auto flow_base<D>::to_range() &&
{
    return detail::flow_range<D>{consume()};
}

}

#endif

namespace flow {

namespace detail {

template <typename Flow>
struct collector {

    constexpr explicit collector(Flow&& flow)
        : flow_(std::move(flow))
    {}

    template <typename C>
    constexpr operator C() &&
    {
        if constexpr (std::is_constructible_v<C, Flow&&>) {
            return C(std::move(flow_));
        } else {
            using iter_t = decltype(std::move(flow_).to_range().begin());
            static_assert(std::is_constructible_v<C, iter_t, iter_t>,
                          "Incompatible type on LHS of collect()");
            auto rng = std::move(flow_).to_range();
            return C(rng.begin(), rng.end());
        }
    }

private:
    Flow flow_;
};

}

template <typename Flowable>
constexpr auto collect(Flowable&& flowable)
{
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).collect();
}

template <typename Derived>
constexpr auto flow_base<Derived>::collect() &&
{
    return detail::collector<Derived>(consume());
}

}

#endif


#ifndef FLOW_OP_CONTAINS_HPP_INCLUDED
#define FLOW_OP_CONTAINS_HPP_INCLUDED


#ifndef FLOW_OP_FIND_HPP_INCLUDED
#define FLOW_OP_FIND_HPP_INCLUDED



namespace flow {

namespace detail {

struct find_op {

    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& item, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::find() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).find(item, std::move(cmp));
    }

};

} // namespace detail

inline constexpr auto find = detail::find_op{};

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::find(const T& item, Cmp cmp)
{
    struct out {
        next_t<Derived> val{};
        constexpr explicit operator bool() const { return !val; }
    };

    return derived().try_for_each([&item, &cmp](auto m) {
      return invoke(cmp, *m, item) ? out{std::move(m)} : out{};
    }).val;
}

}

#endif


namespace flow {

namespace detail {

struct contains_op {
    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& item, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::contains() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).contains(item, std::move(cmp));
    }
};

}

inline constexpr auto contains = detail::contains_op{};

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::contains(const T &item, Cmp cmp) -> bool
{
    static_assert(std::is_invocable_v<Cmp&, value_t<Derived> const&, const T&>,
        "Incompatible comparator used with contains()");
    static_assert(std::is_invocable_r_v<bool, Cmp&, value_t<Derived> const&, const T&>,
        "Comparator used with contains() must return bool");

    return derived().any([&item, &cmp] (auto const& val) {
          return invoke(cmp, val, item);
    });
}

}

#endif


#ifndef FLOW_OP_COUNT_HPP_INCLUDED
#define FLOW_OP_COUNT_HPP_INCLUDED


#ifndef FLOW_OP_COUNT_IF_HPP_INCLUDED
#define FLOW_OP_COUNT_IF_HPP_INCLUDED


#ifndef FLOW_OP_FOLD_HPP_INCLUDED
#define FLOW_OP_FOLD_HPP_INCLUDED



namespace flow {

namespace detail {

struct fold_op {

    template <typename Flowable, typename Func, typename Init>
    constexpr auto operator()(Flowable&& flowable, Func func, Init init) const
    {
        static_assert(is_flowable<Flowable>,
                "First argument to flow::fold() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold(std::move(func), std::move(init));
    }

    template <typename Flowable, typename Func>
    constexpr auto operator()(Flowable&& flowable, Func func) const
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::fold() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold(std::move(func));
    }
};

struct fold_first_op {
    template <typename Flowable, typename Func>
    constexpr auto operator()(Flowable&& flowable, Func func) const
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::fold_first must be Flowable");
        return flow::from(FLOW_FWD(flowable)).fold_first(std::move(func));
    }

};

} // namespace detail

inline constexpr auto fold = detail::fold_op{};

inline constexpr auto fold_first = detail::fold_first_op{};

template <typename Derived>
template <typename Func, typename Init>
constexpr auto flow_base<Derived>::fold(Func func, Init init) -> Init
{
    static_assert(std::is_invocable_v<Func&, Init&&, item_t<Derived>>,
                  "Incompatible callable passed to fold()");
    static_assert(std::is_assignable_v<Init&, std::invoke_result_t<Func&, Init&&, item_t<Derived>>>,
                  "Accumulator of fold() is not assignable from the result of the function");
    static_assert(!is_infinite_flow<Derived>,
                  "Cannot perform a fold over an infinite flow");

    struct always {
        Init val;
        constexpr explicit operator bool() const { return true; }
    };

    return derived().try_fold([&func](always acc, auto m) {
        return always{func(std::move(acc).val, *std::move(m))};
    }, always{std::move(init)}).val;
}

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::fold(Func func)
{
    static_assert(std::is_default_constructible_v<value_t<Derived>>,
            "This flow's value type is not default constructible. "
            "Use the fold(function, initial_value) overload instead");
    return consume().fold(std::move(func), value_t<Derived>{});
}

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::fold_first(Func func)
{
    using return_t = maybe<value_t<Derived>>;

    auto val = derived().next();
    if (!val) {
        return return_t{};
    }

    return return_t{derived().fold(std::move(func), *std::move(val))};
}

}

#endif


namespace flow {

inline constexpr auto count_if = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
        "First argument to flow::count_if must be Flowable");
    return flow::from(FLOW_FWD(flowable)).count_if(std::move(pred));
};

template <typename Derived>
template <typename Pred>
constexpr auto flow_base<Derived>::count_if(Pred pred) -> dist_t
{
    static_assert(std::is_invocable_r_v<bool, Pred&, item_t<Derived>>,
                  "Predicate must be callable with the Flow's item_type,"
                  " and must return bool");
    return consume().fold([&pred](dist_t count, auto&& val) {
      return count + static_cast<dist_t>(invoke(pred, FLOW_FWD(val)));
    }, dist_t{0});
}

} // namespace flow

#endif


namespace flow {

namespace detail {

struct count_op {

    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable) const
    {
        static_assert(is_flow<Flowable>,
            "Argument to flow::count() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).count();
    }

    template <typename Flowable, typename T, typename Cmp = std::equal_to<>>
    constexpr auto operator()(Flowable&& flowable, const T& value, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::count() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).count(value, std::move(cmp));
    }

};

} // namespace detail

inline constexpr auto count = detail::count_op{};

template <typename Derived>
constexpr auto flow_base<Derived>::count() -> dist_t
{
    return consume().count_if([](auto const& /*unused*/) {
        return true;
    });
}

template <typename Derived>
template <typename T, typename Cmp>
constexpr auto flow_base<Derived>::count(const T& item, Cmp cmp) -> dist_t
{
    using const_item_t = std::add_lvalue_reference_t<
        std::add_const_t<std::remove_reference_t<item_t<Derived>>>>;
    static_assert(std::is_invocable_r_v<bool, Cmp&, const T&, const_item_t>,
        "Incompatible comparator used with count()");

    return consume().count_if([&item, &cmp] (auto const& val) {
        return invoke(cmp, item, val);
    });
}

}

#endif



#ifndef FLOW_OP_CYCLE_HPP_INCLUDED
#define FLOW_OP_CYCLE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow>
struct cycle_adaptor : flow_base<cycle_adaptor<Flow>>
{
    static constexpr bool is_infinite = true;

    constexpr explicit cycle_adaptor(Flow&& flow)
        : flow_(std::move(flow))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        while (true) {
            if (auto m = flow_.next()) {
                return m;
            }
            flow_ = saved_;
        }
    }

private:
    Flow flow_;
    Flow saved_ = flow_;
};

}

inline constexpr auto cycle = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>);
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).cycle();
};

template <typename D>
constexpr auto flow_base<D>::cycle() &&
{
    static_assert(std::is_copy_constructible_v<D> && std::is_copy_assignable_v<D>,
                  "cycle() can only be used with flows which are copy-constructible "
                  "and copy-assignable");
    static_assert(!is_infinite_flow<D>,
                  "cycle() cannot be used with infinite flows");
    return detail::cycle_adaptor<D>(consume());
}

}

#endif


#ifndef FLOW_OP_DEREF_HPP_INCLUDED
#define FLOW_OP_DEREF_HPP_INCLUDED



namespace flow {

inline constexpr auto deref = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::deref must be a flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).deref();
};

template <typename D>
constexpr auto flow_base<D>::deref() &&
{
    const auto bool_check = [](const auto& i) -> decltype(static_cast<bool>(i)) {
        return static_cast<bool>(i);
    };

    static_assert(std::is_invocable_v<decltype(bool_check), item_t<D>>,
                  "Flow's item type is not explicitly convertible to bool");

    return consume().filter(bool_check).unchecked_deref();
}

}

#endif

#ifndef FLOW_OP_DROP_HPP_INCLUDED
#define FLOW_OP_DROP_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow>
struct drop_adaptor : flow_base<drop_adaptor<Flow>>
{
    static constexpr bool is_infinite = is_infinite_flow<Flow>;

    constexpr drop_adaptor(Flow&& flow, dist_t count)
        : flow_(std::move(flow)),
          count_(count)
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (count_ > 0) {
            (void) flow_.advance(count_);
            count_ = 0;
        }
        return flow_.next();
    }

    constexpr auto advance(dist_t dist) -> next_t<Flow>
    {
        if (count_ > 0) {
            (void) flow_.advance(count_);
            count_ = 0;
        }
        return flow_.advance(dist);
    }

    template <typename F = Flow>
    constexpr auto next_back() -> std::enable_if_t<
        is_reversible_flow<F> && is_sized_flow<F>, next_t<Flow>>
    {
        if (size() > 0) {
            return flow_.next_back();
        }
        return {};
    }

    template <typename F = Flow>
    constexpr auto size() const -> std::enable_if_t<is_sized_flow<F>, dist_t>
    {
        return max(flow_.size() - count_, dist_t{0});
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> drop_adaptor<subflow_t<F>>
    {
        return {flow_.subflow(), count_};
    }

private:
    Flow flow_;
    dist_t count_;
};

}

inline constexpr auto drop = [](auto&& flowable, dist_t count)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::drop() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).drop(count);
};

template <typename D>
constexpr auto flow_base<D>::drop(dist_t count) &&
{
    assert(count >= 0 && "Cannot drop a negative number of items");
    return detail::drop_adaptor<D>(consume(), count);
}

}

#endif


#ifndef FLOW_OP_DROP_WHILE_HPP_INCLUDED
#define FLOW_OP_DROP_WHILE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow, typename Pred>
struct drop_while_adaptor : flow_base<drop_while_adaptor<Flow, Pred>> {

    constexpr drop_while_adaptor(Flow&& flow, Pred&& pred)
        : flow_(std::move(flow)),
          pred_(std::move(pred))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (!done_) {
            while (auto m = flow_.next()) {
                if (invoke(pred_, std::as_const(*m))) {
                    continue;
                }
                done_ = true;
                return m;
            }
            return {};
        }
        return flow_.next();
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> drop_while_adaptor<subflow_t<F>, function_ref<Pred>>
    {
        auto d = drop_while_adaptor<subflow_t<Flow>, function_ref<Pred>>{flow_.subflow(), pred_};
        d.done_ = this->done_;
        return d;
    }

private:
    template <typename, typename>
    friend struct drop_while_adaptor;

    Flow flow_;
    Pred pred_;
    bool done_ = false;
};

}

inline constexpr auto drop_while = [](auto&& flowable, auto pred)
{
    static_assert(flow::is_flowable<decltype(flowable)>,
                  "Argument to flow::drop_while() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).drop_while(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::drop_while(Pred pred) &&
{
    static_assert(std::is_invocable_r_v<dist_t, Pred&, value_t<D> const&>,
                  "Incompatible predicate passed to drop_while()");
    return detail::drop_while_adaptor<D, Pred>(consume(), std::move(pred));
}

}

#endif


#ifndef FLOW_OP_EQUAL_HPP_INCLUDED
#define FLOW_OP_EQUAL_HPP_INCLUDED



namespace flow {

namespace detail {

struct equal_fn {
    template <typename F1, typename F2, typename Cmp = equal_to>
    constexpr auto operator()(F1&& f1, F2&& f2, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<F1> && is_flowable<F2>,
                      "Arguments to flow::equal() must be Flowable types");
        return flow::from(FLOW_FWD(f1)).equal(FLOW_FWD(f2), std::move(cmp));
    }

};


}

inline constexpr auto equal = detail::equal_fn{};

template <typename D>
template <typename Flowable, typename Cmp>
constexpr auto flow_base<D>::equal(Flowable&& flowable, Cmp cmp) -> bool
{
    static_assert(is_flowable<Flowable>,
                  "Argument to equal() must be a Flowable type");
    static_assert(std::is_invocable_r_v<bool, Cmp&, item_t<D>, flow_item_t<Flowable>>,
                  "Incompatible comparator passed to equal()");

    auto&& other = flow::from(FLOW_FWD(flowable));

    while (true) {
        auto m1 = derived().next();
        auto m2 = other.next();

        if (!m1) {
            return !static_cast<bool>(m2);
        }
        if (!m2) {
            return false;
        }

        if (!cmp(*m1, *m2)) {
            return false;
        }
    }
}

}

#endif


#ifndef FLOW_OP_FILTER_HPP_INCLUDED
#define FLOW_OP_FILTER_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow, typename Pred>
struct filter_adaptor : flow_base<filter_adaptor<Flow, Pred>> {

    constexpr filter_adaptor(Flow&& flow, Pred&& pred)
        : flow_(std::move(flow)),
          pred_(std::move(pred))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        while (auto m = flow_.next()) {
            if (invoke(pred_, std::as_const(*m))) {
                return m;
            }
        }
        return {};
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> filter_adaptor<subflow_t<F>, function_ref<Pred>>
    {
        return {flow_.subflow(), pred_};
    }

private:
    Flow flow_;
    Pred pred_;
};

} // namespace detail

constexpr auto filter = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::filter() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).filter(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::filter(Pred pred) &&
{
    static_assert(std::is_invocable_r_v<bool, Pred&, value_t<D> const&>,
                  "Incompatible predicate passed to filter()");

    return detail::filter_adaptor<D, Pred>(consume(), std::move(pred));
}

}

#endif



#ifndef FLOW_OP_FLATTEN_HPP_INCLUDED
#define FLOW_OP_FLATTEN_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Base>
struct flatten_adaptor : flow_base<flatten_adaptor<Base>> {

    constexpr explicit flatten_adaptor(Base&& base)
        : base_(std::move(base))
    {}

    constexpr auto next() -> next_t<flow_t<item_t<Base>>>
    {
        while (true) {
            if (!inner_) {
                inner_ = base_.next().map(flow::from);
                if (!inner_) {
                    return {};
                }
            }

            if (auto m = inner_->next()) {
                return m;
            } else {
                inner_ = {};
            }
        }
    }

    template <typename F = Base,
              typename = std::enable_if_t<is_multipass_flow<flow_t<item_t<F>>>>>
    constexpr auto subflow() -> flatten_adaptor<subflow_t<F>>
    {
        if (!inner_) {
            return flatten_adaptor<subflow_t<F>>{base_.subflow()};
        }
        return flatten_adaptor<subflow_t<F>>{base_.subflow(), inner_->subflow()};
    }

private:
    template <typename>
    friend struct flatten_adaptor;

    constexpr flatten_adaptor(Base&& base, flow_t<item_t<Base>>&& inner)
            : base_(std::move(base)),
              inner_(std::move(inner))
    {}

    Base base_;
    maybe<flow_t<item_t<Base>>> inner_{};
};

}

inline constexpr auto flatten = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::flatten() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).flatten();
};

inline constexpr auto flat_map = [](auto&& flowable, auto func)
{
  static_assert(is_flowable<decltype(flowable)>,
                "Argument to flow::flat_map must be a Flowable type");
  return FLOW_COPY(flow::from(FLOW_FWD(flowable))).flat_map(std::move(func));
};

template <typename Derived>
constexpr auto flow_base<Derived>::flatten() &&
{
    static_assert(is_flowable<item_t<Derived>>,
        "flatten() requires a flow whose item type is Flowable");
    return detail::flatten_adaptor<Derived>(consume());
}

template <typename D>
template <typename Func>
constexpr auto flow_base<D>::flat_map(Func func) &&
{
    static_assert(std::is_invocable_v<Func&, item_t<D>>,
    "Incompatible callable passed to flat_map()");
    static_assert(is_flowable<std::invoke_result_t<Func&, item_t<D>>>,
    "Function passed to flat_map() must return a Flowable type");
    return consume().map(std::move(func)).flatten();
}

}

#endif



#ifndef FLOW_OP_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_FOR_EACH_HPP_INCLUDED



namespace flow {

inline constexpr auto for_each = [](auto flowable, auto func) {
    static_assert(is_flowable<decltype(flowable)>,
        "First argument to flow::for_each() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).for_each(std::move(func));
};

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::for_each(Func func) -> Func
{
    static_assert(std::is_invocable_v<Func&, item_t<Derived>>,
                  "Incompatible callable passed to for_each()");
    consume().fold([&func](bool, auto&& val) {
      (void) invoke(func, FLOW_FWD(val));
      return true;
    }, true);
    return func;
}

}

#endif



#ifndef FLOW_OP_GROUP_BY_HPP_INCLUDED
#define FLOW_OP_GROUP_BY_HPP_INCLUDED




namespace flow {

namespace detail {

template <typename Flow, typename KeyFn>
struct group_by_adaptor : flow_base<group_by_adaptor<Flow, KeyFn>> {
private:
    Flow flow_;
    FLOW_NO_UNIQUE_ADDRESS KeyFn key_fn_;



    using group = take_adaptor<subflow_t<Flow>>;

public:
    constexpr group_by_adaptor(Flow&& flow, KeyFn&& key_fn)
        : flow_(std::move(flow)),
          key_fn_(std::move(key_fn))
    {}

    constexpr auto next() -> maybe<group>
    {
        auto image = flow_.subflow();

        auto m = flow_.next();
        if (!m) {
            return {};
        }
        auto&& last_key = invoke(key_fn_, *m);

        auto peek = flow_.subflow();
        dist_t counter = 1;

        while (true) {
            auto n = peek.next();
            if (!n) {
                break;
            }

            if (invoke(key_fn_, *n) != last_key) {
                break;
            }

            ++counter;
            (void) flow_.next();
        }

        return {std::move(image).take(counter)};
    }

    constexpr auto subflow() & -> group_by_adaptor<subflow_t<Flow>, function_ref<KeyFn>>
    {
        return {flow_.subflow(), function_ref{key_fn_}};
    }
};

}

inline constexpr auto group_by = [](auto&& flowable, auto key_fn)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::group_by() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).group_by(std::move(key_fn));
};

template <typename D>
template <typename Key>
constexpr auto flow_base<D>::group_by(Key key) &&
{
    static_assert(is_multipass_flow<D>,
                  "group_by() requires a multipass flow");
    static_assert(std::is_invocable_v<Key&, item_t<D>>,
                  "Incompatible key function passed to group_by()");
    using R = std::invoke_result_t<Key&, item_t<D>>;
    static_assert(std::is_invocable_v<equal_to, R, R>,
                  "The result type of the key function passed to group_by() "
                  "must be equality comparable");

    return detail::group_by_adaptor<D, Key>(consume(), std::move(key));
}

}

#endif


#ifndef FLOW_OP_INSPECT_HPP_INCLUDED
#define FLOW_OP_INSPECT_HPP_INCLUDED



namespace flow {

template <typename Derived>
template <typename Func>
constexpr auto flow_base<Derived>::inspect(Func func) &&
{
    static_assert(std::is_invocable_v<Func&, value_t<Derived> const&>,
        "Incompatible callable used with inspect()");

    return consume().map([func = std::move(func)] (auto&& val) mutable -> item_t<Derived> {
          (void) invoke(func, std::as_const(val));
          return FLOW_FWD(val);
    });
}

}

#endif


#ifndef FLOW_OP_INTERLEAVE_HPP_INCLUDED
#define FLOW_OP_INTERLEAVE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow1, typename Flow2>
struct interleave_adaptor : flow_base<interleave_adaptor<Flow1, Flow2>> {

    constexpr interleave_adaptor(Flow1&& flow1, Flow2&& flow2)
        : flow1_(std::move(flow1)),
          flow2_(std::move(flow2))
    {}

    constexpr auto next() -> next_t<Flow1>
    {
        auto item = first_ ? flow1_.next() : flow2_.next();
        first_ = !first_;
        return item;
    }

    template <typename F1 = Flow1, typename F2 = Flow2>
    constexpr auto subflow() & -> interleave_adaptor<subflow_t<F1>, subflow_t<F2>>
    {
        auto i = interleave_adaptor<subflow_t<F1>, subflow_t<F2>>(flow1_.subflow(), flow2_.subflow());
        i.first_ = first_;
        return i;
    }

    template <typename F1 = Flow1, typename F2 = Flow2,
              typename = std::enable_if_t<is_sized_flow<F1> && is_sized_flow<F2>>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return flow1_.size() + flow2_.size();
    }

private:
    template <typename, typename>
    friend struct interleave_adaptor;

    Flow1 flow1_;
    Flow2 flow2_;
    bool first_done_ = false;
    bool second_done_ = false;
    bool first_ = true;
};

}

inline constexpr auto interleave = [](auto&& flowable1, auto&& flowable2)
{
    static_assert(is_flowable<decltype(flowable1)>,
                  "Argument to flow::interleave() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable1))).interleave(FLOW_FWD(flowable2));
};

template <typename D>
template <typename Flowable>
constexpr auto flow_base<D>::interleave(Flowable&& with) &&
{
    static_assert(is_flowable<Flowable>,
        "Argument to interleave() must be a Flowable type");
    static_assert(std::is_same_v<item_t<D>, flow_item_t<Flowable>>,
        "Flows used with interleave() must have the exact same item type");
    return detail::interleave_adaptor<D, std::decay_t<flow_t<Flowable>>>(
        consume(), FLOW_COPY(flow::from(FLOW_FWD(with))));
}

}

#endif

#ifndef FLOW_OP_IS_SORTED_HPP_INCLUDED
#define FLOW_OP_IS_SORTED_HPP_INCLUDED



namespace flow {

namespace detail {

struct is_sorted_fn {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const -> bool
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::is_sorted() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).is_sorted(std::move(cmp));
    }
};

}

inline constexpr auto is_sorted = detail::is_sorted_fn{};

template <typename D>
template <typename Cmp>
constexpr bool flow_base<D>::is_sorted(Cmp cmp)
{
    auto last = derived().next();
    if (!last) {
        // An empty flow is sorted
        return true;
    }

    return derived().try_fold([&cmp, &last] (bool, next_t<D>&& next) {
        if (invoke(cmp, *next, *last)) {
            return false;
        } else {
            last = std::move(next);
            return true;
        }
    }, true);
}

}

#endif


#ifndef FLOW_OP_MAP_HPP_INCLUDED
#define FLOW_OP_MAP_HPP_INCLUDED



namespace flow {

#ifdef DOXYGEN
/// Free-function version of flow_base::map()
///
/// @param flowable A Flowable type
/// @param func A callable to apply to each element
/// @return Equivalent to `flow::from(forward(flowable)).map(move(func))`
constexpr auto map(auto&& flowable, auto func);
#endif

/// @cond

namespace detail {

template <typename Flow, typename Func>
struct map_adaptor : flow_base<map_adaptor<Flow, Func>> {

    static constexpr bool is_infinite = is_infinite_flow<Flow>;

    using item_type = std::invoke_result_t<Func&, item_t<Flow>>;

    constexpr map_adaptor(Flow&& flow, Func func)
        : flow_(std::move(flow)),
          func_(std::move(func))
    {}

    constexpr auto next() -> maybe<item_type>
    {
        if constexpr (is_infinite) {
            return {invoke(func_, *flow_.next())};
        } else {
            return flow_.next().map(func_);
        }
    }

    constexpr auto advance(dist_t dist) -> maybe<item_type>
    {
        if constexpr (is_infinite) {
            return {invoke(func_, *flow_.advance(dist))};
        } else {
            return flow_.advance(dist).map(func_);
        }
    }

    template <bool B = is_reversible_flow<Flow>>
    constexpr auto next_back() -> std::enable_if_t<B, maybe<item_type>>
    {
        return flow_.next_back().map(func_);
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> map_adaptor<subflow_t<F>, function_ref<Func>>
    {
        return {flow_.subflow(), func_};
    }

    template <bool B = is_sized_flow<Flow>>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return flow_.size();
    }

private:
    Flow flow_;
    FLOW_NO_UNIQUE_ADDRESS Func func_;
};

}

inline constexpr auto map = [](auto&& flowable, auto&& func)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::map must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).map(FLOW_FWD(func));
};

template <typename D>
template <typename Func>
constexpr auto flow_base<D>::map(Func func) &&
{
    static_assert(std::is_invocable_v<Func&, item_t<D>>,
        "Incompatible callable passed to map()");
    static_assert(!std::is_void_v<std::invoke_result_t<Func&, item_t<D>>>,
        "Map cannot be used with a function returning void");

    return detail::map_adaptor<D, Func>(consume(), std::move(func));
}

/// @endcond

}

#endif


#ifndef FLOW_OP_MAP_REFINEMENTS_HPP_INCLUDED
#define FLOW_OP_MAP_REFINEMENTS_HPP_INCLUDED



//
// These are all small adaptors which end up calling map(<some lambda>)
//

namespace flow {

// as()

namespace detail {

template <typename T>
struct as_op {
    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable) const
    {
        static_assert(is_flowable<decltype(flowable)>,
                      "Argument to flow::as() must be a Flowable type");
        return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template as<T>();
    }
};

}

template <typename T>
inline constexpr auto as = detail::as_op<T>{};

template <typename D>
template <typename T>
constexpr auto flow_base<D>::as() &&
{
    static_assert(!std::is_void_v<T>,
                  "as() cannot be called with a void type");

    return consume().map([](auto&& val) -> T {
        return static_cast<T>(FLOW_FWD(val));
    });
}

// unchecked_deref()

inline constexpr auto unchecked_deref = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::unchecked_deref() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).unchecked_deref();
};

template <typename D>
constexpr auto flow_base<D>::unchecked_deref() &&
{
    auto deref = [](auto&& val) -> decltype(*FLOW_FWD(val)) {
        return *FLOW_FWD(val);
    };
    static_assert(std::is_invocable_v<decltype(deref), item_t<D>>,
                  "Flow's item type is not dereferenceable with unary operator*");
    static_assert(!std::is_void_v<decltype(*std::declval<item_t<D>>())>,
                  "Flow's item type dereferences to void");
    return consume().map(std::move(deref));
}

// copy()

inline constexpr auto copy = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::copy() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).copy();
};

template <typename D>
constexpr auto flow_base<D>::copy() && -> decltype(auto)
{
    if constexpr (std::is_lvalue_reference_v<item_t<D>>) {
        return consume().template as<value_t<D>>();
    } else {
        return consume();
    }
}

// move()

inline constexpr auto move = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::move() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).move();
};

template <typename D>
constexpr auto flow_base<D>::move() && -> decltype(auto)
{
    if constexpr (std::is_lvalue_reference_v<item_t<D>>) {
        return consume().map([](auto& item) { return std::move(item); });
    } else {
        return consume();
    }
}

// as_const()

inline constexpr auto as_const = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::as_const() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).as_const();
};

template <typename D>
constexpr auto flow_base<D>::as_const() && -> decltype(auto)
{
    using I = item_t<D>;

    if constexpr (std::is_lvalue_reference_v<I> &&
                  !std::is_const_v<std::remove_reference_t<I>>) {
        return consume().template as<std::remove_reference_t<I> const&>();
    } else {
        return consume();
    }
}

// elements

namespace detail {

template <std::size_t N>
struct elements_op {
    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable) const
    {
        static_assert(is_flowable<decltype(flowable)>,
                      "Argument to flow::elements() must be a Flowable type");
        return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template elements<N>();
    }
};

// This should be a lambda inside elements<N>(), but MSVC doesn't like it at all
template <std::size_t N>
struct tuple_getter {
    template <typename Tuple>
    constexpr auto operator()(Tuple&& tup) const
        -> remove_rref_t<decltype(std::get<N>(FLOW_FWD(tup)))>
    {
        return std::get<N>(FLOW_FWD(tup));
    }
};

} // namespace detail

template <std::size_t N>
inline constexpr auto elements = detail::elements_op<N>{};

template <typename D>
template <std::size_t N>
constexpr auto flow_base<D>::elements() &&
{
    static_assert(std::is_invocable_v<detail::tuple_getter<N>&, item_t<D>>,
                  "Flow's item type is not tuple-like");

    return consume().map(detail::tuple_getter<N>{});
}

// keys

inline constexpr auto keys = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::keys() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).keys();
};

template <typename D>
constexpr auto flow_base<D>::keys() &&
{
    return consume().template elements<0>();
}

// values

inline constexpr auto values = [](auto&& flowable) {
  static_assert(is_flowable<decltype(flowable)>,
                "Argument to flow::values() must be a Flowable type");
  return FLOW_COPY(flow::from(FLOW_FWD(flowable))).values();
};

template <typename D>
constexpr auto flow_base<D>::values() &&
{
    return consume().template elements<1>();
}

}

#endif


#ifndef FLOW_OP_MINMAX_HPP_INCLUDED
#define FLOW_OP_MINMAX_HPP_INCLUDED



namespace flow {

namespace detail {

struct min_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::min() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).min(std::move(cmp));
    }
};

struct max_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::max() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).max(std::move(cmp));
    }
};

struct minmax_op {
    template <typename Flowable, typename Cmp = std::less<>>
    constexpr auto operator()(Flowable&& flowable, Cmp cmp = Cmp{}) const
    {
        static_assert(is_flowable<Flowable>,
                      "Argument to flow::minmax() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).minmax(std::move(cmp));
    }
};

}

inline constexpr auto min = detail::min_op{};
inline constexpr auto max = detail::max_op{};
inline constexpr auto minmax = detail::minmax_op{};

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::min(Cmp cmp)
{
    return derived().fold_first([&cmp](auto min, auto&& item) {
        return invoke(cmp, item, min) ? FLOW_FWD(item) : std::move(min);
    });
}

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::max(Cmp cmp)
{
    return derived().fold_first([&cmp](auto max, auto&& item) {
        return !invoke(cmp, item, max) ? FLOW_FWD(item) : std::move(max);
    });
}

template <typename V>
struct minmax_result {
    V min;
    V max;

    friend constexpr bool operator==(const minmax_result& lhs, const minmax_result& rhs)
    {
        return lhs.min == rhs.min && lhs.max == rhs.max;
    }

    friend constexpr bool operator!=(const minmax_result& lhs, const minmax_result& rhs)
    {
        return !(lhs == rhs);
    }
};

template <typename D>
template <typename Cmp>
constexpr auto flow_base<D>::minmax(Cmp cmp)
{
    return derived().next().map([this, &cmp] (auto&& init) {
        return derived().fold([&cmp](auto mm, auto&& item) -> minmax_result<value_t<D>> {

            if (invoke(cmp, item, mm.min)) {
                mm.min = item;
            }

            if (!invoke(cmp, item, mm.max)) {
                mm.max = FLOW_FWD(item);
            }

            return mm;
        }, minmax_result<value_t<D>>{init, FLOW_FWD(init)});
    });
}

}

#endif


#ifndef FLOW_OP_OUTPUT_TO_HPP_INCLUDED
#define FLOW_OP_OUTPUT_TO_HPP_INCLUDED



namespace flow {

inline constexpr auto output_to = [](auto&& flowable, auto oiter) {
    static_assert(is_flowable<decltype(flowable)>,
                  "First argument to flow::output_to() must be Flowable");
    // C++20: static_assert(std::output_iterator<decltype(oiter)>);
    return flow::from(FLOW_FWD(flowable)).output_to(std::move(oiter));
};


template <typename D>
template <typename Iter>
constexpr auto flow_base<D>::output_to(Iter oiter) -> Iter
{
    consume().for_each([&oiter] (auto&& val) {
        *oiter = FLOW_FWD(val);
        ++oiter;
    });
    return oiter;
}

}

#endif


#ifndef FLOW_OP_PRODUCT_HPP_INCLUDED
#define FLOW_OP_PRODUCT_HPP_INCLUDED



namespace flow {

inline constexpr auto product = [](auto&& flowable)
{
  static_assert(is_flowable<decltype(flowable)>,
                "Argument to flow::sum() must be Flowable");
  return flow::from(FLOW_FWD(flowable)).product();
};

template <typename D>
constexpr auto flow_base<D>::product()
{
    static_assert(std::is_constructible_v<value_t<D>, int>,
                  "Flow's value type must be constructible from a literal 1");
    return derived().fold(std::multiplies<>{}, value_t<D>{1});
}

}

#endif


#ifndef FLOW_OP_REVERSE_HPP_INCLUDED
#define FLOW_OP_REVERSE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow>
struct reverse_adaptor : flow_base<reverse_adaptor<Flow>> {

    constexpr explicit reverse_adaptor(Flow&& flow)
        : flow_(std::move(flow))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        return flow_.next_back();
    }

    constexpr auto next_back() -> next_t<Flow>
    {
        return flow_.next();
    }

    template <bool B = is_sized_flow<Flow>>
    constexpr auto size() const -> std::enable_if_t<B, dist_t>
    {
        return flow_.size();
    }

    template <typename F = Flow,
              typename = std::enable_if_t<is_multipass_flow<F>>>
    constexpr auto subflow() const -> reverse_adaptor<subflow_t<F>>
    {
        return reverse_adaptor<Flow>{flow_.subflow()};
    }

    constexpr auto reverse() && -> Flow
    {
        return std::move(*this).flow_;
    }

private:
    Flow flow_;
};

}

inline constexpr auto reverse = [](auto&& flowable) {
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flows::reverse() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).reverse();
};

template <typename D>
constexpr auto flow_base<D>::reverse() &&
{
    static_assert(is_reversible_flow<D>,
                  "reverse() requires a reversible flow");
    return detail::reverse_adaptor<D>{consume()};
}

}

#endif


#ifndef FLOW_OP_SCAN_HPP_INCLUDED
#define FLOW_OP_SCAN_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Base, typename Func, typename Init>
struct scan_adaptor : flow_base<scan_adaptor<Base, Func, Init>> {

    constexpr scan_adaptor(Base base, Func func, Init init)
        : base_(std::move(base)),
          func_(std::move(func)),
          state_(std::move(init))
    {}

    constexpr auto next() -> maybe<Init>
    {
        return base_.next().map([&](auto&& e) {
            state_ = invoke(func_, state_, FLOW_FWD(e));
            return state_;
        });
    }

private:
    Base base_;
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    Init state_;
};

}

template <typename Derived>
template <typename Func, typename, typename Init>
constexpr auto flow_base<Derived>::scan(Func func, Init init) &&
{
    return detail::scan_adaptor{consume(), std::move(func), std::move(init)};
}

template <typename Derived>
constexpr auto flow_base<Derived>::partial_sum() &&
{
    return consume().scan(std::plus<>{});
}

}

#endif


#ifndef FLOW_OP_SLIDE_HPP_INCLUDED
#define FLOW_OP_SLIDE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow>
struct slide_adaptor : flow_base<slide_adaptor<Flow>>
{
private:
    Flow flow_;
    subflow_t<Flow> prev_ = flow_.subflow();
    dist_t win_;
    dist_t step_;
    bool partial_;
    bool first_ = true;
    bool done_ = false;

    using item_type = decltype(flow_.subflow().take(win_));

    constexpr auto do_next_partial() -> maybe<item_type>
    {
        if (step_ > 1 && !first_) {
            if (!flow_.advance(step_ - 1).has_value()) {
                done_ = true;
                return {};
            }
        }

        first_ = false;
        auto sub = flow_.subflow();

        if (flow_.next()) {
            return std::move(sub).take(win_);
        } else {
            done_ = true;
            return {};
        }
    }

    // If we don't want partial windows, we need to iterate the main flow
    // "ahead" window_size places, and return a subflow
    constexpr auto do_next_no_partial() -> maybe<item_type>
    {
        if (first_) {
            first_ = false;
            if (!flow_.advance(win_)) {
                done_ = true;
                return {};
            }
        } else if (step_ > 1) {
            if (!flow_.advance(step_ - 1)) {
                done_ = true;
                return {};
            }

            // We should be able to do this safely since we've already
            // advanced the "main" flow, which is window_size steps ahead
            (void) prev_.advance(step_ - 1);
        }

        auto sub = prev_.subflow();
        if (!flow_.next()) {
            done_ = true; // This is the last window
        }

#ifndef NDEBUG
        assert(prev_.next().has_value());
#else
        (void) prev_.next();
#endif
        return std::move(sub).take(win_);
    }

public:
    constexpr slide_adaptor(Flow&& flow, dist_t win, dist_t step, bool partial)
        : flow_(std::move(flow)),
          win_(win),
          step_(step),
          partial_(win_ == 1 || partial) // partial windows cannot occur with window size of 1
    {}

    constexpr auto next() -> maybe<item_type>
    {
        if (done_) {
            return {};
        }

        if (partial_) {
            return do_next_partial();
        } else {
            return do_next_no_partial();
        }
    }

    template <typename F = Flow>
    constexpr auto subflow() -> slide_adaptor<subflow_t<F>>
    {
        // FIXME: Why do we need to drop an item from the subflows here?
        if (partial_) {
            auto sub = flow_.subflow();
            (void) sub.next();
            return slide_adaptor<subflow_t<Flow>>(std::move(sub), win_, step_, partial_);
        } else {
            auto sub = prev_.subflow();
            (void) sub.next();
            return slide_adaptor<subflow_t<Flow>>(std::move(sub), win_, step_, partial_);
        }
    }
};

}

inline constexpr auto slide = [](auto&& flowable,
                                 dist_t window_size,
                                 dist_t step_size = 1,
                                 bool partial_windows = false)
{
    static_assert(is_multipass_flow<flow_t<decltype(flowable)>>,
                  "Argument to slide() must be a multipass flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).slide(window_size,
                                                           step_size,
                                                           partial_windows);
};

template <typename D>
constexpr auto flow_base<D>::slide(dist_t window_size,
                                   dist_t step_size,
                                   bool partial_windows) &&
{
    static_assert(is_multipass_flow<D>,
                  "slide() can only be used with multipass flows");
    assert(window_size > 0);
    assert(step_size > 0);

    return detail::slide_adaptor<D>(consume(), window_size, step_size, partial_windows);
}

}

#endif


#ifndef FLOW_OP_SPLIT_HPP_INCLUDED
#define FLOW_OP_SPLIT_HPP_INCLUDED



namespace flow {

namespace detail {

struct split_op {
    template <typename Flowable>
    constexpr auto operator()(Flowable&& flowable, flow_value_t<Flowable> delim) const
    {
        return FLOW_COPY(flow::from(FLOW_FWD(flowable))).split(delim);
    }
};

}

inline constexpr auto split = detail::split_op{};

template <typename Derived>
template <typename D>
constexpr auto flow_base<Derived>::split(value_t<D> delim) &&
{
    // So, this works, kinda, but sucks.
    // What we do is split on the delimiter, and then skip those groups
    // which contain the delimiter
    // Doing this with a filter would be too slow, so what we do is inspect
    // the first group (to see whether it was a delim or not), possibly
    // drop that, then continue serving up every second group.
    // I mean, it's cool that we can do this, but a dedicated adaptor would
    // be much better.
    return consume().group_by(flow::pred::eq(std::move(delim)))
            .drop_while([delim](auto f) {
                assert(f.subflow().count() > 0);
                return *f.subflow().next() == delim; })
            .stride(2);
}

}

#endif


#ifndef FLOW_OP_STRIDE_HPP_INCLUDED
#define FLOW_OP_STRIDE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow>
struct stride_adaptor : flow_base<stride_adaptor<Flow>> {

    static constexpr bool is_infinite = is_infinite_flow<Flow>;

    constexpr stride_adaptor(Flow&& flow, dist_t step)
        : flow_(std::move(flow)),
          step_(step)
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (first_) {
            first_ = false;
            return flow_.next();
        }
        return flow_.advance(step_);
    }

    constexpr auto advance(dist_t count) -> next_t<Flow>
    {
        if (first_) {
            first_ = false;
            return flow_.advance(count);
        }
        return flow_.advance(count * step_);
    }

    template <typename F = Flow>
    constexpr auto size() const -> std::enable_if_t<is_sized_flow<F>, dist_t>
    {
        const auto sz = flow_.size();

        return sz/step_ + (sz % step_ != 0);
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> stride_adaptor<subflow_t<F>>
    {
        auto s = stride_adaptor<subflow_t<Flow>>(flow_.subflow(), step_);
        s.first_ = first_;
        return s;
    }

private:
    template <typename>
    friend struct stride_adaptor;

    Flow flow_;
    dist_t step_;
    bool first_ = true;
};

}

inline constexpr auto stride = [](auto&& flowable, dist_t step)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::stride() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).stride(step);
};

template <typename D>
constexpr auto flow_base<D>::stride(dist_t step) &&
{
    assert(step > 0 && "Stride must be positive");
    return detail::stride_adaptor<D>(consume(), step);
}

}

#endif


#ifndef FLOW_OP_SUM_HPP_INCLUDED
#define FLOW_OP_SUM_HPP_INCLUDED



namespace flow {

inline constexpr auto sum = [](auto&& flowable)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::sum() must be Flowable");
    return flow::from(FLOW_FWD(flowable)).sum();
};

template <typename D>
constexpr auto flow_base<D>::sum()
{
    return derived().fold(std::plus<>{});
}

}

#endif



#ifndef FLOW_OP_TAKE_WHILE_HPP_INCLUDED
#define FLOW_OP_TAKE_WHILE_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Flow, typename Pred>
struct take_while_adaptor : flow_base<take_while_adaptor<Flow, Pred>> {

    constexpr take_while_adaptor(Flow&& flow, Pred&& pred)
        : flow_(std::move(flow)),
          pred_(std::move(pred))
    {}

    constexpr auto next() -> next_t<Flow>
    {
        if (!done_) {
            auto m = flow_.next();
            if (!m.has_value() || !invoke(pred_, std::as_const(*m))) {
                done_ = true;
                return {};
            }
            return m;
        }
        return {};
    }

    template <typename F = Flow>
    constexpr auto subflow() & -> take_while_adaptor<subflow_t<F>, function_ref<Pred>>
    {
        auto s = take_while_adaptor<subflow_t<F>, function_ref<Pred>>{flow_.subflow(), pred_};
        s.done_ = done_;
        return s;
    }

private:
    template <typename, typename>
    friend struct take_while_adaptor;

    Flow flow_;
    Pred pred_;
    bool done_ = false;
};

}

inline constexpr auto take_while = [](auto&& flowable, auto pred)
{
    static_assert(is_flowable<decltype(flowable)>,
                  "Argument to flow::take_while() must be a Flowable type");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).take_while(std::move(pred));
};

template <typename D>
template <typename Pred>
constexpr auto flow_base<D>::take_while(Pred pred) &&
{
    static_assert(std::is_invocable_r_v<bool, Pred&, value_t<D> const&>,
                  "Incompatible predicate passed to take_while()");
    return detail::take_while_adaptor<D, Pred>(consume(), std::move(pred));
}

}

#endif


#ifndef FLOW_OP_TO_HPP_INCLUDED
#define FLOW_OP_TO_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename Void, template <typename...> typename C, typename... Args>
inline constexpr bool is_ctad_constructible_v = false;

template <template <typename...> typename C, typename... Args>
inline constexpr bool is_ctad_constructible_v<
    std::void_t<decltype(C(std::declval<Args>()...))>, C, Args...> = true;

}

// These need to be function templates so that the user can supply the template
// arg, and we can overload on both type template parameters and
// template template params
template <typename C, typename Flowable>
constexpr auto to(Flowable&& flowable) -> C
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template to<C>();
}

template <template <typename...> typename C, typename Flowable>
constexpr auto to(Flowable&& flowable)
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template to<C>();
}

template <typename Flowable>
auto to_vector(Flowable&& flowable) -> std::vector<flow_value_t<Flowable>>
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to_vector() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).to_vector();
}

template <typename T, typename Flowable>
auto to_vector(Flowable&& flowable) -> std::vector<T>
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to_vector() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).template to_vector<T>();
}

template <typename Flowable>
auto to_string(Flowable&& flowable) -> std::string
{
    static_assert(is_flowable<Flowable>,
                  "Argument to flow::to_string() must be Flowable");
    return FLOW_COPY(flow::from(FLOW_FWD(flowable))).to_string();
}

template <typename D>
template <typename C>
constexpr auto flow_base<D>::to() && -> C
{
    if constexpr (std::is_constructible_v<C, D&&>) {
        return C(consume());
    } else {
        auto rng = consume().to_range();
        static_assert(std::is_constructible_v<C, decltype(rng.begin()),
                                              decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }
}

template <typename D>
template <template <typename...> typename C>
constexpr auto flow_base<D>::to() &&
{
    if constexpr (detail::is_ctad_constructible_v<void, C, D&&>) {
        return C(consume());
    } else {
        auto rng = consume().to_range();
        static_assert(detail::is_ctad_constructible_v<void, C,
                      decltype(rng.begin()), decltype(rng.end())>);
        return C(rng.begin(), rng.end());
    }
}

template <typename D>
template <typename T>
auto flow_base<D>::to_vector() && -> std::vector<T>
{
    return consume().template to<std::vector<T>>();
}

template <typename D>
auto flow_base<D>::to_vector() &&
{
    return consume().template to<std::vector<value_t<D>>>();
}

template <typename D>
auto flow_base<D>::to_string() && -> std::string
{
    return consume().template to<std::string>();
}

}

#endif




#ifndef FLOW_OP_TRY_FOLD_HPP_INCLUDED
#define FLOW_OP_TRY_FOLD_HPP_INCLUDED



namespace flow {

inline constexpr auto try_fold = [](auto&& flowable, auto func, auto init) {
    static_assert(is_flowable<decltype(flowable)>);
    return flow::from(FLOW_FWD(flowable)).try_fold(std::move(func), std::move(init));
};

template <typename D>
template <typename Func, typename Init>
constexpr auto flow_base<D>::try_fold(Func func, Init init) -> Init
{
    static_assert(std::is_invocable_r_v<Init, Func&, Init&&, next_t<D>&&>,
                  "Incompatible callable passed to try_fold()");
    while (auto m = derived().next()) {
        init = invoke(func, std::move(init), std::move(m));
        if (!static_cast<bool>(init)) {
            break;
        }
    }
    return init;
}

}

#endif


#ifndef FLOW_OP_TRY_FOR_EACH_HPP_INCLUDED
#define FLOW_OP_TRY_FOR_EACH_HPP_INCLUDED



namespace flow {

inline constexpr auto try_for_each = [](auto flowable, auto func) {
    static_assert(is_flowable<decltype(flowable)>);
    return flow::from(FLOW_FWD(flowable), std::move(func));
};

template <typename D>
template <typename Func>
constexpr auto flow_base<D>::try_for_each(Func func)
{
    using result_t = std::invoke_result_t<Func&, next_t<D>&&>;
    return derived().try_fold([&func](auto&& /*unused*/, auto&& m) -> decltype(auto) {
      return invoke(func, FLOW_FWD(m));
    }, result_t{});
}

}

#endif


#ifndef FLOW_OP_WRITE_TO_HPP_INCLUDED
#define FLOW_OP_WRITE_TO_HPP_INCLUDED



namespace flow {

namespace detail {

struct write_to_op {
    template <typename Flowable, typename CharT, typename Traits,
              typename Sep = const char*>
    constexpr auto operator()(Flowable&& flowable, std::basic_ostream<CharT, Traits>& os,
                              Sep sep = ", ") const
        -> std::basic_ostream<CharT, Traits>&
    {
        static_assert(is_flowable<Flowable>,
                      "First argument to flow::write_to() must be Flowable");
        return flow::from(FLOW_FWD(flowable)).write_to(os, sep);
    }
};

}

inline constexpr auto write_to = detail::write_to_op{};

template <typename D>
template <typename Sep, typename CharT, typename Traits>
constexpr auto flow_base<D>::write_to(std::basic_ostream<CharT, Traits>& os, Sep sep)
    -> std::basic_ostream<CharT, Traits>&
{
    consume().for_each([&os, &sep, first = true](auto&& m) mutable {
        if (first) {
            first = false;
        } else {
            os << sep;
        }
        os << FLOW_FWD(m);
    });
    return os;
}

}

#endif




#ifndef FLOW_OP_ZIP_WITH_HPP_INCLUDED
#define FLOW_OP_ZIP_WITH_HPP_INCLUDED




#include <limits>

namespace flow {

namespace detail {

// A zipped flow is sized if all its component flows are sized or infinite,
// and at least one of them is non-infinite.
// Fold expressions are cool.
template <typename... Flows>
inline constexpr bool is_sized_zip =
    ((is_sized_flow<Flows> || is_infinite<Flows>) && ...) &&
    !(is_infinite<Flows> && ...);

template <typename F>
constexpr auto size_or_infinity(const F& flow) -> dist_t
{
    if constexpr (is_infinite<F>) {
        return std::numeric_limits<dist_t>::max();
    } else if (is_sized_flow<F>) {
        return flow.size();
    }
}

template <typename Func, typename... Flows>
struct zip_with_adaptor : flow_base<zip_with_adaptor<Func, Flows...>> {

    static_assert(sizeof...(Flows) > 0);

    // ICE in MSVC if this is changed to invoke_result_t *sigh*
    using item_type = typename std::invoke_result<Func&, item_t<Flows>...>::type;

    static constexpr bool is_infinite = (is_infinite_flow<Flows> && ...);

    constexpr explicit zip_with_adaptor(Func func, Flows&&... flows)
        : func_(std::move(func)), flows_(FLOW_FWD(flows)...)
    {}

    constexpr auto next() -> maybe<item_type>
    {
        auto maybes = std::apply([](auto&... args) {
            return std::tuple<next_t<Flows>...>{args.next()...};
        }, flows_);

        const bool all_engaged = std::apply([](auto&... args) {
            return (static_cast<bool>(args) && ...);
        }, maybes);

        if (all_engaged) {
            return {std::apply([this](auto&&... args) {
                return invoke(func_, *FLOW_FWD(args)...);
            }, std::move(maybes))};
        }
        return {};
    }

    template <bool B = (is_multipass_flow<Flows> && ...),
              typename = std::enable_if_t<B>>
    constexpr auto subflow() & -> zip_with_adaptor<function_ref<Func>, subflow_t<Flows>...>
    {
        return std::apply([&func_ = func_](auto&... args) {
            return zip_with_adaptor<function_ref<Func>, subflow_t<Flows>...>(func_, args.subflow()...);
        }, flows_);
    }

    template <bool B = is_sized_zip<Flows...>,
              typename = std::enable_if_t<B>>
    [[nodiscard]] constexpr auto size() const -> dist_t
    {
        return std::apply([](auto const&... args) {
            // Clang doesn't like flow::of(...).min() here, not sure why
            auto ilist = {size_or_infinity(args)...};
            return *flow::min(ilist);
        }, flows_);
    }

private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    std::tuple<Flows...> flows_;
};

// Specialisation for the common case of zipping two flows
template <typename Func, typename F1, typename F2>
struct zip_with_adaptor<Func, F1, F2> : flow_base<zip_with_adaptor<Func, F1, F2>>
{
    using item_type = std::invoke_result_t<Func&, item_t<F1>, item_t<F2>>;

    static constexpr bool is_infinite = is_infinite_flow<F1> && is_infinite_flow<F2>;

    constexpr zip_with_adaptor(Func func, F1&& f1, F2&& f2)
        : func_(std::move(func)),
          f1_(std::move(f1)),
          f2_(std::move(f2))
    {}

    constexpr auto next() -> maybe<item_type>
    {
        auto m1 = f1_.next();
        auto m2 = f2_.next();

        if ((bool) m1 && (bool) m2) {
            return invoke(func_, *std::move(m1), *std::move(m2));
        }
        return {};
    }

    template <typename S1 = F1, typename S2 = F2>
    constexpr auto subflow() & -> zip_with_adaptor<function_ref<Func>, subflow_t<S1>, subflow_t<S2>>
    {
        return {func_, f1_.subflow(), f2_.subflow()};
    }

    template <bool B = is_sized_zip<F1, F2>,
              typename = std::enable_if_t<B>>
    constexpr auto size() const -> dist_t
    {
        return min(size_or_infinity(f1_), size_or_infinity(f2_));
    }

private:
    FLOW_NO_UNIQUE_ADDRESS Func func_;
    F1 f1_;
    F2 f2_;
};

struct zip_with_fn {
    template <typename Func, typename Flowable0, typename... Flowables>
    constexpr auto operator()(Func func, Flowable0&& flowable0, Flowables&&... flowables) const
        -> zip_with_adaptor<Func, flow_t<Flowable0>, flow_t<Flowables>...>
    {
        static_assert(is_flowable<Flowable0> && (is_flowable<Flowables> && ...),
            "All arguments to zip_with must be Flowable");
        static_assert(std::is_invocable_v<Func&, flow_item_t<Flowable0>, flow_item_t<Flowables>...>,
            "Incompatible callable passed to zip_with");

        return from(FLOW_FWD(flowable0)).zip_with(std::move(func), FLOW_FWD(flowables)...);
    }
};

}

inline constexpr auto zip_with = detail::zip_with_fn{};

template <typename D>
template <typename Func, typename... Flowables>
constexpr auto flow_base<D>::zip_with(Func func, Flowables&&... flowables) && {
    static_assert((is_flowable<Flowables> && ...),
                  "All arguments to zip_with must be Flowable");
    static_assert(std::is_invocable_v<Func&, item_t<D>, item_t<flow_t<Flowables>>...>,
                  "Incompatible callable passed to zip_with");

    return detail::zip_with_adaptor<Func, D, flow_t<Flowables>...>{
        std::move(func), consume(), flow::from(FLOW_FWD(flowables))...
    };
}

}


#endif



#ifndef FLOW_SOURCE_ANY_HPP_INCLUDED
#define FLOW_SOURCE_ANY_HPP_INCLUDED



#include <memory>

namespace flow {

template <typename T>
struct any_flow : flow_base<any_flow<T>> {
private:
    struct iface {
        virtual auto do_next() -> maybe<T> = 0;
        virtual ~iface() = default;
    };

    template <typename F>
    struct impl : iface {
        explicit impl(F flow)
            : flow_(std::move(flow))
        {}

        auto do_next() -> maybe<T> override { return flow_.next(); }
        F flow_;
    };

    std::unique_ptr<iface> ptr_;

public:
    template <typename F,
        std::enable_if_t<!std::is_same_v<remove_cvref_t<F>, any_flow>, int> = 0,
        std::enable_if_t<is_flow<F> && std::is_same_v<item_t<F>, T>, int> = 0>
    any_flow(F flow)
        : ptr_(std::make_unique<impl<F>>(std::move(flow)))
    {}

    auto next() -> maybe<T> { return ptr_->do_next(); }
};


template <typename T>
struct any_flow_ref : flow_base<any_flow_ref<T>> {
private:
    using next_fn_t = auto (void*) -> maybe<T>;

    template <typename F>
    static constexpr next_fn_t* next_fn_impl = +[](void* ptr) {
        return static_cast<F*>(ptr)->next();
    };

    void* ptr_;
    next_fn_t* next_fn_;

public:
    template <typename F,
        std::enable_if_t<!std::is_same_v<std::remove_reference_t<F>, any_flow_ref>, int> = 0,
        std::enable_if_t<is_flow<F> && std::is_same_v<item_t<F>, T>, int> = 0>
    constexpr any_flow_ref(F& flow)
        : ptr_(std::addressof(flow)),
          next_fn_(next_fn_impl<F>)
    {}

    constexpr auto next() -> maybe<T> { return next_fn_(ptr_); }
};

}

#endif


#ifndef FLOW_SOURCE_ASYNC_HPP_INCLUDED
#define FLOW_SOURCE_ASYNC_HPP_INCLUDED



#ifdef FLOW_HAVE_COROUTINES



#include <experimental/coroutine>

namespace flow {

template <typename T>
struct async : flow_base<async<T>>
{
    struct promise_type;

    using handle_type = std::experimental::coroutine_handle<promise_type>;

    struct promise_type {
    private:
        maybe<T> value{};

    public:
        auto initial_suspend() {
            return std::experimental::suspend_always{};
        };

        auto final_suspend() noexcept {
            return std::experimental::suspend_always{};
        }

        auto yield_value(std::remove_reference_t<T>& val)
        {
            value = maybe<T>(val);
            return std::experimental::suspend_always{};
        }

        auto yield_value(remove_cvref_t<T>&& val)
        {
            value = maybe<T>(std::move(val));
            return std::experimental::suspend_always{};
        }

        auto extract_value() -> maybe<T>
        {
            auto ret = std::move(value);
            value.reset();
            return ret;
        }

        auto get_return_object()
        {
            return async{handle_type::from_promise(*this)};
        }

        auto unhandled_exception()
        {
            std::terminate();
        }

        auto return_void()
        {
            return std::experimental::suspend_never{};
        }
    };

    auto next() -> maybe<T>
    {
        if (!coro.done()) {
            coro.resume();
            return coro.promise().extract_value();
        }
        return {};
    }

    async(async&& other) noexcept
        : coro(std::move(other).coro)
    {
        other.coro = nullptr;
    }

    async& operator=(async&& other) noexcept
    {
        std::swap(coro, other.coro);
    }

    ~async()
    {
        if (coro) {
            coro.destroy();
        }
    }

private:
    explicit async(handle_type&& handle)
        : coro(std::move(handle))
    {}

    handle_type coro;
};

} // namespace flow

#endif // FLOW_HAVE_COROUTINES

#endif


#ifndef FLOW_SOURCE_C_STR_HPP_INCLUDED
#define FLOW_SOURCE_C_STR_HPP_INCLUDED



namespace flow {

template <typename CharT, typename Traits = std::char_traits<CharT>>
struct c_str : flow_base<c_str<CharT, Traits>> {

    constexpr explicit c_str(CharT* str)
        : str_(str)
    {}

    constexpr auto next() -> maybe<CharT&>
    {
        CharT& c = str_[idx_];
        if (Traits::eq(c, CharT{})) {
            return {};
        }
        ++idx_;
        return {c};
    }

private:
    CharT* str_;
    dist_t idx_ = 0;
};

}

#endif


#ifndef FLOW_SOURCE_EMPTY_HPP_INCLUDED
#define FLOW_SOURCE_EMPTY_HPP_INCLUDED



namespace flow {

template <typename T>
struct empty : flow_base<empty<T>> {

    explicit constexpr empty() = default;

    static constexpr auto next() -> maybe<T> { return {}; }

    static constexpr auto size() -> dist_t { return 0; }
};

}

#endif



#ifndef FLOW_SOURCE_GENERATE_HPP_INCLUDED
#define FLOW_SOURCE_GENERATE_HPP_INCLUDED



namespace flow {

inline constexpr struct generate_fn {
private:
    template <typename Func>
    struct generator : flow_base<generator<Func>> {

        static constexpr bool is_infinite = true;

        constexpr explicit generator(Func func)
            : func_(std::move(func))
        {}

        constexpr auto next() -> maybe<std::invoke_result_t<Func&>>
        {
            return func_();
        }

    private:
        FLOW_NO_UNIQUE_ADDRESS Func func_;
    };

public:
    template <typename Func>
    constexpr auto operator()(Func func) const
    {
        static_assert(std::is_invocable_v<Func&>);
        return generator<Func>{std::move(func)};
    }

} generate;

}

#endif



#ifndef FLOW_SOURCE_FROM_ISTREAM_HPP_INCLUDED
#define FLOW_SOURCE_FROM_ISTREAM_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename CharT, typename Traits>
struct istream_ref {
    std::basic_istream<CharT, Traits>* ptr_;

    template <typename T>
    friend constexpr auto& operator>>(istream_ref& self, T& item)
    {
        return *self >> item;
    }
};

template <typename>
inline constexpr bool is_istream = false;

template <typename CharT, typename Traits>
inline constexpr bool is_istream<std::basic_istream<CharT, Traits>> = true;

template <typename T, typename CharT, typename Traits>
struct istream_flow : flow_base<istream_flow<T, CharT, Traits>> {
private:
    using istream_type = std::basic_istream<CharT, Traits>;
    istream_type* is_;
    T item_ = T();

public:
    explicit istream_flow(std::basic_istream<CharT, Traits>& is)
        : is_(std::addressof(is))
    {}

    // Move-only
    istream_flow(istream_flow&&) noexcept = default;
    istream_flow& operator=(istream_flow&&) noexcept = default;

    auto next() -> maybe<T>
    {
        if (!(*is_ >> item_)) { return {}; }
        return {item_};
    }
};

} // namespace detail

template <typename T, typename CharT, typename Traits>
auto from_istream(std::basic_istream<CharT, Traits>& is)
{
    return detail::istream_flow<T, CharT, Traits>(is);
}

}

#endif


#ifndef FLOW_SOURCE_ISTREAMBUF_HPP_INCLUDED
#define FLOW_SOURCE_ISTREAMBUF_HPP_INCLUDED



namespace flow {

namespace detail {

template <typename CharT, typename Traits>
struct istreambuf_flow : flow_base<istreambuf_flow<CharT, Traits>> {

    using streambuf_type = std::basic_streambuf<CharT, Traits>;

    explicit istreambuf_flow(streambuf_type* buf)
        : buf_(buf)
    {}

    istreambuf_flow(istreambuf_flow&&) = default;
    istreambuf_flow& operator=(istreambuf_flow&&) = default;

    auto next() -> maybe<CharT>
    {
        if (!buf_) {
            return {};
        }

        auto c = buf_->sbumpc();

        if (c == Traits::eof()) {
            buf_ = nullptr;
            return {};
        }

        return Traits::to_char_type(c);
    }

private:
    streambuf_type* buf_ = nullptr;
};

struct from_istreambuf_fn {

    template <typename CharT, typename Traits>
    auto operator()(std::basic_streambuf<CharT, Traits>* buf) const
    {
        return istreambuf_flow<CharT, Traits>(buf);
    }

    template <typename CharT, typename Traits>
    auto operator()(std::basic_istream<CharT, Traits>& stream) const
    {
        return istreambuf_flow<CharT, Traits>(stream.rdbuf());
    }
};

} // namespace detail

inline constexpr auto from_istreambuf = detail::from_istreambuf_fn{};

} // namespace flow

#endif


#ifndef FLOW_SOURCE_OF_HPP_INCLUDED
#define FLOW_SOURCE_OF_HPP_INCLUDED



#include <array>

namespace flow {

namespace detail {

template <typename T, std::size_t N>
using array_flow = decltype(flow::from(std::declval<std::array<T, N>>()));

}

template <typename T, std::size_t N>
struct of : flow_base<of<T, N>> {
    template <typename... Args>
    constexpr explicit of(Args&&... args)
        : arr_(flow::from(std::array<T, N>{FLOW_FWD(args)...}))
    {}

    constexpr auto next() -> maybe<T&> {
        return arr_.next();
    }

    constexpr auto next_back() -> maybe<T&> {
        return arr_.next_back();
    }

    constexpr auto subflow() &
    {
        return arr_.subflow();
    }

    constexpr auto size() const -> dist_t
    {
        return arr_.size();
    }

private:
    detail::array_flow<T, N> arr_;
};

template <typename T, typename... U>
of(T, U...) -> of<T, 1 + sizeof...(U)>;

}

#endif


#endif
