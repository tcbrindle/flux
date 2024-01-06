
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include "test_utils.hpp"

// Silence warnings about unneeded comparison functions: in fact they are
// needed during concept checks
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

namespace {

// GCC11 std::string isn't usable in constexpr, so here's a quick and dirty
// version instead
struct String {
private:
    using Traits = std::char_traits<char>;

    char* str_ = nullptr;
    size_t sz_ = 0;

public:
    String() = default;

    constexpr explicit(false) String(const char* src)
        : sz_(Traits::length(src))
    {
        str_ = new char[sz_ + 1]{};
        Traits::copy(str_, src, sz_);
    }

    constexpr String(String const& other)
        : String(other.str_)
    {}

    constexpr String& operator=(String const& other)
    {
        if (&other != this) {
            delete[] str_;
            sz_ = other.sz_;
            str_ = new char[sz_ + 1];
            Traits::copy(str_, other.str_, sz_);
        }
        return *this;
    }

    constexpr String(String&& other) noexcept
        : str_(std::exchange(other.str_, nullptr)),
          sz_(std::exchange(other.sz_, 0))
    {}

    constexpr String& operator=(String&& other) noexcept
    {
        str_ = std::exchange(other.str_, str_);
        sz_ = std::exchange(other.sz_, sz_);
        return *this;
    }

    constexpr ~String()
    {
        delete[] str_;
    }

    constexpr size_t size() const { return sz_; }

    constexpr bool operator==(char const* other) const
    {
        if (sz_ != Traits::length(other)) {
            return false;
        } else {
            return Traits::compare(str_, other, sz_) == 0;
        }
    }

    constexpr bool operator==(String const& other) const
    {
        return *this == other.str_;
    }
};

struct Tester {
    int i;
    float f;

    bool operator==(Tester const&) const = default;
    auto operator<=>(Tester const&) const = default;
};

struct TesterManualCompare {
    int i;

    using Self = TesterManualCompare;

    friend constexpr bool operator==(Self const& lhs, Self const& rhs)
    {
        return lhs.i == rhs.i;
    }

    friend constexpr bool operator<(Self const& lhs, Self const& rhs)
    {
        return lhs.i < rhs.i;
    }

    friend constexpr bool operator>(Self const& lhs, Self const& rhs)
    {
        return rhs < lhs;
    }

    friend constexpr bool operator<=(Self const& lhs, Self const& rhs)
    {
        return !(lhs > rhs);
    }

    friend constexpr bool operator>=(Self const& lhs, Self const& rhs)
    {
        return !(lhs < rhs);
    }
};

struct Base {};
struct Derived : Base {};

struct MoveOnly {
    MoveOnly() = default;
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;
};

struct NotCopyAssignable {
    int i;

    constexpr explicit NotCopyAssignable(int i ) : i{i} {}
    NotCopyAssignable(NotCopyAssignable const&) = default;
    NotCopyAssignable(NotCopyAssignable&&) = default;
    NotCopyAssignable& operator=(NotCopyAssignable const&) = delete; // Note
    NotCopyAssignable& operator=(NotCopyAssignable&&) = default;
};

struct NotMoveAssignable {
    int i;

    constexpr explicit NotMoveAssignable(int i) : i{i} {}
    NotMoveAssignable(NotMoveAssignable&&) = default;
    NotMoveAssignable& operator=(NotMoveAssignable&&) = delete; // Note
};

struct TraceMove {
    bool moved_from = false;

    TraceMove() = default;

    constexpr TraceMove(TraceMove&& other)
    {
        other.moved_from = true;
    }

