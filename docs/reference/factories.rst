
Factories
*********

..  namespace:: flux

``array_ptr``
-------------

..  class:: template <typename T> array_ptr : public inline_sequence_base<array_ptr<T>>

    A type with "fat" pointer semantics, implemented as a ``(pointer, length)`` pair. It can be used across API boundaries as a "type erased" contiguous sequence. It is the Flux-native equivalent of :type:`std::span`.

    All :type:`array_ptr` s are trivially movable. If :var:`T` is ``const`` then :type:`array_ptr<T>` is additionally trivially copyable, otherwise it is move-only.

    For the purposes of documentation below, the exposition-only concept :concept:`non_slicing_ptr_convertible` is defined as::

        template <typename From, typename To>
        concept non_slicing_ptr_convertible = std::convertible_to<From (*)[], To (*)[]>;


    :constructors:

    ..  function:: array_ptr() = default;

        Default initializes an empty :type:`array_ptr`

        :postconditions:

            :expr:`data() == nullptr`

            :expr:`size() == 0`

    ..  function::
        template <contiguous_sequence Seq> \
            requires see_below \
        explicit array_ptr(Seq& seq);

        Constructs an :type:`array_ptr` from the contiguous sequence :var:`seq`.

        :postconditions:

            :expr:`data() == flux::data(seq)`

            :expr:`size() == flux::size(seq)`

        :requires:

            * :var:`Seq` is not a specialisation of :type:`array_ptr`
            * :expr:`non_slicing_ptr_convertible<std::remove_reference_t<element_t<Seq>>, T>` is ``true``

    ..  function::
        template <typename U> \
            requires (!std::same_as<U, T> && non_slicing_ptr_convertible<U, T>) \
        array_ptr(array_ptr<U> const& other) noexcept;

        Implicit conversion constructor from a compatible :type:`array_ptr`.

        :postconditions:

            :expr:`data() == other.data()`

            :expr:`size() == other.size()`

    :friend functions:

    ..  function::
        friend auto operator==(array_ptr lhs, array_ptr rhs) -> bool;

        Equivalent to::

            lhs.data() == rhs.data() && lhs.size() == rhs.size()

        but ensures that the pointer comparison is always well defined.

        ..  note::

            :type:`array_ptr` has pointer semantics, and so equality comparison tests the addresses of the pointed-to objects.

            If you want to check whether the elements of two :type:`array_ptr` s compare equal, you can use :func:`flux::equal`.

``empty``
---------

..  var:: template <typename T> requires std::is_object_v<T> contiguous_sequence auto empty;

    A variable template that names a contiguous sequence of zero :var:`T` s. Any attempt to read from an :var:`empty` will result in a runtime error.

    :expr:`is_empty(empty)` is vacuously :texpr:`true`.

``from_istream``
----------------

..  function::
    template <std::default_initializable T, typename CharT, typename Traits> \
    auto from_istream(std::basic_istream<CharT, Traits>& is) -> sequence auto;

    Returns a single-pass, read-only sequence which yields successive :var:`T` s extracted from :var:`is` using :expr:`operator>>()`. The element type of the returned sequence is :expr:`T const&`.


``from_istreambuf``
-------------------

..  function::
    template <typename CharT, typename Traits> \
    auto from_istreambuf(std::basic_streambuf<CharT, Traits>* buf) -> sequence auto;

..  function::
    template <typename CharT, typename Traits> \
    auto from_istreambuf(std::basic_istream<CharT, Traits>& is) -> sequence auto;

    Returns a single-pass, read-only sequence which yields successive characters from the given streambuf using :func:`std::basic_streambuf::sgetc()`. Iteration is complete when the streambuf reaches EOF.

    The second overload is equivalent to::

        from_streambuf(is.rdbuf())

``from_range``
--------------

..  function::
    template <std::ranges::viewable_range R> \
        requires std::ranges::input_range<R> \
    auto from_range(R&& rng) -> sequence auto;

..  function::
    template <typename R, typename C = std::remove_reference<R> const&> \
        requires std::viewable_range<C> && std::input_range<C> \
    auto from_crange(R&& rng) -> sequence auto;

``generator``
-------------

..  class:: template <typename ElemT> generator

``getlines``
------------

..  function::
    template <typename CharT, typename Traits> \
    auto getlines(std::basic_istream<CharT, Traits>& istream, CharT delim) -> sequence auto;

..  function::
    template <typename CharT, typename Traits> \
    auto getlines(std::basic_istream<CharT, Traits>& istream) -> sequence auto;

