// Third-party header files.
#include <catch.hpp>

// MORIS project header files.
#include "cl_PRM_OPT_Parameters.hpp"
#include "cl_OPT_Manager.hpp"
#include "cl_OPT_Interface.hpp"
#include "cl_OPT_Interface_User_Defined.hpp"
#include "fn_OPT_create_interface.hpp"
#include "../src/cl_OPT_Interface_User_Defined.hpp"
#include "cl_Communication_Tools.hpp"

namespace moris
{
    namespace opt
    {
        TEST_CASE( "[optimization]" )
        {

            // ---------------------------------------------------------------------------------------------------------

            SECTION("GCMMA")
            {
                // moris root
                std::string tMorisRoot = std::getenv("MORISROOT");

                // Set up default parameter lists
                moris::Cell<moris::Cell<ParameterList>> tParameterLists(3);
                tParameterLists(0).resize(1);
                tParameterLists(1).resize(1);
                tParameterLists(2).resize(1);
                tParameterLists(0)(0) = moris::prm::create_opt_problem_parameter_list();
                tParameterLists(1)(0) = moris::prm::create_opt_interface_parameter_list();
                tParameterLists(2)(0) = moris::prm::create_gcmma_parameter_list();

                // Change parameters
                tParameterLists(0)(0).set("finite_difference_type", "central");
                tParameterLists(0)(0).set("library", tMorisRoot + "projects/OPT/test/data/Rosenbrock.so");
                tParameterLists(1)(0).set("library", tMorisRoot + "projects/OPT/test/data/Rosenbrock.so");

                // Create manager
                moris::opt::Manager tManager(tParameterLists);

                // Solve optimization problem
                tManager.perform();

                // Check Solution
                REQUIRE(std::abs(tManager.get_objectives()(0)) < 2.0e-01); // check value of objective
                REQUIRE(norm(tManager.get_advs() - 1.0) < 1.0e-01); // check value of design variables
            }

            // ---------------------------------------------------------------------------------------------------------

            SECTION("SQP")
            {
                // moris root
                std::string tMorisRoot = std::getenv("MORISROOT");

                // Set up default parameter lists
                moris::Cell<moris::Cell<ParameterList>> tParameterLists(3);
                tParameterLists(0).resize(1);
                tParameterLists(1).resize(1);
                tParameterLists(2).resize(1);
                tParameterLists(0)(0) = moris::prm::create_opt_problem_parameter_list();
                tParameterLists(1)(0) = moris::prm::create_opt_interface_parameter_list();
                tParameterLists(2)(0) = moris::prm::create_sqp_parameter_list();

                // Change parameters
                tParameterLists(0)(0).set("library", tMorisRoot + "projects/OPT/test/data/Rosenbrock.so");
                tParameterLists(1)(0).set("library", tMorisRoot + "projects/OPT/test/data/Rosenbrock.so");

                // Create manager
                moris::opt::Manager tManager(tParameterLists);

                // Solve optimization problem
                tManager.perform();

                // Check Solution
                REQUIRE(std::abs(tManager.get_objectives()(0)) < 1.0e-03); // check value of objective
                REQUIRE(norm(tManager.get_advs() - 1.0) < 2.0e-02); // check value of design variables
            }

            // ---------------------------------------------------------------------------------------------------------

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

            // ---------------------------------------------------------------------------------------------------------

            SECTION( "sweep" )
            {
                // moris root
                std::string tMorisRoot = std::getenv("MORISROOT");
                std::string tMorisOutput = std::getenv("MORISOUTPUT");

                // Set up default parameter lists
                moris::Cell<moris::Cell<ParameterList>> tParameterLists(3);
                tParameterLists(0).resize(1);
                tParameterLists(1).resize(1);
                tParameterLists(2).resize(1);
                tParameterLists(0)(0) = moris::prm::create_opt_problem_parameter_list();
                tParameterLists(1)(0) = moris::prm::create_opt_interface_parameter_list();
                tParameterLists(2)(0) = moris::prm::create_sweep_parameter_list();

                // Change parameters
                tParameterLists(0)(0).set("library", tMorisRoot + "projects/OPT/test/data/Rosenbrock.so");
                tParameterLists(1)(0).set("library", tMorisRoot + "projects/OPT/test/data/Rosenbrock.so");
                tParameterLists(2)(0).set("hdf5_path", tMorisOutput + "sweep.hdf5");
                tParameterLists(2)(0).set("num_evaluations_per_adv", "3, 2");
                tParameterLists(2)(0).set("finite_difference_type", "all");
                tParameterLists(2)(0).set("finite_difference_epsilons", "0.001, 0.01; 0.00001, 0.00001");

                // Create manager
                moris::opt::Manager tManager(tParameterLists);

                // Solve optimization problem
                tManager.perform();
            }

            // ---------------------------------------------------------------------------------------------------------

            SECTION( "interface" )
            {
                if (par_size() == 4 or par_size() == 8)
                {
                    // moris root
                    std::string tMorisRoot = std::getenv("MORISROOT");

                    // Set up default interface parameter lists
                    moris::Cell<ParameterList> tParameterLists(5);
                    tParameterLists(0) = moris::prm::create_opt_interface_manager_parameter_list();
                    tParameterLists(1) = moris::prm::create_opt_interface_parameter_list();
                    tParameterLists(2) = moris::prm::create_opt_interface_parameter_list();
                    tParameterLists(3) = moris::prm::create_opt_interface_parameter_list();
                    tParameterLists(4) = moris::prm::create_opt_interface_parameter_list();

                    // Set manager parameters
                    tParameterLists(0).set("parallel", true);
                    tParameterLists(0).set("shared_advs", false);
                    if (par_size() == 4)
                    {
                        tParameterLists(0).set("num_processors_per_interface", "1, 1, 1, 1");
                    }
                    else
                    {
                        tParameterLists(0).set("num_processors_per_interface", "1, 2, 3, 2");
                    }

                    // Add individual user-defined dummy interfaces
                    tParameterLists(1).set("library", tMorisRoot + "projects/OPT/test/data/Interface_1.so");
                    tParameterLists(2).set("library", tMorisRoot + "projects/OPT/test/data/Interface_2.so");
                    tParameterLists(3).set("library", tMorisRoot + "projects/OPT/test/data/Interface_3.so");
                    tParameterLists(4).set("library", tMorisRoot + "projects/OPT/test/data/Interface_4.so");

                    // Create interface
                    std::shared_ptr<Interface> tInterface = create_interface(tParameterLists);
                    Matrix<DDRMat> tNewADVs = tInterface->initialize_advs();
                    tNewADVs = tInterface->get_criteria(tNewADVs);
                    for (uint tADVIndex = 0; tADVIndex < 8; tADVIndex++)
                    {
                        REQUIRE(tNewADVs(tADVIndex) == tADVIndex + 1);
                    }
                    tNewADVs = tInterface->get_dcriteria_dadv();
                    for (uint tADVIndex = 0; tADVIndex < 8; tADVIndex++)
                    {
                        REQUIRE(tNewADVs(tADVIndex, tADVIndex) == tADVIndex + 1);
                    }
                }
            }

            // ---------------------------------------------------------------------------------------------------------

        } // test case "[optimization]"
    } // namespace opt
} // namespace moris
