/*
 * cl_DLA_Multigrid_Test.cpp
 *
 *  Created on: Nov 18, 2018
 *      Author: schmidt
 */
#include "../../../FEM/INT/src/cl_FEM_Element_Bulk.hpp"
#include "catch.hpp"

#include "fn_equal_to.hpp" // ALG/src

#include "typedefs.hpp" // COR/src

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"

#include "cl_Communication_Tools.hpp" // COM/src
#include "cl_Matrix_Vector_Factory.hpp" // DLA/src
#include "cl_Vector.hpp" // DLA/src

#include "cl_SDF_Generator.hpp"

#include "cl_MSI_Multigrid.hpp"
#include "cl_MSI_Model_Solver_Interface.hpp"
#include "cl_MSI_Solver_Interface.hpp"

#include "cl_HMR_Parameters.hpp"
#include "cl_HMR.hpp"
#include "cl_HMR_Field.hpp"

#include "cl_FEM_Node_Base.hpp"
#include "cl_FEM_IWG_L2.hpp"

#include "cl_DLA_Solver_Factory.hpp"
#include "cl_DLA_Solver_Interface.hpp"

#include "cl_NLA_Nonlinear_Solver_Factory.hpp"
#include "cl_NLA_Nonlinear_Problem.hpp"
#include "cl_MSI_Solver_Interface.hpp"
#include "cl_MSI_Equation_Object.hpp"
#include "cl_MSI_Model_Solver_Interface.hpp"
#include "cl_DLA_Linear_Solver_Aztec.hpp"
#include "cl_DLA_Linear_Solver.hpp"
#include "cl_NLA_Nonlinear_Solver.hpp"

#include "fn_norm.hpp"

moris::real
LevelSetFunction( const moris::Matrix< moris::DDRMat > & aPoint )
{
    return norm( aPoint ) - 0.9;
}

namespace moris
{
using namespace dla;
using namespace NLA;

TEST_CASE("DLA_Multigrid","[DLA],[DLA_multigrid]")
{
    if( moris::par_size() == 1 )
    {
        // order for this example
        moris::uint tOrder = 1;

        // create parameter object
        moris::hmr::Parameters tParameters;
        tParameters.set_number_of_elements_per_dimension( { { 2 }, { 2 } } );
        tParameters.set_verbose( false );
        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );
        tParameters.set_mesh_orders_simple( tOrder );

        // create HMR object
        moris::hmr::HMR tHMR( tParameters );

        // flag first element for refinement
        tHMR.flag_element( 0 );
        tHMR.perform_refinement( moris::hmr::RefinementMode::SIMPLE );
        tHMR.update_refinement_pattern();

        tHMR.flag_element( 0 );
        tHMR.perform_refinement( moris::hmr::RefinementMode::SIMPLE );
        tHMR.update_refinement_pattern();

        tHMR.finalize();

        // grab pointer to output field
        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tOrder );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tOrder );

        // evaluate node values
        tField->evaluate_scalar_function( LevelSetFunction );

        tHMR.save_bsplines_to_vtk("DLA_BSplines.vtk");

        moris::map< moris::moris_id, moris::moris_index > tMap;
        tMesh->get_adof_map( tOrder, tMap );
        //tMap.print("Adof Map");

         //-------------------------------------------------------------------------------------------

        // create IWG object
        Cell< fem::IWG* > tIWGs ( 1, nullptr );
        tIWGs( 0 ) = new moris::fem::IWG_L2( );

        map< moris_id, moris_index >   tCoefficientsMap;
        Cell< fem::Node_Base* >        tNodes;
        Cell< MSI::Equation_Object* >  tElements;

        // get map from mesh
        tMesh->get_adof_map( tOrder, tCoefficientsMap );

        // ask mesh about number of nodes on proc
        luint tNumberOfNodes = tMesh->get_num_nodes();

        // create node objects
        tNodes.resize( tNumberOfNodes, nullptr );

        for( luint k = 0; k < tNumberOfNodes; ++k )
        {
            tNodes( k ) = new fem::Node( &tMesh->get_mtk_vertex( k ) );
        }

        // ask mesh about number of elements on proc
        luint tNumberOfElements = tMesh->get_num_elems();

         // create equation objects
         tElements.resize( tNumberOfElements, nullptr );

        for( luint k=0; k<tNumberOfElements; ++k )
        {
            // create the element
            tElements( k ) = new fem::Element_Bulk( & tMesh->get_mtk_cell( k ),
                                                    tIWGs,
                                                    tNodes );
        }

        MSI::Model_Solver_Interface * tMSI = new moris::MSI::Model_Solver_Interface( tElements,
                                                                                     tMesh->get_communication_table(),
                                                                                     tCoefficientsMap,
                                                                                     tMesh->get_num_coeffs( tOrder ),
                                                                                     tMesh.get() );

        tMSI->set_param("L2")= (sint)tOrder;

        tMSI->finalize( true );

        moris::Solver_Interface * tSolverInterface = new moris::MSI::MSI_Solver_Interface( tMSI );

