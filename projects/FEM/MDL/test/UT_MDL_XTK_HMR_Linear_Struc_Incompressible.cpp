
#include "catch.hpp"
#include "paths.hpp"
#include "typedefs.hpp"
#include "HDF5_Tools.hpp"
#include "cl_Matrix.hpp"        //LINALG
#include "linalg_typedefs.hpp"
#include "fn_equal_to.hpp"
#include "fn_norm.hpp"

//MTK/src
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Vertex.hpp"
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh.hpp"
#include "cl_MTK_Mesh_Factory.hpp"
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
//XTK/src
#include "cl_XTK_Model.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"
#include "cl_XTK_Enriched_Interpolation_Mesh.hpp"
//FEM/INT/src
#include "cl_FEM_NodeProxy.hpp"
#include "cl_FEM_ElementProxy.hpp"
#include "cl_FEM_Node_Base.hpp"
#include "cl_FEM_Element_Factory.hpp"
#include "cl_FEM_IWG_Factory.hpp"
#include "cl_FEM_IQI_Factory.hpp"
#include "cl_FEM_CM_Factory.hpp"
#include "cl_FEM_SP_Factory.hpp"
#include "cl_FEM_Set_User_Info.hpp"
//FEM/MSI/src
#include "cl_MSI_Solver_Interface.hpp"
#include "cl_MSI_Equation_Object.hpp"
#include "cl_MSI_Model_Solver_Interface.hpp"
//FEM/MDL/src
#include "cl_MDL_Model.hpp"
//FEM/VIS/src
#include "cl_VIS_Factory.hpp"
#include "cl_VIS_Visualization_Mesh.hpp"
#include "cl_VIS_Output_Manager.hpp"
//HMR/src
#include "cl_HMR_Mesh_Interpolation.hpp"
#include "cl_HMR.hpp"
#include "cl_HMR_Background_Mesh.hpp"
#include "cl_HMR_BSpline_Mesh_Base.hpp"
#include "cl_HMR_Element.hpp"
#include "cl_HMR_Factory.hpp"
#include "cl_HMR_Field.hpp"
#include "cl_HMR_Lagrange_Mesh_Base.hpp"
#include "cl_HMR_Parameters.hpp"
//SOL/DLA/src
#include "cl_DLA_Solver_Factory.hpp"
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_DLA_Linear_Solver_Aztec.hpp"
#include "cl_DLA_Linear_Solver.hpp"
//SOL/NLA/src
#include "cl_NLA_Nonlinear_Solver_Factory.hpp"
#include "cl_NLA_Nonlinear_Solver.hpp"
#include "cl_NLA_Nonlinear_Problem.hpp"
//SOL/TSA/src
#include "cl_TSA_Time_Solver_Factory.hpp"
#include "cl_TSA_Monolithic_Time_Solver.hpp"
#include "cl_TSA_Time_Solver.hpp"
//SOL/src
#include "cl_SOL_Warehouse.hpp"
//GEN/src
#include "cl_GEN_Circle.hpp"
#include "cl_GEN_Sphere.hpp"

//PRM/src
#include "fn_PRM_HMR_Parameters.hpp"

