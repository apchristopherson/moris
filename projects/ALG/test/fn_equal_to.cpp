// Third-party header files.
#include <catch.hpp>

// MORIS header files.
#include "algorithms.hpp"

// ----------------------------------------------------------------------------

TEST_CASE(
        "moris::numeric::equal_to",
        "[moris],[numeric],[fn_equal_to],[equal_to]")
{
    #include "fn_equal_to.inc" // snippets ALG

    REQUIRE(pi_equal_16digits);
    REQUIRE(pi_equal_12digits);
    REQUIRE(pi_equal_8digits);
    REQUIRE(pi_equal_4digits);
    REQUIRE(pi_equal_2digits);

    REQUIRE_FALSE(pi_not_equal_8digits);
    REQUIRE_FALSE(pi_not_equal_4digits);
    REQUIRE_FALSE(pi_not_equal_2digits);
}