//---------------------------------------------------------------------------------------------------------------

        Matrix< DDUMat > tAdofMap = tMSI->get_dof_manager()->get_adof_ind_map();

        NLA::Nonlinear_Problem * tNonlinearProblem =  new NLA::Nonlinear_Problem( tSolverInterface, 0, true, MapType::Petsc );

        // create factory for nonlinear solver
        NLA::Nonlinear_Solver_Factory tNonlinFactory;

        // create nonlinear solver
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolver = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        // create factory for linear solver
        dla::Solver_Factory  tSolFactory;

        // create linear solver
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolver = tSolFactory.create_solver( SolverType::PETSC );

        tLinearSolver->set_param("KSPType") = std::string( KSPFGMRES );
        //tLinearSolver->set_param("PCType")  = std::string( PCMG );
        tLinearSolver->set_param("PCType")  = std::string( PCILU );

        tLinearSolver->set_param("ILUFill")  = 3;

        // create solver manager
        dla::Linear_Solver * mLinSolver = new dla::Linear_Solver();
        Nonlinear_Solver  tNonLinSolManager;

        // set manager and settings
        tNonlinearSolver->set_linear_solver( mLinSolver );

        // set first solver
        mLinSolver->set_linear_algorithm( 0, tLinearSolver );

        tNonLinSolManager.set_nonlinear_algorithm( tNonlinearSolver, 0 );

         for( auto tElement : tElements )
         {
             Matrix< DDRMat > & tNodalWeakBCs = tElement->get_weak_bcs();
             uint tNumberOfNodes = tElement->get_num_nodes();
             tNodalWeakBCs.set_size( tNumberOfNodes, 1 );

             for( uint k=0; k<tNumberOfNodes; ++k )
             {
                 // copy weakbc into element
                 tNodalWeakBCs( k ) = tMesh->get_value_of_scalar_field( 3,
                                                                        EntityRank::NODE,
                                                                        tElement->get_node_index( k ) );
             }
         }

         tNonLinSolManager.solve( tNonlinearProblem );

         // temporary array for solver
         Matrix< DDRMat > tSolution;
         tNonlinearSolver->get_full_solution( tSolution );

         CHECK( equal_to( tSolution( 0, 0 ), -0.9010796, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 1, 0 ), -0.7713064956, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 2, 0 ), -0.7713064956, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 3, 0 ), -0.733678875, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 4, 0 ), -0.6539977592, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 5, 0 ), -0.6539977592, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 6, 0 ), -0.54951427221, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 7, 0 ), -0.3992520178, 1.0e+08 ) );
         CHECK( equal_to( tSolution( 8, 0 ), -0.14904048484, 1.0e+08 ) );

         // get index of mesh
         uint tMeshIndex = tHMR.get_mesh_index( tOrder,
                                                tHMR.get_parameters()->get_lagrange_output_pattern() );

         // dump mesh
         tHMR.save_to_exodus ( tMeshIndex,  // index in database
                               "Mesh.exo",  // path
                               0.0 );       // timestep

         delete ( tMSI );
         delete ( tIWGs( 0 ) );
         delete ( tSolverInterface );
         delete ( tNonlinearProblem );
         delete ( mLinSolver );

         for( luint k=0; k<tNumberOfElements; ++k )
         {
             // create the element
             delete tElements( k );
         }

         for( luint k = 0; k < tNumberOfNodes; ++k )
         {
             delete tNodes( k );
         }
    }
}

