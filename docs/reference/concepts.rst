
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

``const_element_t``
-------------------

..  type:: template <typename Seq> const_element_t = \
           std::common_reference_t<value_t<Seq> const&&, element_t<Seq>>;

    The *const element type* of a sequence is the element type which is yielded when requesting read-only access to the sequence. For example, if the element type of a sequence is :expr:`int&`, its corresponding :type:`const_element_t` will usually be :expr:`int const&`.

    A sequence models :concept:`read_only_sequence` if its :type:`element_t` and :type:`const_element_t` are the same type.

    ..  note:: :type:`const_element_t\<Seq>` may differ from :type:`element_t\<Seq const>` when :expr:`Seq` has reference-like semantics, for example :expr:`flux::array_ptr<T>` or :expr:`std::reference_wrapper<Seq>`.

``distance_t``
--------------

..  type:: distance_t = config::int_type;

    Flux uses a single signed integer type, aliased as :type:`distance_t`, to represent all distances, sizes, offsets and so on in the library. This will usually be machine word sized i.e. :type:`int64_t` on a 64-bit machine or :type:`int32_t` on a 32-bit machine. It can optionally be configured to be a larger signed integer type.

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

..  concept::
    template <typename S> sequence

    A Flux sequence is a homogeneous collection of *elements* which we can iterate over. Sequences provide the following four operations:

    * :expr:`first(seq)`, which returns a :concept:`cursor` representing the iteration state
    * :expr:`is_last(seq, cur)` which returns a boolean indicating whether iteration is complete
    * :expr:`read_at(seq, cur)` which accesses the sequence element corresponding to the given cursor position
    * :expr:`inc(seq, cur)` which takes the cursor by reference and advances it so that it refers to the next sequence element

    A sequence may be *single-pass*, meaning we can only visit each position once, or may be *multipass*, indicating that we can revisit cursor positions multiple times.

    The :concept:`sequence` concept is defined as::

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

    A bidirectional sequence is a multipass sequence which additionally allows cursors to be *decremented* as well as *incremented* -- that is, one which allows backwards iteration.

    The :concept:`bidirectional_sequence` concept is defined as::

        template <typename Seq>
        bidirectional_sequence =
            multipass_sequence<Seq> &&
            requires (Seq& seq, cursor_t<Seq>& cur) {
                { Traits<Seq>::dec(seq, cur); }
            };



``random_access_sequence``
--------------------------

..  concept::
    template <typename S> random_access_sequence

    A random-access sequence is a bidirectional sequence which allows cursors to be incremented and decremented an arbitrary number of places in constant time. Additionally, random-access sequences support a :func:`distance` operation which returns the signed offset between two cursor positions in constant time.

    The cursor type for a random-access sequence must model :concept:`ordered_cursor`, meaning we can compare cursor positions to know whether one position is earlier or later in the sequence than the other.

    The :concept:`random_access_sequence` concept is defined as::

        template <typename Seq>
        concept random_access_sequence =
            bidirectional_sequence<Seq> && ordered_cursor<cursor_t<Seq>> &&
            requires (Seq& seq, cursor_t<Seq>& cur, distance_t offset) {
                { Traits::inc(seq, cur, offset) };
            } &&
            requires (Seq& seq, cursor_t<Seq> const& cur) {
                { Traits::distance(seq, cur, cur) } -> std::convertible_to<distance_t>;
            };

``bounded_sequence``
--------------------

..  concept::
    template <typename Seq> bounded_sequence

    A *bounded sequence* is one which can provide a past-the-end cursor in constant time. Iterating over a sequence in reverse requires a sequence that is both *bounded* and *bidirectional*, because we need to be able to find the end and move backwards from there.

    Iterating over a bounded always terminates, so a sequence cannot be both a :concept:`bounded_sequence` and an :concept:`infinite_sequence`.

    The :concept:`bounded_sequence` concept is defined as::

        template <typename Seq>
        concept bounded_sequence =
            sequence<Seq> &&
            requires (Seq& seq) {
                { Traits::last(seq) } -> std::same_as<cursor_t<Seq>>;
            };

``sized_sequence``
------------------

