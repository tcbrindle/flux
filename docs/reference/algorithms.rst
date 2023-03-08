
Algorithms
**********

..  namespace:: flux

``all``
-------

..  function::
    template <sequence Seq, predicate_for<Seq> Pred> \
    auto all(Seq&& seq, Pred pred) -> bool;

    Takes a :concept:`sequence` and a predicate and returns :expr:`true` if every element of the sequence satisfies the predicate. This algorithm is short-circuiting: it will stop iterating (returning :expr:`false`) as soon as it finds an element which does not satisfy the predicate.

    Returns :expr:`true` for an empty sequence.

    Equivalent to :expr:`is_last(seq, find_if_not(seq, pred))`.

    :param seq: A sequence

    :param pred: A callable which accepts a single argument of :var:`seq`'s  element type and returns :expr:`bool`

    :returns: :expr:`true` if :var:`pred` returns :expr:`true` for every element of :var:`seq`.

    :example:

    ..  literalinclude:: ../../example/docs/all.cpp
        :language: cpp
        :dedent:
        :lines: 13-19

    :See also:
        * `std::ranges::all_of() <https://en.cppreference.com/w/cpp/algorithm/ranges/all_any_none_of>`_
        * :func:`flux::any`
        * :func:`flux::none`
        * :func:`flux::find_if`

``any``
-------

..  function::
    template <sequence Seq, predicate_for<Seq> Pred> \
    auto any(Seq&& seq, Pred pred) -> bool;

    Takes a :concept:`sequence` and a predicate and returns :expr:`true` if any element in the sequence satisfies the predicate. This algorithm is short-circuiting: it will stop iterating (returning :expr:`true`) as soon as it has found a satisfactory element.

    Returns :expr:`false` for an empty sequence.

    Equivalent to :expr:`!is_last(seq, find_if(seq, pred))`.

    :param seq: A sequence.

    :param pred: A callable which accepts a single argument of :var:`seq`'s  element type and returns :expr:`bool`

    :returns: :expr:`true` if :var:`pred` returns :expr:`true` for any element of :var:`seq`.

    :example:

    ..  literalinclude:: ../../example/docs/any.cpp
        :language: cpp
        :linenos:
        :dedent:
        :lines: 13-18


    :see also:
        * `std::ranges::any_of() <https://en.cppreference.com/w/cpp/algorithm/ranges/all_any_none_of>`_
        * :func:`flux::all`
        * :func:`flux::none`
        * :func:`flux::find_if`

``compare``
-----------

..  function::
    template <sequence Seq1, sequence Seq2, \
              typename Cmp = std::compare_three_way> \
        requires see_below \
    auto compare(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {});

    Performs a lexicographical three-way comparison between the elements of :var:`seq1` and :var:`seq2`.

    This function iterates over both sequences in lock-step, and compares each respective pair of elements using :var:`cmp`. If the elements are not equivalent, then the result of the comparison is returned. Otherwise, the next pair of elements is compared, and so on.

    .. note:: If you just want to know whether the two sequences contain the same elements, you should prefer :func:`equal`.

    If the end of one of the sequences is reached, then:

    * :expr:`less` is returned if the first sequence had fewer elements (was "less than") the second
    * :expr:`greater` is returned if the second sequence had fewer elements (so the first sequence was "greater")
    * Otherwise, all elements of both sequences were equivalent and so :expr:`equivalent` is returned

    :requires: The comparator :var:`cmp` must return a value of one of the standard comparison categories, that is

        * :expr:`std::strong_ordering`, or
        * :expr:`std::weak_ordering`, or
        * :expr:`std::partial_ordering`

    :param seq1: The first sequence to compare
    :param seq2: The second sequence to compare
    :param cmp: A callable accepting two parameters of the respective element types of :var:`seq1` and :var:`seq2`, and returning a value of one of the standard comparison categories

    :returns: A value of the same comparison category as returned by :var:`cmp`, indicating whether the first sequence is lexicographically *less than*, *greater than* or *equivalent to* the first, or if they are *unordered*.

    :example:

    ..  literalinclude:: ../../example/docs/compare.cpp
        :language: cpp
        :linenos:
        :dedent:
        :lines: 15-43

    :see also:
        * `std::lexicographical_compare_three_way() <https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare_three_way>`_
        * :func:`flux::equal`