namespace moris
{
moris::real LvlSetLin(const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real tOffset = 200;
    return aPoint(0) - 0.317 * aPoint(1) - tOffset;
}

moris::real LvlSetCircle2D(const moris::Matrix< moris::DDRMat > & aPoint )
{
    return std::sqrt( aPoint( 0 ) * aPoint( 0 ) + aPoint( 1 ) * aPoint( 1 ) ) - 0.45;
}

moris::real LvlSetCircle2D_outsideDomain(const moris::Matrix< moris::DDRMat > & aPoint )
{
    Matrix< DDRMat > tCenter{ { -10, -10 } };
    return norm( aPoint - tCenter ) - 0.001;
}

void tConstValFunction
( moris::Matrix< moris::DDRMat >                 & aPropMatrix,
  moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
  moris::fem::Field_Interpolator_Manager         * aFIManager )
{
    aPropMatrix = aParameters( 0 );
}

void tMValFunction2d
( moris::Matrix< moris::DDRMat >                 & aPropMatrix,
  moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
  moris::fem::Field_Interpolator_Manager         * aFIManager )
{
    aPropMatrix = {{ aParameters( 0 )( 0 ), 0.0 },
            { 0.0, aParameters( 0 )( 1 ) }};
}

void tMValFunction3d
( moris::Matrix< moris::DDRMat >                 & aPropMatrix,
  moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
  moris::fem::Field_Interpolator_Manager         * aFIManager )
{
    aPropMatrix = {{ aParameters( 0 )( 0 ), 0.0, 0.0 },
            { 0.0, aParameters( 0 )( 1 ), 0.0 },
            { 0.0, 0.0, aParameters( 0 )( 2 ) }};
}

moris::real
LevelSetFunction_star1( const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real tPhi = std::atan2( aPoint( 0 ), aPoint( 1 ) );
    moris::real tLevelSetVaue = 0.501 + 0.1 * std::sin( 5 * tPhi ) - std::sqrt( std::pow( aPoint( 0 ), 2 ) + std::pow( aPoint( 1 ), 2 ) );
    return -tLevelSetVaue;
}

moris::real
Plane4MatMDL1(const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real mXC = 0.1;
    moris::real mYC = 0.1;
    moris::real mNx = 1.0;
    moris::real mNy = 0.0;
    return ( mNx*( aPoint(0)-mXC ) + mNy*( aPoint(1)-mYC ) );
}

moris::real
Circle4MatMDL(const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real mXCenter = 0.01;
    moris::real mYCenter = 0.01;
    moris::real mRadius = 0.47334;

    return  (aPoint(0) - mXCenter) * (aPoint(0) - mXCenter)
                    + (aPoint(1) - mYCenter) * (aPoint(1) - mYCenter)
                    - (mRadius * mRadius);
}

bool tSolverOutputCriteria( moris::tsa::Time_Solver * )
{
    return true;
}

TEST_CASE("2D XTK HMR Incompressible","[XTK_HMR_I_2D]")
{
    if(par_size()<=1)
    {
        uint tLagrangeMeshIndex = 0;
        std::string tFieldName = "Cylinder";

        moris::ParameterList tParameters = prm::create_hmr_parameter_list();

        tParameters.set( "number_of_elements_per_dimension", "10, 10");
        tParameters.set( "domain_dimensions", "2, 2" );
        tParameters.set( "domain_offset", "-1.0, -1.0" );
        tParameters.set( "domain_sidesets", "1,2,3,4" );
        tParameters.set( "lagrange_output_meshes", "0" );

        tParameters.set( "lagrange_orders", "1" );
        tParameters.set( "lagrange_pattern", "0" );
        tParameters.set( "bspline_orders", "1" );
        tParameters.set( "bspline_pattern", "0" );

        tParameters.set( "lagrange_to_bspline", "0" );

        tParameters.set( "truncate_bsplines", 1 );
        tParameters.set( "refinement_buffer", 3 );
        tParameters.set( "staircase_buffer", 3 );
        tParameters.set( "initial_refinement", "0" );
        tParameters.set( "initial_refinement_pattern", "0" );

        tParameters.set( "use_multigrid", 0 );
        tParameters.set( "severity_level", 2 );

        hmr::HMR tHMR( tParameters );

        // initial refinement
        tHMR.perform_initial_refinement();

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

        // create field
        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );

        tField->evaluate_scalar_function( LvlSetCircle2D );

        // refinement
        for( uint k=0; k<2; ++k )
        {
            tHMR.flag_surface_elements_on_working_pattern( tField );
            tHMR.perform_refinement_based_on_working_pattern( 0 );

            tField->evaluate_scalar_function( LvlSetCircle2D );
        }

        tHMR.finalize();

        tHMR.save_to_exodus( 0, "./xtk_exo/mdl_xtk_hmr_2d.e" );

        moris::hmr::Interpolation_Mesh_HMR * tInterpolationMesh = tHMR.create_interpolation_mesh(tLagrangeMeshIndex);

        Cell<std::shared_ptr<moris::ge::Geometry>> tGeometry(1);
        tGeometry(0) = std::make_shared<moris::ge::Circle>(0.0, 0.0, 0.4501);

        moris::ge::Geometry_Engine_Parameters tGeometryEngineParameters;
        tGeometryEngineParameters.mGeometries = tGeometry;
        moris::ge::Geometry_Engine tGeometryEngine(tInterpolationMesh, tGeometryEngineParameters);

        xtk::Model tXTKModel(2, tInterpolationMesh, &tGeometryEngine);
        tXTKModel.mVerbose = false;

        //Specify decomposition Method and Cut Mesh ---------------------------------------
        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
        tXTKModel.decompose(tDecompositionMethods);

        tXTKModel.perform_basis_enrichment( EntityRank::NODE, 0 );

        // get meshes
        xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
        xtk::Enriched_Integration_Mesh   & tEnrIntegMesh = tXTKModel.get_enriched_integ_mesh();

        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair(&tEnrInterpMesh, &tEnrIntegMesh);

        //------------------------------------------------------------------------------
        // create the properties
        std::string tMeshOutputFile = "./mdl_exo/xtk_hmr_incompressible_2d.exo";
        std::shared_ptr< fem::Property > tPropEMod = std::make_shared< fem::Property >();
        tPropEMod->set_parameters( { {{ 10 }} } );
        tPropEMod->set_val_function( tConstValFunction );

        std::shared_ptr< fem::Property > tPropNu = std::make_shared< fem::Property >();
        tPropNu->set_parameters( { {{ 0.5 }} } );
        tPropNu->set_val_function( tConstValFunction );

        std::shared_ptr< fem::Property > tPropDirichlet = std::make_shared< fem::Property >();
        tPropDirichlet->set_parameters( { {{ 0.0 }, { 0.0 }} } );
        tPropDirichlet->set_val_function( tConstValFunction );

        std::shared_ptr< fem::Property > tPropDirichlet2 = std::make_shared< fem::Property >();
        tPropDirichlet2->set_parameters( { {{ 1.0 }, { 1.0 }} } );
        tPropDirichlet2->set_val_function( tMValFunction2d );

        std::shared_ptr< fem::Property > tPropTraction = std::make_shared< fem::Property >();
        tPropTraction->set_parameters( {{{ 1.0 } , { 0.0 }}} );
        tPropTraction->set_val_function( tConstValFunction );

        // define constitutive models
        fem::CM_Factory tCMFactory;

        std::shared_ptr< fem::Constitutive_Model > tCMStrucLinIsoDev = tCMFactory.create_CM( fem::Constitutive_Type::STRUC_LIN_ISO );
        tCMStrucLinIsoDev->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, {MSI::Dof_Type::P}},
                                              { "Displacement", "Pressure" } );
        tCMStrucLinIsoDev->set_property( tPropEMod, "YoungsModulus" );
        tCMStrucLinIsoDev->set_property( tPropNu, "PoissonRatio" );
        tCMStrucLinIsoDev->set_space_dim( 2 );
        tCMStrucLinIsoDev->set_model_type(fem::Model_Type::PLANE_STRESS);
        tCMStrucLinIsoDev->set_model_type(fem::Model_Type::DEVIATORIC);
        tCMStrucLinIsoDev->set_local_properties();

        // define stabilization parameters
        fem::SP_Factory tSPFactory;

        std::shared_ptr< fem::Stabilization_Parameter > tSPDirichletNitsche = tSPFactory.create_SP( fem::Stabilization_Type::DIRICHLET_NITSCHE );
        tSPDirichletNitsche->set_parameters( { {{ 100000.0 }} } );
        tSPDirichletNitsche->set_property( tPropEMod, "Material", mtk::Master_Slave::MASTER );

        // define the IWGs
        fem::IWG_Factory tIWGFactory;

        std::shared_ptr< fem::IWG > tIWGBulk = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_BULK );
        tIWGBulk->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY} );
        tIWGBulk->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY}, {MSI::Dof_Type::P }} );
        tIWGBulk->set_constitutive_model( tCMStrucLinIsoDev, "ElastLinIso", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGBulkP = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_PRESSURE_BULK );
        tIWGBulkP->set_residual_dof_type( { MSI::Dof_Type::P } );
        tIWGBulkP->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY}, {MSI::Dof_Type::P }} );
        tIWGBulkP->set_constitutive_model( tCMStrucLinIsoDev, "ElastLinIso", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGDirichlet = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_DIRICHLET_UNSYMMETRIC_NITSCHE );
        tIWGDirichlet->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIWGDirichlet->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, { MSI::Dof_Type::P}} );
        tIWGDirichlet->set_stabilization_parameter( tSPDirichletNitsche, "DirichletNitsche" );
        tIWGDirichlet->set_constitutive_model( tCMStrucLinIsoDev, "ElastLinIso", mtk::Master_Slave::MASTER );
        tIWGDirichlet->set_property( tPropDirichlet, "Dirichlet", mtk::Master_Slave::MASTER );
        tIWGDirichlet->set_property( tPropDirichlet2, "Select", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGDirichletP = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_PRESSURE_DIRICHLET_SYMMETRIC_NITSCHE );
        tIWGDirichletP->set_residual_dof_type( { MSI::Dof_Type::P } );
        tIWGDirichletP->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, { MSI::Dof_Type::P}} );
        tIWGDirichletP->set_constitutive_model( tCMStrucLinIsoDev, "ElastLinIso", mtk::Master_Slave::MASTER );
        tIWGDirichletP->set_property( tPropDirichlet, "Dirichlet", mtk::Master_Slave::MASTER );
        tIWGDirichletP->set_property( tPropDirichlet2, "Select", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGNeumann = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_NEUMANN );
        tIWGNeumann->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIWGNeumann->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, { MSI::Dof_Type::P}} );
        tIWGNeumann->set_property( tPropTraction, "Traction", mtk::Master_Slave::MASTER );

        // create the IQIs
        // --------------------------------------------------------------------------------------
        fem::IQI_Factory tIQIFactory;

        std::shared_ptr< fem::IQI > tIQIUX = tIQIFactory.create_IQI( fem::IQI_Type::DOF );
        tIQIUX->set_quantity_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIQIUX->set_dof_type_list( { { MSI::Dof_Type::UX, MSI::Dof_Type::UY } }, mtk::Master_Slave::MASTER );
        tIQIUX->set_output_type_index( 0 );
        tIQIUX->set_name( "IQI_UX" );

        std::shared_ptr< fem::IQI > tIQIUY = tIQIFactory.create_IQI( fem::IQI_Type::DOF );
        tIQIUY->set_quantity_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIQIUY->set_dof_type_list( { { MSI::Dof_Type::UX, MSI::Dof_Type::UY } }, mtk::Master_Slave::MASTER );
        tIQIUY->set_output_type_index( 1 );
        tIQIUY->set_name( "IQI_UY" );

        std::shared_ptr< fem::IQI > tIQIP = tIQIFactory.create_IQI( fem::IQI_Type::DOF );
        tIQIP->set_quantity_dof_type( { MSI::Dof_Type::P } );
        tIQIP->set_dof_type_list( { { MSI::Dof_Type::P } }, mtk::Master_Slave::MASTER );
        tIQIP->set_output_type_index( 0 );
        tIQIP->set_name( "IQI_P" );

        // create a list of active block-sets
        std::string tInterfaceSideSetName = tEnrIntegMesh.get_interface_side_set_name( 0, 0, 1 );

        // define set info
        fem::Set_User_Info tSetBulk1;
        tSetBulk1.set_mesh_set_name( "HMR_dummy_c_p1" );
        tSetBulk1.set_IWGs( { tIWGBulk, tIWGBulkP } );
        tSetBulk1.set_IQIs( { tIQIUX, tIQIUY, tIQIP } );

        fem::Set_User_Info tSetBulk2;
        tSetBulk2.set_mesh_set_name( "HMR_dummy_n_p1" );
        tSetBulk2.set_IWGs( { tIWGBulk, tIWGBulkP } );
        tSetBulk2.set_IQIs( { tIQIUX, tIQIUY, tIQIP } );

        fem::Set_User_Info tSetDirichlet;
        tSetDirichlet.set_mesh_set_name( "SideSet_4_n_p1" );
        tSetDirichlet.set_IWGs( { tIWGDirichlet, tIWGDirichletP } );

        fem::Set_User_Info tSetNeumann;
        tSetNeumann.set_mesh_set_name( "SideSet_2_n_p1" );
        tSetNeumann.set_IWGs( { tIWGNeumann } );

        // create a cell of set info
        moris::Cell< fem::Set_User_Info > tSetInfo(4);
        tSetInfo(0) = tSetBulk1;
        tSetInfo(1) = tSetBulk2;
        tSetInfo(2) = tSetDirichlet;
        tSetInfo(3) = tSetNeumann;

        // create model
        mdl::Model * tModel = new mdl::Model( &tMeshManager,
                                              0,
                                              tSetInfo,
                                              0, false );

        // define outputs
        // --------------------------------------------------------------------------------------
        vis::Output_Manager tOutputData;
        tOutputData.set_outputs( 0,
                                 vis::VIS_Mesh_Type::STANDARD, //OVERLAPPING_INTERFACE
                                 "./",
                                 "MDL_XTK_HMR_Linear_Struc_Incompressible_Test_2D_Output.exo",
                                 "./",
                                  "temp.exo",
                                 { "HMR_dummy_c_p1", "HMR_dummy_n_p1" },
                                 { "UX", "UY", "P" },
                                 { vis::Field_Type::NODAL, vis::Field_Type::NODAL, vis::Field_Type::NODAL },
                                 { "IQI_UX","IQI_UY","IQI_P" } );
        tModel->set_output_manager( &tOutputData );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        moris::Cell< enum MSI::Dof_Type > tDofTypes( 3 );
        tDofTypes( 0 ) = MSI::Dof_Type::UX;
        tDofTypes( 1 ) = MSI::Dof_Type::UY;
        tDofTypes(2) = MSI::Dof_Type::P;

        moris::Cell< enum MSI::Dof_Type > tDofTypesU( 2 );
        tDofTypesU( 0 ) = MSI::Dof_Type::UX;
        tDofTypesU( 1 ) = MSI::Dof_Type::UY;

        moris::Cell< enum MSI::Dof_Type > tDofTypesP( 1 );
        tDofTypesP( 0 ) = MSI::Dof_Type::P;

        dla::Solver_Factory  tSolFactory;
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( sol::SolverType::AZTEC_IMPL );

        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_max_iter") = 10000;
        tLinearSolverAlgorithm->set_param("AZ_solver") = AZ_gmres;
        tLinearSolverAlgorithm->set_param("AZ_subdomain_solve") = AZ_ilu;
        tLinearSolverAlgorithm->set_param("AZ_graph_fill") = 50;
        //        tLinearSolverAlgorithm->set_param("Use_ML_Prec") = true;

        dla::Linear_Solver tLinSolver;
        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 4;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolverMain;
        tNonlinearSolverMain.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );


        tNonlinearSolverMain       .set_dof_type_list( tDofTypes );

        // Create solver database
        sol::SOL_Warehouse tSolverWarehouse( tModel->get_solver_interface() );

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
        tTimeSolver.set_dof_type_list( tDofTypesP );

        tTimeSolver.set_output( 0, tSolverOutputCriteria );

        //------------------------------------------------------------------------------
        tTimeSolver.solve();

        // Start solution comparison
        Matrix<DDRMat> tFullSolution;
        Matrix<DDRMat> tGoldSolution;
        tNonlinearSolverMain.get_full_solution(tFullSolution);

        std::string tMorisRoot = moris::get_base_moris_dir();
        std::string tHdf5FilePath = tMorisRoot + "/projects/FEM/MDL/test/data/incompressible_test_2d.hdf5";

