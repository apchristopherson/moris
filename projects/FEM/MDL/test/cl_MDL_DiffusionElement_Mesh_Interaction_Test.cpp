
#include "catch.hpp"

#include "cl_MTK_Vertex.hpp"    //MTK
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh.hpp"

#include "cl_Mesh_Factory.hpp"
#include "cl_MTK_Mesh_Tools.hpp"
#include "cl_MTK_Mesh_Data_Input.hpp"
#include "cl_MTK_Scalar_Field_Info.hpp"
#include "cl_MTK_Mesh.hpp"

#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"
#include "cl_MTK_Integration_Mesh.hpp"

#include "cl_Matrix.hpp"        //LINALG
#include "linalg_typedefs.hpp"
#include "fn_equal_to.hpp" // ALG/src

#include "cl_FEM_NodeProxy.hpp"                //FEM/INT/src
#include "cl_FEM_ElementProxy.hpp"             //FEM/INT/src
#include "cl_FEM_Node_Base.hpp"                //FEM/INT/src
#include "cl_FEM_Element_Factory.hpp"          //FEM/INT/src
#include "cl_FEM_IWG_Factory.hpp"              //FEM/INT/src
#include "cl_FEM_Property_User_Defined_Info.hpp"              //FEM/INT/src
#include "cl_FEM_IWG_User_Defined_Info.hpp"              //FEM/INT/src
#include "cl_FEM_Constitutive_User_Defined_Info.hpp"      //FEM/INT/src

#include "cl_MDL_Model.hpp"

#include "cl_HMR.hpp"
#include "cl_HMR_Background_Mesh.hpp" //HMR/src
#include "cl_HMR_BSpline_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Element.hpp" //HMR/src
#include "cl_HMR_Factory.hpp" //HMR/src
#include "cl_HMR_Field.hpp"
#include "cl_HMR_Lagrange_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Parameters.hpp" //HMR/src

#include "cl_DLA_Solver_Factory.hpp"
#include "cl_DLA_Solver_Interface.hpp"

#include "cl_NLA_Nonlinear_Solver_Factory.hpp"
#include "cl_NLA_Nonlinear_Solver.hpp"
#include "cl_NLA_Nonlinear_Problem.hpp"
#include "cl_MSI_Solver_Interface.hpp"
#include "cl_MSI_Equation_Object.hpp"
#include "cl_MSI_Model_Solver_Interface.hpp"
#include "cl_DLA_Linear_Solver_Aztec.hpp"
#include "cl_DLA_Linear_Solver.hpp"

#include "cl_TSA_Time_Solver_Factory.hpp"
#include "cl_TSA_Monolithic_Time_Solver.hpp"
#include "cl_TSA_Time_Solver.hpp"

#include "fn_norm.hpp"

moris::real LevelSetFunction( const moris::Matrix< moris::DDRMat > & aPoint )
{
    return norm( aPoint ) - 0.5;
}