..  concept::
    template <typename Seq> sized_sequence

    A *sized sequence* is a sequence that knows the number of elements it contains in constant time.

    For a bounded, random-access sequence we can always calculate the size in constant time by taking the distance between the first and last cursor positions; therefore all bounded, random-access sequences automatically satisfy :concept:`sized_sequence`.

    The :concept:`sized_sequence` concept is defined as::

        template <typename Seq>
        concept sized_sequence =
            sequence<Seq> &&
            (requires (Seq& seq) {
                { Traits::size(seq) } -> std::convertible_to<distance_t>;
            } || (
                random_access_sequence<Seq> && bounded_sequence<Seq>
            ));

``contiguous_sequence``
-----------------------

..  concept::
    template <typename Seq> contiguous_sequence

    A *contiguous sequence* is a bounded, random-access sequence whose elements are stored contiguously in memory. Some algorithms are able to use low-level operations such as :func:`memcpy` when operating on contiguous sequences of trivial types.

    The :concept:`contiguous_sequence` concept is defined as::

        template <typename Seq>
        concept contiguous_sequence =
            random_access_sequence<Seq> &&
            bounded_sequence<Seq> &&
            std::is_lvalue_reference_v<element_t<Seq>> &&
            std::same_as<value_t<Seq>, std::remove_cvref_t<element_t<Seq>>> &&
            requires (Seq& seq) {
                { Traits::data(seq) } -> std::same_as<std::add_pointer_t<element_t<Seq>>>;
            };

``infinite_sequence``
---------------------

..  concept::
    template <typename Seq> infinite_sequence

    An *infinite sequence* is one which is known statically never to terminate: that is, its :func:`is_last` implementation always returns ``false``.

    A sequence which is bounded or sized cannot also be an :concept:`infinite_sequence`.

    ..  note::

        The :concept:`infinite_sequence` concept says that iteration will never terminate, and the :concept:`bounded_sequence` concept says that iteration will terminate at the :func:`last` position after a finite number of steps. Sequences which are neither *infinite* nor *bounded* provide no compile-time information either way.

    A sequence implementation may indicate that is is infinite by setting::

        static constexpr bool is_infinite = true;

    in its :type:`sequence_traits`.

``read_only_sequence``
----------------------

..  concept::
    template <typename Seq> read_only_sequence = \
        sequence<Seq> && \
        std::same_as<element_t<Seq>, const_element_t<Seq>>;

    A *read-only sequence* is one which does not allow modification of its elements via the sequence interface -- that is, its :func:`read_at` method returns either a const reference or a prvalue.

``const_iterable_sequence``
---------------------------

..  concept::
    template <typename Seq> const_iterable_sequence

    A sequence :var:`Seq` is *const-iterable* if we can also use :expr:`Seq const` as a sequence with the expected semantics. That is, ``Seq`` and ``Seq const`` must use the same cursor type, model the same set of sequence concepts, and return the same elements when iterated over (except that ``Seq const`` may strengthen the const-qualification of the elements).

    ..  important::

        The immutability of a sequence object is not necessarily related to the immutability of its elements. For example, it's possible to have a const-qualified sequence whose elements are mutable, or a non-const sequence whose elements are immutable.

        Given a sequence ``S``:

        * use :expr:`const_iterable_sequence<S>` if you need to know whether you can iterate over ``S const``
        * use :expr:`read_only_sequence<S>` if you need to know whether you can mutate the elements of ``S`` via the sequence API

    Single-pass sequences are typically not const-iterable. Multipass and higher sequences which cache elements for performance or correctness reasons are not const-iterable.

    The :concept:`const_iterable_sequence` concept is defined as::

        template <typename Seq>
        concept const_iterable_sequence =
            // Seq and Seq const must both be sequences
            sequence<Seq> && sequence<Seq const> &&
            // Seq and Seq const must have the same cursor and value types
            std::same_as<cursor_t<Seq>, cursor_t<Seq const>> &&
            std::same_as<value_t<Seq>, value_t<Seq const>> &&
            // Seq and Seq const must have the same const_element type
            std::same_as<const_element_t<Seq>, const_element_t<Seq const>> &&
            // Seq and Seq const must model the same extended sequence concepts
            (multipass_sequence<Seq> == multipass_sequence<Seq const>) &&
            (bidirectional_sequence<Seq> == bidirectional_sequence<Seq const>) &&
            (random_access_sequence<Seq> == random_access_sequence<Seq const>) &&
            (contiguous_sequence<Seq> == contiguous_sequence<Seq const>) &&
            (bounded_sequence<Seq> == bounded_sequence<Seq const>) &&
            (sized_sequence<Seq> == sized_sequence<Seq const>) &&
            (infinite_sequence<Seq> == infinite_sequence<Seq const>) &&
            // If Seq is read-only, Seq const must be read-only as well
            (!read_only_sequence<Seq> || read_only_sequence<Seq const>);

