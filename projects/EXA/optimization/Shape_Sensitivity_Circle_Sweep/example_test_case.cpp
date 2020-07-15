//
// example specific interface to moris
//

#include <catch.hpp>

#include "cl_Logger.hpp" // MRS/IOS/src
#include "HDF5_Tools.hpp"

using namespace moris;

//---------------------------------------------------------------

int fn_WRK_Workflow_Main_Interface( int argc, char * argv[] );

//---------------------------------------------------------------

TEST_CASE("Shape_Sensitivity_Circle_Sweep",
        "[moris],[example],[optimization],[sweep]")
{
    // define command line call
    int argc = 2;

    char tString1[] = "";
    char tString2[] = "Shape_Sensitivity_Circle_Sweep.so";

    char * argv[2] = {tString1,tString2};

    // call to performance manager main interface
    int tRet = fn_WRK_Workflow_Main_Interface( argc, argv );

    // catch test statements should follow
    REQUIRE( tRet ==  0 );

    // Sweep HDF5 file
    hid_t tFileID = open_hdf5_file( "shape_opt_test.hdf5" );
    herr_t tStatus = 0;

    // Declare sensitivity matrices for comparison
    Matrix<DDRMat> tObjectiveAnalytical;
    Matrix<DDRMat> tConstraintsAnalytical;
    Matrix<DDRMat> tObjectiveFD;
    Matrix<DDRMat> tConstraintsFD;

    // Read analytical sensitivities
    load_matrix_from_hdf5_file( tFileID, "objective_gradients eval_1-1 analytical", tObjectiveAnalytical, tStatus);
    load_matrix_from_hdf5_file( tFileID, "constraint_gradients eval_1-1 analytical", tConstraintsAnalytical, tStatus);
    REQUIRE(tObjectiveAnalytical.length() == tConstraintsAnalytical.length()); // one objective and one constraint for this problem only

    // Read FD sensitivities and compare
    Cell<std::string> tFDTypes = {"fd_forward", "fd_backward", "fd_central"};
    for (uint tFDIndex = 0; tFDIndex < tFDTypes.size(); tFDIndex++)
    {
        load_matrix_from_hdf5_file( tFileID, "objective_gradients eval_1-1 epsilon_1-1 " + tFDTypes(tFDIndex), tObjectiveFD, tStatus);
        load_matrix_from_hdf5_file( tFileID, "constraint_gradients eval_1-1 epsilon_1-1 " + tFDTypes(tFDIndex), tConstraintsFD, tStatus);

        REQUIRE(tObjectiveAnalytical.length() == tObjectiveFD.length());
        REQUIRE(tConstraintsAnalytical.length() == tConstraintsFD.length());

        for (uint tADVIndex = 0; tADVIndex < tObjectiveAnalytical.length(); tADVIndex++)
        {
            CHECK(tObjectiveAnalytical(tADVIndex) == Approx(tObjectiveFD(tADVIndex)));
            CHECK(tConstraintsAnalytical(tADVIndex) == Approx(tConstraintsFD(tADVIndex)));
        }
    }

    // close file
    close_hdf5_file( tFileID );

}
