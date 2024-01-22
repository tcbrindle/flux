
Adaptors
========

..  namespace:: flux

Flux adaptor functions take one or more sequences as input and return a new adapted sequence as output. When iterating, the elements of the adapted sequence are computed *lazily* from the elements of the underlying sequence. Adaptors are **sink functions**, meaning they take ownership of the sequences that are passed to them.

While functions below are shown as taking their sequence arguments by value for documentation purposes, in fact they will *reject lvalue sequence arguments that are not trivially copyable*. If you have a non-trivially copyable sequence then you need to explicitly copy or move it into the adaptor using :func:`std::move`, :func:`flux::copy` or (in C++23) :expr:`auto()`. This is to prevent accidental copying of, for example, large vectors.

You can pass a reference to a sequence into an adaptor using :func:`flux::ref` or :func:`flux::mut_ref`. Doing so introduces a *data and lifetime dependency* between the original sequence and the returned adaptor object. You **must not modify** the original sequence object (including calling its destructor) between the creation of the referencing adaptor object and its last use.


``adjacent``
^^^^^^^^^^^^

..  function::
    template <distance_t N> \
       requires (N > 0) \
    auto adjacent(multipass_sequence auto seq) -> multipass_sequence auto

    Given a compile-time size :var:`N` and a multipass sequence :var:`seq`, returns a new sequence which yields sliding windows of size :var:`N` as an :var:`N`-tuple of elements of :var:`seq`. If `seq` has fewer than :var:`N` elements, the adapted sequence will be empty.

    The :func:`slide()` adaptor is similar to :func:`adjacent()`, but takes its window size as a run-time rather than a compile-time parameter, and returns length-:any:`n` subsequences rather than tuples.

    Equivalent to::

        zip(seq, drop(seq, 1), drop(seq, 2), ..., drop(seq, N-1));

    :tparam N: The size of the sliding window. Must be greater than zero.

    :param seq: A multipass sequence.

    :returns: A sequence adaptor whose element type is a :expr:`std::tuple` of size :var:`N`, or a :expr:`std::pair` if :expr:`N == 2`.

    :models:

    .. list-table::
       :align: left
       :header-rows: 1

       * - Concept
         - When
       * - :concept:`multipass_sequence`
         - Always
       * - :concept:`bidirectional_sequence`
         - :var:`seq` is bidirectional
       * - :concept:`random_access_sequence`
         - :var:`seq` is random-access
       * - :concept:`contiguous_sequence`
         - Never
       * - :concept:`bounded_sequence`
         - :var:`seq` is bidirectional and bounded
       * - :concept:`sized_sequence`
         - :var:`seq` is sized
       * - :concept:`infinite_sequence`
         - Never
       * - :concept:`read_only_sequence`
         - :var:`seq` is read-only
       * - :concept:`const_iterable_sequence`
         - :var:`seq` is const-iterable

    :example:

    ..  literalinclude:: ../../example/docs/adjacent.cpp
        :language: cpp
        :dedent:
        :lines: 15-26

    :see also:
        * `std::views::adjacent <https://en.cppreference.com/w/cpp/ranges/adjacent_view>`_ (C++23)
        * :func:`flux::adjacent_map`
        * :func:`flux::pairwise`
        * :func:`flux::slide`

``adjacent_filter``
^^^^^^^^^^^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>, element_t<Seq>> \
    auto adjacent_filter(Seq seq, Pred pred) -> multipass_sequence auto;

    Applies the given binary predicate :var:`pred` to each pair of adjacent elements of :var:`seq`. If the predicate returns ``false``, the second element of the pair does not appear in the resulting sequence. The first element of :var:`seq` is always included in the output.

    A common use for :func:`adjacent_filter` is to remove adjacent equal elements from a sequence, which can be achieved by passing :expr:`std::not_equal_to{}` as the predicate. The :func:`dedup` function is a handy alias for :expr:`adjacent_filter(not_equal_to{})`.

    :param seq: A multipass sequence
    :param pred: A binary predicate to compare sequence elements

    :returns: The filtered sequence

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Always
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable
    :example:

    ..  literalinclude:: ../../example/docs/adjacent_filter.cpp
        :language: cpp
        :dedent:
        :lines: 15-34

    :see also:
       * :func:`flux::dedup`
       * :func:`flux::filter`


``adjacent_map``
^^^^^^^^^^^^^^^^

..  function::
    template <distance_t N> \
    auto adjacent_map(multipass_sequence auto seq, auto func) -> multipass_sequence auto;

    Applies the :var:`N`-ary function :var:`func` to overlapping windows of size ``N``.

    Equivalent to :expr:`map(adjacent\<N>(seq), unpack(func))`, but avoids forming an intermediate tuple.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Always
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`seq` is bidirectional and bounded
      * - :concept:`sized_sequence`
        - :var:`seq` is sized
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable and :var:`func` is const-invocable

