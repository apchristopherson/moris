/*
 * cl_Dist_Vector_Test.cpp
 *
 *  Created on: Jun 18, 2018
 *      Author: schmidt
 */
#include "catch.hpp"
#include "fn_equal_to.hpp" // ALG/src
#include "typedefs.hpp" // COR/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"

#include "cl_Communication_Tools.hpp" // COM/src
#include "cl_Matrix_Vector_Factory.hpp" // DLA/src
#include "cl_Solver_Interface_Proxy.hpp" // DLA/src
#include "cl_SOL_Dist_Vector.hpp" // DLA/src
#include "cl_SOL_Dist_Map.hpp" // DLA/src

namespace moris
{
TEST_CASE("Dist Vector","[Dist Vector],[DistLinAlg]")
{
    // Determine process rank
    size_t rank = par_rank();
    size_t size = par_size();

    if (size == 4)
    {
        // Build Input Class
        Solver_Interface* tSolverInput = new Solver_Interface_Proxy( );

        // Build matrix factory
        Matrix_Vector_Factory      tMatFactory;

        // Build map
        Dist_Map  * tMap = tMatFactory.create_map( tSolverInput->get_my_local_global_map(),
                                                    tSolverInput->get_constrained_Ids() );

        // build distributed vector
        Dist_Vector * tVectorA = tMatFactory.create_vector( tSolverInput, tMap, 1 );

        // Loop over all elements and fill values in RHS vector
        for (moris::uint Ii=0; Ii< tSolverInput->get_num_my_elements(); Ii++)
        {
            Matrix< DDSMat > tElementTopology;
            tSolverInput->get_element_topology(Ii, tElementTopology );

            Cell< Matrix< DDRMat > > tElementRHS;
            tSolverInput->get_equation_object_rhs(Ii, tElementRHS );

            // Fill elementRHS in distributed RHS
            tVectorA->sum_into_global_values( tElementTopology,
                                              tElementRHS(0));
        }
        tVectorA->vector_global_asembly();

//        const char* filename = "/home/schmidt/matrixvec.mtx";
//        tVectorA->save_vector_to_matrix_market_file( filename );

        moris::Matrix< DDRMat > tSol ( 15, 1, 0.0 );

        // needed as offset parameter for Epetra. =0
        sint tMyLDA = 0;

        // Get solution and output it in moris::Mat LHSValues
        tVectorA->get_epetra_vector()->ExtractCopy( tSol.data(), tMyLDA );

        if (rank == 0)
        {
            CHECK( equal_to( tSol( 0,0 ), 0.0 ) );
            CHECK( equal_to( tSol( 5,0 ), 0.0 ) );
        }
        if (rank == 2)
        {
            CHECK( equal_to( tSol( 0,0 ), 0.0 ) );
            CHECK( equal_to( tSol( 1,0 ), 1.0 ) );
        }

        delete( tSolverInput );
        delete( tMap );
        delete( tVectorA );
    }
}

TEST_CASE("Sum Dist Vector","[Sum Dist Vector],[DistLinAlg]")
{
    // Determine process rank
    size_t rank = par_rank();
    size_t size = par_size();

    if (size == 4)
    {
        // Build Input Class
        Solver_Interface* tSolverInput = new Solver_Interface_Proxy( );

        // Build matrix factory
        Matrix_Vector_Factory     tMatFactory;

        // Build map
        Dist_Map * tMap = tMatFactory.create_map( tSolverInput->get_my_local_global_map(),
                                                   tSolverInput->get_constrained_Ids() );

        // build distributed vector
        Dist_Vector * tVectorA = tMatFactory.create_vector( tSolverInput, tMap, 1 );
        Dist_Vector * tVectorB = tMatFactory.create_vector( tSolverInput, tMap, 1 );

        // Loop over all elements and fill values in RHS vector
        for (moris::uint Ii=0; Ii< tSolverInput->get_num_my_elements(); Ii++)
        {
            Matrix< DDSMat > tElementTopology;
            tSolverInput->get_element_topology(Ii, tElementTopology );

            Cell< Matrix< DDRMat > > tElementRHS;
            tSolverInput->get_equation_object_rhs(Ii, tElementRHS );

            // Fill elementRHS in distributed RHS
            tVectorA->sum_into_global_values( tElementTopology,
                                              tElementRHS(0));

            tVectorB->sum_into_global_values( tElementTopology,
                                              tElementRHS(0));
        }
        tVectorA->vector_global_asembly();
        tVectorB->vector_global_asembly();

        // Add tVectorB to tVectorA
        tVectorA->vec_plus_vec(1.0, *tVectorB, 1.0 );

        moris::Matrix< DDRMat > tSol ( 15, 1, 0.0 );

        // needed as offset parameter for Epetra. =0
        sint tMyLDA = 0;

        // Get solution and output it in moris::Mat LHSValues
        tVectorA->get_epetra_vector()->ExtractCopy( tSol.data(), tMyLDA );

        if (rank == 0)
        {
            CHECK( equal_to( tSol( 0,0 ), 0.0 ) );
            CHECK( equal_to( tSol( 5,0 ), 0.0 ) );
        }
        if (rank == 2)
        {
            CHECK( equal_to( tSol( 0,0 ), 0.0 ) );
            CHECK( equal_to( tSol( 1,0 ), 2.0 ) );
        }
        delete( tSolverInput );
        delete( tMap );
        delete( tVectorA );
        delete( tVectorB );
    }
}

TEST_CASE("Scale Dist Vector","[Scale Dist Vector],[DistLinAlg]")
{
    // Determine process rank
    size_t rank = par_rank();
    size_t size = par_size();

    if (size == 4)
    {
        // Build Input Class
        Solver_Interface * tSolverInput = new Solver_Interface_Proxy( );

        // Build matrix factory
        Matrix_Vector_Factory      tMatFactory;

        // Build map
        Dist_Map * tMap = tMatFactory.create_map( tSolverInput->get_my_local_global_map(),
                                                   tSolverInput->get_constrained_Ids() );

        // build distributed vector
        Dist_Vector * tVectorA = tMatFactory.create_vector( tSolverInput, tMap, 1 );

        // Loop over all elements and fill values in RHS vector
        for (moris::uint Ii=0; Ii< tSolverInput->get_num_my_elements(); Ii++)
        {
            Matrix< DDSMat > tElementTopology;
            tSolverInput->get_element_topology(Ii, tElementTopology );

            Cell< Matrix< DDRMat > > tElementRHS;
            tSolverInput->get_equation_object_rhs(Ii, tElementRHS );

            // Fill elementRHS in distributed RHS
            tVectorA->sum_into_global_values( tElementTopology,
                                              tElementRHS(0));
        }
        tVectorA->vector_global_asembly();

        // Scale tVectorA
        tVectorA->scale_vector(5.5);

        moris::Matrix< DDRMat > tSol ( 15, 1, 0.0 );

        // needed as offset parameter for Epetra. =0
        sint tMyLDA = 0;

        // Get solution and output it in moris::Mat LHSValues
        tVectorA->get_epetra_vector()->ExtractCopy( tSol.data(), tMyLDA );

        if (rank == 0)
        {
            CHECK( equal_to( tSol( 0,0 ), 0.0 ) );
            CHECK( equal_to( tSol( 5,0 ), 0.0 ) );
        }
        if (rank == 2)
        {
            CHECK( equal_to( tSol( 0,0 ), 0.0 ) );
            CHECK( equal_to( tSol( 1,0 ), 5.5 ) );
        }
        delete( tSolverInput );
        delete( tMap );
        delete( tVectorA );
    }
}

TEST_CASE("Norm/Lenth Dist Vector","[Norm Dist Vector],[DistLinAlg]")
{
    // Determine process rank
    size_t rank = par_rank();
    size_t size = par_size();

    if (size == 4)
    {
        // Build Input Class
        Solver_Interface* tSolverInput = new Solver_Interface_Proxy( );

        // Build matrix factory
        Matrix_Vector_Factory      tMatFactory;

        // Build map
        Dist_Map * tMap = tMatFactory.create_map( tSolverInput->get_my_local_global_map(),
                                                   tSolverInput->get_constrained_Ids());

        // build distributed vector
        Dist_Vector * tVectorA = tMatFactory.create_vector( tSolverInput, tMap, 1 );

        // Loop over all elements and fill values in RHS vector
        for (moris::uint Ii=0; Ii< tSolverInput->get_num_my_elements(); Ii++)
        {
            Matrix< DDSMat > tElementTopology;
            tSolverInput->get_element_topology(Ii, tElementTopology );

            Cell< Matrix< DDRMat > > tElementRHS;
            tSolverInput->get_equation_object_rhs(Ii, tElementRHS );

            // Fill elementRHS in distributed RHS
            tVectorA->sum_into_global_values( tElementTopology,
                                              tElementRHS(0));
        }
        tVectorA->vector_global_asembly();

        // Get Vector norm
        moris::real tNorm = tVectorA->vec_norm2();
        // Get local vector lengt
        moris::uint tLocLength = tVectorA->vec_local_length();
        // Get global vector lengt
        moris::uint tGlobLength = tVectorA->vec_global_length();

        if (rank == 0)
        {
            CHECK( equal_to( tNorm, 1.0 ) );
            CHECK( equal_to( tLocLength, 6.0 ) );
            CHECK( equal_to( tGlobLength, 15.0 ) );
        }
        if (rank == 2)
        {
            CHECK( equal_to( tNorm, 1.0 ) );
            CHECK( equal_to( tLocLength, 2.0 ) );
            CHECK( equal_to( tGlobLength, 15.0 ) );
        }
        delete( tSolverInput );
        delete( tMap );
        delete( tVectorA );
    }
}

TEST_CASE("Import Dist Vector","[Import Dist Vector],[DistLinAlg]")
{
    // Determine process rank
    size_t rank = par_rank();
    size_t size = par_size();

    if (size == 4)
    {
        // Build Input Class
        Solver_Interface* tSolverInput = new Solver_Interface_Proxy( );

        // Build matrix factory
        Matrix_Vector_Factory      tMatFactory;

        // Build map
        Dist_Map * tMap = tMatFactory.create_map( tSolverInput->get_my_local_global_map()  );

        Dist_Map * tMapFree = tMatFactory.create_map( tSolverInput->get_my_local_global_map(),
                                                       tSolverInput->get_constrained_Ids() );

        // build local distributed free vector
        Dist_Vector * tVectorFree = tMatFactory.create_vector( tSolverInput, tMapFree, 1 );
        // build local distributed full vector
        Dist_Vector * tVectorFull = tMatFactory.create_vector( tSolverInput, tMap, 1 );

        // Loop over all elements and fill values in RHS vector
        for (moris::uint Ii=0; Ii< tSolverInput->get_num_my_elements(); Ii++)
        {
            Matrix< DDSMat > tElementTopology;
            tSolverInput->get_element_topology(Ii, tElementTopology );

            Cell< Matrix< DDRMat >> tElementRHS;
            tSolverInput->get_equation_object_rhs(Ii, tElementRHS );

            // Fill elementRHS in distributed RHS
            tVectorFree->sum_into_global_values( tElementTopology,
                                                 tElementRHS(0));
        }
        tVectorFree->vector_global_asembly();

        tVectorFull->import_local_to_global( *tVectorFree );

        moris::Matrix< DDRMat > tSol ( 18, 1, 0.0 );

        // needed as offset parameter for Epetra. =0
        sint tMyLDA = 0;

        // Get solution and output it in moris::Mat LHSValues
        tVectorFull->get_epetra_vector()->ExtractCopy(  tSol.data(), tMyLDA );

        // Get local vector lengt
        moris::uint tLocLength = tVectorFull->vec_local_length();
        // Get global vector lengt
        moris::uint tGlobLength = tVectorFull->vec_global_length();

        if (rank == 0)
        {
            CHECK( equal_to( tSol( 0,0 ), 0.0 ) );
            CHECK( equal_to( tLocLength, 8.0 ) );
            CHECK( equal_to( tGlobLength, 18.0 ) );
        }
        if (rank == 2)
        {
            CHECK( equal_to( tSol( 1,0 ), 1.0 ) );
            CHECK( equal_to( tLocLength, 2.0 ) );
            CHECK( equal_to( tGlobLength, 18.0 ) );
        }
        delete( tSolverInput );
        delete( tMap );
        delete( tVectorFree );
        delete( tVectorFull );
    }
}
}


