
Adaptors
========

.. namespace:: flux

``adjacent``
^^^^^^^^^^^^

.. function::
   template <distance_t N> \
      requires (N > 0) \
   auto adjacent(multipass_sequence auto seq) -> multipass_sequence auto

   Given a compile-time size :var:`N` and a multipass sequence :var:`seq`, returns a new sequence which yields sliding windows of size :var:`N` as an :var:`N`-tuple of elements of :var:`seq`. If `seq` has fewer than :var:`N` elements, the adapted sequence will be empty.

   The returned sequence is always a :concept:`multipass_sequence`. It is also
    * bidirectional when :var:`seq` is bidirectional
    * random-access when :var:`seq` is random-access
    * sized when :var:`seq` is sized
    * bounded when :var:`seq` is *both* bounded and bidirectional

   The :func:`slide()` adaptor is similar to :func:`adjacent()`, but takes its window size as a run-time rather than a compile-time parameter, and returns length-:any:`n` subsequences rather than tuples.

   Equivalent to::

       zip(seq, drop(seq, 1), drop(seq, 2), ..., drop(seq, N-1));

   :tparam N: The size of the sliding window. Must be greater than zero.

   :param seq: A multipass sequence.

   :returns: A sequence adaptor whose element type is a :expr:`std::tuple` of size :var:`N`, or a :expr:`std::pair` if :expr:`N == 2`.

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


``adjacent_map``
^^^^^^^^^^^^^^^^

.. function::
   template <distance_t N> \
   auto adjacent_map(multipass_sequence auto seq, auto func) -> multipass_sequence auto;

``cache_last``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires see_below \
    auto cache_last(Seq seq) -> sequence auto;

``cartesian_product``
^^^^^^^^^^^^^^^^^^^^^

..  function::
    auto cartesian_product(sequence auto seq0, multipass_sequence auto... seqs) -> sequence auto;

``cartesian_product_map``
^^^^^^^^^^^^^^^^^^^^^^^^^

..  function::
    template <typename Func, sequence Seq0, multipass_sequence... Seqs> \
    requires std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...> \
    auto cartesian_product_with(Func func, Seq0 seq0, Seqs... seqs) -> sequence auto;

``chain``
^^^^^^^^^

..  function::
    template <sequence Seq0, sequence... Seqs> \
        requires see_below \
    auto chain(Seq0 seq0, Seqs... seqs) -> sequence auto;

``chunk``
^^^^^^^^^

..  function::
    auto chunk(sequence auto seq, std::integral auto chunk_sz) -> sequence auto;

``chunk_by``
^^^^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Pred> \
        requires std::predicate<Pred, element_t<Seq>, element_t<Seq>> \
    auto chunk_by(Seq seq, Pred pred) -> multipass_sequence auto

``drop``
^^^^^^^^

..  function::
    auto drop(sequence auto seq, std::integral auto count) -> sequence auto;

    Given a sequence :var:`seq` and a non-negative integral value :var:`count`, returns a new sequence which skips the first :var:`count` elements of :var:`seq`.

    The returned sequence has the same capabilities as :var:seq. If :var:`seq` has fewer than :var:`count` elements, the returned sequence is empty.

    :param seq: A sequence.
    :param count: A non-negative integral value indicating the number of elements to be skipped.

    :returns: A sequence adaptor that yields the remaining elements of :var:`seq`, with the first :var:`count` elements skipped.

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

``filter``
^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred, element_t<Seq>&> \
    auto filter(Seq seq, Pred pred) -> sequence auto;

``flatten``
^^^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires sequence<element_t<Seq>> \
    auto flatten(Seq seq) -> sequence auto;

``map``
^^^^^^^

.. function::
   template <sequence Seq, std::copy_constructable Func> \
   auto map(Seq seq, Func func) -> sequence auto

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

``reverse``
^^^^^^^^^^^

..  function::
    template <bidirectional_sequence Seq> \
        requires bounded_sequence<Seq> \
    auto reverse(Seq seq) -> bidirectional_sequence auto;

``slide``
^^^^^^^^^

..  function::
    auto slide(multipass_sequence auto seq, std::integral auto win_sz) -> multipass_sequence auto;

``stride``
^^^^^^^^^^

..  function::
    auto stride(sequence auto seq, std::integral auto stride_len) -> sequence auto;

``split``
^^^^^^^^^

..  function::
    template <multipass_sequence Seq, multipass_sequence Pattern> \
        requires std::equality_comparable_with<element_t<Seq>, element_t<Pattern>> \
    auto split(Seq seq, Pattern pattern) -> sequence auto;

..  function::
    template <multipass_sequence Seq> \
    auto split(Seq seq, value_t<Seq> delim) -> sequence auto;

``take``
^^^^^^^^

..  function::
    auto take(sequence auto seq, std::integral auto count) -> sequence auto;

``take_while``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto take_while(Seq seq, Pred pred) -> sequence auto;

``unchecked``
^^^^^^^^^^^^^

..  function::
    auto unchecked(sequence auto seq) -> sequence auto;

``zip``
^^^^^^^

..  function::
    auto zip(sequence auto... seqs) -> sequence auto;