``cache_last``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires bounded_sequence<Seq> || (multipass_sequence<Seq> && !infinite_sequence<Seq>) \
    auto cache_last(Seq seq) -> sequence auto;

    :func:`cache_last` is used to turn a non-bounded sequence into a :concept:`bounded_sequence`, by internally caching the cursor position of the last element,

    If passed a sequence that is already bounded, it is returned unchanged. Otherwise, the passed-in sequence must be multipass, and not infinite.

    Note that because this adaptor uses internal caching it is not const-iterable.

    :param seq: A sequence we want to ensure is bounded.

    :returns: A bounded sequence

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`Seq` is random-access
      * - :concept:`contiguous_sequence`
        - :var:`Seq` is contiguous (passthrough)
      * - :concept:`bounded_sequence`
        - Always
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is bounded and const-iterable (passthrough)

``cartesian_product``
^^^^^^^^^^^^^^^^^^^^^

..  function::
    auto cartesian_product(sequence auto seq0, multipass_sequence auto... seqs) -> sequence auto;

    Returns an adaptor yielding the `cartesian_product <https://en.wikipedia.org/wiki/Cartesian_product>`_ of the input sequences.

    The element type of the returned sequence is a tuple of the element types of the input sequences.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`seq0` is multipass
      * - :concept:`bidirectional_sequence`
        - All passed-in sequences are bidirectional and bounded
      * - :concept:`random_access_sequence`
        - All passed-in sequences are random-access and bounded
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`seq0` is bounded
      * - :concept:`sized_sequence`
        - All passed-in sequences are sized
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - All passed-in sequences are read-only
      * - :concept:`const_iterable_sequence`
        - All passed-in sequences are const-iterable

``cartesian_product_map``
^^^^^^^^^^^^^^^^^^^^^^^^^

..  function::
    template <typename Func, sequence Seq0, multipass_sequence... Seqs> \
    requires std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...> \
    auto cartesian_product_with(Func func, Seq0 seq0, Seqs... seqs) -> sequence auto;

    Given ``N`` input sequences and an ``N``-ary function :var:`func`, applies :var:`func` to the cartesian product of the elements of the input sequences.

    Equivalent to :expr:`map(cartesian_product(seq0, seqs...), unpack(func))`, but avoids forming an intermediate tuple.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`seq0` is multipass
      * - :concept:`bidirectional_sequence`
        - All passed-in sequences are bidirectional and bounded
      * - :concept:`random_access_sequence`
        - All passed-in sequences are random-access and bounded
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`seq0` is bounded
      * - :concept:`sized_sequence`
        - All passed-in sequences are sized
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - All passed-in sequences are read-only
      * - :concept:`const_iterable_sequence`
        - All passed-in sequences are const-iterable and :var:`func` is const-invocable

``chain``
^^^^^^^^^

..  function::
    template <sequence Seq0, sequence... Seqs> \
        requires see_below \
    auto chain(Seq0 seq0, Seqs... seqs) -> sequence auto;

    Combines the given input sequences into one logical sequence.

    :requires:

        * The element types of the input sequences share a common reference type to which they are all convertible. This is the element type of the returned sequence.
        * The rvalue element types of the input sequences share a common reference type to which they are all convertible. This is the rvalue element type of the returned sequence.
        * The value types of the input sequences share a common type. This is the value type of the returned sequence.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - All passed-in sequences are multipass
      * - :concept:`bidirectional_sequence`
        - All passed-in sequences are bidirectional and bounded
      * - :concept:`random_access_sequence`
        - All passed-in sequences are random-access and bounded
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - The *last* passed-in sequence is bounded
      * - :concept:`sized_sequence`
        - All passed-in sequences are sized
      * - :concept:`infinite_sequence`
        - Any passed-in sequence is infinite
      * - :concept:`read_only_sequence`
        - All passed-in sequences are read-only
      * - :concept:`const_iterable_sequence`
        - All passed-in sequences are const-iterable

``chunk``
^^^^^^^^^

..  function::
    auto chunk(sequence auto seq, std::integral auto chunk_sz) -> sequence auto;

    Splits :var:`seq` into a sequence of non-overlapping sequences, each of :var:`chunk_sz` elements. The final sequence may have fewer elements.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`seq` is multipass and bounded
      * - :concept:`sized_sequence`
        - :var:`seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`seq` is infinite and multipass
      * - :concept:`read_only_sequence`
        - :var:`seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable

``chunk_by``
^^^^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Pred> \
        requires std::predicate<Pred, element_t<Seq>, element_t<Seq>> \
    auto chunk_by(Seq seq, Pred pred) -> multipass_sequence auto;

    Splits :var:`seq` into a sequence of non-overlapping sequences using the binary predicate :var:`pred`. The given predicate should return ``true`` if both elements should be considered part of the same chunk. If the predicate returns ``false``, a new chunk will be started, with the second argument to :var:`pred` belonging to the new chunk.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Always
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable and :var:`Pred` is const-invocable


``cursors``
^^^^^^^^^^^

