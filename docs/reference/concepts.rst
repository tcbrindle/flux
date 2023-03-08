
Concepts
********

..  namespace:: flux

The following definitions use the exposition-only alias::

    template <typename Seq>
    using Traits = sequence_traits<std::remove_cvref_t<Seq>>; // exposition-only

Type aliases
============

``cursor_t``
------------

..  type:: template <typename Seq> cursor_t = decltype(Traits<Seq>::first(std::declval<Seq&>()));

    The sequence's associated cursor type, used for iteration and element access.

    Note that if :expr:`Seq` and :expr:`Seq const` are both iterable, then they must have the same cursor type.

``element_t``
-------------

..  type:: template <typename Seq> element_t = decltype(Traits<Seq>::read_at(std::declval<Seq&>(), std::declval<cursor_t<Seq> const&>()));

    The element type of the sequence. This must be "referenceable", i.e. not :expr:`void`, but otherwise may be a reference type or an object type. The element types of :var:`Seq` and :expr:`Seq const` will often differ.

``rvalue_element_t``
--------------------

..  type:: template <typename Seq> rvalue_element_t

    * If a sequence provides a custom implementation of :expr:`move_at()`, then its rvalue element type is the return type of :expr:`move_at()`.
    * Otherwise, if :type:`element_t\<Seq>` names an lvalue reference type :expr:`T&`, then :type:`rvalue_element_t\<Seq>` is :expr:`T&&`.
    * Otherwise, :type:`rvalue_element_t\<Seq>` is the same as :type:`element_t\<Seq>`

    The *rvalue element type* of a sequence is the type that is used when we want to *move* an element out of the sequence. Sequence implementations can customise this providing their own :expr:`move_at()` specialisation; otherwise, the default is equivalent to :expr:`std::move(read_at(seq, cur))`.

    The :concept:`sequence` concept requires that a sequence's element type and rvalue element type model :concept:`std::common_reference_with`.

``value_t``
-----------

..  type:: template <typename Seq> value_t

    The type alias :type:`value_t` is

    * :type:`Traits\<Seq>::value_type` if that is well-formed and names a type
    * Otherwise, :type:`std::remove_cvref_t\<Seq>::value_type` if that is well-formed and names a type
    * Otherwise, :type:`std::remove_cvref_t\<element_t\<Seq>>` if that is well-formed
    * Otherwise, :type:`value_t\<Seq>` is not defined

    The *value type* of a sequence is that type that is used to "save" an element for later re-use. For example, if a sequence has element type :expr:`int const&`, then its corresponding value type would normally be :expr:`int`. The value type should be an object type, not a reference.

    Flux provides the ability to customise the value type, but this is normally only needed for specialised adaptors like :func:`zip`. Most sequence implementations do not need to provide this themselves.

``distance_t``
--------------

..  type:: distance_t = config::int_type;

    Flux uses a single signed integer type, aliased as :type:`distance_t`, to represent all distances, sizes, offsets and so on in the library. This will usually be machine word sized i.e. :type:`int64_t` on a 64-bit machine or :type`int32_t` on a 32-bit machine. It can optionally be configured to be a larger signed integer type.

``bounds_t``
------------

..  struct:: template <cursor Cur> bounds

..  type:: template <sequence Seq> bounds_t = bounds<cursor_t<Seq>>;

Concepts
========

``cursor``
----------

..  concept::
    template <typename C> cursor = std::movable<C>;

    In Flux, a *cursor* is an object that represents a position of an element in a sequence (or, more precisely, a position *between* two elements in a sequence, or at the beginning or end). Flux requires only that basic cursors are *movable* (that is, move-constructible, move-assignable and destructible), but it is assumed that these operations are "cheap".

``regular_cursor``
------------------