``contains``
------------

..  function::
    template <sequence Seq, typename Value> \
        requires std::equality_comparable_with<element_t<Seq>, Value const&> \
    auto contains(Seq&& seq, Value const& value) -> bool;

    Returns :expr:`true` if any element in :var:`seq` compares equal to :var:`value`. This algorithm is short-circuiting: it will stop iterating as soon as it has found an equal element.

    Equivalent to :expr:`!is_last(seq, find(seq, value))`

    :param seq: A sequence

    :param value: A value to search for

    :returns: :expr:`true` if any element of :var:`seq` compares equal to :var:`value`.

    :example:

    ..  literalinclude:: ../../example/docs/contains.cpp
        :language: cpp
        :linenos:
        :dedent:
        :lines: 13-21

    :see also:
        * `std::ranges::contains() <https://en.cppreference.com/w/cpp/algorithm/ranges/contains>`_
        * :func:`flux::find`

``count``
---------

..  function::
    auto count(sequence auto&& seq) -> distance_t;

    Returns the number of elements in the sequence.

    If :var:`seq` is a :concept:`sized_sequence` then this is equivalent to :func:`flux::size`. Otherwise, :func:`count` will iterate over the sequence, "using up" single-pass sequences.

    Equivalent to::

        if constexpr (sized_sequence<decltype(seq)>) {
            return size(seq);
        } else {
            return count_if(seq, pred::true_);
        }

    :param seq: A sequence

    :returns: The number of elements in :var:`seq`

    :example:

    ..  literalinclude:: ../../example/docs/count.cpp
        :language: cpp
        :linenos:
        :dedent:
        :lines: 14-30

    :see also:
        * `std::ranges::distance() <https://en.cppreference.com/w/cpp/iterator/ranges/distance>`_
        * :func:`flux::count_eq`
        * :func:`flux::count_if`

``count_eq``
------------

..  function::
    template <sequence Seq, typename Value> \
        requires std::equality_comparable_with<element_t<Seq>, Value const&> \
    auto count_eq(Seq&& seq, Value const& value) -> distance_t;

    Iterates over :var:`seq` and returns the number of elements which compare equal to :var:`value`.

    Equivalent to :expr:`count_if(seq, pred::eq(value))`, but does not take a copy of :var:`value`.

    :param seq: A sequence
    :param value: A value which is equality comparable with :var:`seq`'s element type

    :returns: The number of elements of :var:`seq` which compared sequence to :var:`value`.

    :example:

    :see also:
        * `std::ranges::count() <https://en.cppreference.com/w/cpp/algorithm/ranges/count>`_
        * :func:`flux::count()`
        * :func:`flux::count_if()`

``count_if``
------------

..  function::
    template <typename Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto count_if(Seq&& seq, Pred pred) -> distance_t;

    Returns the number of elements in the sequence for which the predicate returned :texpr:`true`.

    Equivalent to::

        fold(seq, [&pred](distance_t count, auto&& elem) {
            if (std::invoke(pred, std::forward(elem))) {
                ++count;
            }
            return count;
        }, distance_t{0})

    :param seq: A sequence
    :param pred: A unary predicate accepting :var:`seq`'s element type, indicating whether the element should be counted

    :returns: The number of elements for which :var:`pred` returned :texpr:`true`.

    :example:

    :see also:
        * `std::ranges::count_if() <https://en.cppreference.com/w/cpp/algorithm/ranges/count>`_
        * :func:`count`
        * :func:`count_eq`
        * :func:`fold`

``equal``
---------

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::equal_to<>> \
        requires std::predicate<Cmp&, element_t<Seq1>, element_t<Seq2>> \
    auto equal(Seq1&& seq1, Seq2&& seq2, Cmp cmp = {}) -> bool;

    Returns :texpr:`true` if :var:`seq1` and :var:`seq2` have the same number of elements and each corresponding pair of elements compares equal according to :var:`cmp`.

    If :var:`seq1` and :var:`seq2` both satisfy :concept:`sized_sequence` and their sizes differ, :func:`equal` immediately returns :texpr:`false` and no comparisons are performed.

    When using the default comparators, :expr:`equal(seq1, seq2)` returns the same answer as :expr:`std::is_eq(compare(seq1, seq2))` but may be more efficient.

    :param seq1: A sequence
    :param seq2: Another sequence
    :param cmp: A binary predicate to use as a comparator, defaulting to :type:`std::equal_to\<>`.

    :returns: :texpr:`true` if :var:`seq1` and :var:`seq2` have the same number of elements and each corresponding pair of elements compares equal.

    :example:

    :see also:
        * `std::ranges::equal() <https://en.cppreference.com/w/cpp/algorithm/ranges/equal>`_
        * :func:`flux::compare`