..  function::
    auto cursors(multipass_sequence auto seq) -> multipass_sequence auto;

    Given a sequence :var:`seq`, :expr:`cursors(seq)` returns a new sequence whose elements are the cursors of the original sequence. The :func:`cursors` sequence retains all the capabilities of the source sequence (bidirectional, random access, sized etc), up to :concept:`contiguous_sequence`.

    This is basically a passthrough adaptor, except that :expr:`read_at(seq, cur)` returns a copy of :var:`cur`.

    :param seq: A multipass sequence

    :returns: A sequence whose elements are the cursors of :var:`seq`

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Always
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`seq` is infinite
      * - :concept:`read_only_sequence`
        - Always
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable

    :example:

    ..  literalinclude:: ../../example/docs/cursors.cpp
        :language: cpp
        :dedent:
        :lines: 16-29


``cycle``
^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires multipass_sequence<Seq> || infinite_sequence<Seq> \
    auto cycle(Seq seq) -> infinite_sequence auto;

..  function::
    template <multipass_sequence Seq>\
    auto cycle(Seq seq, std::integral auto count) \
        -> multipass_sequence auto;

    Repeats the elements of :var:`seq` endlessly (for the first overload) or :var:`count` times (for the second overload).

    For the first overload, if :var:`Seq` is already an :concept:`infinite_sequence`, it is passed through unchanged.

    Otherwise, both overloads require a :concept:`multipass_sequence`, and the output is always a :concept:`multipass_sequence`. The adapted sequence is also a :concept:`bidirectional_sequence` when :var:`Seq` is both bidirectional and bounded, and a :concept:`random_access_sequence` when :var:`Seq` is random-access and bounded.

    For the second overload, the returned sequence is additionally always a :concept:`bounded_sequence` (even if :var:`Seq` is not), and a :concept:`sized_sequence` when the source sequence is sized.

    To avoid "spooky action at a distance" (where mutating :expr:`s[n]` would change the value of some other :expr:`s[m]`) :func:`cycle` provides only immutable access to the elements of :var:`seq`: that is, it behaves as if :var:`seq` were first passed through :func:`read_only`.

    .. caution::

        In order to provide random-access functionality, cursors for cycled sequences keep a :type:`size_t` count of how many times they have looped round. For very long-running programs using the infinite version of :func:`cycle` it may be possible to overflow this counter. (Assuming 1000 iterations per second, this would take approximately 49 days on a machine with a 32-bit :type:`size_t`, or around 500 million years on a 64-bit machine.)

        While this won't cause undefined behaviour, it's possible to encounter incorrect results or runtime errors when using the random-access functions on cursors which have overflowed.

    :param seq: A sequence to cycle through

    :param count: The number of times to loop through the sequence before terminating. If not supplied, the sequence will be repeated endlessly.

    :returns: An adapted sequence which repeatedly loops through the elements of :var:`seq`.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional and bounded
      * - :concept:`random_access_sequence`
        - :var:`Seq` is random-access and bounded
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - If :var:`count` is supplied
      * - :concept:`sized_sequence`
        - If :var:`count` is supplied
      * - :concept:`infinite_sequence`
        - If :var:`count` is not supplied
      * - :concept:`read_only_sequence`
        - Always
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable

    :example:

    ..  literalinclude:: ../../example/docs/cycle.cpp
        :language: cpp
        :dedent:
        :lines: 14-37

    :see also:

``dedup``
^^^^^^^^^

..  function::
    template <multipass_sequence Seq> \
        requires std::equality_comparable<element_t<Seq>> \
    auto dedup(Seq seq) -> multipass_sequence auto;

    An alias for :expr:`adjacent_filter(seq, std::ranges::not_equal_to{})`. This can be used to remove adjacent elements from a sequence.

    :see also:
        * :func:`flux::adjacent_filter`

``drop``
^^^^^^^^

..  function::
    auto drop(sequence auto seq, std::integral auto count) -> sequence auto;

    Given a sequence :var:`seq` and a non-negative integral value :var:`count`, returns a new sequence which skips the first :var:`count` elements of :var:`seq`.

    The returned sequence has the same capabilities as :var:`seq`. If :var:`seq` has fewer than :var:`count` elements, the returned sequence is empty.

    :param seq: A sequence.
    :param count: A non-negative integral value indicating the number of elements to be skipped.

    :returns: A sequence adaptor that yields the remaining elements of :var:`seq`, with the first :var:`count` elements skipped.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - :var:`seq` is contiguous
      * - :concept:`bounded_sequence`
        - :var:`seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`seq` is infinite
      * - :concept:`read_only_sequence`
        - :var:`seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable

    :example:

    ..  literalinclude:: ../../example/docs/drop.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:

        * `std::views::drop <https://en.cppreference.com/w/cpp/ranges/drop_view>`_ (C++20)
        * :func:`flux::take`