    constexpr TraceMove& operator=(TraceMove&& other)
    {
        other.moved_from = true;
        return *this;
    }
};

template <typename T>
constexpr bool test_optional_default_ctor()
{
    {
        flux::optional<T> o;
        STATIC_CHECK(!o.has_value());
    }

    {
        flux::optional<T> o{};
        STATIC_CHECK(!o.has_value());
    }

    return true;
}
static_assert(test_optional_default_ctor<int>());
static_assert(test_optional_default_ctor<String>());
static_assert(test_optional_default_ctor<int&>());
static_assert(test_optional_default_ctor<int const&>());

template <typename T>
constexpr bool test_optional_nullopt_ctor()
{
    static_assert(std::addressof(flux::nullopt) == std::addressof(std::nullopt));

    {
        flux::optional<T> o{flux::nullopt};
        STATIC_CHECK(!o.has_value());
    }

    {
        flux::optional<T> o(flux::nullopt);
        STATIC_CHECK(!o.has_value());
    }

    {
        flux::optional<T> o = flux::nullopt;
        STATIC_CHECK(!o.has_value());
    }

    {
        auto takes_opt = [](flux::optional<T>) {};
        takes_opt(flux::nullopt);
    }

    return true;
}
static_assert(test_optional_nullopt_ctor<int>());
static_assert(test_optional_nullopt_ctor<String>());
static_assert(test_optional_nullopt_ctor<int&>());
static_assert(test_optional_nullopt_ctor<int const&>());

constexpr bool test_optional_value_ctor() {

    // ints
    {
        {
            flux::optional<int> o(1);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 1);
        }

        {
            int i = 0;
            flux::optional<int> o(i);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(*o == 0);
        }

        {
            int const i = 0;
            flux::optional<int> o(i);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(*o == 0);
        }

        static_assert(not std::constructible_from<flux::optional<int>, long>);
    }

    // String
    {
        {
            flux::optional<String> o(String("abc"));
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == String("abc"));
        }

        {
            String s = "abc";
            flux::optional<String> o(s);
            STATIC_CHECK(o.has_value());
        }

        static_assert(not std::constructible_from<flux::optional<String>, char const*>);
    }

    // int& and int const&
    {
        {
            int i = 10;
            flux::optional<int&> o(i);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 10);
        }

        {
            int const i = 10;
            flux::optional<int const&> o(i);
            STATIC_CHECK(o);
            STATIC_CHECK(*o == 10);
        }

        {
            int const i = 10;
            flux::optional<int const&> o(i);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 10);
        }

        {
            int i = 10;
            flux::optional<int const&> o(i);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 10);
        }

        static_assert(not std::constructible_from<flux::optional<int&>, int>);
        static_assert(not std::constructible_from<flux::optional<int&>, int const&>);
        static_assert(not std::constructible_from<flux::optional<int&>, int&&>);
        static_assert(not std::constructible_from<flux::optional<int&>, long&>);
    }

    {
        flux::optional<Tester> o(Tester{.i = 1, .f = 2.0f});
        STATIC_CHECK(o.has_value());
        STATIC_CHECK(o->i == 1);
        STATIC_CHECK(o->f == 2.0f);
    }

    static_assert(not std::constructible_from<flux::optional<Base>, Derived&>);
    static_assert(std::constructible_from<flux::optional<Base const&>, Derived&>);


    return true;
};
static_assert(test_optional_value_ctor());

constexpr bool test_optional_in_place_ctor()
{
    {
        flux::optional<int> o(std::in_place, 3);
        STATIC_CHECK(o.has_value());
        STATIC_CHECK(o.value() == 3);
    }

    {
        flux::optional<String> o(std::in_place, "abc");
        STATIC_CHECK(o.has_value());
        STATIC_CHECK(o.value() == "abc");
    }

    {
        flux::optional<Tester> o(std::in_place, 1, 2.0f);
        STATIC_CHECK(o.has_value());
        STATIC_CHECK(o->i == 1);
        STATIC_CHECK(o->f == 2.0f);
    }

    return true;
}
static_assert(test_optional_in_place_ctor());