``writable_sequence_of``
------------------------

..  concept::
    template <typename Seq, typename T> writable_sequence_of

    A sequence :var:`Seq` models :expr:`writable_sequence_t<Seq, T>` for a type :var:`T` if :expr:`element_t<Seq>` is assignable from an object of type :var:`T`.

    The :concept:`writable_sequence_of` concept is defined as::

        template <typename Seq, typename T>
        concept writable_sequence_of =
            sequence<Seq> &&
            requires (element_t<Seq> e, T&& item) {
                { e = std::forward<T>(item) } -> std::same_as<element_t<Seq>&>;
            };

``element_swappable_with``
--------------------------

..  concept::
    template <typename Seq1, typename Seq2> element_swappable_with

    A pair of sequences :var:`Seq1` and :var:`Seq2` model :concept:`element_swappable_with` if their respective elements can be swapped, that is, we can assign to an element of :var:`Seq1` from an rvalue element of :var:`Seq2` and vice-versa.

    Formally, the :concept:`element_swappable_with` concept is defined as::

        template <typename Seq1, typename Seq2>
        concept element_swappable_with =
            std::constructible_from<value_t<Seq1>, rvalue_element_t<Seq1>> &&
            std::constructible_from<value_t<Seq2>, rvalue_element_t<Seq2>> &&
            writable_sequence_of<Seq1, rvalue_element_t<Seq2>> &&
            writable_sequence_of<Seq1, value_t<Seq2>&&> &&
            writable_sequence_of<Seq2, rvalue_element_t<Seq1>> &&
            writable_sequence_of<Seq2, value_t<Seq1>&&>;


``ordering_invocable``
----------------------

..  concept::
    template <typename Fn, typename T, typename U, typename Cat = std::partial_ordering> \
    ordering_invocable

    The concept :concept:`ordering_invocable` signifies that the binary invocable :var:`Fn` return a value of one of the standard comparison categories, convertible to :var:`Cat`, for all combinations of arguments of types :var:`T` and :var:`U`

    Semantic requirements:

    * Let :expr:`r1 = fn(a, b)` and :expr:`r2 = fn(b, c)`. If :expr:`r1 == r2` and :expr:`r1 != std::partial_ordering::unordered`, then :expr:`fn(a, c) == r1`.
    * :expr:`fn(a, b) == std::partial_ordering::less` if and only if :expr:`fn(b, a) == std::partial_ordering::greater`

    The :concept:`ordering_invocable` concept is defined as::

        template <typename Fn, typename T, typename U, typename Cat>
        concept ordering_invocable_ = // exposition-only
            std::regular_invocable<Fn, T, U> &&
            std::same_as<
                std::common_comparison_category_t<std::decay_t<std::invoke_result_t<Fn, T, U>>>,
                                                  Cat>,
                Cat>;

        template <typename Fn, typename T, typename U, typename Cat = std::partial_ordering>
        concept ordering_invocable =
            ordering_invocable_<Fn, T, U, Cat> &&
            ordering_invocable_<Fn, U, T, Cat> &&
            ordering_invocable_<Fn, T, T, Cat> &&
            ordering_invocable_<Fn, U, U, Cat>;


``weak_ordering_for``
----------------------------

..  concept::
    template <typename Fn, typename Seq1, typename Seq2 = Seq1> \
    weak_ordering_for

    Signifies that a binary callable :var:`Fn` forms a strict weak order over the elements of sequences :var:`Seq1` and :var:`Seq2`.

    It is defined as::

        template <typename Fn, typename Seq1, typename Seq2 = Seq1>
        concept weak_ordering_for =
            sequence<Seq1> &&
            sequence<Seq2> &&
            ordering_invocable<Fn&, element_t<Seq1>, element_t<Seq2>, std::weak_ordering> &&
            ordering_invocable<Fn&, value_t<Seq1>&, element_t<Seq2>, std::weak_ordering> &&
            ordering_invocable<Fn&, element_t<Seq1>, value_t<Seq2>&, std::weak_ordering> &&
            ordering_invocable<Fn&, value_t<Seq1>&, value_t<Seq2>&, std::weak_ordering> &&
            ordering_invocable<Fn&, common_element_t<Seq1>, common_element_t<Seq2>, std::weak_ordering>;

