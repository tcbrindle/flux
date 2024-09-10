
Library Configuration
*********************

..  namespace:: flux

The following C preprocessor defines can be used to configure various aspects of the library behaviour.

These must be set before ``#include`` -ing any Flux headers.

..  important::

    All translation units using Flux which are intended to be linked together **must** use the same configuration flags, otherwise ODR violations will occur.

    It is strongly recommended to use your build system to pass the same flags to all TUs in your project, for example by using :func:`target_compile_definitions()` in CMake

Runtime Error Policy
=====================

When normal execution of a program cannot continue, Flux will raise a *runtime error*. Typically this happens because the library has detected a situation that would otherwise lead to undefined behaviour -- for example, an out-of-bounds read of a sequence or a dereference of an empty :type:`flux::optional`. A runtime error will be handled according to the configured error policy: one of *terminate*, *fail fast* or *unwind*. The default error policy is *terminate*.

Terminate
---------

..  c:macro:: FLUX_TERMINATE_ON_ERROR
..  c:macro:: FLUX_PRINT_ERROR_ON_TERMINATE

If :c:macro:`FLUX_TERMINATE_ON_ERROR` is defined, a Flux runtime error will result in a call to :func:`std::terminate`. This will in turn run the currently set terminate handler before halting the process.

By default, the library will attempt to print a short message to ``stdout`` describing the error before terminating. This can be disabled by setting :c:macro:`FLUX_PRINT_ERROR_ON_TERMINATE` to ``0``.

Fail Fast
---------

..  c:macro:: FLUX_FAIL_FAST_ON_ERROR

Alternatively :c:macro:`FLUX_FAIL_FAST_ON_ERROR` is defined, the library will attempt to halt the running process in the fastest way possible, typically by executing an illegal CPU instruction. No cleanup will occur, no debug info will be printed to the console, and no exit handlers will be called.

Using the fail fast policy typically results in the smallest binary code size.

Unwind
------

..  c:macro:: FLUX_UNWIND_ON_ERROR

..  struct:: unrecoverable_error : std::logic_error

If :c:macro:`FLUX_UNWIND_ON_ERROR` is defined, a runtime error will result in an exception being thrown, allowing "graceful shutdown" by unwinding the call stack and running destructors of automatic lifetime variables along the way. The exception has type :type:`flux::unrecoverable_error`, inheriting from :type:`std::logic_error` (and ultimately from :type:`std::exception`). Its :func:`what` message contains a short description of what went wrong.

..  attention::

    A runtime error means that a serious problem has occurred and the program cannot continue.

    Stack unwinding is intended to be used to allow controlled shutdown as opposed to abrupt termination, much like a "panic" in languages such as Rust and Go. As the name suggests, an exception of type :type:`unrecoverable_error` should **never** just be "caught and ignored".

..  note::

    According to the C++ standard, it is unspecified whether stack unwinding occurs if an exception is not caught -- an implementation may choose to immediately call :func:`std::terminate` without performing unwinding if there is no matching catch clause anywhere in the call stack.

    If using the "unwind" policy, you may also wish to wrap your :func:`main` in an appropriate try-catch block to ensure unwinding occurs on all platforms.

Debug Assertions
================

..  c:macro:: FLUX_ENABLE_DEBUG_ASSERTS

As with many libraries, Flux has extra "sanity check" assertions which are not critical but may detect implementation bugs or unexpected behaviour. By default, these extra checks are enabled in debug builds and disabled in release builds -- that is, they follow whether the :c:macro:`NDEBUG` macro is set.

Setting :c:macro:`FLUX_ENABLE_DEBUG_ASSERTS` to ``1`` will enable extra checks even in release builds, while setting it to ``0`` will disable them even in debug builds.

Static Bounds Checking
======================

..  c:macro:: FLUX_DISABLE_STATIC_BOUNDS_CHECKING