constexpr bool test_optional_copy_ctor()
{
    static_assert(std::is_trivially_copy_constructible_v<flux::optional<int>>);
    static_assert(std::is_trivially_copy_constructible_v<flux::optional<std::string&>>);

    // Int, engaged
    {
        flux::optional<int> o1(3);
        flux::optional<int> o2 = o1;
        STATIC_CHECK(o2.has_value());
        STATIC_CHECK(o2.value() == 3);
    }

    // Int, disengaged
    {
        flux::optional<int> o1;
        flux::optional<int> o2 = o1;
        STATIC_CHECK(not o2.has_value());
    }

    // std::string, engaged
    {
        flux::optional<String> o1(String("hello"));
        flux::optional o2 = o1;
        STATIC_CHECK(o2.has_value());
        STATIC_CHECK(o2.value() == "hello");
    }

    // std::string, disengaged
    {
        flux::optional<std::string> o1;
        flux::optional o2 = o1;
        STATIC_CHECK(not o2.has_value());
    }

    static_assert(not std::is_copy_constructible_v<flux::optional<MoveOnly>>);


    return true;
}
static_assert(test_optional_copy_ctor());

constexpr bool test_optional_move_ctor()
{
    static_assert(std::is_trivially_move_constructible_v<flux::optional<int>>);
    static_assert(std::is_trivially_move_constructible_v<flux::optional<std::string&>>);
    static_assert(std::is_trivially_move_constructible_v<flux::optional<MoveOnly>>);

    static_assert(std::is_nothrow_move_constructible_v<flux::optional<int>>);
    static_assert(std::is_nothrow_move_constructible_v<flux::optional<int&>>);
    static_assert(std::is_nothrow_move_constructible_v<flux::optional<std::string>>);

    // Int, engaged
    {
        flux::optional<int> o1(3);
        flux::optional<int> o2 = std::move(o1);
        STATIC_CHECK(o2.has_value());
        STATIC_CHECK(o2.value() == 3);
    }

    // Int, disengaged
    {
        flux::optional<int> o1;
        flux::optional<int> o2 = std::move(o1);
        STATIC_CHECK(not o2.has_value());
    }

    // String, engaged
    {
        flux::optional<String> o1(String("hello"));
        flux::optional o2 = std::move(o1);
        STATIC_CHECK(o2.has_value());
        STATIC_CHECK(o2.value() == "hello");
    }

    // std::string, disengaged
    {
        flux::optional<String> o1;
        flux::optional o2 = std::move(o1);
        STATIC_CHECK(not o2.has_value());
    }

    // Trace move
    {
        flux::optional<TraceMove> src(TraceMove{});
        STATIC_CHECK(not src->moved_from);

        auto dest = std::move(src);
        STATIC_CHECK(src->moved_from);
        STATIC_CHECK(not dest->moved_from);
    }

    return true;
}
static_assert(test_optional_move_ctor());

constexpr bool test_optional_copy_assign()
{
    static_assert(std::is_trivially_copy_assignable_v<flux::optional<int>>);
    static_assert(std::is_trivially_copy_assignable_v<flux::optional<std::string&>>);
    static_assert(not std::is_copy_assignable_v<flux::optional<MoveOnly>>);

    // int, engaged -> engaged
    {
        flux::optional<int> src(3);
        flux::optional<int> dest(-200);

        dest = src;

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == 3);
    }

    // int, engaged -> disengaged
    {
        flux::optional<int> src(3);
        flux::optional<int> dest;

        dest = src;

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == 3);
    }

    // int, disengaged -> engaged
    {
        flux::optional<int> src;
        flux::optional<int> dest(-200);

        dest = src;

        STATIC_CHECK(not dest.has_value());
    }

    // int, disengaged -> disengaged
    {
        flux::optional<int> src;
        flux::optional<int> dest;

        dest = src;

        STATIC_CHECK(not dest.has_value());
    }

    // String, engaged -> engaged
    {
        flux::optional<String> src(String("abc"));
        flux::optional<String> dest(String("xyz"));

        dest = src;

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == "abc");
    }

    // String, engaged -> disengaged
    {
        flux::optional<String> src(String("abc"));
        flux::optional<String> dest;

        dest = src;

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == "abc");
    }

    // String, disengaged -> engaged
    {
        flux::optional<String> src;
        flux::optional<String> dest(String("xyz"));

        dest = src;

        STATIC_CHECK(not dest.has_value());
    }

    // String, disengaged -> disengaged
    {
        flux::optional<String> src;
        flux::optional<String> dest;

        dest = src;

        STATIC_CHECK(not dest.has_value());
    }

    // Test optional ref assignment rebinds
    {
        int i = 0;
        flux::optional<int&> o(i);

        STATIC_CHECK(&*o == &i);

        int j = 10;
        o = flux::optional<int&>(j);

        STATIC_CHECK(&*o == &j);
    }

    // Test optional<NotCopyAssignable> can actually be copy-assigned
    {
        static_assert(std::copy_constructible<NotCopyAssignable>);
        static_assert(not std::copyable<NotCopyAssignable>);
        static_assert(std::copyable<flux::optional<NotCopyAssignable>>);

        auto opt1 = flux::optional(NotCopyAssignable(1));
        auto opt2 = flux::optional(NotCopyAssignable(2));

        opt1 = opt2;

        STATIC_CHECK(opt1->i == 2);
    }

    return true;
}
static_assert(test_optional_copy_assign());

