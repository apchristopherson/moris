/*
 * cl_Beam_Study.cpp
 *
 *  Created on: Jan 2, 2020
 *      Author: sonne
 */

#include "catch.hpp"
#include "cl_Star.hpp"
#include "cl_Circle.hpp"
#include "cl_Plane.hpp"

#include "cl_XTK_Model.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"
#include "cl_XTK_Enriched_Interpolation_Mesh.hpp"
#include "cl_Geom_Field.hpp"
#include "typedefs.hpp"

#include "cl_MTK_Mesh_Manager.hpp"

#include "cl_MTK_Vertex.hpp"    //MTK
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh.hpp"

#include "cl_Mesh_Factory.hpp"
#include "cl_MTK_Mesh_Tools.hpp"
#include "cl_MTK_Mesh_Data_Input.hpp"
#include "cl_MTK_Scalar_Field_Info.hpp"
#include "cl_MTK_Mesh.hpp"
#include "cl_MTK_Mesh_Data_STK.hpp"
#include "cl_MTK_Mesh_Core_STK.hpp"
#include "cl_MTK_Interpolation_Mesh_STK.hpp"
#include "cl_MTK_Integration_Mesh_STK.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Double_Side_Cluster.hpp"
#include "cl_MTK_Double_Side_Cluster_Input.hpp"
#include "cl_MTK_Side_Cluster.hpp"
#include "cl_MTK_Side_Cluster_Input.hpp"

#include "cl_Matrix.hpp"        //LINALG
#include "linalg_typedefs.hpp"
#include "fn_equal_to.hpp" // ALG/src

#include "cl_FEM_NodeProxy.hpp"                //FEM/INT/src
#include "cl_FEM_ElementProxy.hpp"             //FEM/INT/src
#include "cl_FEM_Node_Base.hpp"                //FEM/INT/src
#include "cl_FEM_Element_Factory.hpp"          //FEM/INT/src
#include "cl_FEM_IWG_Factory.hpp"              //FEM/INT/src
#include "cl_FEM_CM_Factory.hpp"              //FEM/INT/src
#include "cl_FEM_SP_Factory.hpp"              //FEM/INT/src
#include "cl_FEM_Set_User_Info.hpp"              //FEM/INT/src

#include "cl_MDL_Model.hpp"

#include "cl_HMR_Mesh_Interpolation.hpp"
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

#include "../projects/GEN/src/geometry/cl_GEN_Circle.hpp"
#include "../projects/GEN/src/geometry/cl_GEN_Geom_Field.hpp"
#include "../projects/GEN/src/geometry/cl_GEN_Geometry.hpp"

