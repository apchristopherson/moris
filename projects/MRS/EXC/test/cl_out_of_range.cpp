// C++ header files.
#include <sstream>

// Third-party header files.
#include <catch.hpp>

// moris header files.
#include "exceptions.hpp"

#include <iostream>
// ----------------------------------------------------------------------------

TEST_CASE(
		"moris::exceptions::out_of_range",
		"[moris],[exceptions],[cl_out_of_range],[out_of_range]")
{
	#include "exceptions/cl_out_of_range.inc"

	REQUIRE_THROWS_AS(throw error, moris::exceptions::out_of_range);
	REQUIRE(error_msg.str() == std::string(error.what()));
}
