#define CATCH_CONFIG_RUNNER

#include <catch.hpp>

// MORIS header files.
#ifdef MORIS_HAVE_PARALLEL
#include <mpi.h>
#endif

#include <string>

// ---------------------------------------------------------------------

// MORIS header files.
#include "cl_Communication_Manager.hpp" // COM/src
#include "cl_Logger.hpp"                // MRS/IOS/src

moris::Comm_Manager gMorisComm;
moris::Logger       gLogger;

int
main(
        int    argc,
        char * argv[] )
{
    // Initialize Moris global communication manager
    gMorisComm.initialize(&argc, &argv);

    // Severity level 0 - all outputs
    gLogger.initialize( 0 );

    // check if path is set
    int result;

    // Run Tests
    result = Catch::Session().run( argc, argv );

    // finalize moris global communication manager
    gMorisComm.finalize();

    return result;
}