``drop_while``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto drop_while(Seq seq, Pred pred) -> sequence auto;

    Skips elements from the from of :var:`seq` while :var:`pred` returns ``true``. Once :var:`pred` has returned ``false``, the adaptor has done its job and the remaining elements of :var:`seq` are returned as normal.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`Seq` is random-access
      * - :concept:`contiguous_sequence`
        - :var:`Seq` is contiguous
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`Seq` is infinite
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable and :var:`Pred` is const-invocable

``filter``
^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred, element_t<Seq>&> \
    auto filter(Seq seq, Pred pred) -> sequence auto;

    Skips elements of :var:`seq` for which unary predicate :var:`pred` returns ``false``.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - :var:`Seq` is infinite
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable and :var:`Pred` is const-invocable

``flatten``
^^^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires sequence<element_t<Seq>> \
    auto flatten(Seq seq) -> sequence auto;

    Given a sequence-of-sequences, removes one level of nesting.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass, :expr:`element_t<Seq>` is multipass and :expr:`element_t<Seq>` is a reference type
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional, :expr:`element_t<Seq>` is bidirectional and :expr:`element_t<Seq>` is a reference type
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` and :expr:`element_t<Seq>` are both const-iterable multipass sequences, and :expr:`element_t<Seq>` is a reference type

``map``
^^^^^^^

..  function::
    template <sequence Seq, typename Func> \
        requires std::regular_invocable<Func&, element_t<Seq>> \
    auto map(Seq seq, Func func) -> sequence auto;

    Turns a sequence-of-``T`` into a sequence-of-``U``, where ``U`` is the return type of :var:`func`.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`Seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`Seq` is infinite
      * - :concept:`read_only_sequence`
        - The return type of :var:`func` is a prvalue or a const reference
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable and :var:`func` is const-invocable


``mask``
^^^^^^^^

..  function::
    template <sequence Seq, sequence Mask> \
        requires boolean_testable<element_t<Mask>> \
    auto mask(Seq seq, Mask where) -> sequence auto;

    Given a sequence of values and a sequence of booleans, :func:`mask` yields those elements of :var:`seq` for which the corresponding element of :var:`where` evaluates to :expr:`true`. Iteration is complete when either of the two input sequences is exhausted.

    The returned sequence models the lowest common category of the two input sequences, up to :concept:`bidirectional_sequence`. It is also a :concept:`bounded_sequence` and a :concept:`sized_sequence` when both inputs model these concepts.

    :param seq: A sequence of values
    :param where: A sequence whose element type is convertible to :expr:`bool`

    :returns: An adapted sequence whose elements are the elements of :var:`seq`, filtered by the corresponding elements of :var:`where`

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` and :var:`Mask` are both multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` and :var:`Mask` are both bidirectional
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` and :var:`Mask` are both bounded
      * - :concept:`sized_sequence`
        - :var:`Seq` and :var:`Mask` are both sized
      * - :concept:`infinite_sequence`
        - :var:`Seq` and :var:`Mask` are both infinite
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` and :var:`Mask` are both const-iterable

    :example:

    ..  literalinclude:: ../../example/docs/mask.cpp
        :language: cpp
        :dedent:
        :lines: 16-30

    :see also:
        * :func:`flux::filter`

``pairwise``
^^^^^^^^^^^^

..  function::
    auto pairwise(multipass_sequence auto seq) -> multipass_sequence auto;

    Returns an adaptor which yields pairs of elements of :var:`seq`. It is an alias for :func:`adjacent\<2>`.

    :param seq: A multipass sequence.

    :returns: A multipass sequence yielding pairs of elements of :var:`seq`.


``pairwise_map``
^^^^^^^^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Func> \
    requires std::regular_invocable<Func&, element_t<Seq>, element_t<Seq>> && \
             can_reference<std::invoke_result_t<Func&, element_t<Seq>, element_t<Seq>>> \
    auto pairwise_map(Seq seq, Func func) -> multipass_sequence auto;

    An alias for :expr:`adjacent_map\<2>`.

``prescan``
^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Func, std::movable Init> \
        requires foldable<Seq, Func, Init> \
    auto prescan(Seq seq, Func func, Init init) -> sequence auto;

    Returns a stateful sequence adaptor which yields "partial folds" using the binary function :var:`func`.

    First, this adaptor initialises an internal variable :var:`state` to :var:`init` and yields a read-only reference to this state. Then, for each successive element :var:`elem` of the underlying sequence, it sets::

        state = func(std::move(state), std::forward(elem));

    and yields a read-only reference to the new state.

    The final value yielded by this adaptor is the same as :expr:`fold(seq, func, init)`.

    Because this adaptor needs to maintain internal state, it is only ever single-pass. However it is a :concept:`bounded_sequence` when the underlying sequence is bounded and a :concept:`sized_sequence` when the underlying sequence is sized.

    Unlike :func:`scan`, this function performs an *exclusive scan*, that is, the Nth element of the adapted sequence does not include the Nth element of the underlying sequence. The adaptor returned by :func:`prescan` always yields at least one element -- the initial value -- followed by the elements that would be yielded by the :func:`scan` adaptor.

    :param seq: A sequence to adapt
    :param func: A binary callable of the form :expr:`R(R, element_t<Seq>)`, where :type:`R` is constructible from :var:`Init`
    :param init: The initial value for the scan

    :returns: A sequence adaptor which performs an exclusive scan of the elements of :var:`seq` using :var:`func`.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Never
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`Seq` is infinite
      * - :concept:`read_only_sequence`
        - Always
      * - :concept:`const_iterable_sequence`
        - Never

    :example:

    ..  literalinclude:: ../../example/docs/prescan.cpp
        :language: cpp
        :dedent:
        :lines: 13-20

    :see also:
        * `std::exclusive_scan() <https://en.cppreference.com/w/cpp/algorithm/exclusive_scan>`_
        * :func:`flux::scan`
        * :func:`flux::fold`