constexpr bool test_optional_move_assign()
{
    static_assert(std::is_trivially_move_assignable_v<flux::optional<int>>);
    static_assert(std::is_trivially_move_assignable_v<flux::optional<String&>>);

    static_assert(std::is_nothrow_move_assignable_v<flux::optional<int>>);
    static_assert(std::is_nothrow_move_assignable_v<flux::optional<int&>>);
    static_assert(std::is_nothrow_move_assignable_v<flux::optional<String>>);

    // int, engaged -> engaged
    {
        flux::optional<int> src(3);
        flux::optional<int> dest(-200);

        dest = std::move(src);

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == 3);
    }

    // int, engaged -> disengaged
    {
        flux::optional<int> src(3);
        flux::optional<int> dest;

        dest = std::move(src);

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == 3);
    }

    // int, disengaged -> engaged
    {
        flux::optional<int> src;
        flux::optional<int> dest(-200);

        dest = std::move(src);

        STATIC_CHECK(not dest.has_value());
    }

    // int, disengaged -> disengaged
    {
        flux::optional<int> src;
        flux::optional<int> dest;

        dest = std::move(src);

        STATIC_CHECK(not dest.has_value());
    }


    // String, engaged -> engaged
    {
        flux::optional<String> src(String("abc"));
        flux::optional<String> dest(String("xyz"));

        dest = std::move(src);

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == "abc");
    }

    // String, engaged -> disengaged
    {
        flux::optional<String> src(String("abc"));
        flux::optional<String> dest;

        dest = std::move(src);

        STATIC_CHECK(dest.has_value());
        STATIC_CHECK(dest.value() == "abc");
    }

    // String, disengaged -> engaged
    {
        flux::optional<String> src;
        flux::optional<String> dest(String("xyz"));

        dest = std::move(src);

        STATIC_CHECK(not dest.has_value());
    }

    // String, disengaged -> disengaged
    {
        flux::optional<String> src;
        flux::optional<String> dest;

        dest = std::move(src);

        STATIC_CHECK(not dest.has_value());
    }

    // Tracing moves
    {
        flux::optional<TraceMove> src(TraceMove{});
        flux::optional<TraceMove> dest;

        dest = std::move(src);

        STATIC_CHECK(src->moved_from);
        STATIC_CHECK(not dest->moved_from);
    }

    // Test optional<NotMoveAssignable> can actually be move-assigned
    {
        static_assert(std::move_constructible<NotMoveAssignable>);
        static_assert(not std::copyable<NotMoveAssignable>);
        static_assert(std::movable<flux::optional<NotMoveAssignable>>);

        auto opt1 = flux::optional(NotCopyAssignable(1));
        auto opt2 = flux::optional(NotCopyAssignable(2));

        opt1 = std::move(opt2);

        STATIC_CHECK(opt1->i == 2);
    }

    return true;
}
static_assert(test_optional_move_assign());

// We're using this everywhere anyway, but still...
constexpr bool test_optional_has_value()
{
    {
        flux::optional<int> const o;
        STATIC_CHECK(not o.has_value());
        STATIC_CHECK(not static_cast<bool>(o));
    }

    {
        flux::optional<int> const o(3);
        STATIC_CHECK(o.has_value());
        STATIC_CHECK(static_cast<bool>(o));
    }

    {
        flux::optional<int&> const o;
        STATIC_CHECK(not o.has_value());
        STATIC_CHECK(not static_cast<bool>(o));
    }

    {
        int i = 3;
        flux::optional<int&> const o(i);
        STATIC_CHECK(o.has_value());
        STATIC_CHECK(static_cast<bool>(o));
    }

    return true;
}
static_assert(test_optional_has_value());

