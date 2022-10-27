
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <flux.hpp>

namespace {

struct incomplete;

struct indestructable {
    ~indestructable() = delete;
};

struct nonmovable {
    nonmovable() = default;
    nonmovable(nonmovable&&) = delete;
    nonmovable& operator=(nonmovable&&) = delete;
};

struct move_only {
    move_only() = default;
    move_only(move_only&&) = default;
    move_only& operator=(move_only&&) = default;
};

struct abstract {
    virtual void f() = 0;
};

enum class scoped_enum {};
enum unscoped_enum {};

enum class fwd_declared_enum;

template <typename T = void>
struct cant_instantiate {
    static_assert(!std::same_as<T, void>);
};

template <typename T>
struct dummy_impl {
    static int first(T&);
    static bool is_last(T&, int);
    static int& inc(T&, int&);
    static int read_at(T&, int);
};

template <typename Elem>
struct minimal_seq_of {
    struct flux_sequence_iface {
        static int first(minimal_seq_of);
        static bool is_last(minimal_seq_of, int);
        static int& inc(minimal_seq_of, int&);
        static Elem read_at(minimal_seq_of, int);
    };
};

template <typename Idx>
struct minimal_with_idx {
    struct flux_sequence_iface {
        static Idx first(minimal_with_idx);
        static bool is_last(minimal_with_idx, Idx const&);
        static Idx& inc(minimal_with_idx, Idx&);
        static int read_at(minimal_with_idx, Idx const&);
    };
};

} // end anon namespace

template <>
struct flux::sequence_iface<incomplete>
    : dummy_impl<incomplete> {};

template <>
struct flux::sequence_iface<indestructable>
    : dummy_impl<indestructable> {};

template <>
struct flux::sequence_iface<cant_instantiate<>>
    : dummy_impl<cant_instantiate<>> {};

static_assert(not flux::cursor<void>);
static_assert(flux::ordered_cursor<void*>);
//static_assert(not flux::cursor<incomplete>);
static_assert(flux::ordered_cursor<incomplete*>);
static_assert(not flux::cursor<indestructable>);
static_assert(not flux::cursor<nonmovable>);
static_assert(flux::cursor<move_only>);
static_assert(not flux::regular_cursor<move_only>);
static_assert(not flux::cursor<abstract>);
static_assert(flux::ordered_cursor<scoped_enum>);
static_assert(flux::ordered_cursor<unscoped_enum>);
static_assert(flux::ordered_cursor<fwd_declared_enum>);
static_assert(flux::cursor<cant_instantiate<>*>);

static_assert(not flux::sequence<void>);
static_assert(not flux::sequence<int>);
static_assert(flux::sequence<incomplete>);
static_assert(flux::sequence<indestructable>);
static_assert(flux::sequence<cant_instantiate<>>);
static_assert(flux::sequence<minimal_seq_of<int>>);
static_assert(not flux::sequence<minimal_seq_of<void>>);
static_assert(flux::sequence<minimal_seq_of<void*>>);
static_assert(not flux::sequence<minimal_seq_of<incomplete>>);
static_assert(flux::sequence<minimal_seq_of<incomplete*>>);
static_assert(not flux::sequence<minimal_seq_of<indestructable>>);
static_assert(flux::sequence<minimal_seq_of<indestructable&>>);
static_assert(not flux::sequence<minimal_seq_of<abstract>>);
static_assert(flux::sequence<minimal_seq_of<abstract&>>);

static_assert(flux::multipass_sequence<minimal_with_idx<int>>);
static_assert(flux::sequence<minimal_with_idx<move_only>>);
static_assert(not flux::multipass_sequence<minimal_with_idx<move_only>>);

namespace {
    using ref_seq = minimal_seq_of<int&>;
    using cref_seq = minimal_seq_of<int const&>;
    using rref_seq = minimal_seq_of<int&&>;
    using crref_seq = minimal_seq_of<int const&&>;
    using val_seq = minimal_seq_of<int>;

    using flux::element_t;
    using flux::value_t;
    using flux::rvalue_element_t;
    using flux::common_element_t;
    using std::same_as;

