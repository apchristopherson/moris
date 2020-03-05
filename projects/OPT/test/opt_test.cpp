// Third-party header files.
#include <catch.hpp>

// MORIS project header files.
#include "cl_PRM_OPT_Parameters.hpp"
#include "cl_OPT_Manager.hpp"

// ---------------------------------------------------------------------
using namespace moris;

TEST_CASE( "[optimization]" )
{

    SECTION("GCMMA")
    {
        // Set up parameter lists
        moris::Cell<moris::Cell<ParameterList>> tParameterLists(2);
        tParameterLists(0).resize(1);
        tParameterLists(1).resize(1);
        tParameterLists(0)(0) = moris::prm::create_opt_manager_parameter_list();
        tParameterLists(1)(0) = moris::prm::create_gcmma_parameter_list();
        tParameterLists(0)(0).set("objective_finite_difference", true);
        tParameterLists(0)(0).set("constraint_finite_difference", true);

        // Create manager, assign cell of algorithms and problem
        moris::opt::Manager tManager(tParameterLists);

        // Solve optimization problem
        tManager.perform();

        // Check Solution
        REQUIRE(std::abs(tManager.get_objectives()(0)) < 1.0e-02); // check value of objective
        REQUIRE(norm(tManager.get_advs() - 1.0) < 1.0e-02); // check value of design variable, x
    }

    SECTION("SQP")
    {
        // Set up parameter lists
        moris::Cell<moris::Cell<ParameterList>> tParameterLists(2);
        tParameterLists(0).resize(1);
        tParameterLists(1).resize(1);
        tParameterLists(0)(0) = moris::prm::create_opt_manager_parameter_list();
        tParameterLists(1)(0) = moris::prm::create_sqp_parameter_list();

        // Create manager, assign cell of algorithms and problem
        moris::opt::Manager tManager(tParameterLists);

        // Solve optimization problem
        tManager.perform();

        // Check Solution
        REQUIRE(std::abs(tManager.get_objectives()(0)) < 1.0e-03); // check value of objective
        REQUIRE(norm(tManager.get_advs() - 1.0) < 2.0e-02); // check value of design variable, x
    }

//    SECTION("LBFGS")
//    {
//
//        // Create test interface
//        moris::opt::Interface_Rosenbrock tInterface;
//
//        // Create problem, assign interface
//        moris::opt::Problem_Rosenbrock tManager(&tInterface);
//        tManager.mConstrained = false;
//
//        // Create algorithm and set parameters
//        opt::Algorithm_API tAlgBFGS("LBFGS");
//
//        // Assign algorithm to cell for manager
//        moris::Cell< opt::Algorithm_API > tAlgorithms(1, tAlgBFGS);
//
//        // Create manager, assign cell of algorithms and problem
//        moris::opt::Manager tManager(tAlgorithms, &tManager);
//
//        // Solve optimization problem
//        tManager.perform();
//
//        moris::print(tManager.get_advs(), "advs");
//        moris::print(tManager.get_objectives(), "objective");
//
//
//    }

    SECTION( "SWEEP" )
    {
        // Set up default parameter lists
        moris::Cell<moris::Cell<ParameterList>> tParameterLists(2);
        tParameterLists(0).resize(1);
        tParameterLists(1).resize(1);
        tParameterLists(0)(0) = moris::prm::create_opt_manager_parameter_list();
        tParameterLists(1)(0) = moris::prm::create_sweep_parameter_list();

        // Path for writing file
        std::string tMorisRoot = std::getenv("MORISOUTPUT");
        std::string tHdf5FilePath = tMorisRoot + "sweep.hdf5";
        tParameterLists(1)(0).set("hdf5_path", tHdf5FilePath);

        // Create manager, assign cell of algorithms and problem
        moris::opt::Manager tManager(tParameterLists);

        // Solve optimization problem
        tManager.perform();
    }
}