constexpr bool test_optional_deref()
{
    {
        flux::optional<String> o(String("abc"));
        STATIC_CHECK(*o == "abc");
        STATIC_CHECK(o.value() == "abc");
        STATIC_CHECK(o->size() == 3);
    }

    {
        String s = "abc";
        flux::optional<String&> o(s);
        STATIC_CHECK(*o == "abc");
        STATIC_CHECK(o.value() == "abc");
        STATIC_CHECK(o->size() == 3);
    }

    // Check that everything is the right value category
    {
        using S = int;
        flux::optional<S> v;
        flux::optional<S> const cv;

        static_assert(std::same_as<decltype(*v), S&>);
        static_assert(std::same_as<decltype(*cv), S const&>);
        static_assert(std::same_as<decltype(*std::move(v)), S&&>);
        static_assert(std::same_as<decltype(*std::move(cv)), S const&&>);

        static_assert(std::same_as<decltype(v.value()), S&>);
        static_assert(std::same_as<decltype(cv.value()), S const&>);
        static_assert(std::same_as<decltype(std::move(v).value()), S&&>);
        static_assert(std::same_as<decltype(std::move(cv).value()), S const&&>);

        static_assert(std::same_as<decltype(v.operator->()), S*>);
        static_assert(std::same_as<decltype(cv.operator->()), S const*>);
        static_assert(std::same_as<decltype(std::move(v).operator->()), S*>);
        static_assert(std::same_as<decltype(std::move(cv).operator->()), S const*>);
    }

    {
        using S = int;
        flux::optional<S&> v;
        flux::optional<S&> const cv;

        static_assert(std::same_as<decltype(*v), S&>);
        static_assert(std::same_as<decltype(*cv), S&>);
        static_assert(std::same_as<decltype(*std::move(v)), S&>);
        static_assert(std::same_as<decltype(*std::move(cv)), S&>);

        static_assert(std::same_as<decltype(v.value()), S&>);
        static_assert(std::same_as<decltype(cv.value()), S&>);
        static_assert(std::same_as<decltype(std::move(v).value()), S&>);
        static_assert(std::same_as<decltype(std::move(cv).value()), S&>);

        static_assert(std::same_as<decltype(v.operator->()), S*>);
        static_assert(std::same_as<decltype(cv.operator->()), S*>);
        static_assert(std::same_as<decltype(std::move(v).operator->()), S*>);
        static_assert(std::same_as<decltype(std::move(cv).operator->()), S*>);
    }

    {
        using S = int;
        flux::optional<S const&> v;
        flux::optional<S const&> const cv;

        static_assert(std::same_as<decltype(*v), S const&>);
        static_assert(std::same_as<decltype(*cv), S const&>);
        static_assert(std::same_as<decltype(*std::move(v)), S const&>);
        static_assert(std::same_as<decltype(*std::move(cv)), S const&>);

        static_assert(std::same_as<decltype(v.value()), S const&>);
        static_assert(std::same_as<decltype(cv.value()), S const&>);
        static_assert(std::same_as<decltype(std::move(v).value()), S const&>);
        static_assert(std::same_as<decltype(std::move(cv).value()), S const&>);

        static_assert(std::same_as<decltype(v.operator->()), S const*>);
        static_assert(std::same_as<decltype(cv.operator->()), S const*>);
        static_assert(std::same_as<decltype(std::move(v).operator->()), S const*>);
        static_assert(std::same_as<decltype(std::move(cv).operator->()), S const*>);
    }

    return true;
}
static_assert(test_optional_deref());

