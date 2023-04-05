[![Standard](https://img.shields.io/badge/standard-C%2B%2B20-blue.svg?logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-BSL-blue.svg)](http://www.boost.org/LICENSE_1_0.txt)
[![windows](https://github.com/tcbrindle/libflux/actions/workflows/windows.yml/badge.svg)](https://github.com/tcbrindle/libflux/actions/workflows/windows.yml)
[![macos](https://github.com/tcbrindle/libflux/actions/workflows/macos.yml/badge.svg)](https://github.com/tcbrindle/libflux/actions/workflows/macos.yml)
[![linux](https://github.com/tcbrindle/libflux/actions/workflows/linux.yml/badge.svg)](https://github.com/tcbrindle/libflux/actions/workflows/linux.yml)
[![codecov](https://codecov.io/gh/tcbrindle/flux/branch/main/graph/badge.svg?token=5YCV2ZG1YT)](https://codecov.io/gh/tcbrindle/flux)

# Flux #

Flux is an experimental C++20 library for working with sequences of values. It offers similar facilities to C++20 ranges, D ranges, Python itertools, Rust iterators and related libraries for other languages.

## Quick Example ##

```cpp
constexpr auto result = flux::from(std::array{1, 2, 3, 4, 5})
                         .filter(flux::pred::even)
                         .map([](int i) { return i * 2; })
                         .sum();
static_assert(result == 12);
```

Try it [on Compiler Explorer](https://godbolt.org/z/WvqeKr1h3)

## Getting Started ##

Right now the easiest way to get started with Flux is to download the [latest automatically generated single header file](https://godbolt.org/z/WvqeKr1h3) and `#include` it in your sources like any other header.

## Compiler support ##

Flux requires a recent compiler with good support for C++20. It is tested with:

 * GCC 11.3 and newer
 * MSVC 2022
 * Clang 16

Note that older compilers are unlikely to work due to missing language and/or standard library support.

## Why Flux? ##

Flux provides a broadly equivalent feature set to C++20 Ranges, but uses a slightly different model based around *cursors* rather than *iterators*. Flux aims to offer:
  * Much improved safety by default
  * Improved ease of use in common cases, particularly for defining your own sequences and adaptors
  * Improved run-time efficiency for some common operations
  * Compatibility with existing ranges algorithms

## The Flux iteration model ##

The Flux iteration model is based around *cursors*, which are a generalisation of array *indices* (in much the same way that STL iterators are a generalisation of array pointers). A Flux `sequence` provides four basis operations:

 * `flux::first(seq)` returns an object called a *cursor*, which represents a position in a sequence. For a sequence with N elements there are N+1 possible cursor positions, including the past-the-end (terminal) position.
 * `flux::is_last(seq, cursor)` returns a boolean value indicating whether the cursor is in the terminal position
 * `flux::inc(seq, cursor)` increments the given cursor, so that it points to the next element in the sequence
 * `flux::read_at(seq, cursor)` returns the sequence element at the given cursor position

These basis operations are equivalent to the basis operations on STL iterators (`begin()`, `iter == end()`, `++iter` and `*iter` respectively). The crucial difference is that in the Flux model, you need to **provide both the sequence and the cursor** to each function call, whereas in the STL model the iterator knows how to increment and dereference itself.

> STL iterators are "smart", but Flux cursors are not!

This seemingly small change as some far-reaching consequences. In particular:

 * Because we have access to the sequence object during increment and dereference operations, we can provide *inexpensive bounds checking* for sequences
 * Because we need the sequence object in order to do anything useful with a cursor, "dangling" cursors are not possible by design: if the sequence object is no longer around, the cursor can't be used
 * Because a cursor only represents a position in a sequence (like an integer index for an array), modifying the underlying sequence is less likely to invalidate a cursor -- if the element at the given position no longer exists, this will be caught by the bounds check at the next attempted read.
 * Because element access requires the original sequence, we don't need to make a distinction between mutable `iterator`s and `const_iterator`s -- the same cursor type is used for both const and non-const access, making cursors considerably simpler to implement than STL iterators.

Like STL input ranges, basic Flux sequences are assumed to be single-pass by default. Flux also provides various for more powerful sequences, closely modeled on their STL counterparts:

 * `multipass_sequence`s allow multiple cursors to iterate over the sequence independently, potentially passing over each position multiple times
 * `bidirectional_sequence`s are multipass sequences whose cursors can be decremented as well as incremented
 * `random_access_sequence`s are bidirectional sequences whose cursors can be incremented or decremented an arbitrary number of places in constant time
 * `contiguous_sequence`s are random-access sequences which are backed by a contiguous, in-memory array, which potentially allow low-level operations like `memcpy()` to be used as an optimisation

## Reference documentation ##

Incomplete, work-in-progress documentation can be found at [tristanbrindle.com/flux](https://tristanbrindle.com/flux)






