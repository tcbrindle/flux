
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

``single``
----------

..  function::
    template <typename T> \
        requires std::move_constructible<T> \
    auto single(T&& obj) -> contiguous_sequence auto;