namespace moris
{
namespace mdl
{

Matrix< DDRMat > tConstValFunction_MDLDIFF( moris::Cell< Matrix< DDRMat > >         & aParameters,
                                            moris::Cell< fem::Field_Interpolator* > & aDofFI,
                                            moris::Cell< fem::Field_Interpolator* > & aDvFI,
                                            fem::Geometry_Interpolator              * aGeometryInterpolator )
{
    return aParameters( 0 );
}

TEST_CASE( "Diffusion_2x2x2", "[moris],[mdl],[Diffusion_2x2x2]" )
                {
    if(par_size() == 1 )
    {
        // Create a 3D mesh of HEX8 using MTK ------------------------------------------
        std::cout<<" Create a 3D mesh of HEX8 using MTK "<<std::endl;
        //------------------------------------------------------------------------------

        std::string tPrefix = std::getenv("MORISROOT");
        std::string tMeshFileName = tPrefix + "projects/FEM/INT/test/data/Cube_with_side_sets.g";
        std::cout<<"Mesh input name = "<<tMeshFileName<<std::endl;

        moris::mtk::Scalar_Field_Info<DDRMat> tNodeField1;
        std::string tFieldName1 = "Temp_Field";
        tNodeField1.set_field_name( tFieldName1 );
        tNodeField1.set_field_entity_rank( EntityRank::NODE );

        // Initialize field information container
        moris::mtk::MtkFieldsInfo tFieldsInfo;

        // Place the node field into the field info container
        add_field_for_mesh_input(&tNodeField1,tFieldsInfo);

        // Declare some supplementary fields
        mtk::MtkMeshData tMeshData;
        tMeshData.FieldsInfo = &tFieldsInfo;


        // construct the mesh data
        mtk::Interpolation_Mesh* tInterpMesh1 = mtk::create_interpolation_mesh( MeshType::STK, tMeshFileName, &tMeshData );
        mtk::Integration_Mesh*   tIntegMesh1  = mtk::create_integration_mesh_from_interpolation_mesh( MeshType::STK, tInterpMesh1 );

        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair(tInterpMesh1,tIntegMesh1);

        // create IWG user defined info
        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 3 );
        tIWGUserDefinedInfo( 0 ).resize( 1 );
        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_LOAD },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 1 ).resize( 1 );
        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_DIRICHLET },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 2 ).resize( 1 );
        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_NEUMANN },
                                                                    moris::Cell< fem::Constitutive_Type >( 0 ) );

        // create the property user defined infos
        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
                                                       Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                       {{{ 1.0 }}},
                                                       tConstValFunction_MDLDIFF,
                                                       Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
                                                        Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                        {{{ 5.0 }}},
                                                        tConstValFunction_MDLDIFF,
                                                        Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempNeumann( fem::Property_Type::TEMP_NEUMANN,
                                                      Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                      {{{ 20.0 }}},
                                                      tConstValFunction_MDLDIFF,
                                                      Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempLoad( fem::Property_Type::TEMP_LOAD,
                                                   Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                   {{{ 0.0 }}},
                                                   tConstValFunction_MDLDIFF,
                                                   Cell< fem::PropertyFunc >( 0 ) );

        // create property user defined info
        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 3 );
        tPropertyUserDefinedInfo( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tTempLoad;
        tPropertyUserDefinedInfo( 1 ).resize( 1 );
        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tTempDirichlet;
        tPropertyUserDefinedInfo( 2 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tTempNeumann;

        // create constitutive user defined info
        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
                                                         {{ MSI::Dof_Type::TEMP }},
                                                         { fem::Property_Type::CONDUCTIVITY } );
        // create constitutive user defined info
        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 3 );
        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );

        // create a list of active sets
        moris::Cell< moris_index >  tSetList = { 0, 3, 5 };

        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
                                                          fem::Element_Type::SIDESET,
                                                          fem::Element_Type::SIDESET };

        // create a model
        mdl::Model * tModel = new mdl::Model( &tMeshManager, 1,
                                              tSetList, tSetTypeList,
                                              tIWGUserDefinedInfo,
                                              tPropertyUserDefinedInfo,
                                              tConstitutiveUserDefinedInfo );

        //------------------------------------------------------------------------------

        moris::Cell< enum MSI::Dof_Type > tDofTypes1( 1, MSI::Dof_Type::TEMP );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        dla::Solver_Factory  tSolFactory;
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );

        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;

        dla::Linear_Solver tLinSolver;

        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 10;
        tNonlinearSolverAlgorithm->set_param("NLA_hard_break") = false;
        tNonlinearSolverAlgorithm->set_param("NLA_max_lin_solver_restarts") = 2;
        tNonlinearSolverAlgorithm->set_param("NLA_rebuild_jacobian") = true;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolver;

        tNonlinearSolver.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: create time Solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        tsa::Time_Solver_Factory tTimeSolverFactory;
        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );

        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolver );

        tsa::Time_Solver tTimeSolver;

        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );

        NLA::SOL_Warehouse tSolverWarehouse;

        tSolverWarehouse.set_solver_interface(tModel->get_solver_interface());

        tNonlinearSolver.set_solver_warehouse( &tSolverWarehouse );
        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );

        tNonlinearSolver.set_dof_type_list( tDofTypes1 );
        tTimeSolver.set_dof_type_list( tDofTypes1 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Solve and check
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        tTimeSolver.solve();

        moris::Matrix< DDRMat > tSolution11;
        tTimeSolver.get_full_solution( tSolution11 );

        // Expected solution
        Matrix< DDRMat > tExpectedSolution = {{ 25.0, 25.0, 25.0,
                                                25.0,  5.0, 25.0,
                                                45.0, 25.0,  5.0,
                                                25.0, 45.0, 25.0,
                                                5.0, 25.0, 45.0,
                                                5.0, 45.0,  5.0,
                                                45.0,  5.0, 45.0,
                                                5.0, 45.0,  5.0,
                                                45.0,  5.0, 45.0 }};

        // define an epsilon environment
        real tEpsilon = 1E-6;

        // define a bool for solution check
        bool tCheckNodalSolution = true;

        // number of mesh nodes
        uint tNumOfNodes = tInterpMesh1->get_num_nodes();

        // loop over the node and chyeck solution
        for ( uint i = 0; i < tNumOfNodes; i++ )
        {
            // check solution
            tCheckNodalSolution = tCheckNodalSolution
                    && ( std::abs( tSolution11( i ) - tExpectedSolution( i ) ) < tEpsilon );
        }
        // check bool is true
        REQUIRE( tCheckNodalSolution );

        tModel->output_solution( tFieldName1 );

        delete tModel;
        delete tInterpMesh1;
        delete tIntegMesh1;

    }/* if( par_size() */
                }