bool test_optional_deref_throws()
{
    {
        flux::optional<String> o(std::nullopt);
        CHECK_THROWS_AS(*o, flux::unrecoverable_error);
        CHECK_THROWS_AS(o.value(), flux::unrecoverable_error);
        CHECK_THROWS_AS(o->size(), flux::unrecoverable_error);
    }

    {
        flux::optional<String&> o;
        CHECK_THROWS_AS(*o, flux::unrecoverable_error);
        CHECK_THROWS_AS(o.value(), flux::unrecoverable_error);
        CHECK_THROWS_AS(o->size(), flux::unrecoverable_error);
    }

    return true;
}

constexpr bool test_optional_value_or()
{
    {
        flux::optional<int> o;
        STATIC_CHECK(o.value_or(3) == 3);
        o = flux::optional(10);
        STATIC_CHECK(o.value_or(3) == 10);
    }

    {
        flux::optional<int&> o;
        STATIC_CHECK(o.value_or(3) == 3);
        int i = 10;
        o = flux::optional<int&>(i);
        STATIC_CHECK(o.value_or(3) == 10);
    }

    return true;
}
static_assert(test_optional_value_or());

constexpr bool test_optional_comparisons()
{
    // value, both engaged, can spaceship
    {
        flux::optional<int> o1(3);
        flux::optional<int> o2(4);
        STATIC_CHECK(o1 == o1);
        STATIC_CHECK(o1 != o2);

        STATIC_CHECK(o1 < o2);
        STATIC_CHECK(o1 <= o2);
        STATIC_CHECK(o2 > o1);
        STATIC_CHECK(o2 >= o1);
        STATIC_CHECK(o1 <=> o2 == std::strong_ordering::less);
        STATIC_CHECK(o2 <=> o1 == std::strong_ordering::greater);
        STATIC_CHECK(o1 <=> o1 == std::strong_ordering::equal);
    }

    // value, both disengaged, can spaceship
    {
        flux::optional<int> o1;
        flux::optional<int> o2;

        STATIC_CHECK(o1 == o2);
        STATIC_CHECK(o1 <=> o2 == std::strong_ordering::equal);
        STATIC_CHECK(o1 >= o2);
        STATIC_CHECK(o2 <= o1);
    }

    // value, mixed, can spaceship
    {
        flux::optional<int> e(3);
        flux::optional<int> d;

        STATIC_CHECK(e != d);
        STATIC_CHECK(e <=> d == std::strong_ordering::greater);
        STATIC_CHECK(d <=> e == std::strong_ordering::less);
        STATIC_CHECK(e > d);
        STATIC_CHECK(e >= d);
        STATIC_CHECK(d < e);
        STATIC_CHECK(d <= e);
    }


    // value, both engaged, no spaceship
    {
        flux::optional o1(TesterManualCompare{3});
        flux::optional o2(TesterManualCompare{4});
        STATIC_CHECK(o1 == o1);
        STATIC_CHECK(o1 != o2);

        STATIC_CHECK(o1 < o2);
        STATIC_CHECK(o1 <= o2);
        STATIC_CHECK(o2 > o1);
        STATIC_CHECK(o2 >= o1);
        STATIC_CHECK(o1 <=> o2 == std::partial_ordering::less);
        STATIC_CHECK(o2 <=> o1 == std::partial_ordering::greater);
        STATIC_CHECK(o1 <=> o1 == std::partial_ordering::equivalent);
    }

    // value, both disengaged, no spaceship
    {
        flux::optional<TesterManualCompare> o1;
        flux::optional<TesterManualCompare> o2;

        STATIC_CHECK(o1 == o2);
        STATIC_CHECK(o1 <=> o2 == std::partial_ordering::equivalent);
        STATIC_CHECK(o1 >= o2);
        STATIC_CHECK(o2 <= o1);
    }

    // value, mixed, no spaceship
    {
        flux::optional<TesterManualCompare> e(TesterManualCompare{3});
        flux::optional<TesterManualCompare> d;

        STATIC_CHECK(e != d);
        STATIC_CHECK(e <=> d == std::partial_ordering::greater);
        STATIC_CHECK(d <=> e == std::partial_ordering::less);
        STATIC_CHECK(e > d);
        STATIC_CHECK(e >= d);
        STATIC_CHECK(d < e);
        STATIC_CHECK(d <= e);
    }

    // Check we get the right comparison categories
    {
        flux::optional<int> t;
        auto r = t <=> t;
        static_assert(std::same_as<decltype(r), std::strong_ordering>);
    }

    {
        flux::optional<Tester> t;
        auto r = t <=> t;
        static_assert(std::same_as<decltype(r), std::partial_ordering>);
    }

    {
        flux::optional<TesterManualCompare> t;
        auto r = t <=> t;
        static_assert(std::same_as<decltype(r), std::partial_ordering>);
    }

    return true;
}
static_assert(test_optional_comparisons());

