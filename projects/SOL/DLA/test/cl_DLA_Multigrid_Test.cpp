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
#include "cl_HMR_Mesh_Interpolation.hpp"
#include "cl_HMR_Mesh_Integration.hpp"

#include "cl_FEM_Node_Base.hpp"
#include "cl_FEM_IWG_L2.hpp"
#include "cl_FEM_Set.hpp"
#include "cl_FEM_IWG_Factory.hpp"

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

moris::real
LevelSetFunction_1( const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real tPhi = std::atan2( aPoint( 0 ), aPoint( 1 ) );
    moris::real tLevelSetVaue = 0.5 + 0.1 * std::sin( 5 * tPhi ) - std::sqrt( std::pow( aPoint( 0 ), 2 ) + std::pow( aPoint( 1 ), 2 ) );

    return tLevelSetVaue;
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

        moris::uint tLagrangeMeshIndex = 0;
        moris::uint tBSplineMeshIndex = 0;

        // create parameter object
        moris::hmr::Parameters tParameters;
        tParameters.set_number_of_elements_per_dimension( { { 2 }, { 2 } } );

        tParameters.set_severity_level( 0 );
        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );

        tParameters.set_output_meshes( { {0} } );

        tParameters.set_lagrange_orders  ( { {1} });
        tParameters.set_lagrange_patterns({ {0} });

        tParameters.set_bspline_orders   ( { {1} } );
        tParameters.set_bspline_patterns ( { {0} } );

        tParameters.set_union_pattern( 2 );
        tParameters.set_working_pattern( 3 );

        tParameters.set_refinement_buffer( 1 );
        tParameters.set_staircase_buffer( 1 );

        Cell< Matrix< DDUMat > > tLagrangeToBSplineMesh( 1 );
        tLagrangeToBSplineMesh( 0 ) = { {0} };

        tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

        // create HMR object
        moris::hmr::HMR tHMR( tParameters );

        // flag first element for refinement
        tHMR.flag_element( 0 );
        tHMR.perform_refinement_based_on_working_pattern( 0 );

        tHMR.flag_element( 0 );
        tHMR.perform_refinement_based_on_working_pattern( 0 );

        tHMR.finalize();

        // grab pointer to output field
        //std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tOrder );
        std::shared_ptr< hmr::Interpolation_Mesh_HMR > tInterpolationMesh =  tHMR.create_interpolation_mesh( tLagrangeMeshIndex);
        std::shared_ptr< hmr::Integration_Mesh_HMR > tIntegrationMesh =  tHMR.create_integration_mesh( 1, 0, *tInterpolationMesh );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tInterpolationMesh->create_field( "Circle", tLagrangeMeshIndex );

        // evaluate node values
        tField->evaluate_scalar_function( LevelSetFunction );

//        tHMR.save_bsplines_to_vtk("DLA_BSplines.vtk");

        moris::map< moris::moris_id, moris::moris_index > tMap;
        tInterpolationMesh->get_adof_map( tBSplineMeshIndex, tMap );
        //tMap.print("Adof Map");

         //-------------------------------------------------------------------------------------------

        // create IWG object
        // a factory to create the IWGs
        fem::IWG_Factory tIWGFactory;

        Cell< fem::IWG* > tIWGs( 1, nullptr );
        tIWGs( 0 ) = tIWGFactory.create_IWGs( fem::IWG_Type::L2 );

        // set residual dof type
        tIWGs( 0 )->set_residual_dof_type( { MSI::Dof_Type::L2 } );

        // set active dof types
        tIWGs( 0 )->set_dof_type_list( {{ MSI::Dof_Type::L2 }} );

        // create user defined info for properties
        fem::Property_User_Defined_Info tPropertyUserDefinedInfo;

        map< moris_id, moris_index >   tCoefficientsMap;
        Cell< fem::Node_Base* >        tNodes;
        Cell< MSI::Equation_Object* >  tElements;

        // get map from mesh
        tInterpolationMesh->get_adof_map( tBSplineMeshIndex, tCoefficientsMap );

        // ask mesh about number of nodes on proc
        luint tNumberOfNodes = tInterpolationMesh->get_num_nodes();

        // create node objects
        tNodes.resize( tNumberOfNodes, nullptr );

        for( luint k = 0; k < tNumberOfNodes; ++k )
        {
            tNodes( k ) = new fem::Node( &tInterpolationMesh->get_mtk_vertex( k ) );
        }

        // ask mesh about number of elements on proc
        luint tNumberOfElements = tIntegrationMesh->get_num_elems();

        // create equation objects
        tElements.reserve( tNumberOfElements );

        Cell< MSI::Equation_Set * >      tElementBlocks(1,nullptr);

        // init the fem set counter
        moris::uint tFemSetCounter = 0;

        // loop over the used mesh block-set
        for( luint Ik = 0; Ik < 1; ++Ik )
        {
            // create a list of cell clusters (this needs to stay in scope somehow)
            moris::mtk::Set * tBlockSet = tIntegrationMesh->get_block_by_index( 0 );

            // create new fem set
            tElementBlocks( tFemSetCounter ) = new fem::Set( tBlockSet,
                                                             fem::Element_Type::BULK,
                                                             tIWGs,
                                                             &tPropertyUserDefinedInfo,
                                                             tNodes );

            // collect equation objects associated with the block-set
            tElements.append( tElementBlocks( tFemSetCounter )->get_equation_object_list() );

            // update fem set counter
            tFemSetCounter++;
        }


