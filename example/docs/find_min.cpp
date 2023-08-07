
#include <flux.hpp>

#include <cassert>
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
    auto min_cur = flux::find_min(people, flux::proj(std::less{}, &Person::age));

    // The youngest person is 29
    assert(flux::read_at(people, min_cur).age == 29);

    // Note that find_min() return a cursor to the first of several
    // equally-minimum elements
    assert(flux::read_at(people, min_cur).name == "Chris");
}