``read_only``
^^^^^^^^^^^^^

..  function::
    template <sequence Seq> \
    auto read_only(Seq seq) -> read_only_sequence auto;

    Returns an adapted sequence which prevents direct modification of the elements of :var:`seq`. The returned sequence retains the capabilities of the source sequence, all the way up to :concept:`contiguous_sequence`.

    If :var:`Seq` is already a :concept:`read_only_sequence`, then it is returned unchanged. Otherwise, :func:`read_only` is equivalent to::

        map(seq, [](auto&& elem) -> const_element_t<Seq> {
            return static_cast<const_element_t<Seq>>(std::forward(elem));
        });

    except that the returned sequence will be a :concept:`contiguous_sequence` if the source sequence models that concept. In this case, the pointer returned from :func:`data` will have type :expr:`value_t<Seq> const*`.

    :param seq: A sequence

    :returns: An adapted sequence which provides read-only access to the elements of :var:`seq`

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`Seq` is random-access
      * - :concept:`contiguous_sequence`
        - :var:`Seq` is contiguous
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`Seq` is infinite
      * - :concept:`read_only_sequence`
        - Always
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable

    :example:

    ..  literalinclude:: ../../example/docs/read_only.cpp
        :language: cpp
        :dedent:
        :lines: 12-34

    :see also:
        * `std::views::as_const() <https://en.cppreference.com/w/cpp/ranges/as_const_view>`_ (C++23)
        * :func:`flux::map`

``reverse``
^^^^^^^^^^^

..  function::
    template <bidirectional_sequence Seq> \
        requires bounded_sequence<Seq> \
    auto reverse(Seq seq) -> bidirectional_sequence auto;

    Given a bounded, bidirectional sequence :var:`seq`, returns an adaptor which yields the elements of :var:`seq` in reverse order.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Always
      * - :concept:`bidirectional_sequence`
        - Always
      * - :concept:`random_access_sequence`
        - :var:`Seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - Always
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - Always
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable

``scan``
^^^^^^^^

..  function::
    template <sequence Seq, typename Func, std::movable Init = value_t<Seq>> \
        requires foldable<Seq, Func, Init> \
    auto scan(Seq seq, Func func, Init init = {}) -> sequence auto;

    Returns a stateful sequence adaptor which yields "partial folds" using the binary function :var:`func`.

    First, this adaptor initialises an internal variable :var:`state` to :var:`init`. Then, for each successive element :var:`elem` of the underlying sequence, it sets::

        state = func(std::move(state), std::forward(elem));

    and yields a read-only reference to the new state.

    The final value yielded by this adaptor is the same as :expr:`fold(seq, func, init)`.

    Because this adaptor needs to maintain internal state, it is only ever single-pass. However it is a :concept:`bounded_sequence` when the underlying sequence is bounded and a :concept:`sized_sequence` when the underlying sequence is sized.

    Unlike :func:`prescan`, this function performs an *inclusive scan*, that is, the Nth element of the adapted sequence includes the Nth element of the underlying sequence. The adapted sequence always yields the same number of elements as the underlying sequence.

    :param seq: A sequence to adapt
    :param func: A binary callable of the form :expr:`R(R, element_t<Seq>)`, where :type:`R` is constructible from :var:`Init`
    :param init: The initial value for the scan. If not supplied, a default constructed object of type :type:`value_t\<Seq>` is used.

    :returns: A sequence adaptor which performs an inclusive scan of the elements of :var:`seq` using :var:`func`.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Never
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`Seq` is infinite
      * - :concept:`read_only_sequence`
        - Always
      * - :concept:`const_iterable_sequence`
        - Never

    :example:

    ..  literalinclude:: ../../example/docs/scan.cpp
        :language: cpp
        :dedent:
        :lines: 13-20

    :see also:
        * `std::partial_sum() <https://en.cppreference.com/w/cpp/algorithm/partial_sum>`_
        * `std::inclusive_scan() <https://en.cppreference.com/w/cpp/algorithm/inclusive_scan>`_
        * :func:`flux::scan_first`
        * :func:`flux::prescan`
        * :func:`flux::fold`

