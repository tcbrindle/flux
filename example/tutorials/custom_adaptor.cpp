
// Copyright (c) 2023 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <iostream>
#include <vector>

/*
 * In this example we'll show a couple of ways of writing a custom sequence
 * adaptor in Flux, first by using a simple generator and then a "full"
 * adaptor class.
 *
 * We'll be implementing an adaptor which repeats each element of the underlying
 * sequence a given number of times. For example, given a sequence
 *
 *     [1, 2, 3, 4, 5]
 *
 * and a repeat argument of 3, our adapted sequence would be
 *
 *     [1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5]
 *
 * And given an input sequence of
 *
 *    ["hello", "world"]
 *
 * and a repeat argument of 2, our adapted sequence would be
 *
 *    ["hello", "hello", "world", "world"]
 */


/* The easiest way to write a custom adaptor is to use a flux::generator.
 * This uses coroutines behind the scenes and so has some limitations:
 *  - the adapted sequence is only ever single-pass, not multipass, bidirectional, etc
 *  - it cannot be used in constexpr contexts
 *  - we need to be careful with lifetimes when passing args by reference (notice
 *    we pass by value into the function below)
 *  - compilers generally cannot optimise coroutines as we as other functions
 *
 * Nonetheless, this approach is very useful for quick "one-shot" custom adaptors
 * and for prototyping complex adaptors.
 */


// We'll write a function which returns flux::generator<T>, where T is the element
// type of our adapted sequence. This turns the function into a coroutine, so we
// can use co_yield inside it to yield our elements.
//
// We don't have to use a lambda -- this could be an ordinary function template,
// but a lambda makes the eventual usage a little nicer as we don't have to
// specify template parameters.
auto repeat_elements_v1 = []<flux::sequence Seq>(Seq seq, int rep)
    // Note that flux::generator<T> always yields T const& if T is an object type,
    // so this does the right thing even if Seq returns prvalues
    -> flux::generator<flux::element_t<Seq>>
{
    // first we'll iterate over the sequence we were given...
    for (auto&& elem : seq) {
        // ...and then co_yield the given element `rep` times
        for (int i = 0; i < rep; i++) {
            co_yield elem;
        }
    }
};

void test_repeat_elements_v1()
{
    using namespace std::string_view_literals;

    std::vector const vec{1, 2, 3, 4, 5};

    // We can use our custom adaptor in a Flux pipeline using the _() function.
    // The argument (3 in this case) is passed to the adaptor function along
    // with the sequence.
    //
    // We can then go on to add more adaptors to the pipeline, or as in this case
    // call an algorithm to print the elements to std::cout
    flux::ref(vec)
        ._(repeat_elements_v1, 3)
        .write_to(std::cout);
         // prints [1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5]

    std::cout << std::endl;

    auto seq = flux::split_string("hello world"sv, ' ')._(repeat_elements_v1, 2);

    for (auto str : seq) {
        std::cout << str << ' ';
    }
    // prints "hello hello world world"

    std::cout << std::endl;
}

/*
 * Using a generator works well for simple cases, but to get full functionality
 * we need to write an adaptor class template.
 *
 * In order to build up a pipeline, this class is templated on some underlying
 * sequence type
 */