namespace moris
{

Matrix< DDRMat > tConstValFunction( moris::Cell< Matrix< DDRMat > >         & aCoeff,
                                    moris::Cell< fem::Field_Interpolator* > & aDofFieldInterpolator,
                                    moris::Cell< fem::Field_Interpolator* > & aDvFieldInterpolator,
                                    fem::Geometry_Interpolator              * aGeometryInterpolator )
{
    return aCoeff( 0 );
}

moris::Matrix< moris::DDRMat > tMValFunction( moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
                                              moris::Cell< moris::fem::Field_Interpolator* > & aDofFI,
                                              moris::Cell< moris::fem::Field_Interpolator* > & aDvFI,
                                              moris::fem::Geometry_Interpolator              * aGeometryInterpolator )
{
    return {{ aParameters( 0 )( 0 ),                   0.0,                       0.0 },
            { 0.0,                   aParameters( 0 )( 1 ),                       0.0 },
            { 0.0,                                     0.0,     aParameters( 0 )( 2 ) }};
}

moris::real LvlSetSphere_3D_outsideDomain(const moris::Matrix< moris::DDRMat > & aPoint )
{
    Matrix< DDRMat > tCenter{ { -10, -10, -10 } };
    return    norm( aPoint - tCenter ) - 0.001;
}



TEST_CASE("beamStudy","[beamStudy_3D]")
{
    if(par_size()<=1)
    {
        uint tLagrangeMeshIndex = 0;
        std::string tFieldName = "Cylinder";

        hmr::ParameterList tParameters = hmr::create_hmr_parameter_list();

        int tXDim = 1600;    int tYDim = 16;    int tZDim = 16;
        std::string tDim00 = std::to_string(tXDim);    std::string tDim01 = std::to_string(tYDim);    std::string tDim02 = std::to_string(tZDim);
        std::string tMeshInput = tDim00 + ", " + tDim01 + ", " + tDim02;

        tParameters.set( "number_of_elements_per_dimension", tMeshInput);
        tParameters.set( "domain_dimensions", "100, 2, 2" );
        tParameters.set( "domain_offset", "0.0, 0.0, 0.0" );
        tParameters.set( "domain_sidesets", "1,2,3,4,5,6" );
        tParameters.set( "lagrange_output_meshes", "0" );

        int tBSplineOrder = 1;
        std::string tOrder = std::to_string( tBSplineOrder );
        tParameters.set( "lagrange_orders", "1" );
        tParameters.set( "lagrange_pattern", "0" );
        tParameters.set( "bspline_orders", tOrder );
        tParameters.set( "bspline_pattern", "0" );

        tParameters.set( "lagrange_to_bspline", "0" );

        tParameters.set( "truncate_bsplines", 1 );
        tParameters.set( "refinement_buffer", 3 );
        tParameters.set( "staircase_buffer", 3 );
        tParameters.set( "initial_refinement", 0 );

        tParameters.set( "use_multigrid", 0 );
//        tParameters.set( "severity_level", 2 );

        hmr::HMR tHMR( tParameters );

        //initial refinement
        tHMR.perform_initial_refinement( 0 );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

        //  create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );

        tField->evaluate_scalar_function( LvlSetSphere_3D_outsideDomain );

        for( uint k=0; k<2; ++k )
        {
            tHMR.flag_surface_elements_on_working_pattern( tField );
            tHMR.perform_refinement_based_on_working_pattern( 0 );

            tField->evaluate_scalar_function( LvlSetSphere_3D_outsideDomain );
        }

        tHMR.finalize();

        std::shared_ptr< moris::hmr::Interpolation_Mesh_HMR > tInterpolationMesh = tHMR.create_interpolation_mesh(tLagrangeMeshIndex);

        //-----------------------------------------------------------------------------------------------

        moris::ge::Circle tCircle( 0.001, -100.0, -100.0 );

        moris::ge::GEN_Phase_Table     tPhaseTable(1,  Phase_Table_Structure::EXP_BASE_2);
        moris::ge::GEN_Geometry_Engine tGeometryEngine(tCircle,tPhaseTable, 2);

        //------------------------------------------------------------------------------
        // dummy pdv information ( not used in this test )
        Cell< enum moris::ge::GEN_PDV >  tPdvList(1);
        tPdvList(0) = moris::ge::GEN_PDV::TEMP;

        std::shared_ptr< moris::ge::GEN_Property > tConstTempProp = std::make_shared< moris::ge::GEN_Property >();

        Cell< moris::ge::GEN_Property* > tPropertyList(1);
        tPropertyList(0) = tConstTempProp.get();

        tGeometryEngine.set_pdv_property_list( tPdvList, tPropertyList );
        //------------------------------------------------------------------------------

         xtk::Model tXTKModel(3, tInterpolationMesh.get(), tGeometryEngine);

        tXTKModel.mVerbose = false;

        //Specify decomposition Method and Cut Mesh ---------------------------------------
        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8, Subdivision_Method::C_HIERARCHY_TET4};
        tXTKModel.decompose(tDecompositionMethods);
        //=============================== temporary ============================================
//        // output solution and meshes
//        xtk::Output_Options tOutputOptions1;
//        tOutputOptions1.mAddNodeSets = false;
//        tOutputOptions1.mAddSideSets = true;
//        tOutputOptions1.mAddClusters = false;
//
//        std::string tIntegSolFieldNameUX = "UX";
//        std::string tIntegSolFieldNameUY = "UY";
//        std::string tIntegSolFieldNameUZ = "UZ";
//        tOutputOptions1.mRealNodeExternalFieldNames = { tIntegSolFieldNameUX, tIntegSolFieldNameUY, tIntegSolFieldNameUZ };
//
//        moris::mtk::Integration_Mesh* tIntegMesh11 = tXTKModel.get_output_mesh(tOutputOptions1);
//
//        std::string tMeshOutputFile1 = "./tempMesh.e";
//        tIntegMesh11->create_output_mesh(tMeshOutputFile1);
//        delete tIntegMesh11;
        //============================= end temporary ==========================================
        tXTKModel.perform_basis_enrichment(EntityRank::BSPLINE_1,0);

        // get meshes
        xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
        xtk::Enriched_Integration_Mesh   & tEnrIntegMesh  = tXTKModel.get_enriched_integ_mesh();

//        tEnrInterpMesh.print_enriched_cells();
//        tEnrIntegMesh.print_double_side_sets(2);

        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair(&tEnrInterpMesh, &tEnrIntegMesh);

        //------------------------------------------------------------------------------
        // create the properties
        std::shared_ptr< fem::Property > tPropEMod = std::make_shared< fem::Property >();
        tPropEMod->set_parameters( { {{ 100.0 }} } );
        tPropEMod->set_val_function( tConstValFunction );

        std::shared_ptr< fem::Property > tPropNu = std::make_shared< fem::Property >();
        tPropNu->set_parameters( { {{ 0.0 }} } );
        tPropNu->set_val_function( tConstValFunction );

        std::shared_ptr< fem::Property > tPropDirichletUXUY_ss4 = std::make_shared< fem::Property >();    // fix displacement at side-set 4 to be zero in x, y, and z
        tPropDirichletUXUY_ss4->set_parameters( { {{ 0.0 }, {0.0}, {0.0}} } );
        tPropDirichletUXUY_ss4->set_val_function( tConstValFunction );
//        tPropDirichletUX_ss4->set_dof_type( MSI::Dof_Type::UX );

        std::shared_ptr< fem::Property > tPropDirichletUXUY_ss4_select = std::make_shared< fem::Property >();    // fix displacement at side-set 4 to be zero in x, y, and z
        tPropDirichletUXUY_ss4_select->set_parameters( { {{ 1.0 }, {1.0}, {1.0}} } );
        tPropDirichletUXUY_ss4_select->set_val_function( tMValFunction );
//        tPropDirichletUX_ss4->set_dof_type( MSI::Dof_Type::UX );

        std::shared_ptr< fem::Property > tPropDirichletUX_ss2 = std::make_shared< fem::Property >();        // allow only for y-displacement at other end (side-set 2)
        tPropDirichletUX_ss2->set_parameters( { {{ 0.0 }, {1.0}, {0.0}} } );                                       // fix x-displ. and z-dizpl. to zero
        tPropDirichletUX_ss2->set_val_function( tConstValFunction );
//        tPropDirichletUX->set_dof_type( MSI::Dof_Type::UX );

        std::shared_ptr< fem::Property > tPropDirichletUX_ss2_select = std::make_shared< fem::Property >();        // allow only for y-displacement at other end (side-set 2)
        tPropDirichletUX_ss2_select->set_parameters( { {{ 1.0 }, {0.0}, {1.0}} } );                                       // fix x-displ. to zero
        tPropDirichletUX_ss2_select->set_val_function( tMValFunction );
//        tPropDirichletUX->set_dof_type( MSI::Dof_Type::UX );

        std::shared_ptr< fem::Property > tPropNeumann = std::make_shared< fem::Property >();
        tPropNeumann->set_parameters( {{{ 0.0 } , { 10.0 }, {0.0}}} );
        tPropNeumann->set_val_function( tConstValFunction );

        // define constitutive models
        fem::CM_Factory tCMFactory;

        std::shared_ptr< fem::Constitutive_Model > tCMStrucLinIso1 = tCMFactory.create_CM( fem::Constitutive_Type::STRUC_LIN_ISO );
        tCMStrucLinIso1->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tCMStrucLinIso1->set_property( tPropEMod, "YoungsModulus" );
        tCMStrucLinIso1->set_property( tPropNu, "PoissonRatio" );
        tCMStrucLinIso1->set_space_dim( 3 );

        // define stabilization parameters
        fem::SP_Factory tSPFactory;

        std::shared_ptr< fem::Stabilization_Parameter > tSPDirichletNitsche = tSPFactory.create_SP( fem::Stabilization_Type::DIRICHLET_NITSCHE );
        tSPDirichletNitsche->set_parameters( { {{ 100.0 }} } );
        tSPDirichletNitsche->set_property( tPropEMod, "Material", mtk::Master_Slave::MASTER );

        // define the IWGs
        fem::IWG_Factory tIWGFactory;

        std::shared_ptr< fem::IWG > tIWGBulk1 = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_BULK );
        tIWGBulk1->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ } );
        tIWGBulk1->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }} );
        tIWGBulk1->set_constitutive_model( tCMStrucLinIso1, "ElastLinIso", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGDirichlet_ss2 = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_DIRICHLET );
        tIWGDirichlet_ss2->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ } );
        tIWGDirichlet_ss2->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }} );
        tIWGDirichlet_ss2->set_stabilization_parameter( tSPDirichletNitsche, "DirichletNitsche" );
        tIWGDirichlet_ss2->set_constitutive_model( tCMStrucLinIso1, "ElastLinIso", mtk::Master_Slave::MASTER );
        tIWGDirichlet_ss2->set_property( tPropDirichletUX_ss2, "Dirichlet", mtk::Master_Slave::MASTER );
        tIWGDirichlet_ss2->set_property( tPropDirichletUX_ss2_select, "Select", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGDirichletFixed_ss4 = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_DIRICHLET );
        tIWGDirichletFixed_ss4->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ } );
        tIWGDirichletFixed_ss4->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }} );
        tIWGDirichletFixed_ss4->set_stabilization_parameter( tSPDirichletNitsche, "DirichletNitsche" );
        tIWGDirichletFixed_ss4->set_constitutive_model( tCMStrucLinIso1, "ElastLinIso", mtk::Master_Slave::MASTER );
        tIWGDirichletFixed_ss4->set_property( tPropDirichletUXUY_ss4, "Dirichlet", mtk::Master_Slave::MASTER );
        tIWGDirichletFixed_ss4->set_property( tPropDirichletUXUY_ss4_select, "Select", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGNeumann = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_NEUMANN );
        tIWGNeumann->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ } );
        tIWGNeumann->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }} );
        tIWGNeumann->set_property( tPropNeumann, "Neumann", mtk::Master_Slave::MASTER );

        // create a list of active block-sets