``ints``
--------

..  function::
    auto ints() -> random_access_sequence auto;

..  function::
    auto ints(distance_t from) -> random_access_sequence auto;

..  function::
    auto ints(distance_t from, distance_t to) -> random_access_sequence auto;

``iota``
--------

..  function::
    template <typename T> \
        requires see_below \
    auto iota(T from) -> multipass_sequence auto;

..  function::
    template <typename T> \
        requires see_below \
    auto iota(T from, T to) -> multipass_sequence auto;

``repeat``
----------

..  function::
    template <typename T> \
        requires std::movable<std::decay_t<T>> \
    auto repeat(T&& obj) -> infinite_sequence auto;

..  function::
    template <typename T> \
        requires std::movable<std::decay_t<T>> \
    auto repeat(T&& obj, std::integral auto count) -> random_access_sequence auto;

    Returns a sequence which yields a const reference to :var:`obj` endlessly (for the first overload) or exactly :var:`count` times (for the second overload).

    For both overloads, the returned sequence is always a :concept:`random_access_sequence`. For the second overload it additionally models :concept:`sized_sequence` and :concept:`bounded_sequence`.

    ..  caution::
        In order to provide random-access functionality, cursors for repeat sequences keep a :type:`size_t` count of how many times they have been incremented. For very long-running programs using the infinite version of :func:`repeat` it may be possible to overflow this counter. (Assuming 1000 iterations per second, this would take approximately 49 days on a machine with a 32-bit :type:`size_t`, or around 500 million years on a 64-bit machine.)

        While this won't cause undefined behaviour, calling :func:`distance` with cursors that have rolled over may give incorrect results, and may result in a runtime error in debug mode if the result cannot be represented as a :type:`distance_t`.

    :param obj: A movable object which will be stored in the returned sequence object
    :param count: If provided, a non-negative value to indicate the size of the returned sequence. If not provided, the returned sequence will be infinite.

    :returns: A sequence which repeatedly yields a const reference to :var:`obj`

    :example:

    ..  literalinclude:: ../../example/docs/repeat.cpp
        :language: cpp
        :dedent:
        :lines: 16-32

    :see also:

        * `std::views::repeat <https://en.cppreference.com/w/cpp/ranges/repeat_view>`_ (C++23)
        * :func:`flux::cycle`
        * :func:`flux::single`
        * :func:`flux::iota`


``single``
----------

..  function::
    template <typename T> \
        requires std::move_constructible<T> \
    auto single(T&& obj) -> contiguous_sequence auto;

``unfold``
----------

..  function::
    template <typename Func, typename Seed> \
        requires see_below \
    auto unfold(Func func, Seed&& seed) -> infinite_sequence auto;

    Generates an infinite single-pass sequence by repeatedly invoking the unary function :var:`func`, starting with the given :var:`seed`.

    Whereas :func:`fold` takes a sequence and a function and produces a single value, :func:`unfold` does the opposite: it takes a function and a single value an produces a sequence.

    Let :type:`R` be :expr:`std::decay_t<std::invoke_result_t<Func&, Seed>>`. The sequence object contains variable :expr:`state` of type :type:`R`, which is initialised from :var:`seed`. At every call to :func:`inc`, the internal state is updated as if by::

         state = std::invoke(func, std::move(state));

    A call to :func:`read_at()` returns a read-only reference to the internal state, with type :expr:`R const&`.

    .. note::

        As the provided function can potentially be called again and again "forever", it's important to make sure that this can't cause undefined behaviour, for example by signed integer overflow -- perhaps by using unsigned ints instead, or by ensuring that iteration is terminated before this occurs.

    :requires:

    Let :type:`R` be :expr:`std::decay_t<std::invoke_result_t<Func&, Seed>>`. Then the expression in the ``requires`` clause is equivalent to::

        std::constructible_from<R, Seed> &&
        std::invocable<Func&, R> &&
        std::assignable_from<R&, std::invoke_result_t<Func&, R>>

    :param func: A unary callable with a signature compatible with ``R(R)``
    :param seed: The initial seed value. Must be convertible to the result type of :var:`func`.

    :returns: An infinite single-pass sequence generated by repeated invocations of :var:`func`, starting with the :var:`seed` value.

    :example:

    ..  literalinclude:: ../../example/docs/unfold.cpp
        :language: cpp
        :dedent:
        :lines: 16-29

    :see also:
        * :func:`flux::fold`
        * :class:`flux::generator`