template <flux::sequence Base>
struct repeat_elements_adaptor
    // To use the pipeline syntax, we need to inherit from this base class
    // using the "curiously recurring template pattern"
    : flux::inline_sequence_base<repeat_elements_adaptor<Base>>
{
private:
    Base base_; // We store the "upstream" sequence as a member variable
    int rep_;   // We also need to store how many times to repeat each element

public:
    // This constructor just initialises our two member variables.
    // The "rule of zero" means we don't need to provide any other special members
    constexpr repeat_elements_adaptor(Base&& base, int rep)
        : base_(std::move(base)),
          rep_(rep)
    {}

    /*
     * To implement the Flux sequence API, we use a public nested class named
     * `flux_sequence_traits`.
     *
     * Alternatively we could provide a class template specialisation
     *     flux::sequence_traits<repeat_elements_adaptor<Base>>
     * at global namespace scope, but a nested class is usually much more
     * convenient.
     */

    struct flux_sequence_traits {
    private:
        // For this particular adaptor we need to wrap the "upstream" cursor
        // to add extra data. We can call this class anything we like, but
        // cursor_type is nice and descriptive.
        struct cursor_type {
            flux::cursor_t<Base> base_cursor; // The upstream cursor
            int n = 0; // how many times we have repeated each upstream element

            // Cursors for multipass sequences need to be equality comparable.
            // Luckily for us the default operator== generated by the compiler
            // does the job perfectly, and will automatically give us operator!=
            // as well.
            constexpr bool operator==(cursor_type const&) const = default;
        };

        // A handy alias to save typing
        using self_t = repeat_elements_adaptor;

    public:
        // Because this adaptor returns the same elements as the upstream
        // sequence, we need to specify that the value type is also the same
        // as the upstream value type.
        // This handles cases where the upstream sequence is something like a
        // zip_adaptor, where value type can't be inferred from the element type.
        using value_type = flux::value_t<Base>;

        // This is tells the library that this sequence is infinite when the
        // upstream sequence is infinite. Not all adaptors are able to provide
        // this information, but since we can in this case it's good practise
        // to do so
        inline static constexpr bool is_infinite = flux::infinite_sequence<Base>;

        // For the basic sequence API, we need to provide four functions:
        // * first(), which returns a cursor to the first element
        // * is_last(), which tells the user when to stop iterating
        // * inc(), which increments a cursor to point to the next element
        // * read_at(), which returns the element at the given cursor position
        // These are all written as static member functions taking the
        // repeat_element_sequence as their first argument.

        static constexpr auto first(self_t& self) -> cursor_type
        {
            // Call first() on the upstream sequence and wrap the returned
            // cursor in our own cursor_type
            return cursor_type{.base_cursor = flux::first(self.base_)};
        }

        static constexpr auto is_last(self_t& self, cursor_type const& cur) -> bool
        {
            // Iteration is complete when the upstream cursor has reached the
            // terminal position
            return flux::is_last(self.base_, cur.base_cursor);
        }

        static constexpr auto inc(self_t& self, cursor_type& cur) -> void
        {
            // First, we increment the counter in our cursor
            ++cur.n;
            // If the counter has reached the maximum value, we increment the
            // upstream cursor and reset the count to zero
            if (cur.n == self.rep_) {
                flux::inc(self.base_, cur.base_cursor);
                cur.n = 0;
            }
        }

        static constexpr auto read_at(self_t& self, cursor_type const& cur)
            -> decltype(auto)
        {
            // We don't need to do anything special here, just call the
            // read_at() method on the upstream sequence.
            return flux::read_at(self.base_, cur.base_cursor);
        }

        /*
         * At this point our adaptor is fully-functioning, and will be
         * a multipass_sequence whenever the upstream sequence is multipass.
         * There are a few more functions we can optionally provide though,
         * to enable more features when the upstream sequence supports them.
         */

        // The size() function can be provided when we know the number of
        // sequence elements ahead of time, and can calculate that number in
        // constant time (i.e. without iterating over the whole sequence).
        // In this case we can calculate our size whenever the upstream sequence
        // can provide it
        // Note the use of requires clause to ensure that the upstream sequence
        // supports the operation
        static constexpr auto size(self_t& self) -> flux::distance_t
            requires flux::sized_sequence<Base>
        {
            return self.rep_ * flux::size(self.base_);
        }

        // The last() function returns a cursor in the last (past-the-end)
        // position, as an O(1) operation. Not all sequences are able to
        // provide this, but here we can when the upstream sequence does.
        static constexpr auto last(self_t& self) -> cursor_type
            requires flux::bounded_sequence<Base>
        {
            // As with first(), we call last() on the upstream sequence
            // and wrap the result in our own cursor type
            return cursor_type{.base_cursor = flux::last(self.base_)};
        }

        // The dec() function is the opposite of inc(): it decrements a
        // cursor so that it points to the previous element. If the upstream
        // sequence provides this then we should too, so that we can become
        // a bidirectional_sequence
        static constexpr auto dec(self_t& self, cursor_type& cur) -> void
            requires flux::bidirectional_sequence<Base>
        {
            // If the counter in our cursor is zero, we need to decrement
            // the upstream cursor and reset our count to rep-1
            if (cur.n == 0) {
                flux::dec(self.base_, cur.base_cursor);
                cur.n = self.rep_ - 1;
            } else {
                // Otherwise, we just decrement the counter
                --cur.n;
            }
        }

        /*
         * Our adaptor can now be a sized, bounded, bidirectional sequence
         * whenever the upstream sequence supports those operations. Pretty good!
         *
         * If we wanted to we could conditionally implement the remaining
         * functions needed to go all the way to random_access:
         * - an inc() overload that takes an arbitrary offset
         * - a distance() function that reports the distance between two cursors
         * - an operator<=> function for our cursor_type
         *
         * We could also implement the optional for_each_while() customisation
         * point in terms of for_each_while() on the upstream sequence,to enable
         * more efficient "internal iteration" for some pipelines.
         *
         * If you're interested, try writing them yourself!
         */
    };
};

/*
 * To make things nice and easy to use, we'll add a factory function which
 * constructs a repeat_elements_adaptor for us
 */
auto repeat_elements_v2 = []<flux::sequence Seq>(Seq seq, int n) {
    return repeat_elements_adaptor<Seq>(std::move(seq), n);
};

void test_repeat_elements_v2()
{
    std::vector const vec{1, 2, 3, 4, 5};

    // Use our repeat_elements_v2 in a pipeline.
    // Note that because std::vector is a bounded and bidirectional sequence,
    // and our adaptor also implements those traits, we can use additional
    // adaptors like reverse()
    flux::ref(vec)
        ._(repeat_elements_v2, 3)
        .reverse()
        .write_to(std::cout);
        // prints [5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1]


    // We can even use our adaptor in a constexpr context if we like
    constexpr auto sum = flux::ints(1)
                             .take(5)
                             ._(repeat_elements_v2, 2)
                             .sum(); // 1 + 1 + 2 + 2 + 3 + 3 + 4 + 4 + 5 + 5
    static_assert(sum == 30);
}

int main()
{
    test_repeat_elements_v1();
    test_repeat_elements_v2();
}