On supported compilers, Flux can use compiler extensions to turn certain runtime bounds checks which would always fail into compile-time errors -- see `this blog post <https://tristanbrindle.com/posts/compile-time-bounds-checking-in-flux>`_ for more details.

Defining the macro :c:macro:`FLUX_DISABLE_STATIC_BOUNDS_CHECKING` will disable this functionality, so that a runtime error will occur instead regardless of the compiler and optimisation settings.

Default Integer Type
====================

..  c:macro:: FLUX_INT_TYPE

Flux uses a single signed integer type, aliased as :type:`distance_t`, for all sizes, distances, offsets etc in the library. By default, this is the same as :type:`std::ptrdiff_t`, but may be customised by defining :c:macro:`FLUX_INT_TYPE` as the desired type. For example, you can use this macro to tell Flux to use 64-bit sizes even on a system with a 32-bit :type:`ptrdiff_t`::

    #define FLUX_INT_TYPE std::int64_t
    #include <flux.hpp>

    static_assert(std::same_as<flux::distance_t, std::int64_t>);

A custom :c:macro:`FLUX_INT_TYPE` must be a built-in signed integer type at least as large as :type:`std::ptrdiff_t`.


Numeric Error Policies
======================

Flux provides a selection of checked integer functions, which are used internally by the library when performing operations on ints. The behaviour of these functions can be customised by setting the overflow, divide by zero and integer cast policies as desired.

Overflow policy
---------------

..  c:macro:: FLUX_ERROR_ON_OVERFLOW
..  c:macro:: FLUX_WRAP_ON_OVERFLOW
..  c:macro:: FLUX_IGNORE_OVERFLOW

If :c:macro:`FLUX_ERROR_ON_OVERFLOW` is set, an integer operation which would overflow will instead raise a runtime error. This is the default in debug builds (i.e. ``NDEBUG`` is not set).

Alternatively, if :c:macro:`FLUX_WRAP_ON_OVERFLOW` is set, integer operations are performed as if by casting to the equivalent unsigned type, performing the operation, and then casting back to the original type. This avoids undefined behaviour (since overflow is well defined on unsigned ints) and avoids needing to generate error handing code, at the cost of giving numerically incorrect answers if overflow occurs. This is the default in release builds (i.e. ``NDEBUG`` is set).

Finally, if :c:macro:`FLUX_IGNORE_OVERFLOW` is set, the standard built-in integer operations will be used. This means that an operation which overflows will result in undefined behaviour. Use this setting if you are already handling signed integer UB by some other means (for example compiling with ``-ftrapv`` or using UB Sanitizer) and wish to avoid "double checking".

Divide by zero policy
---------------------

..  c:macro:: FLUX_ERROR_ON_DIVIDE_BY_ZERO
..  c:macro:: FLUX_IGNORE_DIVIDE_BY_ZERO

If :c:macro:`FLUX_ERROR_ON_DIVIDE_BY_ZERO` is set then a runtime error will be raised if zero is passed as the second argument to :func:`flux::num::div` or :func:`flux::num::mod`. This is the default in debug builds.

Alternatively, if :c:macro:`FLUX_IGNORE_DIVIDE_BY_ZERO` is set then no extra zero check will be used in :func:`flux::num::div` or :func:`flux::num::mod`. This is the default for release builds.

Integer cast policy
-------------------

..  c:macro:: FLUX_INTEGER_CAST_POLICY_CHECKED
..  c:macro:: FLUX_INTEGER_CAST_POLICY_UNCHECKED

If :c:macro:`FLUX_INTEGER_CAST_POLICY_CHECKED` is defined, then :expr:`flux::num::cast<To>(from)`  will (if necessary) perform a runtime check to ensure that the source value is within the bounds of the destination type -- that is, that the cast is not lossy. This is the default for debug builds.

Alternatively :c:macro:`FLUX_INTEGER_CAST_POLICY_UNCHECKED` is defined then no runtime check will occur, and :expr:`flux::num::cast<To>(from)` is equivalent to a plain :expr:`static_cast<To>(from)`. This is the default in release builds.