``fill``
--------

..  function::
    template <sequence Seq, typename Value> \
        requires writeable_sequence_of<Seq, Value const&> \
    auto fill(Seq&& seq, Value const& value) -> void;

    Assigns :var:`value` to every element of :var:`seq`.

    Equivalent to::

        for_each(seq, [&value](auto&& elem) {
            std::forward(elem) = value;
        })

    :param seq: A mutable sequence whose element type is assignable from :expr:`Value const&`
    :param value: A value to assign to each element of :var:`seq`.

    :example:

    :see also:
        * `std::ranges::fill() <https://en.cppreference.com/w/cpp/algorithm/ranges/fill>`_

``find``
--------

..  function::
    template <sequence Seq, typename Value> \
        requires std::equality_comparable_with<element_t<Seq>, Value const&> \
    auto find(Seq&& seq, Value const& value) -> cursor_t<Seq>;

``find_if``
-----------

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto find_if(Seq&& seq, Pred pred) -> cursor_t<Seq>;

``find_if_not``
---------------

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto find_if_not(Seq&& seq, Pred pred) -> cursor_t<Seq>;

``fold``
--------

..  type::
    template <sequence Seq, typename Func, typename Init> \
    fold_result_t = std::decay_t<std::invoke_result_t<Func&, Init, element_t<Seq>>>;

..  function::
    template <sequence Seq, typename Func, typename Init = value_t<Seq>> \
        requires see_below \
    auto fold(Seq&& seq, Func func, Init init = {}) -> fold_result_t<Seq, Func, Init>;

``fold_first``
--------------

..  function::
    template <typename Seq, typename Func> \
        requires see_below \
    auto fold_first(Seq&& seq, Func func) -> optional<value_t<Seq>>;

``for_each``
------------

..  function::
    template <typename Seq, typename Func> \
        requires std::invocable<Func&, element_t<Seq>> \
    auto for_each(Seq&& seq, Func func) -> Func;

``for_each_while``
------------------

..  function::
    template <typename Seq, typename Func> \
        requires see_below \
    auto for_each_while(Seq&& seq, Func func) -> cursor_t<Seq>;

``inplace_reverse``
-------------------

..  function::
    template <bidirectional_sequence Seq> \
        requires bounded_sequence<Seq> && element_swappable_with<Seq, Seq> \
    auto inplace_reverse(Seq&& seq) -> void;

``max``
-------

..  function::
    template <sequence Seq, typename Cmp = std::ranges::less> \
        requires std::predicate<Cmp&, value_t<Seq>, element_t<Seq>> \
    auto max(Seq&& seq, Cmp cmp = {}) -> optional<value_t<Seq>>;

``min``
-------

..  function::
    template <sequence Seq, typename Cmp = std::ranges::less> \
        requires std::predicate<Cmp&, value_t<Seq>, element_t<Seq>> \
    auto min(Seq&& seq, Cmp cmp = {}) -> optional<value_t<Seq>>;

``minmax``
----------

..  struct:: template <sequence Seq> minmax_result;

..  function::
    template <sequence Seq, typename Cmp = std::ranges::less> \
        requires std::predicate<Cmp&, value_t<Seq>, element_t<Seq>> \
    auto minmax(Seq&& seq, Cmp cmp = {}) -> optional<minmax_result<Seq>>;

``none``
--------

..  function::
    template <sequence Seq, predicate_for<Seq> Pred> \
    auto none(Seq&& seq, Pred pred) -> bool;

``output_to``
-------------

..  function::
    template <sequence Seq, std::weakly_incrementable Iter> \
        requires std::indirectly_writable<Iter, element_t<Seq>> \
    auto output_to(Seq&& seq, Iter iter) -> Iter;

``product``
-----------

