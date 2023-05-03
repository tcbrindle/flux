
Sequence Access
***************

..  namespace:: flux

``first``
---------

..  function::
    template <sequence Seq> \
    auto first(Seq& seq) -> cursor_t<Seq>;

    Returns a :concept:`cursor` to the first element of :var:`seq`.

``is_last``
-----------

..  function::
    template <sequence Seq> \
    auto is_last(Seq& seq, cursor_t<Seq> const& cur) -> bool;

    Returns :texpr:`true` if :var:`cur` is at the past-the-end position of :var:`seq`.

``last``
--------

..  function::
    template <bounded_sequence Seq> \
    auto last(Seq& seq) -> cursor_t<Seq>;

    Return a :concept:`cursor` which is in the past-the-end position.

    :expr:`is_last(seq, last(seq))` is always :texpr:`true`.

``inc``
-------

..  function::
    template <sequence Seq> \
    auto inc(Seq& seq, cursor_t<Seq>& cur) -> cursor_t<Seq>&;

..  function::
    template <random_access_sequence Seq> \
    auto inc(Seq& seq, cursor_t<Seq>& cur, distance_t offset) -> cursor_t<Seq>&

``dec``
-------

..  function::
    template <bidirectional_sequence Seq> \
    auto dec(Seq& seq, cursor_t<Seq>& cur) -> cursor_t<Seq>&;

``read_at``
-----------

..  function::
    template <sequence Seq> \
    auto read_at(Seq& seq, cursor_t<Seq> const& cur) -> element_t<Seq>;

``read_at_unchecked``
---------------------

..  function::
    template <sequence Seq> \
    auto read_at_unchecked(Seq& seq, cursor_t<Seq> const& cur) -> element_t<Seq>;

``move_at``
-----------

..  function::
    template <sequence Seq> \
    auto move_at(Seq& seq, cursor_t<Seq> const& cur) -> rvalue_element_t<Seq>;

``move_at_unchecked``
---------------------

..  function::
    template <sequence Seq> \
    auto move_at_unchecked(Seq& seq, cursor_t<Seq> const& cur) -> rvalue_element_t<Seq>;


``distance``
------------

..  function::
    template <multipass_sequence Seq> \
    auto distance(Seq& seq, cursor_t<Seq> const& from, cursor_t<seq> const& to) -> distance_t;

..  function::
    template <random_access_sequence Seq> \
    auto distance(Seq& seq, cursor_t<Seq> const& from, cursor_t<Seq> const& to) -> distance_t;

``data``
--------

..  function::
    template <contiguous_sequence Seq> \
    auto data(Seq& seq) -> std::add_pointer_t<std::remove_reference_t<element_t<Seq>>>;

    Provides a pointer to the start of the underlying raw storage of a :concept:`contiguous_sequence`.

``size``
--------

..  function::
    auto size(sized_sequence auto& seq) -> distance_t;

``usize``
---------

..  function::
    auto usize(sized_sequence auto& seq) -> std::size_t;

``next``
--------

..  function::
    template <sequence Seq> \
    auto next(Seq& seq, cursor_t<Seq> cur) -> cursor_t<Seq>;

``prev``
--------

..  function::
    template <bidirectional_sequence Seq> \
    auto prev(Seq& seq, cursor_t<Seq> cur) -> cursor_t<Seq>;

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

``swap_with``
-------------

..  function::
    template <sequence Seq1, sequence Seq2> \
        requires element_swappable_with<Seq1, Seq2> \
    auto swap_with(Seq1& seq1, cursor_t<Seq1> const& cur1, \
                   Seq2& seq2, cursor_t<Seq2> const& cur2) -> void;

``swap_at``
-----------

..  function::
    template <multipass_sequence Seq> \
        requires element_swappable_with<Seq, Seq> \
    auto swap_at(Seq& seq, cursor_t<Seq> const& cur1, cursor_t<Seq> const& cur2) -> void;

``front``
---------

..  function::
    template <multipass_sequence Seq> \
    auto front(Seq& seq) -> optional<element_t<Seq>>;

    If :var:`seq` is empty, returns a disengaged :type:`optional`. Otherwise, returns an engaged :type:`optional` containing the first element of :var:`seq` (which may be a reference).

``back``
--------

..  function::
    template <bidirectional_sequence Seq> \
        requires bounded_sequence<Seq> \
    auto back(Seq& seq) -> optional<element_t<Seq>>;

    If :var:`seq` is empty, returns a disengaged :type:`optional`. Otherwise, returns an engaged :type:`optional` containing the rear-most element of :var:`seq` (which may be a reference).

``begin``
---------

..  function::
    auto begin(sequence auto& seq) -> std::input_iterator auto;

    Returns a C++20 :concept:`std::input_iterator` of implementation-defined type pointing to the first element of :var:`seq`.

    The resulting iterator may be compared with the sentinel returned from :func:`end`.

``end``
-------

..  function::
    auto end(sequence auto& seq)

    If :var:`seq` is a :concept:`bounded_sequence` whose cursor type satisfies :concept:`std::copy_constructible`, returns an iterator of the same type as :expr:`begin(seq)` which points to the past-the-end position of :var:`seq`.

    Otherwise, returns :expr:`std::default_sentinel`.

``view``
--------

..  function::
    template <sequence Seq> \
        requires std::ranges::viewable_range<Seq> || std::is_lvalue_reference_v<Seq> \
    auto view(Seq&& seq) -> std::ranges::view auto;

    If :var:`Seq` satisfies :concept:`std::ranges::viewable_range`, returns :expr:`std::views::all(std::forward(seq))`.

    Otherwise, :var:`seq` must be an lvalue and :func:`view` returns :expr:`std::ranges::subrange(begin(seq), end(seq))`.