TEST_CASE( "Element_Diffusion_3", "[moris],[mdl],[Diffusion_block_7x8x9]" )
{
    if(par_size() == 1 )
    {
        // Create a 3D mesh of HEX8 using MTK ------------------------------------------
        std::cout<<" Create a 3D mesh of HEX8 using MTK "<<std::endl;
        //------------------------------------------------------------------------------

        std::string tPrefix = std::getenv("MORISROOT");
        std::string tMeshFileName = tPrefix + "projects/FEM/MDL/test/data/Block_7x8x9.g";

        std::cout<<"Mesh input name = "<< tMeshFileName<<std::endl;

        moris::mtk::Scalar_Field_Info<DDRMat> tNodeField1;
        std::string tFieldName1 = "Temp_Field";
        tNodeField1.set_field_name(tFieldName1);
        tNodeField1.set_field_entity_rank(EntityRank::NODE);

        // Initialize field information container
        moris::mtk::MtkFieldsInfo tFieldsInfo;

        // Place the node field into the field info container
        add_field_for_mesh_input(&tNodeField1,tFieldsInfo);

        // Declare some supplementary fields
        mtk::MtkMeshData tMeshData;
        tMeshData.FieldsInfo = &tFieldsInfo;

        // construct the mesh data
        mtk::Interpolation_Mesh* tInterpMesh1 = mtk::create_interpolation_mesh( MeshType::STK, tMeshFileName, &tMeshData );
        mtk::Integration_Mesh*   tIntegMesh1  = mtk::create_integration_mesh_from_interpolation_mesh(MeshType::STK,tInterpMesh1);

        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair(tInterpMesh1,tIntegMesh1);

        //1) Create the fem nodes ------------------------------------------------------
        std::cout<<" Create the fem nodes "<<std::endl;
        //------------------------------------------------------------------------------

        // create IWG user defined info
        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 3 );
        tIWGUserDefinedInfo( 0 ).resize( 1 );
        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_LOAD },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 1 ).resize( 1 );
        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET, { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_DIRICHLET },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 2 ).resize( 1 );
        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN, { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_NEUMANN },
                                                                    moris::Cell< fem::Constitutive_Type >( 0 ) );

        // create the property user defined infos
        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
                                                       Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                       {{{ 1.0 }}},
                                                       tConstValFunction_MDLDIFF,
                                                       Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
                                                        Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                        {{{ 5.0 }}},
                                                        tConstValFunction_MDLDIFF,
                                                        Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempNeumann( fem::Property_Type::TEMP_NEUMANN,
                                                      Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                      {{{ 20.0 }}},
                                                      tConstValFunction_MDLDIFF,
                                                      Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempLoad( fem::Property_Type::TEMP_LOAD,
                                                   Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                   {{{ 0.0 }}},
                                                   tConstValFunction_MDLDIFF,
                                                   Cell< fem::PropertyFunc >( 0 ) );

        // create property user defined info
        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 3 );
        tPropertyUserDefinedInfo( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tTempLoad;
        tPropertyUserDefinedInfo( 1 ).resize( 1 );
        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tTempDirichlet;
        tPropertyUserDefinedInfo( 2 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tTempNeumann;

        // create constitutive user defined info
        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
                                                         {{ MSI::Dof_Type::TEMP }},
                                                         { fem::Property_Type::CONDUCTIVITY } );
        // create constitutive user defined info
        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 3 );
        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );

        // create a list of active sets
        moris::Cell< moris_index >  tSetList = { 0, 3, 5 };

        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
                                                          fem::Element_Type::SIDESET,
                                                          fem::Element_Type::SIDESET };

        // create a model
        mdl::Model * tModel = new mdl::Model( &tMeshManager, 1,
                                              tSetList, tSetTypeList,
                                              tIWGUserDefinedInfo,
                                              tPropertyUserDefinedInfo,
                                              tConstitutiveUserDefinedInfo );

        moris::Cell< enum MSI::Dof_Type > tDofTypes1( 1, MSI::Dof_Type::TEMP );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        dla::Solver_Factory  tSolFactory;
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );

        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;

        dla::Linear_Solver tLinSolver;

        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 10;
        tNonlinearSolverAlgorithm->set_param("NLA_hard_break") = false;
        tNonlinearSolverAlgorithm->set_param("NLA_max_lin_solver_restarts") = 2;
        tNonlinearSolverAlgorithm->set_param("NLA_rebuild_jacobian") = true;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolver;

        tNonlinearSolver.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: create time Solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        tsa::Time_Solver_Factory tTimeSolverFactory;
        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );

        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolver );

        tsa::Time_Solver tTimeSolver;

        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );

        NLA::SOL_Warehouse tSolverWarehouse;

        tSolverWarehouse.set_solver_interface(tModel->get_solver_interface());

        tNonlinearSolver.set_solver_warehouse( &tSolverWarehouse );
        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );

        tNonlinearSolver.set_dof_type_list( tDofTypes1 );
        tTimeSolver.set_dof_type_list( tDofTypes1 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Solve and check
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        tTimeSolver.solve();

        moris::Matrix< DDRMat > tSolution11;
        tTimeSolver.get_full_solution( tSolution11 );

        tModel->output_solution( tFieldName1 );

        // print( tSolution11, "Solution" );

        // Expected solution
        Matrix< DDRMat > tExpectedSolution( 1, 25, 2.5e+01 );

        // define an epsilon environment
        real tEpsilon = 1E-6;

        // define a bool for solution check
        bool tCheckNodalSolution = true;

        // loop over the node and check solution
        for ( uint i = 0; i < 25; i++ )
        {
            // check solution
            tCheckNodalSolution = tCheckNodalSolution
                    && ( std::abs( tSolution11( i ) - tExpectedSolution( i ) ) < tEpsilon );
        }
        // check bool is true
        REQUIRE( tCheckNodalSolution );


        delete tIntegMesh1;
        delete tInterpMesh1;
        delete tModel;

    }/* if( par_size() */
}