/*
TEST_CASE("DLA_Multigrid_Sphere","[DLA],[DLA_multigrid_circle]")
{
    if( moris::par_size() == 1 )
    {
        // order for this example
        moris::uint tOrder = 1;

        // create parameter object
        moris::hmr::Parameters tParameters;
        tParameters.set_number_of_elements_per_dimension( { { 4 }, { 4 } } );
        tParameters.set_verbose( false );
        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );
        tParameters.set_mesh_orders_simple( tOrder );
        tParameters.set_refinement_buffer( 3 );

        // create HMR object
        moris::hmr::HMR tHMR( tParameters );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tOrder );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tOrder );

        for( uint k=0; k<2; ++k )
        {
            tField->evaluate_scalar_function( LevelSetFunction );
            tHMR.flag_surface_elements( tField );
            tHMR.perform_refinement( moris::hmr::RefinementMode::SIMPLE );
            tHMR.update_refinement_pattern();
        }

        tHMR.finalize();

        // evaluate node values
        tField->evaluate_scalar_function( LevelSetFunction );

        tHMR.save_to_exodus( "Circle.exo" );

         //tHMR.save_bsplines_to_vtk("BSplines.vtk");

         moris::map< moris::moris_id, moris::moris_index > tMap;
         tMesh->get_adof_map( tOrder, tMap );
         //tMap.print("Adof Map");

         //-------------------------------------------------------------------------------------------

         // create IWG object
         fem::IWG_L2 * tIWG = new moris::fem::IWG_L2( );

         map< moris_id, moris_index >   tCoefficientsMap;
         Cell< fem::Node_Base* >        tNodes;
         Cell< MSI::Equation_Object* >  tElements;

         // get map from mesh
         tMesh->get_adof_map( tOrder, tCoefficientsMap );

         // ask mesh about number of nodes on proc
         luint tNumberOfNodes = tMesh->get_num_nodes();

         // create node objects
         tNodes.resize( tNumberOfNodes, nullptr );

         for( luint k = 0; k < tNumberOfNodes; ++k )
         {
             tNodes( k ) = new fem::Node( &tMesh->get_mtk_vertex( k ) );
         }

         // ask mesh about number of elements on proc
         luint tNumberOfElements = tMesh->get_num_elems();

         // create equation objects
         tElements.resize( tNumberOfElements, nullptr );

         for( luint k=0; k<tNumberOfElements; ++k )
         {
             // create the element
             tElements( k ) = new fem::Element( & tMesh->get_mtk_cell( k ),
                                                tIWG,
                                                tNodes );
         }

         MSI::Model_Solver_Interface * tMSI = new moris::MSI::Model_Solver_Interface( tElements,
                                                                                      tMesh->get_communication_table(),
                                                                                      tCoefficientsMap,
                                                                                      tMesh->get_num_coeffs( tOrder ),
                                                                                      tMesh.get() );

         tMSI->set_param("L2")= (sint)tOrder;

         tMSI->finalize( true );

         moris::Solver_Interface * tSolverInterface = new moris::MSI::MSI_Solver_Interface( tMSI );

//---------------------------------------------------------------------------------------------------------------

         Matrix< DDUMat > tAdofMap = tMSI->get_dof_manager()->get_adof_ind_map();

         NLA::Nonlinear_Problem * tNonlinerarProblem =  new NLA::Nonlinear_Problem( tSolverInterface, true, MapType::Petsc );

         // create factory for nonlinear solver
         NLA::Nonlinear_Solver_Factory tNonlinFactory;

         // create nonlinear solver
         std::shared_ptr< NLA::Nonlinear_Solver > tNonlinearSolver = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

         // create factory for linear solver
         dla::Solver_Factory  tSolFactory;

         // create linear solver
         std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolver = tSolFactory.create_solver( SolverType::PETSC );

         tLinearSolver->set_param("KSPType") = std::string(KSPFGMRES);
         //tLinearSolver->set_param("PCType")  = std::string(PCMG);

         tLinearSolver->set_param("ILUFill")  = 0;

         // create solver manager
         dla::Linear_Solver * mLinSolver = new dla::Linear_Solver();

         // set manager and settings
         tNonlinearSolver->set_linear_solver( mLinSolver );

         // set first solver
         mLinSolver->set_linear_algorithm( 0, tLinearSolver );

         for( auto tElement : tElements )
         {
             Matrix< DDRMat > & tNodalWeakBCs = tElement->get_weak_bcs();
             uint tNumberOfNodes = tElement->get_num_nodes();
             tNodalWeakBCs.set_size( tNumberOfNodes, 1 );

             for( uint k=0; k<tNumberOfNodes; ++k )
             {
                 // copy weakbc into element
                 tNodalWeakBCs( k ) = tMesh->get_value_of_scalar_field( 3,
                                                                        EntityRank::NODE,
                                                                        tElement->get_node_index( k ) );
             }
         }

         tNonlinearSolver->solver_nonlinear_system( tNonlinerarProblem );

         moris::Matrix< DDSMat > tExternalIndices( 9, 1 );
         tExternalIndices( 0, 0 ) = 17;
         tExternalIndices( 1, 0 ) = 18;
         tExternalIndices( 2, 0 ) = 21;
         tExternalIndices( 3, 0 ) = 22;
         tExternalIndices( 4, 0 ) = 23;
         tExternalIndices( 5, 0 ) = 25;
         tExternalIndices( 6, 0 ) = 27;
         tExternalIndices( 7, 0 ) = 26;
         tExternalIndices( 8, 0 ) = 28;

         moris::Matrix< DDSMat > tInternalIndices;

         tMSI->read_multigrid_maps( 2, tExternalIndices, 0, tInternalIndices );

         // get index of mesh
         uint tMeshIndex = tHMR.get_mesh_index( tOrder,
                                                tHMR.get_parameters()->get_lagrange_output_pattern() );

         // dump mesh
         tHMR.save_to_exodus ( tMeshIndex,  // index in database
                               "Meshcircle.exo",  // path
                               0.0 );       // timestep

         delete tMSI;
         delete tIWG;
         delete tSolverInterface;

         for( luint k=0; k<tNumberOfElements; ++k )
         {
             // create the element
             delete tElements( k );
         }

         for( luint k = 0; k < tNumberOfNodes; ++k )
         {
             delete tNodes( k );
         }
    }
}
*/
TEST_CASE("DLA_Multigrid_Circle","[DLA],[DLA_multigrid_sphere]")
{
    if( moris::par_size() == 1 )
    {
        // order for this example
        moris::uint tOrder = 1;

        // create parameter object
        moris::hmr::Parameters tParameters;
        tParameters.set_number_of_elements_per_dimension( { { 4 }, { 4 }, { 4 } } );
        tParameters.set_verbose( false );
        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );
        tParameters.set_mesh_orders_simple( tOrder );
        tParameters.set_refinement_buffer( 1 );

        // create HMR object
        moris::hmr::HMR tHMR( tParameters );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tOrder );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tOrder );

        for( uint k=0; k<3; ++k )
        {
            tField->evaluate_scalar_function( LevelSetFunction );
            tHMR.flag_surface_elements( tField );
            tHMR.perform_refinement( moris::hmr::RefinementMode::SIMPLE );
            tHMR.update_refinement_pattern();
        }

        tHMR.finalize();

        // evaluate node values
        tField->evaluate_scalar_function( LevelSetFunction );

        tHMR.save_to_exodus( "Sphere11.exo" );

         //tHMR.save_bsplines_to_vtk("BSplines.vtk");