//        std::string tInterfaceSideSetName = tEnrIntegMesh.get_interface_side_set_name( 0, 0, 1 );
//        std::string tDblInterfaceSideSetName = tEnrIntegMesh.get_dbl_interface_side_set_name(0,1);

        // define set info
        fem::Set_User_Info tSetBulk4;
        tSetBulk4.set_mesh_index( tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p1") );
        tSetBulk4.set_IWGs( { tIWGBulk1 } );
        //------------------------------------------------------------------------------

        fem::Set_User_Info tSetDirichletFixed;
        tSetDirichletFixed.set_mesh_index( tEnrIntegMesh.get_set_index_by_name("SideSet_4_n_p1") );
        tSetDirichletFixed.set_IWGs( { tIWGDirichletFixed_ss4 } );

        fem::Set_User_Info tSetDirichlet;
        tSetDirichlet.set_mesh_index( tEnrIntegMesh.get_set_index_by_name("SideSet_2_n_p1") );
        tSetDirichlet.set_IWGs( { tIWGDirichlet_ss2 } );

        fem::Set_User_Info tSetNeumann;
        tSetNeumann.set_mesh_index( tEnrIntegMesh.get_set_index_by_name("SideSet_2_n_p1") );
        tSetNeumann.set_IWGs( { tIWGNeumann } );

        //------------------------------------------------------------------------------
        // create a cell of set info
        moris::Cell< fem::Set_User_Info > tSetInfo( 4 );
        tSetInfo( 0 ) = tSetBulk4;
        tSetInfo( 1 ) = tSetDirichletFixed;
        tSetInfo( 2 ) = tSetDirichlet;
        tSetInfo( 3 ) = tSetNeumann;

        // create model
        mdl::Model * tModel = new mdl::Model( &tMeshManager,
                                              0,
                                              tSetInfo,
                                              0,
                                              false );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        moris::Cell< enum MSI::Dof_Type > tDofTypesU( 3 );
        tDofTypesU( 0 ) = MSI::Dof_Type::UX;    tDofTypesU( 1 ) = MSI::Dof_Type::UY;    tDofTypesU( 2 ) = MSI::Dof_Type::UZ;

        dla::Solver_Factory  tSolFactory;
//        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AMESOS_IMPL );

//        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_max_iter") = 100;
//        tLinearSolverAlgorithm->set_param("AZ_solver") = AZ_gmres;
//        tLinearSolverAlgorithm->set_param("AZ_subdomain_solve") = AZ_ilu;
//        tLinearSolverAlgorithm->set_param("AZ_graph_fill") = 3;
        //        tLinearSolverAlgorithm->set_param("Use_ML_Prec") = true;

        dla::Linear_Solver tLinSolver;
        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
        //        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMonolythicU = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter") = 10;
        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_hard_break") = false;
        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_max_lin_solver_restarts") = 2;
        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_rebuild_jacobian") = true;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );
        //        tNonlinearSolverAlgorithmMonolythicU->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolverMain;
        tNonlinearSolverMain.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );


        tNonlinearSolverMain       .set_dof_type_list( tDofTypesU );

        // Create solver database
        NLA::SOL_Warehouse tSolverWarehouse( tModel->get_solver_interface() );

        tNonlinearSolverMain       .set_solver_warehouse( &tSolverWarehouse );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: create time Solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        tsa::Time_Solver_Factory tTimeSolverFactory;
        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );

        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolverMain );

        tsa::Time_Solver tTimeSolver;
        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );
        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );

        tTimeSolver.set_dof_type_list( tDofTypesU );

        //------------------------------------------------------------------------------
        tTimeSolver.solve();
        //-----  print solution vector -----