//-------------------------------------------------------------------------------------------------------
TEST_CASE( "Diffusion_hmr_10x4x4", "[moris],[mdl],[Diffusion_hmr_10x4x4]" )
{
    if( par_size() == 2 )
    {
        gLogger.set_severity_level( 0 );
        // Create a 3D mesh of HEX8 using MTK ------------------------------------------
        std::cout<<" Create a 3D mesh of HEX8 using MTK "<<std::endl;
        //------------------------------------------------------------------------------

        moris::uint tLagrangeMeshIndex = 0;
        moris::uint tBSplineMeshIndex = 0;

        moris::hmr::Parameters tParameters;

        tParameters.set_number_of_elements_per_dimension( { {10}, {4}, {4} } );
        tParameters.set_domain_dimensions({ {10}, {4}, {4} });
        tParameters.set_domain_offset({ {-10.0}, {-2.0}, {-2.0} });
        tParameters.set_bspline_truncation( true );
        tParameters.set_side_sets({ {1}, {6}, {3}, {4}, {5}, {2} });

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

        hmr::HMR tHMR( tParameters );

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

        //1) Create the fem nodes ------------------------------------------------------
        std::cout<<" Create the fem nodes "<<std::endl;
        //------------------------------------------------------------------------------
        // create IWG user defined info
        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 3 );
        tIWGUserDefinedInfo( 0 ).resize( 1 );
        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_LOAD },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 1 ).resize( 1 );
        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_DIRICHLET },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 2 ).resize( 1 );
        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_NEUMANN },
                                                                    moris::Cell< fem::Constitutive_Type >( 0 ) );

        // create the property user defined infos
        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
                                                       Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                       {{{ 1.0 }}},
                                                       tConstValFunction_MDLDIFF,
                                                       Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
                                                        Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                        {{{ 5.0 }}},
                                                        tConstValFunction_MDLDIFF,
                                                        Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempNeumann( fem::Property_Type::TEMP_NEUMANN,
                                                      Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                      {{{ 20.0 }}},
                                                      tConstValFunction_MDLDIFF,
                                                      Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempLoad( fem::Property_Type::TEMP_LOAD,
                                                   Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                   {{{ 0.0 }}},
                                                   tConstValFunction_MDLDIFF,
                                                   Cell< fem::PropertyFunc >( 0 ) );

        // create property user defined info
        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 3 );
        tPropertyUserDefinedInfo( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tTempLoad;
        tPropertyUserDefinedInfo( 1 ).resize( 1 );
        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tTempDirichlet;
        tPropertyUserDefinedInfo( 2 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tTempNeumann;

        // create constitutive user defined info
        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
                                                         {{ MSI::Dof_Type::TEMP }},
                                                         { fem::Property_Type::CONDUCTIVITY } );
        // create constitutive user defined info
        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 3 );
        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );

        // create a list of active sets
        moris::Cell< moris_index >  tSetList = { 0, 3, 5 };

        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
                                                          fem::Element_Type::SIDESET,
                                                          fem::Element_Type::SIDESET };

        // construct a mesh manager for the fem
        std::shared_ptr< moris::hmr::Interpolation_Mesh_HMR > tInterpolationMesh = tHMR.create_interpolation_mesh(tLagrangeMeshIndex);
        std::shared_ptr< moris::hmr::Integration_Mesh_HMR >   tIntegrationMesh   = tHMR.create_integration_mesh(1, 0,*tInterpolationMesh);

        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair(tInterpolationMesh.get(),tIntegrationMesh.get());

        // create model
        mdl::Model * tModel = new mdl::Model( &tMeshManager, tBSplineMeshIndex,
                                              tSetList, tSetTypeList,
                                              tIWGUserDefinedInfo,
                                              tPropertyUserDefinedInfo,
                                              tConstitutiveUserDefinedInfo );

        moris::Cell< enum MSI::Dof_Type > tDofTypes1( 1, MSI::Dof_Type::TEMP );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        dla::Solver_Factory  tSolFactory;
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );

        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;

        dla::Linear_Solver tLinSolver;

        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 10;
        tNonlinearSolverAlgorithm->set_param("NLA_hard_break") = false;
        tNonlinearSolverAlgorithm->set_param("NLA_max_lin_solver_restarts") = 2;
        tNonlinearSolverAlgorithm->set_param("NLA_rebuild_jacobian") = true;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolver;

        tNonlinearSolver.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: create time Solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        tsa::Time_Solver_Factory tTimeSolverFactory;
        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );

        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolver );

        tsa::Time_Solver tTimeSolver;

        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );

        NLA::SOL_Warehouse tSolverWarehouse;

        tSolverWarehouse.set_solver_interface(tModel->get_solver_interface());

        tNonlinearSolver.set_solver_warehouse( &tSolverWarehouse );
        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );

        tNonlinearSolver.set_dof_type_list( tDofTypes1 );
        tTimeSolver.set_dof_type_list( tDofTypes1 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Solve and check
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        tTimeSolver.solve();

        moris::Matrix< DDRMat > tSolution11;
        tTimeSolver.get_full_solution( tSolution11 );


        tModel->output_solution( "Circle" );

        tField->put_scalar_values_on_field( tModel->get_mSolHMR() );

        //            tHMR.save_to_exodus( "Circle_diff_temp.exo" );

        // Expected solution
        Matrix< DDRMat > tExpectedSolution;

        if ( par_rank() == 0 )
        {
            // Expected solution for first processor
            tExpectedSolution = {{ +4.999999999823309e+00,    +2.499999999379938e+01,    +4.499999998728015e+01,
                                   +6.499999997824342e+01,    +8.499999996238753e+01,    +1.049999999295709e+02,
                                   +4.999999999800635e+00,    +2.499999999394931e+01,    +4.499999998731124e+01,
                                   +6.499999997819508e+01,    +8.499999996482042e+01,    +1.049999999328457e+02,
                                   +4.999999999792283e+00,    +2.499999999418053e+01,    +4.499999998799023e+01,
                                   +6.499999998037731e+01,    +8.499999996742547e+01,    +1.049999999424134e+02,
                                   +4.999999999779568e+00,    +2.499999999442362e+01,    +4.499999998863433e+01,
                                   +6.499999998244976e+01,    +8.499999997341547e+01,    +1.049999999520176e+02,
                                   +4.999999999779885e+00, }};
            // print(tSolution11,"Processor_ONE");
        }
        else if ( par_rank() == 1 )
        {
            // Expected solution for second processor
            tExpectedSolution = {{ +1.049999999295709e+02,    +1.249999999264930e+02,    +1.449999999406017e+02,
                                   +1.649999999466469e+02,    +1.749999999495133e+02,    +1.749999999500786e+02,
                                   +1.749999999489033e+02,    +1.749999999492710e+02,    +1.849999999512872e+02,
                                   +1.949999999603241e+02,    +1.849999999536432e+02,    +1.949999999629034e+02,
                                   +1.849999999509966e+02,    +1.949999999600533e+02,    +1.849999999538221e+02,
                                   +1.949999999627871e+02,    +2.049999999627414e+02,    +2.049999999653978e+02,
                                   +2.049999999635851e+02,    +2.049999999665377e+02,    +1.049999999328457e+02,
                                   +1.249999999292042e+02,    +1.449999999398408e+02,    +1.649999999465366e+02,
                                   +1.749999999506973e+02, }};
        }

        // define an epsilon environment
        double tEpsilon = 1E-6;

        // define a bool for solution check
        bool tCheckNodalSolution = true;

        // loop over the node and chyeck solution
        for ( uint i = 0; i < 25; i++ )
        {
            // check solution
            tCheckNodalSolution = tCheckNodalSolution
                    && ( std::abs( tSolution11( i ) - tExpectedSolution( i ) ) < tEpsilon );
        }

        // check bool is true
        REQUIRE( tCheckNodalSolution );

        delete tModel;
    }/* if( par_size() */
}