//        // ask mesh about number of elements on proc
//        moris::Cell<std::string> tBlockSetsNames = tMesh->get_set_names( EntityRank::ELEMENT);
//
//        moris::Cell<mtk::Cell const *> tBlockSetElement( tMesh->get_set_entity_loc_inds( EntityRank::ELEMENT, tBlockSetsNames( 0 ) ).numel(), nullptr );
//
//        for( luint Ik=0; Ik < tBlockSetsNames.size(); ++Ik )
//        {
//            Matrix< IndexMat > tBlockSetElementInd = tMesh->get_set_entity_loc_inds( EntityRank::ELEMENT, tBlockSetsNames( Ik ) );
//
//            for( luint k=0; k < tBlockSetElementInd.numel(); ++k )
//            {
//                tBlockSetElement( k ) = & tMesh->get_mtk_cell( k );
//            }
//
//        }
//        tElementBlocks( 0 ) = new fem::Set( tBlockSetElement, fem::Element_Type::BULK, tIWGs, tNodes );
//
//        tElements.append( tElementBlocks( 0 )->get_equation_object_list() );

        MSI::Model_Solver_Interface * tMSI = new moris::MSI::Model_Solver_Interface( tElementBlocks,
                                                                                     tInterpolationMesh->get_communication_table(),
                                                                                     tCoefficientsMap,
                                                                                     tInterpolationMesh->get_num_coeffs( tBSplineMeshIndex ),
                                                                                     tInterpolationMesh.get() );

        tMSI->set_param("L2")= (sint)tBSplineMeshIndex;

        tElementBlocks( 0 )->finalize( tMSI );

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
        tLinearSolver->set_param("PCType")  = std::string( PCMG );
        //tLinearSolver->set_param("PCType")  = std::string( PCILU );

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
                 tNodalWeakBCs( k ) = tInterpolationMesh->get_value_of_scalar_field( 3,
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

         // dump mesh
//         tHMR.save_to_exodus ( 0,  // index in database
//                               "Mesh.exo",  // path
//                               0.0 );       // timestep

         delete ( tMSI );
         delete ( tIWGs( 0 ) );
         delete ( tSolverInterface );
         delete ( tNonlinearProblem );
         delete ( mLinSolver );

         for( auto k :tNodes)
         {
             delete k ;
         }
         tNodes.clear();

         for( auto k :tElementBlocks)
         {
             delete k ;
         }
         tElementBlocks.clear();
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

        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );
        tParameters.set_refinement_buffer( 3 );

        // create HMR object
        moris::hmr::HMR tHMR( tParameters );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tOrder );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tOrder );

        for( uint k=0; k<2; ++k )
        {
            tField->evaluate_scalar_function( LevelSetFunction );
            tHMR.flag_surface_elements_on_working_pattern( tField );
            tHMR.perform_refinement_based_on_working_pattern(0 );
        }

        tHMR.finalize();

        // evaluate node values
        tField->evaluate_scalar_function( LevelSetFunction );

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
        moris::uint tLagrangeMeshIndex = 0;
        moris::uint tBSplineMeshIndex = 0;

        // create parameter object
        moris::hmr::Parameters tParameters;
        tParameters.set_number_of_elements_per_dimension( { { 4 }, { 4 }, { 4 } } );

        tParameters.set_severity_level( 0 );
        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );

        tParameters.set_output_meshes( { {0} } );

        tParameters.set_lagrange_orders  ( { {1} });
        tParameters.set_lagrange_patterns({ {0} });

        tParameters.set_bspline_orders   ( { {1} } );
        tParameters.set_bspline_patterns ( { {0} } );

        tParameters.set_union_pattern( 2 );
        tParameters.set_working_pattern( 3 );

        tParameters.set_refinement_buffer( 1 );
        tParameters.set_staircase_buffer( 1 );

        Cell< Matrix< DDUMat > > tLagrangeToBSplineMesh( 1 );
        tLagrangeToBSplineMesh( 0 ) = { {0} };

        tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

        // create HMR object
        moris::hmr::HMR tHMR( tParameters );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tLagrangeMeshIndex );

        for( uint k=0; k<3; ++k )
        {
            tField->evaluate_scalar_function( LevelSetFunction );
            tHMR.flag_surface_elements_on_working_pattern( tField );
            tHMR.perform_refinement_based_on_working_pattern( 0 );
        }

        tHMR.finalize();

        // evaluate node values
        tField->evaluate_scalar_function( LevelSetFunction );

//        tHMR.save_to_exodus( 0,"Sphere11.exo" );

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

        tParameters.set_multigrid( true );
        tParameters.set_bspline_truncation( true );
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

            tHMR.perform_refinement_based_on_working_pattern( 0  );
        }

        tHMR.finalize();

        // calculate SDF
        auto tField = tMesh->create_field( "SDF", 1);

        tSdfGen.calculate_sdf( tMesh, tField->get_node_values() );

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