constexpr bool test_optional_nullopt_cmp()
{
    constexpr auto& n = flux::nullopt;

    struct Incomparable {};

    // value, engaged
    {
        flux::optional<Incomparable> o(Incomparable{});
        STATIC_CHECK(o != n);
        STATIC_CHECK(n != o);
        STATIC_CHECK(o <=> n == std::strong_ordering::greater);
        STATIC_CHECK(n <=> o == std::strong_ordering::less);
        STATIC_CHECK(o > n);
        STATIC_CHECK(o >= n);
        STATIC_CHECK(n < o);
        STATIC_CHECK(n <= o);
    }

    // value, disengaged
    {
        flux::optional<Incomparable> o;
        STATIC_CHECK(o == n);
        STATIC_CHECK(n == o);
        STATIC_CHECK(o <=> n == std::strong_ordering::equal);
        STATIC_CHECK(n <=> o == std::strong_ordering::equal);
        STATIC_CHECK(!(o > n));
        STATIC_CHECK(o >= n);
        STATIC_CHECK(!(n < o));
        STATIC_CHECK(n <= o);
    }

    // ref, engaged
    {
        Incomparable i{};
        flux::optional<Incomparable&> o(i);
        STATIC_CHECK(o != n);
        STATIC_CHECK(n != o);
        STATIC_CHECK(o <=> n == std::strong_ordering::greater);
        STATIC_CHECK(n <=> o == std::strong_ordering::less);
        STATIC_CHECK(o > n);
        STATIC_CHECK(o >= n);
        STATIC_CHECK(n < o);
        STATIC_CHECK(n <= o);
    }

    // ref, disengaged
    {
        flux::optional<Incomparable&> o;
        STATIC_CHECK(o == n);
        STATIC_CHECK(n == o);
        STATIC_CHECK(o <=> n == std::strong_ordering::equal);
        STATIC_CHECK(n <=> o == std::strong_ordering::equal);
        STATIC_CHECK(!(o > n));
        STATIC_CHECK(o >= n);
        STATIC_CHECK(!(n < o));
        STATIC_CHECK(n <= o);
    }

    return true;
}
static_assert(test_optional_nullopt_cmp());

constexpr bool test_optional_reset()
{
    {
        flux::optional<int> o{3};
        STATIC_CHECK(o.has_value());
        STATIC_CHECK(o.value() == 3);

        o.reset();

        STATIC_CHECK(!o.has_value());
    }

    {
        int i = 0;
        flux::optional<int&> o(i);
        STATIC_CHECK(o.has_value());

        o.reset();

        STATIC_CHECK(!o.has_value());
    }

    return true;
}
static_assert(test_optional_reset());

enum class cv_qual {
    mut_lref,
    const_lref,
    mut_rref,
    const_rref
};