//         moris::map< moris::moris_id, moris::moris_index > tMap;
//         tMesh->get_adof_map( tOrder, tMap );
//         //tMap.print("Adof Map");
//
//         //-------------------------------------------------------------------------------------------
//
//         // create IWG object
//         fem::IWG_L2 * tIWG = new moris::fem::IWG_L2( );
//
//         map< moris_id, moris_index >   tCoefficientsMap;
//         Cell< fem::Node_Base* >        tNodes;
//         Cell< MSI::Equation_Object* >  tElements;
//
//         // get map from mesh
//         tMesh->get_adof_map( tOrder, tCoefficientsMap );
//
//         // ask mesh about number of nodes on proc
//         luint tNumberOfNodes = tMesh->get_num_nodes();
//
//         // create node objects
//         tNodes.resize( tNumberOfNodes, nullptr );
//
//         for( luint k = 0; k < tNumberOfNodes; ++k )
//         {
//             tNodes( k ) = new fem::Node( &tMesh->get_mtk_vertex( k ) );
//         }
//
//         // ask mesh about number of elements on proc
//         luint tNumberOfElements = tMesh->get_num_elems();
//
//         // create equation objects
//         tElements.resize( tNumberOfElements, nullptr );
//
//         for( luint k=0; k<tNumberOfElements; ++k )
//         {
//             // create the element
//             tElements( k ) = new fem::Element( & tMesh->get_mtk_cell( k ),
//                                                tIWG,
//                                                tNodes );
//         }
//
//         MSI::Model_Solver_Interface * tMSI = new moris::MSI::Model_Solver_Interface( tElements,
//                                                                                      tMesh->get_communication_table(),
//                                                                                      tCoefficientsMap,
//                                                                                      tMesh->get_num_coeffs( tOrder ),
//                                                                                      tMesh.get() );
//
//         tMSI->set_param("L2")= (sint)tOrder;
//
//         tMSI->finalize( true );
//
//         moris::Solver_Interface * tSolverInterface = new moris::MSI::MSI_Solver_Interface( tMSI );
//
////---------------------------------------------------------------------------------------------------------------
//
//         Matrix< DDUMat > tAdofMap = tMSI->get_dof_manager()->get_adof_ind_map();
//
//         NLA::Nonlinear_Problem * tNonlinerarProblem =  new NLA::Nonlinear_Problem( tSolverInterface, true, MapType::Petsc );
//
//         // create factory for nonlinear solver
//         NLA::Nonlinear_Solver_Factory tNonlinFactory;
//
//         // create nonlinear solver
//         std::shared_ptr< NLA::Nonlinear_Solver > tNonlinearSolver = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
//
//         // create factory for linear solver
//         dla::Solver_Factory  tSolFactory;
//
//         // create linear solver
//         std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolver = tSolFactory.create_solver( SolverType::PETSC );
//
//         tLinearSolver->set_param("KSPType") = std::string(KSPFGMRES);
//         tLinearSolver->set_param("PCType")  = std::string(PCMG);
//
//         tLinearSolver->set_param("ILUFill")  = 0;
//
//         // create solver manager
//         dla::Linear_Solver * mLinSolver = new dla::Linear_Solver();
//
//         // set manager and settings
//         tNonlinearSolver->set_linear_solver( mLinSolver );
//
//         // set first solver
//         mLinSolver->set_linear_algorithm( 0, tLinearSolver );
//
//         for( auto tElement : tElements )
//         {
//             Matrix< DDRMat > & tNodalWeakBCs = tElement->get_weak_bcs();
//             uint tNumberOfNodes = tElement->get_num_nodes();
//             tNodalWeakBCs.set_size( tNumberOfNodes, 1 );
//
//             for( uint k=0; k<tNumberOfNodes; ++k )
//             {
//                 // copy weakbc into element
//                 tNodalWeakBCs( k ) = tMesh->get_value_of_scalar_field( 3,
//                                                                        EntityRank::NODE,
//                                                                        tElement->get_node_index( k ) );
//             }
//         }
//
//         tNonlinearSolver->solver_nonlinear_system( tNonlinerarProblem );
//
//         moris::Matrix< DDSMat > tExternalIndices( 9, 1 );
//         tExternalIndices( 0, 0 ) = 17;
//         tExternalIndices( 1, 0 ) = 18;
//         tExternalIndices( 2, 0 ) = 21;
//         tExternalIndices( 3, 0 ) = 22;
//         tExternalIndices( 4, 0 ) = 23;
//         tExternalIndices( 5, 0 ) = 25;
//         tExternalIndices( 6, 0 ) = 27;
//         tExternalIndices( 7, 0 ) = 26;
//         tExternalIndices( 8, 0 ) = 28;
//
//         moris::Matrix< DDSMat > tInternalIndices;
//
//         tMSI->read_multigrid_maps( 2, tExternalIndices, 0, tInternalIndices );
//
//         // get index of mesh
//         uint tMeshIndex = tHMR.get_mesh_index( tOrder,
//                                                tHMR.get_parameters()->get_lagrange_output_pattern() );
//
//         // dump mesh
//         tHMR.save_to_exodus ( tMeshIndex,  // index in database
//                               "Meshsphere_11.exo",  // path
//                               0.0 );       // timestep
//
//         delete tMSI;
//         delete tIWG;
//         delete tSolverInterface;
//
//         for( luint k=0; k<tNumberOfElements; ++k )
//         {
//             // create the element
//             delete tElements( k );
//         }
//
//         for( luint k = 0; k < tNumberOfNodes; ++k )
//         {
//             delete tNodes( k );
//         }
    }
}
/*

TEST_CASE("DLA_Multigrid_SDF","[DLA],[DLA_multigrid_sdf]")
{
    if( moris::par_size() == 1 )
    {
        // order for this example
        moris::uint tOrder = 1;

        // create parameter object
        moris::hmr::Parameters tParameters;
        tParameters.set_number_of_elements_per_dimension( { { 16 }, { 16 }, { 16 } } );
        tParameters.set_verbose( false );
        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );
        tParameters.set_mesh_orders_simple( tOrder );
        tParameters.set_refinement_buffer( 3 );

        std::string tObjectPath = "/projects/HMR/tutorials/bracket.obj";
        // get path for STL file to load
        tObjectPath = std::getenv("MORISROOT") + tObjectPath;

        // create SDF generator
        sdf::SDF_Generator tSdfGen( tObjectPath );

        // create HMR object
        moris::hmr::HMR tHMR( tParameters );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tOrder );

        for( uint k=0; k<2; ++k )
        {
            // matrices with surface element IDs
            Matrix< IndexMat > tSurfaceElements;
            tSdfGen.raycast( tMesh, tSurfaceElements );

            // get number of surface elements
            uint tNumberOfSurfaceElements = tSurfaceElements.length();

            // loop over all elements
            for( uint e=0; e<tNumberOfSurfaceElements; ++e )
            {
                // manually flag element
                tHMR.flag_element( tSurfaceElements( e ) );
            }

            tHMR.perform_refinement( moris::hmr::RefinementMode::SIMPLE  );

            tHMR.update_refinement_pattern();
        }

        tHMR.finalize();

        // calculate SDF
        auto tField = tMesh->create_field( "SDF", 1);

        tSdfGen.calculate_sdf( tMesh, tField->get_node_values() );

        tHMR.save_to_exodus( "SDF.exo" );

         //tHMR.save_bsplines_to_vtk("BSplines.vtk");

         moris::map< moris::moris_id, moris::moris_index > tMap;
         tMesh->get_adof_map( tOrder, tMap );
         //tMap.print("Adof Map");

         //-------------------------------------------------------------------------------------------

         // create IWG object
         fem::IWG_L2 * tIWG = new moris::fem::IWG_L2( );

         map< moris_id, moris_index >   tCoefficientsMap;
         Cell< fem::Node_Base* >        tNodes;
         Cell< MSI::Equation_Object* >  tElements;

         // get map from mesh
         tMesh->get_adof_map( tOrder, tCoefficientsMap );

         // ask mesh about number of nodes on proc
         luint tNumberOfNodes = tMesh->get_num_nodes();

         // create node objects
         tNodes.resize( tNumberOfNodes, nullptr );

         for( luint k = 0; k < tNumberOfNodes; ++k )
         {
             tNodes( k ) = new fem::Node( &tMesh->get_mtk_vertex( k ) );
         }

         // ask mesh about number of elements on proc
         luint tNumberOfElements = tMesh->get_num_elems();

         // create equation objects
         tElements.resize( tNumberOfElements, nullptr );

         for( luint k=0; k<tNumberOfElements; ++k )
         {
             // create the element
             tElements( k ) = new fem::Element( & tMesh->get_mtk_cell( k ),
                                                tIWG,
                                                tNodes );
         }

         MSI::Model_Solver_Interface * tMSI = new moris::MSI::Model_Solver_Interface( tElements,
                                                                                      tMesh->get_communication_table(),
                                                                                      tCoefficientsMap,
                                                                                      tMesh->get_num_coeffs( tOrder ),
                                                                                      tMesh.get() );

         tMSI->set_param("L2")= (sint)tOrder;

         tMSI->finalize( true );

         moris::Solver_Interface * tSolverInterface = new moris::MSI::MSI_Solver_Interface( tMSI );

//---------------------------------------------------------------------------------------------------------------

         Matrix< DDUMat > tAdofMap = tMSI->get_dof_manager()->get_adof_ind_map();

         NLA::Nonlinear_Problem * tNonlinerarProblem =  new NLA::Nonlinear_Problem( tSolverInterface, true, MapType::Petsc );

         // create factory for nonlinear solver
         NLA::Nonlinear_Solver_Factory tNonlinFactory;

         // create nonlinear solver
         std::shared_ptr< NLA::Nonlinear_Solver > tNonlinearSolver = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

         // create factory for linear solver
         dla::Solver_Factory  tSolFactory;

         // create linear solver
         std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolver = tSolFactory.create_solver( SolverType::PETSC );

         tLinearSolver->set_param("KSPType") = std::string(KSPFGMRES);
         tLinearSolver->set_param("PCType")  = std::string(PCMG);

         tLinearSolver->set_param("ILUFill")  = 3;

         // create solver manager
         dla::Linear_Solver * mLinSolver = new dla::Linear_Solver();

         // set manager and settings
         tNonlinearSolver->set_linear_solver( mLinSolver );

         // set first solver
         mLinSolver->set_linear_algorithm( 0, tLinearSolver );

         for( auto tElement : tElements )
         {
             Matrix< DDRMat > & tNodalWeakBCs = tElement->get_weak_bcs();
             uint tNumberOfNodes = tElement->get_num_nodes();
             tNodalWeakBCs.set_size( tNumberOfNodes, 1 );

             for( uint k=0; k<tNumberOfNodes; ++k )
             {
                 // copy weakbc into element
                 tNodalWeakBCs( k ) = tMesh->get_value_of_scalar_field( 3,
                                                                        EntityRank::NODE,
                                                                        tElement->get_node_index( k ) );
             }
         }

         tNonlinearSolver->solver_nonlinear_system( tNonlinerarProblem );

         moris::Matrix< DDSMat > tExternalIndices( 9, 1 );
         tExternalIndices( 0, 0 ) = 17;
         tExternalIndices( 1, 0 ) = 18;
         tExternalIndices( 2, 0 ) = 21;
         tExternalIndices( 3, 0 ) = 22;
         tExternalIndices( 4, 0 ) = 23;
         tExternalIndices( 5, 0 ) = 25;
         tExternalIndices( 6, 0 ) = 27;
         tExternalIndices( 7, 0 ) = 26;
         tExternalIndices( 8, 0 ) = 28;

         moris::Matrix< DDSMat > tInternalIndices;

         tMSI->read_multigrid_maps( 2, tExternalIndices, 0, tInternalIndices );

         // get index of mesh
         uint tMeshIndex = tHMR.get_mesh_index( tOrder,
                                                tHMR.get_parameters()->get_lagrange_output_pattern() );

         // dump mesh
         tHMR.save_to_exodus ( tMeshIndex,  // index in database
                               "Mesh.exo",  // path
                               0.0 );       // timestep

         delete tMSI;
         delete tIWG;
         delete tSolverInterface;

         for( luint k=0; k<tNumberOfElements; ++k )
         {
             // create the element
             delete tElements( k );
         }

         for( luint k = 0; k < tNumberOfNodes; ++k )
         {
             delete tNodes( k );
         }
    }

}
*/
}


