
#include "catch.hpp"

#include <flux/op/output_to.hpp>
#include <flux/op/take.hpp>
#include <flux/ranges.hpp>
#include <flux/source/istream.hpp>
#include <flux/source/istreambuf.hpp>

#include <array>
#include <list>
#include <sstream>
#include <vector>

#include "test_utils.hpp"

namespace {

constexpr bool test_output_to()
{
    {
        int arr1[] = {1, 2, 3, 4, 5};
        int arr2[5] = {};

        int* p = flux::output_to(arr1, arr2);

        STATIC_CHECK(check_equal(arr2, {1, 2, 3, 4, 5}));
        STATIC_CHECK(p == arr2 + 5);
    }

    return true;
}
static_assert(test_output_to());

}

TEST_CASE("output to")
{
    bool result = test_output_to();
    REQUIRE(result);

    SECTION("...with contiguous iterators")
    {
        std::vector<int> const in{1, 2, 3, 4, 5};
        std::vector<int> out(5);

        auto iter = flux::output_to(in, out.begin());

        REQUIRE(out == in);
        REQUIRE(iter == out.cend());
    }

    SECTION("...with back_inserter")
    {
        std::vector<int> const in{1, 2, 3, 4, 5};
        std::list<int> out;

        flux::output_to(in, std::back_inserter(out));

        REQUIRE(check_equal(in, out));
    }

    SECTION("...with iostreams")
    {
        std::istringstream iss("1 2 3");
        std::ostringstream oss;

        flux::from_istream<int>(iss).output_to(std::ostream_iterator<int>(oss));

        REQUIRE(oss.str() == "123");
    }

    SECTION("...with streambufs")
    {
        std::istringstream iss(" hello world!! ");
        std::ostringstream oss;

        flux::from_istreambuf(iss).output_to(std::ostreambuf_iterator(oss));

        REQUIRE(oss.str() == " hello world!! ");
    }
}