TEST_CASE( "Diffusion_hmr3_10x4x4", "[moris],[mdl],[Diffusion_hmr3_10x4x4]" )
{
    if( (par_size() == 1) || (par_size() == 2) )
    {
        // Create a 3D mesh of HEX8 using MTK ------------------------------------------
        std::cout<<" Create a 3D mesh of HEX8 using HMR "<<std::endl;
        //------------------------------------------------------------------------------

        moris::uint tLagrangeMeshIndex = 0;
        moris::uint tBSplineMeshIndex = 0;

        moris::hmr::Parameters tParameters;

        tParameters.set_number_of_elements_per_dimension( { {4}, {2}, {2} } );
        tParameters.set_domain_dimensions({ {4}, {2}, {2} });
        tParameters.set_domain_offset({ {-2.0}, {0.0}, {0.0} });
        tParameters.set_bspline_truncation( true );
        tParameters.set_side_sets({ {1}, {6}, {3}, {4}, {5}, {2} });

        tParameters.set_output_meshes( { {0} } );

        tParameters.set_lagrange_orders  ( { {2} });
        tParameters.set_lagrange_patterns({ {0} });

        tParameters.set_bspline_orders   ( { {2} } );
        tParameters.set_bspline_patterns ( { {0} } );

        tParameters.set_union_pattern( 2 );
        tParameters.set_working_pattern( 3 );

        tParameters.set_refinement_buffer( 2 );
        tParameters.set_staircase_buffer( 1 );

        Cell< Matrix< DDUMat > > tLagrangeToBSplineMesh( 1 );
        tLagrangeToBSplineMesh( 0 ) = { {0} };

        tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

        hmr::HMR tHMR( tParameters );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tLagrangeMeshIndex );

        for( uint k=0; k<3; ++k )
        {
            tField->evaluate_scalar_function( LevelSetFunction );
            tHMR.flag_surface_elements_on_working_pattern( tField );

            //tDatabase->flag_element( 0 );
            tHMR.perform_refinement_based_on_working_pattern( 0 );
        }

        tHMR.finalize();

        //1) Create the fem nodes ------------------------------------------------------
        std::cout<<" Create the fem nodes "<<std::endl;
        //------------------------------------------------------------------------------
        // create IWG user defined info
        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 3 );
        tIWGUserDefinedInfo( 0 ).resize( 1 );
        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_LOAD },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 1 ).resize( 1 );
        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_DIRICHLET },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 2 ).resize( 1 );
        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_NEUMANN },
                                                                    moris::Cell< fem::Constitutive_Type >( 0 ) );

        // create the property user defined infos
        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
                                                       Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                       {{{ 1.0 }}},
                                                       tConstValFunction_MDLDIFF,
                                                       Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
                                                        Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                        {{{ 5.0 }}},
                                                        tConstValFunction_MDLDIFF,
                                                        Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempNeumann( fem::Property_Type::TEMP_NEUMANN,
                                                      Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                      {{{ 20.0 }}},
                                                      tConstValFunction_MDLDIFF,
                                                      Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempLoad( fem::Property_Type::TEMP_LOAD,
                                                   Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                   {{{ 0.0 }}},
                                                   tConstValFunction_MDLDIFF,
                                                   Cell< fem::PropertyFunc >( 0 ) );

        // create property user defined info
        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 3 );
        tPropertyUserDefinedInfo( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tTempLoad;
        tPropertyUserDefinedInfo( 1 ).resize( 1 );
        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tTempDirichlet;
        tPropertyUserDefinedInfo( 2 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tTempNeumann;

        // create constitutive user defined info
        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
                                                         {{ MSI::Dof_Type::TEMP }},
                                                         { fem::Property_Type::CONDUCTIVITY } );
        // create constitutive user defined info
        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 3 );
        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );

        // create a list of active sets
        moris::Cell< moris_index >  tSetList = { 0, 3, 5 };

        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
                                                          fem::Element_Type::SIDESET,
                                                          fem::Element_Type::SIDESET };

        // construct a mesh manager for the fem
        std::shared_ptr< moris::hmr::Interpolation_Mesh_HMR > tInterpolationMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex );
        std::shared_ptr< moris::hmr::Integration_Mesh_HMR >   tIntegrationMesh   = tHMR.create_integration_mesh( 2, 0, *tInterpolationMesh );


        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair( tInterpolationMesh.get(),tIntegrationMesh.get() );

        // create model
        mdl::Model * tModel = new mdl::Model( &tMeshManager, tBSplineMeshIndex,
                                              tSetList, tSetTypeList,
                                              tIWGUserDefinedInfo,
                                              tPropertyUserDefinedInfo,
                                              tConstitutiveUserDefinedInfo );

        moris::Cell< enum MSI::Dof_Type > tDofTypes1( 1, MSI::Dof_Type::TEMP );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        dla::Solver_Factory  tSolFactory;
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );

        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;

        dla::Linear_Solver tLinSolver;

        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 10;
        tNonlinearSolverAlgorithm->set_param("NLA_hard_break") = false;
        tNonlinearSolverAlgorithm->set_param("NLA_max_lin_solver_restarts") = 2;
        tNonlinearSolverAlgorithm->set_param("NLA_rebuild_jacobian") = true;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolver;

        tNonlinearSolver.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: create time Solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        tsa::Time_Solver_Factory tTimeSolverFactory;
        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );

        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolver );

        tsa::Time_Solver tTimeSolver;

        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );

        NLA::SOL_Warehouse tSolverWarehouse;

        tSolverWarehouse.set_solver_interface(tModel->get_solver_interface());

        tNonlinearSolver.set_solver_warehouse( &tSolverWarehouse );
        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );

        tNonlinearSolver.set_dof_type_list( tDofTypes1 );
        tTimeSolver.set_dof_type_list( tDofTypes1 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Solve and check
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        tTimeSolver.solve();

        moris::Matrix< DDRMat > tSolution11;
        tTimeSolver.get_full_solution( tSolution11 );
        tModel->output_solution( "Circle" );

        tField->put_scalar_values_on_field( tModel->get_mSolHMR() );

        tHMR.save_to_exodus( 0,"Circle_diff_temp.exo" );

        // Expected solution when running in serial
        Matrix< DDRMat > tExpectedSolution = {{ +1.976384396893782e-09, +9.999999997638666e+00, +2.299478928887239e-09,
                                                +9.999999997438143e+00, +4.152303135013222e-09, +9.999999996543764e+00,
                                                +2.631924777316510e-09, +9.999999997284709e+00, +1.999999999016610e+01,
                                                +2.249999998863940e+01, +2.749999998937424e+01, +2.249999999091407e+01,
                                                +2.749999998558418e+01, +2.249999998975273e+01, +2.749999998741196e+01,
                                                +2.249999999001834e+01, +2.749999998686932e+01, +3.249999997374252e+01,
                                                +3.374999998746107e+01, +3.624999997886972e+01, +3.374999998168578e+01,
                                                +3.624999998243668e+01, +3.374999998299653e+01, +3.624999998061379e+01,
                                                +3.374999998348704e+01 }};

        // expected solutions when running in parallel
        if (par_size() == 2)
        {
            if ( par_rank() == 0 )
            {
                // Expected solution for first processor
                tExpectedSolution = {{ -3.302872243818668e-08, +1.000000001738268e+01, +1.085000671093155e-08,
                                       +9.999999995258710e+00, +1.725491188274901e-08, +9.999999992226838e+00,
                                       -8.821987998234748e-09, +1.000000000506248e+01, +1.999999997518040e+01,
                                       +2.250000000695395e+01, +2.750000001516867e+01, +2.250000000553710e+01,
                                       +2.750000000184574e+01, +2.250000000522549e+01, +2.750000000242706e+01,
                                       +2.250000000469339e+01, +2.750000000978408e+01, +3.249999997587295e+01,
                                       +3.375000001858257e+01, +3.624999998752415e+01, +3.375000000630910e+01,
                                       +3.625000002645388e+01, +3.375000000782835e+01, +3.625000002339387e+01,
                                       +3.375000001143644e+01 }};
                 //print(tSolution11,"Processor_ONE");
            }
            else if ( par_rank() == 1 )
            {
                // Expected solution for second processor
                tExpectedSolution = {{ +4.249999984172224e+01, +4.750000055380576e+01, +4.250000015181325e+01,
                                       +4.749999966007174e+01, +4.250000011538052e+01, +4.749999976236016e+01,
                                       +4.249999995438112e+01, +4.750000020612755e+01, +5.249999984162645e+01,
                                       +5.375000021335701e+01, +5.624999996839500e+01, +5.374999994715878e+01,
                                       +5.625000007162272e+01, +5.374999996649412e+01, +5.625000006434173e+01,
                                       +5.375000007562546e+01, +5.625000003060088e+01, +5.750000035171650e+01,
                                       +5.250000045361771e+01, +5.749999987371483e+01, +5.250000035870602e+01,
                                       +5.749999990229202e+01, +5.249999989014849e+01, +5.750000009994142e+01,
                                       +4.249999992652671e+01 }};
            }
            else {} //do nothing
        }
        else {} // end expected solutions for parallel

        // define an epsilon environment
        real tEpsilon = 1E-6;

        // define a bool for solution check
        bool tCheckNodalSolution = true;

        // loop over the node and check solution
        for ( uint i = 0; i < 25; i++ )
        {
            // check solution
            tCheckNodalSolution = tCheckNodalSolution
                    && ( std::abs( tSolution11( i ) - tExpectedSolution( i ) ) < tEpsilon );
        }
        // check bool is true
        REQUIRE( tCheckNodalSolution );
    }/* if( par_size() */
}