//        //------------------------------------------------------------------------------
//        //    write solution ( uncomment this if you want to recreate solution files )
//        //------------------------------------------------------------------------------
//
//        // create file
//        hid_t tFileID = create_hdf5_file( tHdf5FilePath );
//
//        // error handler
//        herr_t tStatus = 0;
//
//        // save data
//        save_matrix_to_hdf5_file( tFileID, "Gold Solution", tFullSolution, tStatus );
//
//        // close file
//        close_hdf5_file( tFileID );

        //------------------------------------------------------------------------------
        //    check solution
        //------------------------------------------------------------------------------

        // open file
        hid_t tFileID = open_hdf5_file( tHdf5FilePath );

        // error handler
        herr_t tStatus = 0;

        // read solution from file
        load_matrix_from_hdf5_file( tFileID, "Gold Solution", tGoldSolution, tStatus );

        // close file
        close_hdf5_file( tFileID );

        // verify solution
        bool tSolutionCheck = true;
        for( uint i = 0; i < tFullSolution.numel(); i++ )
        {
            tSolutionCheck = tSolutionCheck && ( tFullSolution( i ) - tGoldSolution( i ) < 1e-03 );
            if( !tSolutionCheck )
            {
                std::cout<<"tFullSolution( i ) "<<tFullSolution( i )<<" tGoldSolution( i ) "<<tGoldSolution( i )<<std::endl;
            }
        }
        CHECK( tSolutionCheck );

        // clean up
        delete tModel;
        delete tInterpolationMesh;
    }
}
}