``scan_first``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Func> \
        requires foldable<Seq, Func, element_t<Seq>> \
    auto scan_first(Seq seq, Func func) -> sequence auto;

    Returns a stateful sequence adaptor which yields "partial folds" using the binary function :var:`func`.

    When iterated over, the returned sequence first initialises an internal variable ``state`` with the first element of the underlying sequence, and yields a read-only reference to this state. For each subsequent element ``elem``, it sets::

        state = func(std::move(state), std::forward(elem));

    and yields a read-only reference to the internal state. If :var:`seq` is empty, the internal state is never initialised and the resulting sequence is also empty. For a non-empty sequence, the final value yielded by :func:`scan_first` is the same as would be obtained from :expr:`fold_first(seq, func)`.

    Because this adaptor needs to maintain internal state, it is only ever single-pass. However it is a :concept:`bounded_sequence` when the underlying sequence is bounded and a :concept:`sized_sequence` when the underlying sequence is sized.

    Like :func:`scan`, this function performs an *inclusive scan*, that is, the Nth element of the adapted sequence includes the Nth element of the underlying sequence. The adapted sequence always yields the same number of elements as the underlying sequence. Unlike :func:`scan`, the first element of :func:`scan_first` is simply the first element of the underlying sequence, and the supplied :var:`func` is only applied to subsequent elements (this is equivalent to the differing behaviours of :func:`fold` and :func:`fold_first` respectively).

    :param seq: A sequence to adapt
    :param func: A binary callable of the form :expr:`R(R, element_t<Seq>)`, where :type:`R` is constructible from :expr:`element_t<Seq>`

    :returns: A sequence adaptor which performs an inclusive scan of the elements of :var:`seq` using :var:`func`.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Never
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`Seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`Seq` is infinite
      * - :concept:`read_only_sequence`
        - Always
      * - :concept:`const_iterable_sequence`
        - Never

    :example:

    ..  literalinclude:: ../../example/docs/scan_first.cpp
        :language: cpp
        :dedent:
        :lines: 13-21

    :see also:
        * `std::partial_sum() <https://en.cppreference.com/w/cpp/algorithm/partial_sum>`_
        * `std::inclusive_scan() <https://en.cppreference.com/w/cpp/algorithm/inclusive_scan>`_
        * :func:`flux::scan`
        * :func:`flux::fold_first`