constexpr bool test_optional_map()
{
    // Test basic mapping
    {
        auto fn = [](int i) { return double(i); };

        // value, engaged
        flux::optional<int> ve(3);
        {
            flux::optional<double> o = ve.map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }
        {
            flux::optional<double> o = std::as_const(ve).map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }
        {
            flux::optional<double> o = std::move(ve).map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }
        {
            flux::optional<double> o = std::move(std::as_const(ve)).map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }

        // value, disengaged
        flux::optional<int> vd;
        {
            flux::optional<double> o = vd.map(fn);
            STATIC_CHECK(!o.has_value());
        }
        {
            flux::optional<double> o = std::as_const(vd).map(fn);
            STATIC_CHECK(!o.has_value());
        }
        {
            flux::optional<double> o = std::move(vd).map(fn);
            STATIC_CHECK(!o.has_value());
        }
        {
            flux::optional<double> o = std::move(std::as_const(vd)).map(fn);
            STATIC_CHECK(!o.has_value());
        }

        // reference, engaged
        int i = 3;
        flux::optional<int&> re(i);
        {
            flux::optional<double> o = re.map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }
        {
            flux::optional<double> o = std::as_const(re).map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }
        {
            flux::optional<double> o = std::move(re).map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }
        {
            flux::optional<double> o = std::move(std::as_const(re)).map(fn);
            STATIC_CHECK(o.has_value());
            STATIC_CHECK(o.value() == 3.0);
        }

        // reference, disengaged
        flux::optional<int&> rd;
        {
            flux::optional<double> o = rd.map(fn);
            STATIC_CHECK(!o.has_value());
        }
        {
            flux::optional<double> o = std::as_const(rd).map(fn);
            STATIC_CHECK(!o.has_value());
        }
        {
            flux::optional<double> o = std::move(rd).map(fn);
            STATIC_CHECK(!o.has_value());
        }
        {
            flux::optional<double> o = std::move(std::as_const(rd)).map(fn);
            STATIC_CHECK(!o.has_value());
        }

    }


    auto get_cv = []<typename T>(T&&) -> cv_qual {
        if constexpr (std::is_lvalue_reference_v<T>) {
            using R = std::remove_reference_t<T>;
            if constexpr (std::is_const_v<R>) {
                return cv_qual::const_lref;
            } else {
                return cv_qual::mut_lref;
            }
        } else {
            if constexpr (std::is_const_v<T>) {
                return cv_qual::const_rref;
            } else {
                return cv_qual::mut_rref;
            }
        }
    };

    // make sure optional::map is using the right value categories
    {
        flux::optional<int> o{10};
        flux::optional<int> const co{10};

        STATIC_CHECK(*o.map(get_cv) == cv_qual::mut_lref);
        STATIC_CHECK(*co.map(get_cv) == cv_qual::const_lref);
        STATIC_CHECK(*std::move(o).map(get_cv) == cv_qual::mut_rref);
        STATIC_CHECK(*std::move(co).map(get_cv) == cv_qual::const_rref);

        int i = 0;
        flux::optional<int&> r(i);
        flux::optional<int&> const cr(i);

        STATIC_CHECK(*r.map(get_cv) == cv_qual::mut_lref);
        STATIC_CHECK(*cr.map(get_cv) == cv_qual::mut_lref);
        STATIC_CHECK(*std::move(r).map(get_cv) == cv_qual::mut_lref);
        STATIC_CHECK(*std::move(cr).map(get_cv) == cv_qual::mut_lref);
    }

    // Test optional::map returning lvalue ref
    {
        auto return_ref = [](int& i) -> int& { return i; };

        flux::optional<int> o{3};

        STATIC_CHECK(o.map(return_ref).value() == 3);
    }

    return true;
}
static_assert(test_optional_map());

}

TEST_CASE("optional")
{
    REQUIRE(test_optional_default_ctor<int>());
    REQUIRE(test_optional_default_ctor<int&>());
    REQUIRE(test_optional_default_ctor<String>());

    REQUIRE(test_optional_nullopt_ctor<int>());
    REQUIRE(test_optional_nullopt_ctor<int&>());
    REQUIRE(test_optional_nullopt_ctor<String>());

    REQUIRE(test_optional_value_ctor());

    REQUIRE(test_optional_in_place_ctor());

    REQUIRE(test_optional_copy_ctor());

    REQUIRE(test_optional_move_ctor());

    REQUIRE(test_optional_copy_assign());

    REQUIRE(test_optional_move_assign());

    REQUIRE(test_optional_has_value());

    REQUIRE(test_optional_deref());

    REQUIRE(test_optional_deref_throws());

    REQUIRE(test_optional_value_or());

    REQUIRE(test_optional_comparisons());

    REQUIRE(test_optional_nullopt_cmp());

    REQUIRE(test_optional_reset());

    REQUIRE(test_optional_map());
}