    static_assert(same_as<element_t<ref_seq>, int&>);
    static_assert(same_as<value_t<ref_seq>, int>);
    static_assert(same_as<rvalue_element_t<ref_seq>, int&&>);
    static_assert(same_as<common_element_t<ref_seq>, int&>);

    static_assert(same_as<element_t<cref_seq>, int const&>);
    static_assert(same_as<value_t<cref_seq>, int>);
    static_assert(same_as<rvalue_element_t<cref_seq>, int const&&>);
    static_assert(same_as<common_element_t<cref_seq>, int const&>);

    static_assert(same_as<element_t<rref_seq>, int&&>);
    static_assert(same_as<value_t<rref_seq>, int>);
    static_assert(same_as<rvalue_element_t<rref_seq>, int&&>);
    static_assert(same_as<common_element_t<rref_seq>, int const&>); // Yes, really

    static_assert(same_as<element_t<crref_seq>, int const&&>);
    static_assert(same_as<value_t<crref_seq>, int>);
    static_assert(same_as<rvalue_element_t<crref_seq>, int const&&>);
    static_assert(same_as<common_element_t<crref_seq>, int const&>); // Yes, really

    static_assert(same_as<element_t<val_seq>, int>);
    static_assert(same_as<value_t<val_seq>, int>);
    static_assert(same_as<rvalue_element_t<val_seq>, int>);
    static_assert(same_as<common_element_t<val_seq>, int>);
}

namespace {

struct Base {};
struct Derived1 : Base {};
struct Derived2 : Base {};

}

template <>
struct flux::sequence_iface<Base>
    : dummy_impl<Base> {};

static_assert(flux::sequence<Base>);
static_assert(not flux::sequence<Derived1>);

template <>
struct flux::sequence_iface<Derived2>
    : flux::sequence_iface<Base> {};

static_assert(flux::sequence<Derived2>);

// Adaptable sequence tests
namespace {
    struct movable_seq {
        struct flux_sequence_iface {
            static int first(movable_seq);
            static bool is_last(movable_seq, int);
            static int& inc(movable_seq, int&);
            static int read_at(movable_seq, int);
        };
    };

    struct move_only_seq {
        move_only_seq() = default;
        move_only_seq(move_only_seq&&) = default;
        move_only_seq& operator=(move_only_seq&&) = default;

        struct flux_sequence_iface {
            static int first(move_only_seq const&);
            static bool is_last(move_only_seq const&, int);
            static int& inc(move_only_seq const&, int&);
            static int read_at(move_only_seq const&, int);
        };
    };

    struct unmovable_seq {
        unmovable_seq() = default;
        unmovable_seq(unmovable_seq&&) = delete;
        unmovable_seq& operator=(unmovable_seq&&) = delete;

        struct flux_sequence_iface {
            static int first(unmovable_seq const&);
            static bool is_last(unmovable_seq const&, int);
            static int& inc(unmovable_seq const&, int&);
            static int read_at(unmovable_seq const&, int);
        };
    };

    static_assert(flux::sequence<movable_seq>);
    static_assert(flux::adaptable_sequence<movable_seq>);
    static_assert(not flux::adaptable_sequence<movable_seq const&&>);

    static_assert(flux::sequence<move_only_seq>);
    static_assert(flux::adaptable_sequence<move_only_seq>);
    static_assert(not flux::adaptable_sequence<move_only_seq const&&>);

    static_assert(flux::sequence<unmovable_seq>);
    static_assert(not flux::adaptable_sequence<unmovable_seq>);
    static_assert(not flux::adaptable_sequence<unmovable_seq&&>);
    static_assert(not flux::adaptable_sequence<unmovable_seq const&&>);

    using ilist_t = std::initializer_list<int>;

    static_assert(flux::sequence<ilist_t>);
    static_assert(not flux::adaptable_sequence<ilist_t>);
    static_assert(not flux::adaptable_sequence<ilist_t&&>);
    static_assert(not flux::adaptable_sequence<ilist_t const&&>);
}


