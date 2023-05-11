
Sources
*******

..  namespace:: flux

``array_ptr``
-------------

..  class:: template <typename T> array_ptr

    :constructors:

    ..  function:: array_ptr() = default;

    ..  function::
        template <contiguous_sequence Seq> \
            requires see_below \
        array_ptr::array_ptr(Seq& seq);

    ..  function::
        template <typename U> \
            requires see_below \
        array_ptr::array_ptr(array_ptr<U> other);

    :friend functions:

    ..  function::
        friend auto operator==(array_ptr lhs, array_ptr rhs) -> bool;

``empty``
---------

..  var:: template <typename T> requires std::is_object_v<T> contiguous_sequence auto empty;

``from_istream``
----------------

..  function::
    template <std::default_initializable T, typename CharT, typename Traits> \
    auto from_istream(std::basic_istream<CharT, Traits>& is) -> sequence auto;

``from_istreambuf``
-------------------

..  function::
    template <typename CharT, typename Traits> \
    auto from_istreambuf(std::basic_streambuf<CharT, Traits>* buf) -> sequence auto;

..  function::
    template <typename CharT, typename Traits> \
    auto from_istream(std::basic_istream<CharT, Traits>& is) -> sequence auto;

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