//
// example specific interface to moris
//

#include <catch.hpp>

#include "cl_Logger.hpp" // MRS/IOS/src

//---------------------------------------------------------------

int fn_WRK_Workflow_Main_Interface( int argc, char * argv[] );

//---------------------------------------------------------------

TEST_CASE("Channel_with_Four_Cylinders_Static",
        "[moris],[example],[thermal],[advection]")
{
    // define command line call
    int argc = 2;

    char tString1[] = "";
    char tString2[] = "./Channel_with_Four_Cylinders_Static.so";

    char * argv[2] = {tString1,tString2};

    // call to performance manager main interface
    int tRet = fn_WRK_Workflow_Main_Interface( argc, argv );

    // catch test statements should follow
    REQUIRE( tRet ==  0 );
}