``set_difference``
^^^^^^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires strict_weak_order_for<Cmp, Seq1> && strict_weak_order_for<Cmp, Seq2> \
    auto set_difference(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set difference of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements from :var:`seq1` which are not found in the :var:`seq2` according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly :expr:`std::max(m - n, 0)` elements.

    :param seq1: The first sorted sequence.
    :param seq2: The second sorted sequence.
    :param cmp: A binary predicate that takes two elements as arguments and returns true if the first element is less than the second.

    :returns: A sequence adaptor that yields those elements of `seq1` which do not also appear in `seq2`.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq1` and :var:`Seq2` are both multipass
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - Never
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq1` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq1` and :var:`Seq2` are both const-iterable, and :var:`cmp` is const-invocable

    :example:

    ..  literalinclude:: ../../example/docs/set_difference.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_difference() <https://en.cppreference.com/w/cpp/algorithm/set_difference>`_
        * :func:`flux::set_symmetric_difference`
        * :func:`flux::set_intersection`
        * :func:`flux::set_union`

``set_intersection``
^^^^^^^^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires strict_weak_order_for<Cmp, Seq1> && strict_weak_order_for<Cmp, Seq2> \
    auto set_intersection(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set intersection of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements from :var:`seq1` that are found in both sorted sequences according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly ``std::min(n, m)`` elements.

    :param seq1: The first sorted sequence.
    :param seq2: The second sorted sequence.
    :param cmp: A binary predicate that takes two elements as arguments and returns true if the first element is less than the second.

    :returns: A sequence adaptor that represents the set intersection of the two input sequences.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq1` and :var:`Seq2` are both multipass
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - Never
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq1` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq1` and :var:`Seq2` are both const-iterable, and :var:`cmp` is const-invocable

    :example:

    ..  literalinclude:: ../../example/docs/set_intersection.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_intersection() <https://en.cppreference.com/w/cpp/algorithm/set_intersection>`_
        * :func:`flux::set_difference`
        * :func:`flux::set_union`


``set_symmetric_difference``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires see_below \
    auto set_symmetric_difference(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set symmetric difference of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements that are found in either of the sequence, but not in both of them according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly ``std::abs(m - n)`` elements, preserving order:

    * if :expr:`m > n`, the final :expr:`m - n` of these elements from :var:`seq1`
    * if :expr:`m < n`, the final :expr:`n - m` of these elements from :var:`seq2`

    
    :requires:
        The expression in the ``requires`` clause is equivalent to::

            std::common_reference_with<element_t<Seq1>, element_t<Seq2>> &&
            std::common_reference_with<rvalue_element_t<Seq1>, rvalue_element_t<Seq2>> &&
            requires { typename std::common_type_t<value_t<Seq1>, value_t<Seq2>>; } &&
            strict_weak_order_for<Cmp, Seq1> &&
            strict_weak_order_for<Cmp, Seq2>

    :param seq1: The first sequence to merge.
    :param seq2: The second sequence to merge.
    :returns: A sequence adaptor that yields elements of `seq1` and `seq2` which do not appear in both sequences.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq1` and :var:`Seq2` are both multipass
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq1` and :var:`Seq2` are both bounded
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :expr:`std::common_reference_t<element_t<Seq1>, element_t<Seq2>>` is an object type or a const reference
      * - :concept:`const_iterable_sequence`
        - :var:`Seq1` and :var:`Seq2` are both const-iterable, and :var:`cmp` is const-invocable

    :example:

    ..  literalinclude:: ../../example/docs/set_symmetric_difference.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_symmetric_difference() <https://en.cppreference.com/w/cpp/algorithm/set_symmetric_difference>`_
        * :func:`flux::set_difference`
        * :func:`flux::set_intersection`
        * :func:`flux::set_union`

``set_union``
^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires see_below \
    auto set_union(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set union of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements from the two input sequences in order according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly ``std::max(n, m)`` elements.

    :requires:
        The expression in the ``requires`` clause is equivalent to::

            std::common_reference_with<element_t<Seq1>, element_t<Seq2>> &&
            std::common_reference_with<rvalue_element_t<Seq1>, rvalue_element_t<Seq2>> &&
            requires { typename std::common_type_t<value_t<Seq1>, value_t<Seq2>>; } &&
            strict_weak_order_for<Cmp, Seq1> &&
            strict_weak_order_for<Cmp, Seq2>
    
    :param seq1: The first sorted sequence to merge.
    :param seq2: The second sorted sequence to merge.
    :param cmp: A binary predicate that takes two elements as arguments and returns true if the first element is less than the second.

    :returns: A sequence adaptor that represents the set union of the two input sequences.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq1` and :var:`Seq2` are both multipass
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq1` and :var:`Seq2` are both bounded
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :expr:`std::common_reference_t<element_t<Seq1>, element_t<Seq2>>` is an object type or a const reference
      * - :concept:`const_iterable_sequence`
        - :var:`Seq1` and :var:`Seq2` are both const-iterable, and :var:`cmp` is const-invocable

    :example:

    ..  literalinclude:: ../../example/docs/set_union.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_union() <https://en.cppreference.com/w/cpp/algorithm/set_union>`_
        * :func:`flux::set_intersection`
        * :func:`flux::set_difference`

``slide``
^^^^^^^^^

..  function::
    auto slide(multipass_sequence auto seq, std::integral auto win_sz) -> multipass_sequence auto;

    Returns an adaptor which yields length-:var:`win_sz` overlapping subsequences of :var:`seq`.

    The :func:`adjacent` adaptor is similar to :func:`slide`, but takes its window size argument as a compile-time rather than a run-time parameter, and yield :any:`N`-tuples rather than subsequences.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Always
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`seq` is bidirectional and bounded
      * - :concept:`sized_sequence`
        - :var:`seq` is sized
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable

``split``
^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Delim> \
        requires std::equality_comparable_with<element_t<Seq>, Delim const&> \
    auto split(Seq seq, Delim delim) -> multipass_sequence auto;

..  function::
    template <multipass_sequence Seq, multipass_sequence Pattern> \
        requires std::equality_comparable_with<element_t<Seq>, element_t<Pattern>> \
    auto split(Seq seq, Pattern pattern) -> multipass_sequence auto;

..  function::
    template <multipass_sequence Seq, typename Pred> \
        requires std::predicate<Pred const&, element_t<seq>> \
    auto split(Seq seq, Pred pred) -> multipass_sequence auto;

    Splits a :concept:`multipass_sequence` into a sequence-of-subsequences using the given argument.

    The first overload takes a delimiter, which must be equality comparable with the source sequence's value type. The source sequence will be split on each occurrence of the delimiter, with the delimiter itself removed. Consecutive delimiters will result in empty subsequences in the output. If the source sequence begins with a delimiter then the first subsequence will be empty, and likewise if it ends with a delimiter then the final subsequence will be empty.

    The second overload takes another sequence, the :var:`pattern`, whose elements must be equality comparable with the elements of the source sequence. The source is split whenever the pattern occurs as a subsequence. Consecutive (non-overlapping) occurrences of the pattern will result in empty sequences in the output. If :expr:`ends_with(seq, pattern)` is :expr:`true`, the final subsequence will be empty.

    The third overload takes a unary predicate which will be called with successive elements of the source sequence and returns :expr:`true` when a split should occur. The "``true``" element will be removed from the output. If the predicate returns ``true`` for two consecutive of the source, then the output will contain an empty subsequence. If the predicate returns ``true``` for the final element of the source, then the final subsequence will be empty.

    The returned sequence is always a :concept:`multipass_sequence`. It is additionally a :concept:`bounded_sequence` when :var:`Seq` is bounded.

    :param seq: A multipass sequence to split.
    :param delim: For the first overload, a delimiter to split on. Must be equality comparable with the element type of :var:`seq`
    :param pattern: For the second overload, a multipass sequence to split on. Its element type must be equality comparable with the element type of :var:`seq`.
    :param pred: For the third overload, a unary predicate accepting elements of :var:`seq`, returning ``true`` when a split should occur.

    :returns: A multipass sequence whose elements are subsequences of :var:`seq`.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - Always
      * - :concept:`bidirectional_sequence`
        - Never
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`Seq` is bounded
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable, and :var:`Pattern` is const-iterable (for the second overload) or :var:`Pred` is const-invocable (for the third overload)

    :example:

    ..  literalinclude:: ../../example/docs/split.cpp
        :language: cpp
        :dedent:
        :lines: 18-79

    :see also:
        * `std::views::split() <https://en.cppreference.com/w/cpp/ranges/split_view>`_
        * :func:`flux::chunk_by`

``stride``
^^^^^^^^^^

..  function::
    auto stride(sequence auto seq, std::integral auto stride_len) -> sequence auto;

    Returns an adapted sequence which yields the same first element as :var:`seq` and then yields every :var:`stride_len`-th element after that.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - :var:`seq` is bidirectional, bounded and sized
      * - :concept:`sized_sequence`
        - :var:`seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`seq` is infinite
      * - :concept:`read_only_sequence`
        - :var:`seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable

``take``
^^^^^^^^

..  function::
    auto take(sequence auto seq, std::integral auto count) -> sequence auto;

    Returns an adapted sequence with at most :var:`count` elements.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - :var:`seq` is contiguous
      * - :concept:`bounded_sequence`
        - :var:`seq` is either infinite or both random-access and sized
      * - :concept:`sized_sequence`
        - :var:`seq` is sized or infinite
      * - :concept:`infinite_sequence`
        - :var:`seq` is infinite
      * - :concept:`read_only_sequence`
        - :var:`seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable

``take_while``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto take_while(Seq seq, Pred pred) -> sequence auto;

    Returns an adapted sequence which yields elements of :var:`seq` while :var:`pred` returns ``true``. Iteration is complete when :var:`pred` returns ``false`` or the underlying sequence is exhausted.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`Seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`Seq` is bidirectional
      * - :concept:`random_access_sequence`
        - Never
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - Never
      * - :concept:`sized_sequence`
        - Never
      * - :concept:`infinite_sequence`
        - Never
      * - :concept:`read_only_sequence`
        - :var:`Seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`Seq` is const-iterable

``unchecked``
^^^^^^^^^^^^^

..  function::
    auto unchecked(sequence auto seq) -> sequence auto;

    A passthrough adaptor which disables bounds checking.

    It does so by forwarding each call to :func:`read_at` on the adapted sequence to the :func:`read_at_unchecked` implementation of the underlying sequence.

    ..  danger:: Using this adaptor can result in undefined behaviour that would otherwise be caught by Flux's safety checks. Use it with great caution.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - :var:`seq` is multipass
      * - :concept:`bidirectional_sequence`
        - :var:`seq` is bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - :var:`seq` is contiguous
      * - :concept:`bounded_sequence`
        - :var:`seq` is bounded
      * - :concept:`sized_sequence`
        - :var:`seq` is sized
      * - :concept:`infinite_sequence`
        - :var:`seq` is infinite
      * - :concept:`read_only_sequence`
        - :var:`seq` is read-only
      * - :concept:`const_iterable_sequence`
        - :var:`seq` is const-iterable

``zip``
^^^^^^^

..  function::
    auto zip(sequence auto... seqs) -> sequence auto;

    Returns a sequence adaptor which iterates over the input sequences in lock-step. Iteration is complete when any of the input sequences is exhausted.

    The element type of the returned sequence is a :expr:`std::tuple` of the element types of the input sequences, or a :expr:`std::pair` for two inputs.

    :models:

    .. list-table::
      :align: left
      :header-rows: 1

      * - Concept
        - When
      * - :concept:`multipass_sequence`
        - All inputs are multipass
      * - :concept:`bidirectional_sequence`
        - All inputs are bidirectional
      * - :concept:`random_access_sequence`
        - :var:`seq` is random-access
      * - :concept:`contiguous_sequence`
        - Never
      * - :concept:`bounded_sequence`
        - All inputs are bounded
      * - :concept:`sized_sequence`
        - All inputs are sized
      * - :concept:`infinite_sequence`
        - All inputs are infinite
      * - :concept:`read_only_sequence`
        - All inputs are read-only
      * - :concept:`const_iterable_sequence`
        - All inputs are const-iterable