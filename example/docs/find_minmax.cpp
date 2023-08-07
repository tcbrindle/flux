
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

    // find_minmax() returns a pair of cursors which we can destructure
    // Here we'll get the min and max of the people vector, according to age
    auto [min, max] = flux::find_minmax(people, flux::proj(std::less{}, &Person::age));

    // The "minimum" is Chris. Dani is the same age, but Chris appears earlier
    // in the sequence
    assert(flux::read_at(people, min).name == "Chris");

    // The "maximum" is Eddy. Bob is the same age, but Eddy appears later in the
    // sequence
    assert(flux::read_at(people, max).name == "Eddy");
}