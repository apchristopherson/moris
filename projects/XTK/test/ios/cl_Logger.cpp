/*
 * cl_Logger.cpp
 *
 *  Created on: Jun 19, 2017
 *      Author: ktdoble
 */

// Test suite
#include <mpi.h>
#include <iostream>

#include "ios/cl_Logger.hpp"
#include "catch.hpp"


TEST_CASE("Test the logger","[LOGGER][!throws]")
{
    // This is not an exception just a informational log, therefore this should not throw.
    REQUIRE_NOTHROW(XTK_INFO<<"Test the logger");

    // XTK_ERROR does not raise an exception
    REQUIRE_NOTHROW(XTK_ERROR<<"Error warning");

    int tProcRank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &tProcRank);

    // See if it works in parallel
    if (tProcRank == 0)
    {
        REQUIRE_NOTHROW(XTK_INFO<<"My rank is "<< tProcRank);
    }

    else
    {
        REQUIRE_NOTHROW(XTK_INFO<<"I am not processor 0");
    }

}