//        Matrix< DDRMat > tSolVec;
//        tNonlinearSolverMain.get_full_solution(tSolVec);
//        print( tSolVec, " Solution Vector ");

        //----------------------------------

        // output solution and meshes
        xtk::Output_Options tOutputOptions;
        tOutputOptions.mAddNodeSets = false;
        tOutputOptions.mAddSideSets = false;
        tOutputOptions.mAddClusters = false;

        // add solution field to integration mesh
        std::string tIntegSolFieldNameUX = "UX";
        std::string tIntegSolFieldNameUY = "UY";
        std::string tIntegSolFieldNameUZ = "UZ";
        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldNameUX, tIntegSolFieldNameUY, tIntegSolFieldNameUZ};

        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);

        // Write to Integration mesh for visualization
        Matrix<DDRMat> tIntegSolUX = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UX );
        Matrix<DDRMat> tIntegSolUY = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UY );
        Matrix<DDRMat> tIntegSolUZ = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UZ );

        Matrix<DDRMat> tSTKIntegSolUX(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
        Matrix<DDRMat> tSTKIntegSolUY(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
        Matrix<DDRMat> tSTKIntegSolUZ(tIntegMesh1->get_num_entities(EntityRank::NODE),1);

        for(moris::uint i = 0; i < tIntegMesh1->get_num_entities(EntityRank::NODE); i++)
        {
            moris::moris_id tID = tIntegMesh1->get_glb_entity_id_from_entity_loc_index(i,EntityRank::NODE);
            tSTKIntegSolUX(i) = tIntegSolUX(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
            tSTKIntegSolUY(i) = tIntegSolUY(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
            tSTKIntegSolUZ(i) = tIntegSolUZ(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
        }

        // add solution field to integration mesh
        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUX,EntityRank::NODE,tSTKIntegSolUX);
        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUY,EntityRank::NODE,tSTKIntegSolUY);
        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUX,EntityRank::NODE,tSTKIntegSolUZ);

        // -------------- output solution mesh -----------------------
        std::string tMeshOutputFile = "./beamStudy/beamDims_" + tDim00 + "_" + tDim01 + "_" + tDim02 + "_bSplineOrder_" + tOrder + ".e";
        tIntegMesh1->create_output_mesh(tMeshOutputFile);
        // -----------------------------------------------------------
        delete tIntegMesh1;
        delete tModel;
    }
}
}   // end moris namespace


