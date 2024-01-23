
Sequence Access
***************

..  namespace:: flux

``first``
---------

..  function::
    template <sequence Seq> \
    auto first(Seq& seq) -> cursor_t<Seq>;

    Returns a :concept:`cursor` to the first element of :var:`seq`.

    :complexity:

        Constant if :expr:`multipass_sequence<Seq>`, otherwise linear

``is_last``
-----------

..  function::
    template <sequence Seq> \
    auto is_last(Seq& seq, cursor_t<Seq> const& cur) -> bool;

    Returns :texpr:`true` if :var:`cur` is at the past-the-end position of :var:`seq`.

    :complexity:

        Constant

``last``
--------

..  function::
    template <bounded_sequence Seq> \
    auto last(Seq& seq) -> cursor_t<Seq>;

    Returns a :concept:`cursor` which is in the past-the-end position of :var:`seq`.

    :expr:`is_last(seq, last(seq))` is always :texpr:`true`.

    :complexity:

        Constant

``inc``
-------

..  function::
    template <sequence Seq> \
    auto inc(Seq& seq, cursor_t<Seq>& cur) -> cursor_t<Seq>&;

    Advances the cursor by one position.

    :returns:

        A reference to :var:`cur`

    :complexity:

        Constant if :expr:`random_access_sequence<Seq>`, otherwise linear.

..  function::
    template <random_access_sequence Seq> \
    auto inc(Seq& seq, cursor_t<Seq>& cur, distance_t offset) -> cursor_t<Seq>&

    Advances the cursor by :var:`offset` positions, or decrements it if :var:`offset` is negative.

    :returns:

        A reference to :var:`cur`

    :complexity:

        Constant


``dec``
-------

..  function::
    template <bidirectional_sequence Seq> \
    auto dec(Seq& seq, cursor_t<Seq>& cur) -> cursor_t<Seq>&;

    Decrements the cursor by one position

    :returns:

        A reference to :var:`cur`

    :complexity:

        Constant if :expr:`random_access_sequence<Seq>`, otherwise linear

``read_at``
-----------

..  function::
    template <sequence Seq> \
    auto read_at(Seq& seq, cursor_t<Seq> const& cur) -> element_t<Seq>;

    Accesses the element at the given cursor position.

    :complexity:

        Constant

``read_at_unchecked``
---------------------

..  function::
    template <sequence Seq> \
    auto read_at_unchecked(Seq& seq, cursor_t<Seq> const& cur) -> element_t<Seq>;

    Accesses the element at the given cursor position without performing safety checks.

    ..  danger:: Using this function can result in undefined behaviour that would otherwise be caught by Flux's safety checks.

    :complexity:

        Constant

``move_at``
-----------

..  function::
    template <sequence Seq> \
    auto move_at(Seq& seq, cursor_t<Seq> const& cur) -> rvalue_element_t<Seq>;

    Accesses the element at the given cursor position as an rvalue reference or prvalue temporary.

    :complexity:

        Constant

``move_at_unchecked``
---------------------

..  function::
    template <sequence Seq> \
    auto move_at_unchecked(Seq& seq, cursor_t<Seq> const& cur) -> rvalue_element_t<Seq>;

    Accesses the element at the given cursor position as an rvalue reference or prvalue temporary without performing safety checks.

    ..  danger:: Using this function can result in undefined behaviour that would otherwise be caught by Flux's safety checks.

    :complexity:

        Constant

``distance``
------------

..  function::
    template <multipass_sequence Seq> \
    auto distance(Seq& seq, cursor_t<Seq> const& from, cursor_t<seq> const& to) -> distance_t;

    Returns number of times :var:`from` needs to be incremented until it is equal to :var:`to`.

    For random-access sequences, :var:`to` may point to an earlier position than :var:`from`, in which case the result will be negative.

    :complexity:

        Constant if :expr:`random_access_sequence<Seq>`, otherwise linear


``data``
--------

..  function::
    template <contiguous_sequence Seq> \
    auto data(Seq& seq) -> std::add_pointer_t<std::remove_reference_t<element_t<Seq>>>;

    Provides a pointer to the start of the underlying raw storage of a :concept:`contiguous_sequence`.

    :complexity:

        Constant

``size``
--------

..  function::
    auto size(sized_sequence auto& seq) -> distance_t;

    Returns the number of elements in the sequence. Performs no iteration.

    :complexity:

        Constant

``usize``
---------

