[![Standard](https://img.shields.io/badge/standard-C%2B%2B20-blue.svg?logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-BSL-blue.svg)](http://www.boost.org/LICENSE_1_0.txt)
[![windows](https://github.com/tcbrindle/libflux/actions/workflows/windows.yml/badge.svg)](https://github.com/tcbrindle/libflux/actions/workflows/windows.yml)
[![macos](https://github.com/tcbrindle/libflux/actions/workflows/macos.yml/badge.svg)](https://github.com/tcbrindle/libflux/actions/workflows/macos.yml)
[![linux](https://github.com/tcbrindle/libflux/actions/workflows/linux.yml/badge.svg)](https://github.com/tcbrindle/libflux/actions/workflows/linux.yml)
[![codecov](https://codecov.io/gh/tcbrindle/flux/branch/main/graph/badge.svg?token=5YCV2ZG1YT)](https://codecov.io/gh/tcbrindle/flux)

# 🅕🅛🅤🅧 #

Flux is a C++20 library for *sequence-orientated programming*, similar in spirit to C++20 ranges, Python itertools, Rust iterators and others.

Flux offers:

* A large selection of [algorithms](https://tristanbrindle.com/flux/reference/algorithms.html) and [sequence adaptors](https://tristanbrindle.com/flux/reference/adaptors.html) for creating powerful and efficient data pipelines
* Much improved safety compared with standard library iterators and ranges
* Improved ease of use in common cases, particularly for defining your own sequences and adaptors
* Improved run-time efficiency for some common operations
* Compatibility with existing standard library types and concepts

## A Quick Example ##

```cpp
constexpr auto result = flux::ints()                        // 0,1,2,3,...
                         .filter(flux::pred::even)          // 0,2,4,6,...
                         .map([](int i) { return i * 2; })  // 0,4,8,12,...
                         .take(3)                           // 0,4,8
                         .sum();                            // 12

static_assert(result == 12);
```

Try it [on Compiler Explorer](https://flux.godbolt.org/z/K86EsGMcT)!

## Getting Started ##

### Single header ###

Flux can be used with any C++ build system by downloading the [latest automatically generated single header file](https://raw.githubusercontent.com/tcbrindle/flux/main/single_include/flux.hpp) and `#include`-ing it along with your own sources.

### CMake ###

Flux can be used with CMake's [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html) to download the library and keep it up to date. Add the following to your CMakeLists.txt:

```cmake
include(FetchContent)

FetchContent_Declare(
    flux
    GIT_REPOSITORY https://github.com/tcbrindle/flux.git
    GIT_TAG main # Replace with a git commit id to fix a particular revision
)

FetchContent_MakeAvailable(flux)
```

and then add

```cmake
target_link_libraries(my_target PUBLIC flux::flux)
```

where `my_target` is the name of the library or executable target that you want to build with Flux.

If you don't have an existing CMake project or just want to play around, you can find a starter project in [this repository](https://github.com/tcbrindle/flux_cmake_demo).

### vcpkg ###

Flux is available in [vcpkg](https://vcpkg.io) and can be installed with

```
vcpkg install flux
```

See the vcpkg documentation for more details.

## Compiler support ##

Flux requires a recent compiler with good support for the C++20 standard. It is tested with:

* GCC 11.3 and newer
* LLVM Clang 16 and newer
* MSVC 2022

AppleClang is currently not usable due to missing C++20 support.

## The Flux difference ##

Flux provides a broadly equivalent feature set to C++20 Ranges, but uses a slightly different iteration model based around *cursors* rather than *iterators*. Flux cursors are a generalisation of array *indices*, whereas STL iterators are a generalisation of array *pointers*.

A Flux `sequence` provides four basis operations:

* `flux::first(seq)` returns an object called a *cursor*, which represents a position in a sequence. For a sequence with N elements there are N+1 possible cursor positions, including the past-the-end (terminal) position.
* `flux::is_last(seq, cursor)` returns a boolean value indicating whether the cursor is in the terminal position
* `flux::inc(seq, cursor)` increments the given cursor, so that it points to the next element in the sequence
* `flux::read_at(seq, cursor)` returns the sequence element at the given cursor position

These basis operations are equivalent to the basis operations on STL iterators (`begin()`, `iter == end()`, `++iter` and `*iter` respectively). The crucial difference is that in the Flux model, you need to **provide both the sequence and the cursor** to each function call, whereas in the STL model the iterator must know how to increment and dereference itself.

> STL iterators are "smart", but Flux cursors are not!

This seemingly small change has some far-reaching consequences. In particular:

* Because we have access to the sequence object during increment and dereference operations, we can provide **inexpensive universal bounds checking** for sequences (with a clearly marked opt-out where needed)
* Because we need the sequence object in order to do anything useful with a cursor, **dangling cursors are not possible by design**: if the sequence object is no longer around, the cursor can't be used
* Because a cursor only represents a position in a sequence (like an integer index for an array), cursor **invalidation is much less likely** when modifying the underlying sequence -- and if the element at the given position no longer exists, this will be caught by the bounds check at the next attempted read.
* Because element access requires the original sequence, we don't need to make a distinction between mutable `iterator`s and `const_iterator`s -- the same cursor type is used for both const and non-const access, making cursors and sequences **considerably simpler to implement** than STL iterators and ranges.

Like STL input ranges, basic Flux sequences are assumed to be single-pass by default. Flux also provides various far more powerful sequences, closely modeled on their STL counterparts:

* `multipass_sequence`s allow multiple cursors to iterate over the sequence independently, potentially passing over each position multiple times
* `bidirectional_sequence`s are multipass sequences whose cursors can be decremented as well as incremented
* `random_access_sequence`s are bidirectional sequences whose cursors can be incremented or decremented an arbitrary number of places in constant time
* `contiguous_sequence`s are random-access sequences which are backed by a contiguous, in-memory array

The close correspondence between Flux's sequence concepts and their ranges counterparts means that we can easily bridge the gap between the two libraries. In particular, we provide STL-compatible iterators to ensure that **every Flux sequence is also a C++20 range**, meaning they can be used with existing STL algorithms (and with range-for loops!) just like any other range.

## Documentation ##

Work-in-progress reference documentation can be found at [tristanbrindle.com/flux](https://tristanbrindle.com/flux)

## Audio and Video ##

### Conference Talks ###

| Conference     |  Year  |                                                           Title                                                           |
| :--------:     | :----: | :-----------------------------------------------------------------------------------------------------------------------:
| C++ on Sea     |  2023  | [Iteration Revisited: A Safer Iteration Model for C++](https://youtu.be/4dADc4RRC48)
| CppNorth       |  2023  | [Lightning Talk: Faster Filtering with Flux](https://youtu.be/wAOgEWzi4bk)

### Podcasts ###

| Podcast | Episode |     Date   | Title                            |
| :-----: | :-----: | :--------: | :------------------------------: |
|  ADSP   |   125   | 2023-04-14 | [NanoRange with Tristan Brindle](https://adspthepodcast.com/2023/04/14/Episode-125.html)
|  ADSP   |   126   | 2023-04-21 | [Flux (and Flow) with Tristan Brindle](https://adspthepodcast.com/2023/04/21/Episode-126.html)
|  ADSP   |   127   | 2023-04-28 | [Flux, ChatGPT & More with Tristan Brindle](https://adspthepodcast.com/2023/04/28/Episode-127.html)
| CppCast |   364   | 2023-07-07 | [Sequence-Oriented Programming](https://cppcast.com/sequence_oriented_programming/)

## Stability ##

Flux is still pre-1.0 and in rapid development. As such, there are no API stability guarantees at this time.

Once Flux 1.0 is released we aim to follow [semantic versioning](https://semver.org).

## License ##

Flux is available under the [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)