//-------------------------------------------------------------------------------------------------------

TEST_CASE( "Diffusion_hmr_cubic_10x4x4", "[moris],[mdl],[Diffusion_hmr_cubic_10x4x4]" )
{
    if( par_size() == 1 )
    {
        // Create a 3D mesh of HEX8 using MTK ------------------------------------------
        std::cout<<" Create a 3D mesh of HEX8 using HMR "<<std::endl;
        //------------------------------------------------------------------------------

        moris::uint tLagrangeMeshIndex = 0;
        moris::uint tBSplineMeshIndex = 0;

        moris::hmr::Parameters tParameters;

        tParameters.set_number_of_elements_per_dimension( { {4}, {2}, {2} } );
        tParameters.set_domain_dimensions({ {4}, {2}, {2} });
        tParameters.set_domain_offset({ {-2.0}, {0.0}, {0.0} });
        tParameters.set_bspline_truncation( true );
        tParameters.set_side_sets({ {1}, {6}, {3}, {4}, {5}, {2} });

        tParameters.set_output_meshes( { {0} } );

        tParameters.set_lagrange_orders  ( { {3} });
        tParameters.set_lagrange_patterns({ {0} });

        tParameters.set_bspline_orders   ( { {3} } );
        tParameters.set_bspline_patterns ( { {0} } );

        tParameters.set_union_pattern( 2 );
        tParameters.set_working_pattern( 3 );

        tParameters.set_refinement_buffer( 1 );
        tParameters.set_staircase_buffer( 1 );

        Cell< Matrix< DDUMat > > tLagrangeToBSplineMesh( 1 );
        tLagrangeToBSplineMesh( 0 ) = { {0} };

        tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

        hmr::HMR tHMR( tParameters );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( "Circle", tLagrangeMeshIndex );

        for( uint k=0; k<2; ++k )
        {
            tField->evaluate_scalar_function( LevelSetFunction );
            tHMR.flag_surface_elements_on_working_pattern( tField );
            tHMR.perform_refinement_based_on_working_pattern( 0 );
        }

        tHMR.finalize();

        //1) Create the fem nodes ------------------------------------------------------
        std::cout<<" Create the fem nodes "<<std::endl;
        //------------------------------------------------------------------------------
        // create IWG user defined info
        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 3 );
        tIWGUserDefinedInfo( 0 ).resize( 1 );
        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_LOAD },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 1 ).resize( 1 );
        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_DIRICHLET },
                                                                    { fem::Constitutive_Type::DIFF_LIN_ISO } );
        tIWGUserDefinedInfo( 2 ).resize( 1 );
        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN,
                                                                    { MSI::Dof_Type::TEMP },
                                                                    {{ MSI::Dof_Type::TEMP }},
                                                                    { fem::Property_Type::TEMP_NEUMANN },
                                                                    moris::Cell< fem::Constitutive_Type >( 0 ) );

        // create the property user defined infos
        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
                                                       Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                       {{{ 1.0 }}},
                                                       tConstValFunction_MDLDIFF,
                                                       Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
                                                        Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                        {{{ 5.0 }}},
                                                        tConstValFunction_MDLDIFF,
                                                        Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempNeumann( fem::Property_Type::TEMP_NEUMANN,
                                                      Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                      {{{ 20.0 }}},
                                                      tConstValFunction_MDLDIFF,
                                                      Cell< fem::PropertyFunc >( 0 ) );
        fem::Property_User_Defined_Info tTempLoad( fem::Property_Type::TEMP_LOAD,
                                                   Cell< Cell< MSI::Dof_Type > >( 0 ),
                                                   {{{ 0.0 }}},
                                                   tConstValFunction_MDLDIFF,
                                                   Cell< fem::PropertyFunc >( 0 ) );

        // create property user defined info
        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 3 );
        tPropertyUserDefinedInfo( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tTempLoad;
        tPropertyUserDefinedInfo( 1 ).resize( 1 );
        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 2 );
        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tConductivity;
        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tTempDirichlet;
        tPropertyUserDefinedInfo( 2 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 1 );
        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tTempNeumann;

        // create constitutive user defined info
        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
                                                         {{ MSI::Dof_Type::TEMP }},
                                                         { fem::Property_Type::CONDUCTIVITY } );
        // create constitutive user defined info
        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 3 );
        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 1 );
        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tDiffLinIso;
        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );

        // create a list of active sets
        moris::Cell< moris_index >  tSetList = { 0, 3, 5 };

        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
                                                          fem::Element_Type::SIDESET,
                                                          fem::Element_Type::SIDESET };

        // construct a mesh manager for the fem
        std::shared_ptr< moris::hmr::Interpolation_Mesh_HMR > tInterpolationMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex );
        std::shared_ptr< moris::hmr::Integration_Mesh_HMR >   tIntegrationMesh   = tHMR.create_integration_mesh( 3, 0, *tInterpolationMesh );


        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair(tInterpolationMesh.get(),tIntegrationMesh.get());

        // create model
        mdl::Model * tModel = new mdl::Model( &tMeshManager, tBSplineMeshIndex,
                                              tSetList, tSetTypeList,
                                              tIWGUserDefinedInfo,
                                              tPropertyUserDefinedInfo,
                                              tConstitutiveUserDefinedInfo );

        moris::Cell< enum MSI::Dof_Type > tDofTypes1( 1, MSI::Dof_Type::TEMP );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        dla::Solver_Factory  tSolFactory;
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );

        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;

        dla::Linear_Solver tLinSolver;

        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algortihm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 10;
        tNonlinearSolverAlgorithm->set_param("NLA_hard_break") = false;
        tNonlinearSolverAlgorithm->set_param("NLA_max_lin_solver_restarts") = 2;
        tNonlinearSolverAlgorithm->set_param("NLA_rebuild_jacobian") = true;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolver;

        tNonlinearSolver.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: create time Solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        tsa::Time_Solver_Factory tTimeSolverFactory;
        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );

        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolver );

        tsa::Time_Solver tTimeSolver;

        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );

        NLA::SOL_Warehouse tSolverWarehouse;

        tSolverWarehouse.set_solver_interface(tModel->get_solver_interface());

        tNonlinearSolver.set_solver_warehouse( &tSolverWarehouse );
        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );

        tNonlinearSolver.set_dof_type_list( tDofTypes1 );
        tTimeSolver.set_dof_type_list( tDofTypes1 );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Solve and check
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        tTimeSolver.solve();

        moris::Matrix< DDRMat > tSolution11;
        tTimeSolver.get_full_solution( tSolution11 );

        tModel->output_solution( "Circle" );

        tField->put_scalar_values_on_field( tModel->get_mSolHMR() );

        // Expected solution when running in serial
        Matrix< DDRMat > tExpectedSolution = {{ -5.0e+00,    +5.0e+00,    -5.0e+00,
                                                +5.0e+00,    -5.0e+00,    +5.0e+00,
                                                -5.0e+00,    +5.0e+00,    +1.5e+01,
                                                +2.5e+01,    +3.5e+01,    +3.5e+01,
                                                +3.5e+01,    +3.5e+01,    +1.5e+01,
                                                +2.5e+01,    +1.5e+01,    +2.5e+01,
                                                +1.5e+01,    +2.5e+01,    +3.5e+01,
                                                +4.0e+01,    +4.5e+01,    +4.0e+01,
                                                +4.5e+01 }};

        // define an epsilon environment
        real tEpsilon = 1E-6;

        // define a bool for solution check
        bool tCheckNodalSolution = true;

        // loop over the node and check solution of the first 25 values
        for ( uint i = 0; i < 25; i++ )
        {
            // check solution
            tCheckNodalSolution = tCheckNodalSolution
                    && ( std::abs( tSolution11( i ) - tExpectedSolution( i ) ) < tEpsilon );
        }
        // check bool is true
        REQUIRE( tCheckNodalSolution );

        delete tModel;
    }/* if( par_size() */
}

}/* namespace fem */
}/* namespace moris */