..  function::
    auto usize(sized_sequence auto& seq) -> std::size_t;

    Returns the number of elements in the sequence as an unsigned :type:`std::size_t`.

    :complexity:

        Constant

``next``
--------

..  function::
    template <sequence Seq> \
    auto next(Seq& seq, cursor_t<Seq> cur) -> cursor_t<Seq>;

    Returns a new cursor which points to the next position after :var:`cur`.

    :complexity:

        Constant if :expr:`random_access_sequence<Seq>`, otherwise linear

..  function::
    template <sequence Seq> \
    auto next(Seq& seq, cursor_t<Seq> cur, distance_t offset) -> cursor_t<Seq>;

    If :var:`offset` is non-negative, returns a new cursor which points to a position :var:`offset` places past :var:`cur`.

    If :var:`offset` is negative:
        * If :var:`Seq` is bidirectional, returns a new cursor which points to a position :expr:`-offset` places before :var:`cur`
        * Otherwise, returns :var:`cur`

    :complexity:

        Constant if :expr:`random_access_sequence<Seq>`, otherwise linear

``prev``
--------

..  function::
    template <bidirectional_sequence Seq> \
    auto prev(Seq& seq, cursor_t<Seq> cur) -> cursor_t<Seq>;

    Returns a new cursor which points to the position before :var:`cur`

    :complexity:

        Constant if :expr:`random_access_sequence<Seq>`, otherwise linear

``is_empty``
------------

..  function::
    template <sequence Seq> \
        requires sized_sequence<Seq> || multipass_sequence<Seq> \
    auto is_empty(Seq& seq) -> bool;

    Returns :texpr:`true` if :var:`seq` contains no elements.

    Equivalent to::

        if constexpr (sized_sequence<Seq>) {
            return size(seq) == 0;
        } else {
            return is_last(seq, first(seq));
        }

    :complexity:

        Constant

``swap_with``
-------------

..  function::
    template <sequence Seq1, sequence Seq2> \
        requires element_swappable_with<Seq1, Seq2> \
    auto swap_with(Seq1& seq1, cursor_t<Seq1> const& cur1, \
                   Seq2& seq2, cursor_t<Seq2> const& cur2) -> void;

    Swaps the elements at position :var:`cur1` of :var:`seq1` and position :var:`cur2` of :var:`seq2`.

    :complexity:

        Constant

``swap_at``
-----------

..  function::
    template <multipass_sequence Seq> \
        requires element_swappable_with<Seq, Seq> \
    auto swap_at(Seq& seq, cursor_t<Seq> const& cur1, cursor_t<Seq> const& cur2) -> void;

    Swaps the elements at position :var:`cur1` and :var:`cur2`.

    Equivalent to :expr:`swap_with(seq, cur1, seq, cur2)`

    :complexity:

        Constant

``front``
---------

..  function::
    template <multipass_sequence Seq> \
    auto front(Seq& seq) -> optional<element_t<Seq>>;

    If :var:`seq` is empty, returns a disengaged :type:`optional`. Otherwise, returns an engaged :type:`optional` containing the first element of :var:`seq` (which may be a reference).

    :complexity:

        Constant

``back``
--------

..  function::
    template <bidirectional_sequence Seq> \
        requires bounded_sequence<Seq> \
    auto back(Seq& seq) -> optional<element_t<Seq>>;

    If :var:`seq` is empty, returns a disengaged :type:`optional`. Otherwise, returns an engaged :type:`optional` containing the rear-most element of :var:`seq` (which may be a reference).

    :complexity:

        Constant

``begin``
---------

..  function::
    auto begin(sequence auto& seq) -> std::input_iterator auto;

    Returns a C++20 iterator of implementation-defined type pointing to the first element of :var:`seq`. The resulting iterator forms a valid range with the sentinel returned by :expr:`end(seq)`.

    The category of the returned iterator depends on the properties of :var:`seq`.
    Multipass, bidirectional, random-access and contiguous sequences will return
    forward, bidirectional, random-access and contiguous iterators respectively.

    :complexity:

        Constant


``end``
-------

..  function::
    auto end(sequence auto& seq)

    If :var:`seq` is a :concept:`bounded_sequence` whose cursor type satisfies :concept:`std::copy_constructible`, returns an iterator of the same type as :expr:`begin(seq)` which points to the past-the-end position of :var:`seq`.

    Otherwise, returns :expr:`std::default_sentinel`.

    :complexity:

        Constant