..  function::
    template <sequence Seq> \
        requires see_below \
    auto product(Seq&& seq) -> value_t<Seq>;

``search``
----------

..  function::
    template <multipass_sequence Haystack, multipass_sequence Needle, \
              typename Cmp = std::ranges::equal_to> \
        requires std::predicate<Cmp&, element_t<Haystack>, element_t<Needle>> \
    auto search(Haystack&& h, Needle&& n, Cmp cmp = {}) -> bounds_t<Haystack>;

``sort``
--------

..  function::
    template <random_access_sequence Seq, typename Cmp = std::ranges::less> \
        requires see_below \
    auto sort(Seq&& seq, Cmp cmp = {}) -> void;

``sum``
-------

..  function::
    template <sequence Seq> \
        requires see_below \
    auto sum(Seq&& seq) -> value_t<Seq>;

``swap_elements``
-----------------

..  function::
    template <sequence Seq1, sequence Seq2> \
        requires element_swappable_with<Seq1, Seq2> \
    auto swap_elements(Seq1&& seq1, Seq2&& seq2) -> void;

``to``
------

..  function::
    template <typename Container> \
        requires see_below \
    auto to(sequence auto&& seq, auto&&... args) -> Container;

..  function::
    template <template <typename...> typename Container> \
        requires see_below \
    auto to(sequence auto&& seq, auto&&... args);

    Converts a Flux sequence to a container, for example a :expr:`std::vector`.

    The first overload takes a "complete" container type name as its template argument, for example :expr:`std::vector\<int>` or :expr:`std::list\<std::string>`. The second overload takes a *template name* as its template argument, for example just :expr:`std::vector` or :expr:`std::map`, and will then use `CTAD <https://en.cppreference.com/w/cpp/language/class_template_argument_deduction>`_ to deduce the appropriate template arguments.

    The optional extra arguments provided to :func:`flux::to`, denoted :expr:`args...` above, will be forwarded to the selected container constructor as detailed below. The intention is to allow using (for example) custom allocator arguments.

    Let :expr:`C` be the target container type (either explicitly specified for the first overload, or deduced via CTAD). If the sequence's element type is convertible to :expr:`C::value_type`, then :func:`flux::to` will try to construct a :expr:`C` using the following methods, in order of priority:

    * Direct sequence construction using :expr:`C(std::forward(seq), std::forward(args)...)`

      ..  note:: If :expr:`C` is the same type as :expr:`seq`, this allows it to be copy- or move-constructed

    * Tagged sequence construction using :expr:`C(flux::from_sequence, std::forward(seq), std::forward(args)...)`

    * (In C++23) Tagged range construction as if by::

        auto sub = std::ranges::subrange(begin(seq), end(seq));
        return C(std::from_range, sub, std::forward(args)...);

    * C++17 iterator pair construction, as if by::

        auto view = std::ranges::subrange(begin(seq), end(seq)) | std::views::common;
        return C(view.begin(), view.end(), std::forward(args)...);

    * Inserting elements as if by::

        auto container = C(std::forward(args)...);
        output_to(std::forward(seq), range_inserter);
        return container;

      where :expr:`range_inserter` is :expr:`std::back_inserter(container)` if the container has a compatible :expr:`push_back()` member function, or :expr:`std::inserter(container, container.end())` otherwise. Will also attempt to call :expr:`container.reserve()` if possible to avoid reallocations during construction.

    If the sequence's element type is convertible to the container's value type but none of the above methods work, compilation will fail.

    If the sequence's element type itself satisfies :concept:`sequence`, but is *not* convertible to the container value type, then :expr:`flux::to\<C>(seq, args...)` is equivalent to::

        flux::to<C>(flux::map(std::forward(seq), [](auto&& elem) {
            flux::to<typename C::value_type>(std::forward(elem));
        }), std::forward(args)...);

    That is, :func:`to` will attempt to first convert each *inner* sequence to the container value type before proceeding as above.

    :tparam Container: A type name (for the first overload) or a template name (for the second overload) which names a compatible container type

    :param seq: A sequence to be converted to a container

    :param args: Optional extra arguments to be forwarded to the container constructor

    :example:

    :see also:

``write_to``
-------------

..  function::
    auto write_to(sequence auto&& seq, std::ostream& os) -> std::ostream&;