..  concept::
    template <typename C> regular_cursor = cursor<C> && std::regular<C>;

    A *regular cursor* is a cursor that is additionally a *regular type* -- that is, it is default constructible, copy-constructible, copy-assignable and equality-comparable. Regular cursors may be copied within algorithm implementations, so copying should be "cheap".

    The equality operator for :concept:`regular_cursor` s belonging to the same sequence should return :texpr:`true` if the cursors represent the same position, and :texpr:`false` otherwise.

    A sequence whose cursor type satisfies :concept:`regular_cursor` is assumed to be a *multipass sequence* unless specifically disabled with :any:`disable_multipass`.


``ordered_cursor``
------------------

..  concept::
    template <typename C> \
    ordered_cursor = \
        regular_cursor<C> && \
        std::three_way_comparable<C, std::strong_ordering>;

    An *ordered cursor* is a regular cursor which is additionally *totally ordered* and may be compared using the "spaceship" operator ``<=>``.

    For two :concept:`ordered_cursor` s :expr:`a` and :expr:`b` belonging to the same sequence, :expr:`a <=> b` should return :expr:`std::strong_ordering::less` if :expr:`a` represents a position earlier in the sequence, :expr:`greater` if :expr:`b` represents a position earlier in the sequence, and :expr:`equal` otherwise.

``sequence``
------------

.. concept::
   template <typename S> sequence

   .. code-block:: cpp

    template <typename Seq> sequence =
        requires (Seq& seq) {
            { Traits<Seq>::first(seq) } -> cursor;
        } &&
        requires (Seq& seq, cursor_t<Seq> const& cur) {
            { Traits<Seq>::is_last(seq, cur) } -> std::same_as<bool>;
            { Traits<Seq>::read_at(seq, cur) } -> /*can-reference*/;
        } &&
        requires (Seq& seq, cursor_t<Seq>& cur) {
            { Traits<Seq>::inc(seq, cur) }
        };

``multipass_sequence``
----------------------

..  concept::
    template <typename Seq> multipass_sequence = \
        sequence<Seq> && \
        regular_cursor<cursor_t<Seq>> && \
        !disable_multipass<Seq>;

    A *multipass sequence* is a sequence which supports separate iteration by two or more cursors independently. A container is typically a multipass sequence; a :type:`std::istream` is a non-multipass sequence, also known as a *single-pass* sequence.

    By default, Flux assumes that a sequence is multipass if its cursor satisfies :concept:`regular_cursor` (that is, it is default-constructible, copyable and equality-comparable). It is recommended that single-pass sequences make their cursors move-only.

    Sometimes a sequence's cursor type may satisfy `regular_cursor`, even though the sequence itself is semantically only single-pass. In this case, the sequence can explicitly opt-out of multipass behaviour either by

    * providing a variable template specialisation::

        template <>
        inline constexpr bool flux::disable_multipass<SeqType> = true;

    * or, providing a `static constexpr bool` member variable :expr:`Traits<Seq>::disable_multipass = true`.

``bidirectional_sequence``
--------------------------

..  concept::
    template <typename Seq> bidirectional_sequence

    The :concept:`bidirectional_sequence` concept is defined as::

        template <typename Seq>
        bidirectional_sequence =
            multipass_sequence<Seq> &&
            requires (Seq& seq, cursor_t<Seq>& cur) {
                { Traits<Seq>::dec(seq, cur); }
        }

    A :concept:`bidirectional_sequence` is a multipass sequence which additionally allows cursors to be *decremented* as well as *incremented* -- that is, one which allows backwards iteration.

``random_access_sequence``
--------------------------

.. concept::
   template <typename S> random_access_sequence

``bounded_sequence``
--------------------

..  concept::
    template <typename Seq> bounded_sequence

``sized_sequence``
------------------

..  concept::
    template <typename Seq> sized_sequence

``contiguous_sequence``
-----------------------

..  concept::
    template <typename Seq> contiguous_sequence

``infinite_sequence``
---------------------

..  concept::
    template <typename Seq> infinite_sequence

``writable_sequence_of``
------------------------

..  concept::
    template <typename Seq, typename T> writable_sequence_of


