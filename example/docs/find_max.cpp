
#include <flux.hpp>

#include "assert.hpp"
#include <string>
#include <vector>

int main()
{
    struct Person {
        std::string name;
        int age;
    };

    std::vector<Person> people{
        {"Alice", 44},
        {"Bob", 63},
        {"Chris", 29},
        {"Dani",  29},
        {"Eddy", 63}
    };

    // Get a cursor to the maximum of the people vector, according to age
    auto max_cur = flux::find_max(people, flux::proj(flux::cmp::compare, &Person::age));

    // The oldest person is 63
    assert(flux::read_at(people, max_cur).age == 63);

    // Note that (unlike std::max_element) find_max() return a cursor to the
    // *last* of several equally-maximum elements
    assert(flux::read_at(people, max_cur).name == "Eddy");
}