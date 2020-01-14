/*
 * UT_MDL_XTK_HMR_2D.cpp
 *
 *  Created on: Sep 18, 2019
 *      Author: schmidt
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


namespace moris
{

Matrix< DDRMat > exactTempFunc( moris::Cell< Matrix< DDRMat > >         & aCoeff,
                                moris::Cell< fem::Field_Interpolator* > & aFieldInterpolator,
                                fem::Geometry_Interpolator             * aGeometryInterpolator )
{
    Matrix< DDRMat > tCoord = aGeometryInterpolator->valx();
    real xcoord = tCoord(0);
    real ycoord = tCoord(1);

    real rad = std::pow (  std::pow( xcoord - 0, 2.0) + std::pow( ycoord - 0, 2.0), 0.5);
    return {{(1.0/3.0)*(1.0/rad-0.501)}};
}

moris::real LvlSetCircle_2D(const moris::Matrix< moris::DDRMat > & aPoint )
{
    return    std::sqrt( aPoint( 0 ) * aPoint( 0 ) + aPoint( 1 ) * aPoint( 1 ) ) - 0.2505;
}


Matrix< DDRMat > tConstValFunction( moris::Cell< Matrix< DDRMat > >         & aCoeff,
                                    moris::Cell< fem::Field_Interpolator* > & aDofFieldInterpolator,
                                    moris::Cell< fem::Field_Interpolator* > & aDvFieldInterpolator,
                                    fem::Geometry_Interpolator             * aGeometryInterpolator )
{
    return aCoeff( 0 );
}

moris::real LevelSetFunction_star1( const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real tPhi = std::atan2( aPoint( 0 ), aPoint( 1 ) );

    moris::real tLevelSetVaue = 0.501 + 0.1 * std::sin( 5 * tPhi ) - std::sqrt( std::pow( aPoint( 0 ), 2 ) + std::pow( aPoint( 1 ), 2 ) );

    return -tLevelSetVaue;
}

moris::real Plane4MatMDL1(const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real mXC = 0.1;
    moris::real mYC = 0.1;
    moris::real mNx = 1.0;
    moris::real mNy = 0.0;
    return ( mNx*( aPoint(0)-mXC ) + mNy*( aPoint(1)-mYC ) );
}

moris::real Circle4MatMDL(const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real mXCenter = 0.01;
    moris::real mYCenter = 0.01;
    moris::real mRadius = 0.47334;

    return  (aPoint(0) - mXCenter) * (aPoint(0) - mXCenter)
                    + (aPoint(1) - mYCenter) * (aPoint(1) - mYCenter)
                    - (mRadius * mRadius);
}
moris::Matrix< moris::DDRMat > tFIVal_STRUCDIRICHLET( moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
                                                              moris::Cell< moris::fem::Field_Interpolator* > & aDofFI,
                                                              moris::Cell< moris::fem::Field_Interpolator* > & aDvFI,
                                                              moris::fem::Geometry_Interpolator              * aGeometryInterpolator )
{
    return aParameters( 0 ) + aParameters( 1 ) * ( aDofFI( 0 )->val() - aParameters( 2 ) );
}

moris::Matrix< moris::DDRMat > tFIDer_STRUCDIRICHLET( moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
                                                              moris::Cell< moris::fem::Field_Interpolator* > & aDofFI,
                                                              moris::Cell< moris::fem::Field_Interpolator* > & aDvFI,
                                                              moris::fem::Geometry_Interpolator              * aGeometryInterpolator )
{
    return aParameters( 1 ) * aDofFI( 0 )->N();
}

TEST_CASE("2D XTK WITH HMR Thermal Property 2D","[XTK_HMR_thermal_property_2D]")
{
//    if(par_size()<=1)
//    {
//        uint tLagrangeMeshIndex = 0;
//        std::string tFieldName = "Cylinder";
//
//        hmr::ParameterList tParameters = hmr::create_hmr_parameter_list();
//
//        tParameters.set( "number_of_elements_per_dimension", "20, 20");
//        tParameters.set( "domain_dimensions", "2, 2" );
//        tParameters.set( "domain_offset", "-1.0, -1.0" );
//        tParameters.set( "domain_sidesets", "1,2,3,4" );
//        tParameters.set( "lagrange_output_meshes", "0" );
//
//        tParameters.set( "lagrange_orders", "1" );
//        tParameters.set( "lagrange_pattern", "0" );
//        tParameters.set( "bspline_orders", "1" );
//        tParameters.set( "bspline_pattern", "0" );
//
//        tParameters.set( "lagrange_to_bspline", "0" );
//
//        tParameters.set( "truncate_bsplines", 1 );
//        tParameters.set( "refinement_buffer", 3 );
//        tParameters.set( "staircase_buffer", 3 );
//        tParameters.set( "initial_refinement", 0 );
//
//        tParameters.set( "use_multigrid", 0 );
//        tParameters.set( "severity_level", 2 );
//
//        hmr::HMR tHMR( tParameters );
//
//        // initial refinement
//        tHMR.perform_initial_refinement( 0 );
//
//        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );
//
//        //// create field
//        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );
//
//        tField->evaluate_scalar_function( LvlSetCircle_2D );
//        //
//        for( uint k=0; k<2; ++k )
//        {
//            tHMR.flag_surface_elements_on_working_pattern( tField );
//            tHMR.perform_refinement_based_on_working_pattern( 0 );
//
//            tField->evaluate_scalar_function( LvlSetCircle_2D );
//        }
//
//        tHMR.finalize();
//
//        tHMR.save_to_exodus( 0, "./xtk_exo/mdl_xtk_hmr_2d.e" );
//
//        std::shared_ptr< moris::hmr::Interpolation_Mesh_HMR > tInterpolationMesh = tHMR.create_interpolation_mesh(tLagrangeMeshIndex);
//
////        xtk::Circle tCircle( 0.4501, 2.0, 0.0 );
//        xtk::Geom_Field tCircleFieldAsGeom(tField);
//
//        xtk::Phase_Table tPhaseTable (1,  Phase_Table_Structure::EXP_BASE_2);
//        xtk::Geometry_Engine tGeometryEngine(tCircleFieldAsGeom,tPhaseTable, 2);
//
//        xtk::Model tXTKModel(2, tInterpolationMesh.get(), tGeometryEngine);
//
//        tXTKModel.mVerbose = false;
//
//        //Specify decomposition Method and Cut Mesh ---------------------------------------
//        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
//        tXTKModel.decompose(tDecompositionMethods);
//
//        tXTKModel.perform_basis_enrichment(EntityRank::NODE,0);
//
//        // get meshes
//        xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
//        xtk::Enriched_Integration_Mesh   & tEnrIntegMesh = tXTKModel.get_enriched_integ_mesh();
//
//        // place the pair in mesh manager
//        mtk::Mesh_Manager tMeshManager;
//        tMeshManager.register_mesh_pair(&tEnrInterpMesh, &tEnrIntegMesh);
//
//        uint tSpatialDimension = 2;
//
//        // create IWG user defined info
//        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 4 );
//        tIWGUserDefinedInfo( 0 ).resize( 2 );
//        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_BULK,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, { MSI::Dof_Type::TEMP } },
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 0 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 1 ).resize( 2 );
//        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_BULK,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, { MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 1 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 2 ).resize( 2 );
//        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_DIRICHLET,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, { MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::STRUC_DIRICHLET },
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 2 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::TEMP_DIRICHLET },
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 3 ).resize( 2 );
//        tIWGUserDefinedInfo( 3 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_NEUMANN,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }, { MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::STRUC_NEUMANN },
//                moris::Cell< fem::Constitutive_Type >( 0 ) );
//        tIWGUserDefinedInfo( 3 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::TEMP_NEUMANN },
//                moris::Cell< fem::Constitutive_Type >( 0 ) );
//
//        // create property user defined info
//        fem::Property_User_Defined_Info tYoungs_Modulus( fem::Property_Type::YOUNGS_MODULUS,
//                {{ MSI::Dof_Type::TEMP}},
//                {{ { 10.0} }, { {1.0} }, { {5.0 } }},
//                tFIVal_STRUCDIRICHLET,
//                { tFIDer_STRUCDIRICHLET } );
//        fem::Property_User_Defined_Info tPoissons_Ratio( fem::Property_Type::POISSONS_RATIO,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucDirichlet( fem::Property_Type::STRUC_DIRICHLET,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucNeumann( fem::Property_Type::STRUC_NEUMANN,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 10.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tDiffNeumann( fem::Property_Type::TEMP_NEUMANN,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
//                 Cell< Cell< MSI::Dof_Type > >( 0 ),
//                 {{{ 5.0 }}},
//                 tConstValFunction,
//                 Cell< fem::PropertyFunc >( 0 ) );
//
//        // create property user defined info
//        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 4 );
//        tPropertyUserDefinedInfo( 0 ).resize( 1 );
//        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 3 );
//        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 2 ) = tConductivity;
//        tPropertyUserDefinedInfo( 1 ).resize( 1 );
//        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 3 );
//        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 2 ) = tConductivity;
//        tPropertyUserDefinedInfo( 2 ).resize( 1 );
//        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 5 );
//        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 2 ) = tStrucDirichlet;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 3 ) = tConductivity;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 4 ) = tTempDirichlet;
//        tPropertyUserDefinedInfo( 3 ).resize( 1 );
//        tPropertyUserDefinedInfo( 3 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 3 )( 0 )( 0 ) = tStrucNeumann;
//        tPropertyUserDefinedInfo( 3 )( 0 )( 1 ) = tDiffNeumann;
//
//        fem::Constitutive_User_Defined_Info tStrucLinIso( fem::Constitutive_Type::STRUC_LIN_ISO,
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY  }},
//                { fem::Property_Type::YOUNGS_MODULUS, fem::Property_Type::POISSONS_RATIO } );
//
//        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::CONDUCTIVITY } );
//
//        // create constitutive user defined info
//        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 4 );
//        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 0 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 1 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 2 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 3 ).resize( 1 );
//
//        // create a list of active block-sets
//        std::string tInterfaceSideSetName = tEnrIntegMesh.get_interface_side_set_name( 0, 0, 1 );
//
//        // create a list of active block-sets
//        moris::Cell< moris_index >  tSetList = { tEnrIntegMesh.get_set_index_by_name("HMR_dummy_c_p1"),
//                tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p1"),
//                tEnrIntegMesh.get_set_index_by_name("SideSet_4_n_p1"),
//                tEnrIntegMesh.get_set_index_by_name("SideSet_2_n_p1")};
//
//        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
//                fem::Element_Type::BULK,
//                fem::Element_Type::SIDESET,
//                fem::Element_Type::SIDESET};
//
//        uint tBSplineMeshIndex = 0;
//        // create model
//        mdl::Model * tModel = new mdl::Model( &tMeshManager,
//                tBSplineMeshIndex,
//                tSetList, tSetTypeList,
//                tIWGUserDefinedInfo,
//                tPropertyUserDefinedInfo,
//                tConstitutiveUserDefinedInfo );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 1: create linear solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        moris::Cell< enum MSI::Dof_Type > tDofTypesT( 1 );            tDofTypesT( 0 ) = MSI::Dof_Type::TEMP;
//        moris::Cell< enum MSI::Dof_Type > tDofTypesU( 2 );            tDofTypesU( 0 ) = MSI::Dof_Type::UX;              tDofTypesU( 1 ) = MSI::Dof_Type::UY;
//
//        dla::Solver_Factory  tSolFactory;
//        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );
//
//        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_max_iter") = 1000;
//        tLinearSolverAlgorithm->set_param("AZ_solver") = AZ_gmres;
//        tLinearSolverAlgorithm->set_param("AZ_subdomain_solve") = AZ_ilu;
//        tLinearSolverAlgorithm->set_param("AZ_graph_fill") = 5;
////        tLinearSolverAlgorithm->set_param("Use_ML_Prec") = true;
//
//        dla::Linear_Solver tLinSolver;
//        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 2: create nonlinear solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        NLA::Nonlinear_Problem * tNonlinearProblem =  new NLA::Nonlinear_Problem( tModel->get_solver_interface(), 0, true, MapType::Epetra );
//        //    NLA::Nonlinear_Problem * tNonlinearProblem =  new NLA::Nonlinear_Problem( tModel->get_solver_interface(), 0, true, MapType::Petsc );
//
//        NLA::Nonlinear_Solver_Factory tNonlinFactory;
//        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMain = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
//        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMonolythicT = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
//        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMonolythicU = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
//
//        tNonlinearSolverAlgorithmMonolythicT->set_param("NLA_max_iter")   = 2;
//        tNonlinearSolverAlgorithmMonolythicU->set_param("NLA_max_iter")   = 2;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_hard_break") = false;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_max_lin_solver_restarts") = 2;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_rebuild_jacobian") = true;
//
//        tNonlinearSolverAlgorithmMonolythicT->set_linear_solver( &tLinSolver );
//        tNonlinearSolverAlgorithmMonolythicU->set_linear_solver( &tLinSolver );
//
//        NLA::Nonlinear_Solver tNonlinearSolverMain;
//        NLA::Nonlinear_Solver tNonlinearSolverMonolythicT;
//        NLA::Nonlinear_Solver tNonlinearSolverMonolythicU;
//        tNonlinearSolverMonolythicT.set_nonlinear_algorithm( tNonlinearSolverAlgorithmMonolythicT, 0 );
//        tNonlinearSolverMonolythicU.set_nonlinear_algorithm( tNonlinearSolverAlgorithmMonolythicU, 0 );
//
//        tNonlinearSolverMain.set_dof_type_list( tDofTypesT );
//        tNonlinearSolverMain.set_dof_type_list( tDofTypesU );
//        tNonlinearSolverMonolythicT.set_dof_type_list( tDofTypesT );
//        tNonlinearSolverMonolythicU.set_dof_type_list( tDofTypesU );
//
//        tNonlinearSolverMonolythicU.set_secondiry_dof_type_list( tDofTypesT );
//
//        tNonlinearSolverMain.set_sub_nonlinear_solver( &tNonlinearSolverMonolythicT );
//        tNonlinearSolverMain.set_sub_nonlinear_solver( &tNonlinearSolverMonolythicU );
//
//        // Create solver database
//        NLA::SOL_Warehouse tSolverWarehouse( tModel->get_solver_interface() );
//
//        tNonlinearSolverMain.set_solver_warehouse( &tSolverWarehouse );
//        tNonlinearSolverMonolythicT.set_solver_warehouse( &tSolverWarehouse );
//        tNonlinearSolverMonolythicU.set_solver_warehouse( &tSolverWarehouse );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 3: create time Solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        tsa::Time_Solver_Factory tTimeSolverFactory;
//        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );
//
//        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolverMain );
//
//        tsa::Time_Solver tTimeSolver;
//        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );
//        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );
//
//        tTimeSolver.set_dof_type_list( tDofTypesT );
//        tTimeSolver.set_dof_type_list( tDofTypesU );
//
//        //------------------------------------------------------------------------------
//        tTimeSolver.solve();
//
//        Matrix<DDRMat> tFullSol;
//        tTimeSolver.get_full_solution(tFullSol);
//
//        print( tFullSol, "tFullSol");
//
//
//
////        // output solution and meshes
////        xtk::Output_Options tOutputOptions;
////        tOutputOptions.mAddNodeSets = false;
////        tOutputOptions.mAddSideSets = false;
////        tOutputOptions.mAddClusters = false;
////
////        // add solution field to integration mesh
////        std::string tIntegSolFieldNameUX = "UX";
////        std::string tIntegSolFieldNameUY = "UY";
////        std::string tIntegSolFieldNameTEMP = "TEMP";
////        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldNameUX, tIntegSolFieldNameUY, tIntegSolFieldNameTEMP};
////
////        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);
////
////        // Write to Integration mesh for visualization
////        Matrix<DDRMat> tIntegSolUX = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UX );
////        Matrix<DDRMat> tIntegSolUY = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UY );
////        Matrix<DDRMat> tIntegSolTEMP = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::TEMP );
////
////        Matrix<DDRMat> tSTKIntegSolUX(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
////        Matrix<DDRMat> tSTKIntegSolUY(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
////        Matrix<DDRMat> tSTKIntegSolTEMP(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
////
////        for(moris::uint i = 0; i < tIntegMesh1->get_num_entities(EntityRank::NODE); i++)
////        {
////            moris::moris_id tID = tIntegMesh1->get_glb_entity_id_from_entity_loc_index(i,EntityRank::NODE);
////            tSTKIntegSolUX(i) = tIntegSolUX(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
////            tSTKIntegSolUY(i) = tIntegSolUY(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
////            tSTKIntegSolTEMP(i) = tIntegSolTEMP(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
////        }
////
////        // add solution field to integration mesh
////        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUX,EntityRank::NODE,tSTKIntegSolUX);
////        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUY,EntityRank::NODE,tSTKIntegSolUY);
////        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameTEMP,EntityRank::NODE,tSTKIntegSolTEMP);
////
////        std::string tMeshOutputFile = "./mdl_exo/stk_xtk_thermoelastic.e";
////
////        tIntegMesh1->create_output_mesh(tMeshOutputFile);
////
////        delete tIntegMesh1;
//
//        delete tModel;
//    }
}

TEST_CASE("2D XTK WITH HMR ThermoElastic 2D","[XTK_HMR_thermoelastic_2D]")
{
    if(par_size()<=1)
    {
//        uint tLagrangeMeshIndex = 0;
//        std::string tFieldName = "Cylinder";
//
//        hmr::ParameterList tParameters = hmr::create_hmr_parameter_list();
//
//        tParameters.set( "number_of_elements_per_dimension", "2, 1");
//        tParameters.set( "domain_dimensions", "2, 2" );
//        tParameters.set( "domain_offset", "-1.0, -1.0" );
//        tParameters.set( "domain_sidesets", "1,2,3,4" );
//        tParameters.set( "lagrange_output_meshes", "0" );
//
//        tParameters.set( "lagrange_orders", "1" );
//        tParameters.set( "lagrange_pattern", "0" );
//        tParameters.set( "bspline_orders", "1" );
//        tParameters.set( "bspline_pattern", "0" );
//
//        tParameters.set( "lagrange_to_bspline", "0" );
//
//        tParameters.set( "truncate_bsplines", 1 );
//        tParameters.set( "refinement_buffer", 3 );
//        tParameters.set( "staircase_buffer", 3 );
//        tParameters.set( "initial_refinement", 0 );
//
//        tParameters.set( "use_multigrid", 0 );
//        tParameters.set( "severity_level", 2 );
//
//        hmr::HMR tHMR( tParameters );
//
//        // initial refinement
//        tHMR.perform_initial_refinement( 0 );
//
//        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );
//
//        //// create field
//        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );
//
//        tField->evaluate_scalar_function( LvlSetCircle_2D );
//        //
//        // for( uint k=0; k<2; ++k )
//        // {
//            // tHMR.flag_surface_elements_on_working_pattern( tField );
//            // tHMR.perform_refinement_based_on_working_pattern( 0 );
//
//            // tField->evaluate_scalar_function( LvlSetCircle_2D );
//        // }
//
//        tHMR.finalize();
//
////        // tHMR.save_to_exodus( 0, "./xtk_exo/mdl_xtk_hmr_2d.e" );
//
//        std::shared_ptr< moris::hmr::Interpolation_Mesh_HMR > tInterpolationMesh = tHMR.create_interpolation_mesh(tLagrangeMeshIndex);
//
//        xtk::Circle tCircle( 0.2501, 2.0, 0.0 );
//
//        xtk::Phase_Table tPhaseTable (1,  Phase_Table_Structure::EXP_BASE_2);
//        xtk::Geometry_Engine tGeometryEngine(tCircle,tPhaseTable, 2);
//
//        xtk::Model tXTKModel(2, tInterpolationMesh.get(), tGeometryEngine);
//
//        tXTKModel.mVerbose = false;
//
//        //Specify decomposition Method and Cut Mesh ---------------------------------------
//        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
//        tXTKModel.decompose(tDecompositionMethods);
//
//        tXTKModel.perform_basis_enrichment(EntityRank::NODE,0);
//
//        // get meshes
//        xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
//        xtk::Enriched_Integration_Mesh   & tEnrIntegMesh = tXTKModel.get_enriched_integ_mesh();
//
//        // place the pair in mesh manager
//        mtk::Mesh_Manager tMeshManager;
//        tMeshManager.register_mesh_pair(&tEnrInterpMesh, &tEnrIntegMesh);
//
//        uint tSpatialDimension = 2;

        //------------------------------------------------------------------------------
        // create the properties
//        std::shared_ptr< fem::Property > tPropConductivity1 = std::make_shared< fem::Property >();
//        tPropConductivity1->set_parameters( { {{ 1.0 }} } );
//        tPropConductivity1->set_val_function( tConstValFunction2MatMDL );
//
//        std::shared_ptr< fem::Property > tPropConductivity2 = std::make_shared< fem::Property >();
//        tPropConductivity2->set_parameters( { {{ 5.0 }} } );
//        tPropConductivity2->set_val_function( tConstValFunction2MatMDL );
//
//        std::shared_ptr< fem::Property > tPropDirichletTEMP = std::make_shared< fem::Property >();
//        tPropDirichlet->set_parameters( { {{ 5.0 }} } );
//        tPropDirichlet->set_val_function( tConstValFunction2MatMDL );
//
//        std::shared_ptr< fem::Property > tPropNeumannTEMP = std::make_shared< fem::Property >();
//        tPropNeumann->set_parameters( { {{ 20.0 }} } );
//        tPropNeumann->set_val_function( tConstValFunction2MatMDL );
//
//        std::shared_ptr< fem::Property > tPropEMod1 = std::make_shared< fem::Property >();
//        tPropEMod1->set_parameters( { {{ 1.0 }} } );
//        tPropEMod1->set_val_function( tConstValFunction );
//
//        std::shared_ptr< fem::Property > tPropEMod2 = std::make_shared< fem::Property >();
//        tPropEMod2->set_parameters( { {{ 1.0 }} } );
//        tPropEMod2->set_val_function( tConstValFunction );
//
//        std::shared_ptr< fem::Property > tPropPoisson = std::make_shared< fem::Property >();
//        tPropNeumann->set_parameters( { {{ 0.0 }} } );
//        tPropNeumann->set_val_function( tConstValFunction2MatMDL );
//
//        std::shared_ptr< fem::Property > tPropDirichletUX = std::make_shared< fem::Property >();
//        tPropDirichletUX->set_parameters( { {{ 0.0 }} } );
//        tPropDirichletUX->set_val_function( tConstValFunction );
//        tPropDirichletUX->set_dof_type( MSI::Dof_Type::UX );
//
//        std::shared_ptr< fem::Property > tPropDirichletUY = std::make_shared< fem::Property >();
//        tPropDirichletUY->set_parameters( { {{ 0.0 }} } );  // specify UY displacement
//        tPropDirichletUY->set_val_function( tConstValFunction );
//        tPropDirichletUY->set_dof_type( MSI::Dof_Type::UY );
//
//        std::shared_ptr< fem::Property > tCTE = std::make_shared< fem::Property >();
//        tPropNeumann->set_parameters( { {{ 0.0 }} } );
//        tPropNeumann->set_val_function( tConstValFunction2MatMDL );
//
//        std::shared_ptr< fem::Property > tTRef = std::make_shared< fem::Property >();
//        tPropNeumann->set_parameters( { {{ 1.0 }} } );
//        tPropNeumann->set_val_function( tConstValFunction2MatMDL );
//
//        // define constitutive models
//        fem::CM_Factory tCMFactory;
//
//        std::shared_ptr< fem::Constitutive_Model > tCMDiffLinIso1 = tCMFactory.create_CM( fem::Constitutive_Type::DIFF_LIN_ISO );
//        tCMDiffLinIso1->set_dof_type_list( {{ MSI::Dof_Type::TEMP }} );
//        tCMDiffLinIso1->set_properties( { tPropConductivity1 } );
//        tCMDiffLinIso1->set_space_dim( 2 );
//
//        std::shared_ptr< fem::Constitutive_Model > tCMDiffLinIso2 = tCMFactory.create_CM( fem::Constitutive_Type::DIFF_LIN_ISO );
//        tCMDiffLinIso2->set_dof_type_list( {{ MSI::Dof_Type::TEMP }} );
//        tCMDiffLinIso2->set_properties( { tPropConductivity2 } );
//        tCMDiffLinIso2->set_space_dim( 2 );
//
//        std::shared_ptr< fem::Constitutive_Model > tCMStrucLinIso1 = tCMFactory.create_CM( fem::Constitutive_Type::STRUC_LIN_ISO );
//        tCMStrucLinIso1->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
//        tCMStrucLinIso1->set_properties( { tPropEMod1, tPropNu } );
//        tCMStrucLinIso1->set_space_dim( 2 );
//
//        std::shared_ptr< fem::Constitutive_Model > tCMStrucLinIso2 = tCMFactory.create_CM( fem::Constitutive_Type::STRUC_LIN_ISO );
//        tCMStrucLinIso2->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
//        tCMStrucLinIso2->set_properties( { tPropEMod2, tPropNu } );
//        tCMStrucLinIso2->set_space_dim( 2 );

//        // create IWG user defined info
//        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 4 );
//        tIWGUserDefinedInfo( 0 ).resize( 2 );
//        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_BULK,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 0 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 1 ).resize( 2 );
//        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_BULK,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 1 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 2 ).resize( 2 );
//        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_DIRICHLET,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }},
//                { fem::Property_Type::STRUC_DIRICHLET },
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 2 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::TEMP_DIRICHLET },
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 3 ).resize( 2 );
//        tIWGUserDefinedInfo( 3 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_NEUMANN,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }},
//                { fem::Property_Type::STRUC_NEUMANN },
//                moris::Cell< fem::Constitutive_Type >( 0 ) );
//        tIWGUserDefinedInfo( 3 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::TEMP_NEUMANN },
//                moris::Cell< fem::Constitutive_Type >( 0 ) );
//
//        // create property user defined info
//        fem::Property_User_Defined_Info tYoungs_Modulus( fem::Property_Type::YOUNGS_MODULUS,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{ { 1.0} } },
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tPoissons_Ratio( fem::Property_Type::POISSONS_RATIO,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucDirichlet( fem::Property_Type::STRUC_DIRICHLET,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucNeumann( fem::Property_Type::STRUC_NEUMANN,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tDiffNeumann( fem::Property_Type::TEMP_NEUMANN,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
//                 Cell< Cell< MSI::Dof_Type > >( 0 ),
//                 {{{ 2.0 }}},
//                 tConstValFunction,
//                 Cell< fem::PropertyFunc >( 0 ) );
//
//        fem::Property_User_Defined_Info tCTE( fem::Property_Type::CTE,
//                 Cell< Cell< MSI::Dof_Type > >( 0 ),
//                 {{{ 0.0 }}},
//                 tConstValFunction,
//                 Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tTRef( fem::Property_Type::TREF,
//                 Cell< Cell< MSI::Dof_Type > >( 0 ),
//                 {{{ 1.0 }}},
//                 tConstValFunction,
//                 Cell< fem::PropertyFunc >( 0 ) );
//
//        // create property user defined info
//        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 4 );
//        tPropertyUserDefinedInfo( 0 ).resize( 1 );
//        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 5 );
//        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 2 ) = tCTE;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 3 ) = tTRef;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 4 ) = tConductivity;
//        tPropertyUserDefinedInfo( 1 ).resize( 1 );
//        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 5 );
//        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 2 ) = tCTE;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 3 ) = tTRef;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 4 ) = tConductivity;
//        tPropertyUserDefinedInfo( 2 ).resize( 1 );
//        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 7 );
//        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 2 ) = tStrucDirichlet;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 3 ) = tCTE;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 4 ) = tTRef;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 5 ) = tConductivity;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 6 ) = tTempDirichlet;
//        tPropertyUserDefinedInfo( 3 ).resize( 1 );
//        tPropertyUserDefinedInfo( 3 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 3 )( 0 )( 0 ) = tStrucNeumann;
//        tPropertyUserDefinedInfo( 3 )( 0 )( 1 ) = tDiffNeumann;
//
//        fem::Constitutive_User_Defined_Info tStrucLinIso( fem::Constitutive_Type::STRUC_LIN_ISO,
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY },{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::YOUNGS_MODULUS, fem::Property_Type::POISSONS_RATIO, fem::Property_Type::CTE, fem::Property_Type::TREF } );
//
//        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::CONDUCTIVITY } );
//
//        // create constitutive user defined info
//        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 4 );
//        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 0 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 1 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 2 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 3 ).resize( 1 );
//
//        // create a list of active block-sets
//        std::string tInterfaceSideSetName = tEnrIntegMesh.get_interface_side_set_name( 0, 0, 1 );
//
//        // create a list of active block-sets
//        moris::Cell< moris_index >  tSetList = { tEnrIntegMesh.get_set_index_by_name("HMR_dummy_c_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name ("SideSet_4_n_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name ("SideSet_2_n_p1")};
//
//        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::SIDESET,
//                                                          fem::Element_Type::SIDESET};
//
//        uint tBSplineMeshIndex = 0;
//        // create model
//        mdl::Model * tModel = new mdl::Model( &tMeshManager,
//                tBSplineMeshIndex,
//                tSetList, tSetTypeList,
//                tIWGUserDefinedInfo,
//                tPropertyUserDefinedInfo,
//                tConstitutiveUserDefinedInfo );
//
//        moris::Cell< enum MSI::Dof_Type > tDofTypesT( 1 );            tDofTypesT( 0 ) = MSI::Dof_Type::TEMP;
//        moris::Cell< enum MSI::Dof_Type > tDofTypesU( 2 );            tDofTypesU( 0 ) = MSI::Dof_Type::UX;              tDofTypesU( 1 ) = MSI::Dof_Type::UY;
//
//        dla::Solver_Factory  tSolFactory;
//        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );
//
//        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_max_iter") = 10000;
//        tLinearSolverAlgorithm->set_param("AZ_solver") = AZ_gmres;
//        tLinearSolverAlgorithm->set_param("AZ_subdomain_solve") = AZ_ilu;
//        tLinearSolverAlgorithm->set_param("AZ_graph_fill") = 10;
////        tLinearSolverAlgorithm->set_param("Use_ML_Prec") = true;
//
//        dla::Linear_Solver tLinSolver;
//        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 2: create nonlinear solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        NLA::Nonlinear_Solver_Factory tNonlinFactory;
//        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
////        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMonolythicU = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
//
//        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 3;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_hard_break") = false;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_max_lin_solver_restarts") = 2;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_rebuild_jacobian") = true;
//
//        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );
////        tNonlinearSolverAlgorithmMonolythicU->set_linear_solver( &tLinSolver );
//
//        NLA::Nonlinear_Solver tNonlinearSolverMain;
//        tNonlinearSolverMain.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );
//
//
//        tNonlinearSolverMain       .set_dof_type_list( tDofTypesU );
//        tNonlinearSolverMain       .set_dof_type_list( tDofTypesT );
//
//        // Create solver database
//        NLA::SOL_Warehouse tSolverWarehouse( tModel->get_solver_interface() );
//
//        tNonlinearSolverMain       .set_solver_warehouse( &tSolverWarehouse );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 3: create time Solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        tsa::Time_Solver_Factory tTimeSolverFactory;
//        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );
//
//        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolverMain );
//
//        tsa::Time_Solver tTimeSolver;
//        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );
//        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );
//
//        tTimeSolver.set_dof_type_list( tDofTypesU );
//        tTimeSolver.set_dof_type_list( tDofTypesT );
//
//        //------------------------------------------------------------------------------
//        tTimeSolver.solve();
//
//        //        Matrix<DDRMat> tFullSol;
////        tNonlinearProblem->get_full_vector()->print();
//
//        // output solution and meshes
//        xtk::Output_Options tOutputOptions;
//        tOutputOptions.mAddNodeSets = false;
//        tOutputOptions.mAddSideSets = false;
//        tOutputOptions.mAddClusters = false;
//
//        // add solution field to integration mesh
//        std::string tIntegSolFieldNameUX = "UX";
//        std::string tIntegSolFieldNameUY = "UY";
//        std::string tIntegSolFieldNameTEMP = "TEMP";
//        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldNameUX, tIntegSolFieldNameUY, tIntegSolFieldNameTEMP};
//
//        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);
//
//        // Write to Integration mesh for visualization
//        Matrix<DDRMat> tIntegSolUX = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UX );
//        Matrix<DDRMat> tIntegSolUY = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UY );
//        Matrix<DDRMat> tIntegSolTEMP = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::TEMP );
//
//        //    print(tIntegSolUX,"tIntegSolUX");
//        //    print(tIntegSolUY,"tIntegSolUY");
//
//        Matrix<DDRMat> tSTKIntegSolUX(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//        Matrix<DDRMat> tSTKIntegSolUY(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//        Matrix<DDRMat> tSTKIntegSolTEMP(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//
//        for(moris::uint i = 0; i < tIntegMesh1->get_num_entities(EntityRank::NODE); i++)
//        {
//            moris::moris_id tID = tIntegMesh1->get_glb_entity_id_from_entity_loc_index(i,EntityRank::NODE);
//            tSTKIntegSolUX(i) = tIntegSolUX(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//            tSTKIntegSolUY(i) = tIntegSolUY(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//            tSTKIntegSolTEMP(i) = tIntegSolTEMP(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//        }
//
//        // add solution field to integration mesh
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUX,EntityRank::NODE,tSTKIntegSolUX);
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUY,EntityRank::NODE,tSTKIntegSolUY);
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameTEMP,EntityRank::NODE,tSTKIntegSolTEMP);
//
//        //    Matrix<DDRMat> tFullSol;
//        //    tNonlinearSolver.get_full_solution(tFullSol);
//        //
//        //    print(tFullSol,"tFullSol");
//
//        std::string tMeshOutputFile = "./mdl_exo/stk_xtk_thermoelastic_2D.e";
//
//        tIntegMesh1->create_output_mesh(tMeshOutputFile);
//
//        delete tIntegMesh1;
//
//        delete tModel;
    }
}

TEST_CASE("2D XTK WITH HMR ThermoElastic 2D Staggered","[XTK_HMR_thermoelastic_2D_staggered]")
{
//    if(par_size()<=1)
//    {
//        uint tLagrangeMeshIndex = 0;
//        std::string tFieldName = "Cylinder";
//
//        hmr::ParameterList tParameters = hmr::create_hmr_parameter_list();
//
//        tParameters.set( "number_of_elements_per_dimension", "20, 20");
//        tParameters.set( "domain_dimensions", "2, 2" );
//        tParameters.set( "domain_offset", "-1.0, -1.0" );
//        tParameters.set( "domain_sidesets", "1,2,3,4" );
//        tParameters.set( "lagrange_output_meshes", "0" );
//
//        tParameters.set( "lagrange_orders", "1" );
//        tParameters.set( "lagrange_pattern", "0" );
//        tParameters.set( "bspline_orders", "1" );
//        tParameters.set( "bspline_pattern", "0" );
//
//        tParameters.set( "lagrange_to_bspline", "0" );
//
//        tParameters.set( "truncate_bsplines", 1 );
//        tParameters.set( "refinement_buffer", 3 );
//        tParameters.set( "staircase_buffer", 3 );
//        tParameters.set( "initial_refinement", 0 );
//
//        tParameters.set( "use_multigrid", 0 );
//        tParameters.set( "severity_level", 2 );
//
//        hmr::HMR tHMR( tParameters );
//
//        // initial refinement
//        tHMR.perform_initial_refinement( 0 );
//
//        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );
//
//        //// create field
//        std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );
//
//        tField->evaluate_scalar_function( LvlSetCircle_2D );
//        //
////        for( uint k=0; k<2; ++k )
////        {
////            tHMR.flag_surface_elements_on_working_pattern( tField );
////            tHMR.perform_refinement_based_on_working_pattern( 0 );
////
////            tField->evaluate_scalar_function( LvlSetCircle_2D );
////        }
//
//        tHMR.finalize();
//
//        tHMR.save_to_exodus( 0, "./xtk_exo/mdl_xtk_hmr_2d.e" );
//
//        std::shared_ptr< moris::hmr::Interpolation_Mesh_HMR > tInterpolationMesh = tHMR.create_interpolation_mesh(tLagrangeMeshIndex);
//
//        xtk::Circle tCircle( 0.2501, 2.0, 0.0 );
//
//        xtk::Phase_Table tPhaseTable (1,  Phase_Table_Structure::EXP_BASE_2);
//        xtk::Geometry_Engine tGeometryEngine(tCircle,tPhaseTable, 2);
//
//        xtk::Model tXTKModel(2, tInterpolationMesh.get(), tGeometryEngine);
//
//        tXTKModel.mVerbose = false;
//
//        //Specify decomposition Method and Cut Mesh ---------------------------------------
//        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
//        tXTKModel.decompose(tDecompositionMethods);
//
//        tXTKModel.perform_basis_enrichment(EntityRank::NODE,0);
//
//        // get meshes
//        xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
//        xtk::Enriched_Integration_Mesh   & tEnrIntegMesh = tXTKModel.get_enriched_integ_mesh();
//
//        // place the pair in mesh manager
//        mtk::Mesh_Manager tMeshManager;
//        tMeshManager.register_mesh_pair(&tEnrInterpMesh, &tEnrIntegMesh);
//
//        uint tSpatialDimension = 2;
//
//        // create IWG user defined info
//        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 4 );
//        tIWGUserDefinedInfo( 0 ).resize( 2 );
//        tIWGUserDefinedInfo( 0 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_BULK,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY },{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 0 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 1 ).resize( 2 );
//        tIWGUserDefinedInfo( 1 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_BULK,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY },{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 1 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_BULK,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                moris::Cell< fem::Property_Type >( 0 ),
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 2 ).resize( 2 );
//        tIWGUserDefinedInfo( 2 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_DIRICHLET,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY },{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::STRUC_DIRICHLET },
//                { fem::Constitutive_Type::STRUC_LIN_ISO } );
//        tIWGUserDefinedInfo( 2 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_DIRICHLET,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::TEMP_DIRICHLET },
//                { fem::Constitutive_Type::DIFF_LIN_ISO } );
//
//        tIWGUserDefinedInfo( 3 ).resize( 2 );
//        tIWGUserDefinedInfo( 3 )( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_NEUMANN,
//                { MSI::Dof_Type::UX, MSI::Dof_Type::UY },
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY },{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::STRUC_NEUMANN },
//                moris::Cell< fem::Constitutive_Type >( 0 ) );
//        tIWGUserDefinedInfo( 3 )( 1 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::SPATIALDIFF_NEUMANN,
//                { MSI::Dof_Type::TEMP },
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::TEMP_NEUMANN },
//                moris::Cell< fem::Constitutive_Type >( 0 ) );
//
//        // create property user defined info
//        fem::Property_User_Defined_Info tYoungs_Modulus( fem::Property_Type::YOUNGS_MODULUS,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{ { 1.0 } } },
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tPoissons_Ratio( fem::Property_Type::POISSONS_RATIO,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucDirichlet( fem::Property_Type::STRUC_DIRICHLET,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucNeumann( fem::Property_Type::STRUC_NEUMANN,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tDiffNeumann( fem::Property_Type::TEMP_NEUMANN,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tConductivity( fem::Property_Type::CONDUCTIVITY,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tTempDirichlet( fem::Property_Type::TEMP_DIRICHLET,
//                 Cell< Cell< MSI::Dof_Type > >( 0 ),
//                 {{{ 2.0 }}},
//                 tConstValFunction,
//                 Cell< fem::PropertyFunc >( 0 ) );
//
//        fem::Property_User_Defined_Info tCTE( fem::Property_Type::CTE,
//                 Cell< Cell< MSI::Dof_Type > >( 0 ),
//                 {{{ 0.0 }}},
//                 tConstValFunction,
//                 Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tTRef( fem::Property_Type::TREF,
//                 Cell< Cell< MSI::Dof_Type > >( 0 ),
//                 {{{ 1.0 }}},
//                 tConstValFunction,
//                 Cell< fem::PropertyFunc >( 0 ) );
//
//        // create property user defined info
//        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 4 );
//        tPropertyUserDefinedInfo( 0 ).resize( 1 );
//        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 5 );
//        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 2 ) = tCTE;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 3 ) = tTRef;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 4 ) = tConductivity;
//        tPropertyUserDefinedInfo( 1 ).resize( 1 );
//        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 5 );
//        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 2 ) = tCTE;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 3 ) = tTRef;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 4 ) = tConductivity;
//        tPropertyUserDefinedInfo( 2 ).resize( 1 );
//        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 7 );
//        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 2 ) = tStrucDirichlet;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 3 ) = tCTE;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 4 ) = tTRef;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 5 ) = tConductivity;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 6 ) = tTempDirichlet;
//        tPropertyUserDefinedInfo( 3 ).resize( 1 );
//        tPropertyUserDefinedInfo( 3 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 3 )( 0 )( 0 ) = tStrucNeumann;
//        tPropertyUserDefinedInfo( 3 )( 0 )( 1 ) = tDiffNeumann;
//
//        fem::Constitutive_User_Defined_Info tStrucLinIso( fem::Constitutive_Type::STRUC_LIN_ISO,
//                {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY },{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::YOUNGS_MODULUS, fem::Property_Type::POISSONS_RATIO, fem::Property_Type::CTE, fem::Property_Type::TREF } );
//
//        fem::Constitutive_User_Defined_Info tDiffLinIso( fem::Constitutive_Type::DIFF_LIN_ISO,
//                {{ MSI::Dof_Type::TEMP }},
//                { fem::Property_Type::CONDUCTIVITY } );
//
//        // create constitutive user defined info
//        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 4 );
//        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 0 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 1 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 2 )( 0 )( 1 ) = tDiffLinIso;
//        tConstitutiveUserDefinedInfo( 3 ).resize( 1 );
//
//        // create a list of active block-sets
//        std::string tInterfaceSideSetName = tEnrIntegMesh.get_interface_side_set_name( 0, 0, 1 );
//
//        // create a list of active block-sets
//        moris::Cell< moris_index >  tSetList = { tEnrIntegMesh.get_set_index_by_name("HMR_dummy_c_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name ("SideSet_4_n_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name ("SideSet_2_n_p1")};
//
//        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::SIDESET,
//                                                          fem::Element_Type::SIDESET};
//
//        uint tBSplineMeshIndex = 0;
//        // create model
//        mdl::Model * tModel = new mdl::Model( &tMeshManager,
//                tBSplineMeshIndex,
//                tSetList, tSetTypeList,
//                tIWGUserDefinedInfo,
//                tPropertyUserDefinedInfo,
//                tConstitutiveUserDefinedInfo );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 1: create linear solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        moris::Cell< enum MSI::Dof_Type > tDofTypesT( 1 );            tDofTypesT( 0 ) = MSI::Dof_Type::TEMP;
//        moris::Cell< enum MSI::Dof_Type > tDofTypesU( 2 );            tDofTypesU( 0 ) = MSI::Dof_Type::UX;              tDofTypesU( 1 ) = MSI::Dof_Type::UY;
//
//        dla::Solver_Factory  tSolFactory;
//        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );
//
//        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_max_iter") = 10000;
//        tLinearSolverAlgorithm->set_param("AZ_solver") = AZ_gmres;
//        tLinearSolverAlgorithm->set_param("AZ_subdomain_solve") = AZ_ilu;
//        tLinearSolverAlgorithm->set_param("AZ_graph_fill") = 10;
////        tLinearSolverAlgorithm->set_param("Use_ML_Prec") = true;
//
//        dla::Linear_Solver tLinSolver;
//        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 2: create nonlinear solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        NLA::Nonlinear_Solver_Factory tNonlinFactory;
//        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMain = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NLBGS_SOLVER );
//        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMonolythic = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
////        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMonolythicU = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
//
//        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_max_iter")   = 3;
//        tNonlinearSolverAlgorithmMain->set_param("NLA_max_iter")   = 1;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_hard_break") = false;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_max_lin_solver_restarts") = 2;
//        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_rebuild_jacobian") = true;
//
//        tNonlinearSolverAlgorithmMonolythic->set_linear_solver( &tLinSolver );
////        tNonlinearSolverAlgorithmMonolythicU->set_linear_solver( &tLinSolver );
//
//        NLA::Nonlinear_Solver tNonlinearSolverMain;
//        NLA::Nonlinear_Solver tNonlinearSolverMonolythicT;
//        NLA::Nonlinear_Solver tNonlinearSolverMonolythicU;
//        tNonlinearSolverMain       .set_nonlinear_algorithm( tNonlinearSolverAlgorithmMain, 0 );
//        tNonlinearSolverMonolythicT.set_nonlinear_algorithm( tNonlinearSolverAlgorithmMonolythic, 0 );
//        tNonlinearSolverMonolythicU.set_nonlinear_algorithm( tNonlinearSolverAlgorithmMonolythic, 0 );
//
//        tNonlinearSolverMain       .set_dof_type_list( tDofTypesU );
//        tNonlinearSolverMain       .set_dof_type_list( tDofTypesT );
//        tNonlinearSolverMonolythicT.set_dof_type_list( tDofTypesT );
//        tNonlinearSolverMonolythicU.set_dof_type_list( tDofTypesU );
//
//        tNonlinearSolverMonolythicU.set_secondiry_dof_type_list(tDofTypesT);
//
//        tNonlinearSolverMain.set_sub_nonlinear_solver( &tNonlinearSolverMonolythicT );
//        tNonlinearSolverMain.set_sub_nonlinear_solver( &tNonlinearSolverMonolythicU );
//
//        // Create solver database
//        NLA::SOL_Warehouse tSolverWarehouse( tModel->get_solver_interface() );
//
//        tNonlinearSolverMain       .set_solver_warehouse( &tSolverWarehouse );
//        tNonlinearSolverMonolythicT.set_solver_warehouse( &tSolverWarehouse );
//        tNonlinearSolverMonolythicU.set_solver_warehouse( &tSolverWarehouse );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 3: create time Solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        tsa::Time_Solver_Factory tTimeSolverFactory;
//        std::shared_ptr< tsa::Time_Solver_Algorithm > tTimeSolverAlgorithm = tTimeSolverFactory.create_time_solver( tsa::TimeSolverType::MONOLITHIC );
//
//        tTimeSolverAlgorithm->set_nonlinear_solver( &tNonlinearSolverMain );
//
//        tsa::Time_Solver tTimeSolver;
//        tTimeSolver.set_time_solver_algorithm( tTimeSolverAlgorithm );
//        tTimeSolver.set_solver_warehouse( &tSolverWarehouse );
//
//        tTimeSolver.set_dof_type_list( tDofTypesU );
//        tTimeSolver.set_dof_type_list( tDofTypesT );
//
//        //------------------------------------------------------------------------------
//        tTimeSolver.solve();
//
//        Matrix<DDRMat> tFullSol;
//        tTimeSolver.get_full_solution(tFullSol);
//
////        print( tFullSol, "tFullSol");
//
//        // output solution and meshes
//        xtk::Output_Options tOutputOptions;
//        tOutputOptions.mAddNodeSets = false;
//        tOutputOptions.mAddSideSets = false;
//        tOutputOptions.mAddClusters = false;
//
//        // add solution field to integration mesh
//        std::string tIntegSolFieldNameUX = "UX";
//        std::string tIntegSolFieldNameUY = "UY";
//        std::string tIntegSolFieldNameTEMP = "TEMP";
//        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldNameUX, tIntegSolFieldNameUY, tIntegSolFieldNameTEMP};
//
//        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);
//
//        // Write to Integration mesh for visualization
//        Matrix<DDRMat> tIntegSolUX = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UX );
//        Matrix<DDRMat> tIntegSolUY = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UY );
//        Matrix<DDRMat> tIntegSolTEMP = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::TEMP );
//
//        //    print(tIntegSolUX,"tIntegSolUX");
//        //    print(tIntegSolUY,"tIntegSolUY");
//
//        Matrix<DDRMat> tSTKIntegSolUX(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//        Matrix<DDRMat> tSTKIntegSolUY(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//        Matrix<DDRMat> tSTKIntegSolTEMP(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//
//        for(moris::uint i = 0; i < tIntegMesh1->get_num_entities(EntityRank::NODE); i++)
//        {
//            moris::moris_id tID = tIntegMesh1->get_glb_entity_id_from_entity_loc_index(i,EntityRank::NODE);
//            tSTKIntegSolUX(i) = tIntegSolUX(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//            tSTKIntegSolUY(i) = tIntegSolUY(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//            tSTKIntegSolTEMP(i) = tIntegSolTEMP(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//        }
//
//        // add solution field to integration mesh
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUX,EntityRank::NODE,tSTKIntegSolUX);
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUY,EntityRank::NODE,tSTKIntegSolUY);
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameTEMP,EntityRank::NODE,tSTKIntegSolTEMP);
//
//        //    Matrix<DDRMat> tFullSol;
//        //    tNonlinearSolver.get_full_solution(tFullSol);
//        //
//        //    print(tFullSol,"tFullSol");
//
//        std::string tMeshOutputFile = "./mdl_exo/stk_xtk_thermoelastic1.e";
//
//        tIntegMesh1->create_output_mesh(tMeshOutputFile);
//
//        delete tIntegMesh1;
//
//        delete tModel;
//    }
}

//TEST_CASE("2D XTK WITH HMR Struc Interface 3D","[XTK_HMR_Struc_Interface_3D]")
//{
//    if(par_size()<=1)
//    {
//        uint tLagrangeMeshIndex = 0;
//        std::string tFieldName = "Cylinder";
//
//        uint tSpatialDimension = 3;
//
//         hmr::ParameterList tParameters = hmr::create_hmr_parameter_list();
//
//         tParameters.set( "number_of_elements_per_dimension", "22, 8, 2");
//         tParameters.set( "domain_dimensions", "6, 2, 1" );
//         tParameters.set( "domain_offset", "-3.0, -1.0, -0.5" );
//         tParameters.set( "domain_sidesets", "1,2,3,4,5,6" );
//         tParameters.set( "lagrange_output_meshes", "0" );
//
//         tParameters.set( "lagrange_orders", "1" );
//         tParameters.set( "lagrange_pattern", "0" );
//         tParameters.set( "bspline_orders", "1" );
//         tParameters.set( "bspline_pattern", "0" );
//
//         tParameters.set( "lagrange_to_bspline", "0" );
//
//         tParameters.set( "truncate_bsplines", 1 );
//         tParameters.set( "refinement_buffer", 3 );
//         tParameters.set( "staircase_buffer", 3 );
//         tParameters.set( "initial_refinement", 0 );
//
//         tParameters.set( "use_multigrid", 0 );
//         tParameters.set( "severity_level", 2 );
//
//         hmr::HMR tHMR( tParameters );
//
//        //initial refinement
//         tHMR.perform_initial_refinement( 0 );
//
//         std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );
//
//       //  create field
//       Cell<std::shared_ptr< moris::hmr::Field > > tHMRFields;
//       tHMRFields.resize(2);
//
//       // create field
//       tHMRFields(0) = tMesh->create_field( "Geom", tLagrangeMeshIndex );
//       tHMRFields(1) = tMesh->create_field( "Geom", tLagrangeMeshIndex );
//
//       tHMRFields(0)->evaluate_scalar_function( LevelSetFunction_star1 );
//       tHMRFields(1)->evaluate_scalar_function( Plane4MatMDL1 );
//
//       for( uint k=0; k<2; ++k )
//       {
//           tHMR.flag_surface_elements_on_working_pattern( tHMRFields(0) );
//           tHMR.flag_surface_elements_on_working_pattern( tHMRFields(1) );
//
//           tHMR.perform_refinement_based_on_working_pattern( 0 );
//
//           tHMRFields(0)->evaluate_scalar_function( LevelSetFunction_star1 );
//           tHMRFields(1)->evaluate_scalar_function( Plane4MatMDL1 );
//       }
//
//      tHMR.finalize();
//
////      tHMR.save_to_exodus( 0, "./xtk_exo/mdl_xtk_hmr_3d.e" );
//
//      std::shared_ptr< hmr::Interpolation_Mesh_HMR > tInterpMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex  );
//
//      xtk::Geom_Field tCircleFieldAsGeom(tHMRFields(0));
//      xtk::Geom_Field tPlaneFieldAsGeom2(tHMRFields(1));
//      moris::Cell<xtk::Geometry*> tGeometryVector = {&tCircleFieldAsGeom,&tPlaneFieldAsGeom2};
//
//      xtk::Phase_Table     tPhaseTable (tGeometryVector.size(),  Phase_Table_Structure::EXP_BASE_2);
//      xtk::Geometry_Engine tGeometryEngine(tGeometryVector,tPhaseTable,tSpatialDimension);
//      xtk::Model           tXTKModel(tSpatialDimension,tInterpMesh.get(),tGeometryEngine);
//      tXTKModel.mVerbose = false;
//
//      Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8, Subdivision_Method::C_HIERARCHY_TET4};
//      tXTKModel.decompose(tDecompositionMethods);
//      tXTKModel.perform_basis_enrichment(EntityRank::BSPLINE_1,0);
//
//      // get meshes
//      xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
//      xtk::Enriched_Integration_Mesh   & tEnrIntegMesh = tXTKModel.get_enriched_integ_mesh();
//
//      // place the pair in mesh manager
//      mtk::Mesh_Manager tMeshManager;
//      tMeshManager.register_mesh_pair(&tEnrInterpMesh, &tEnrIntegMesh);
//
//      Cell< fem::IWG_User_Defined_Info > tBulkIWG(1);
//      Cell< fem::IWG_User_Defined_Info > tDBCIWG(1);
//      Cell< fem::IWG_User_Defined_Info > tNBCIWG(1);
//      Cell< fem::IWG_User_Defined_Info > tIntIWG(1);
//
//        // create IWG user defined info
//      tBulkIWG( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_BULK,
//                                                  { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ },
//                                                  {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }},
//                                                  moris::Cell< fem::Property_Type >( 0 ),
//                                                  { fem::Constitutive_Type::STRUC_LIN_ISO } );
//
//      tDBCIWG( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_DIRICHLET,
//                                                 { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ },
//                                                 {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }},
//                                                 { fem::Property_Type::STRUC_DIRICHLET },
//                                                 { fem::Constitutive_Type::STRUC_LIN_ISO } );
//
//      tNBCIWG( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_NEUMANN,
//                                                 { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ },
//                                                 {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ },},
//                                                 { fem::Property_Type::STRUC_NEUMANN },
//                                                 moris::Cell< fem::Constitutive_Type >( 0 ) );
//      tIntIWG( 0 ) = fem::IWG_User_Defined_Info( fem::IWG_Type::STRUC_LINEAR_INTERFACE,
//                                                 { MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ },
//                                                 {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }},
//                                                 Cell< fem::Property_Type >( 0 ),
//                                                 {fem::Constitutive_Type::STRUC_LIN_ISO },
//                                                 {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }},
//                                                 Cell< fem::Property_Type >( 0 ),
//                                                 {fem::Constitutive_Type::STRUC_LIN_ISO } );
//
//        Cell< Cell< fem::IWG_User_Defined_Info > > tIWGUserDefinedInfo( 14 );
//
//        tIWGUserDefinedInfo( 0 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 1 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 2 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 3 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 4 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 5 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 6 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 7 )  = tBulkIWG;
//        tIWGUserDefinedInfo( 8 )  = tDBCIWG;
//        tIWGUserDefinedInfo( 9 )  = tNBCIWG;
//        tIWGUserDefinedInfo( 10 ) = tIntIWG;
//        tIWGUserDefinedInfo( 11 ) = tIntIWG;
//        tIWGUserDefinedInfo( 12 ) = tIntIWG;
//        tIWGUserDefinedInfo( 13 ) = tIntIWG;
//
//        // create property user defined info
//        fem::Property_User_Defined_Info tYoungs_Modulus( fem::Property_Type::YOUNGS_MODULUS,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tYoungs_Modulus2( fem::Property_Type::YOUNGS_MODULUS,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tPoissons_Ratio( fem::Property_Type::POISSONS_RATIO,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucDirichlet( fem::Property_Type::STRUC_DIRICHLET,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 0.0, 0.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//        fem::Property_User_Defined_Info tStrucNeumann( fem::Property_Type::STRUC_NEUMANN,
//                Cell< Cell< MSI::Dof_Type > >( 0 ),
//                {{{ 1.0, 0.0, 0.0 }}},
//                tConstValFunction,
//                Cell< fem::PropertyFunc >( 0 ) );
//
//        // create property user defined info
//        Cell< Cell< Cell< fem::Property_User_Defined_Info > > > tPropertyUserDefinedInfo( 14 );
//        tPropertyUserDefinedInfo( 0 ).resize( 1 );
//        tPropertyUserDefinedInfo( 0 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 0 )( 0 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 0 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 1 ).resize( 1 );
//        tPropertyUserDefinedInfo( 1 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 1 )( 0 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 1 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 2 ).resize( 1 );
//        tPropertyUserDefinedInfo( 2 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 2 )( 0 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 2 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 3 ).resize( 1 );
//        tPropertyUserDefinedInfo( 3 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 3 )( 0 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 3 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 4 ).resize( 1 );
//        tPropertyUserDefinedInfo( 4 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 4 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 4 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 5 ).resize( 1 );
//        tPropertyUserDefinedInfo( 5 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 5 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 5 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 6 ).resize( 1 );
//        tPropertyUserDefinedInfo( 6 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 6 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 6 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 7 ).resize( 1 );
//        tPropertyUserDefinedInfo( 7 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 7 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 7 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 8 ).resize( 1 );
//        tPropertyUserDefinedInfo( 8 )( 0 ).resize( 3 );
//        tPropertyUserDefinedInfo( 8 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 8 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 8 )( 0 )( 2 ) = tStrucDirichlet;
//        tPropertyUserDefinedInfo( 9 ).resize( 1 );
//        tPropertyUserDefinedInfo( 9 )( 0 ).resize( 1 );
//        tPropertyUserDefinedInfo( 9 )( 0 )( 0 ) = tStrucNeumann;
//        tPropertyUserDefinedInfo( 10 ).resize( 2 );
//        tPropertyUserDefinedInfo( 10 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 10 )( 0 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 10 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 10 )( 1 ).resize( 2 );
//        tPropertyUserDefinedInfo( 10 )( 1 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 10 )( 1 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 11 ).resize( 2 );
//        tPropertyUserDefinedInfo( 11 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 11 )( 0 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 11 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 11 )( 1 ).resize( 2 );
//        tPropertyUserDefinedInfo( 11 )( 1 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 11 )( 1 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 12 ).resize( 2 );
//        tPropertyUserDefinedInfo( 12 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 12 )( 0 )( 0 ) = tYoungs_Modulus2;
//        tPropertyUserDefinedInfo( 12 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 12 )( 1 ).resize( 2 );
//        tPropertyUserDefinedInfo( 12 )( 1 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 12 )( 1 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 13 ).resize( 2 );
//        tPropertyUserDefinedInfo( 13 )( 0 ).resize( 2 );
//        tPropertyUserDefinedInfo( 13 )( 0 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 13 )( 0 )( 1 ) = tPoissons_Ratio;
//        tPropertyUserDefinedInfo( 13 )( 1 ).resize( 2 );
//        tPropertyUserDefinedInfo( 13 )( 1 )( 0 ) = tYoungs_Modulus;
//        tPropertyUserDefinedInfo( 13 )( 1 )( 1 ) = tPoissons_Ratio;
//
//        fem::Constitutive_User_Defined_Info tStrucLinIso( fem::Constitutive_Type::STRUC_LIN_ISO,
//                                                          {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY, MSI::Dof_Type::UZ }},
//                                                          { fem::Property_Type::YOUNGS_MODULUS, fem::Property_Type::POISSONS_RATIO } );
//
//        // create constitutive user defined info
//        Cell< Cell< Cell< fem::Constitutive_User_Defined_Info > > > tConstitutiveUserDefinedInfo( 14 );
//        tConstitutiveUserDefinedInfo( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 0 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 1 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 2 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 2 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 3 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 3 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 3 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 4 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 4 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 4 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 5 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 5 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 5 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 6 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 6 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 6 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 7 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 7 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 7 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 8 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 8 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 8 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 9 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 10 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 10 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 10 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 10 )( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 10 )( 1 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 11 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 11 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 11 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 11 )( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 11 )( 1 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 12 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 12 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 12 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 12 )( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 12 )( 1 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 13 ).resize( 2 );
//        tConstitutiveUserDefinedInfo( 13 )( 0 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 13 )( 0 )( 0 ) = tStrucLinIso;
//        tConstitutiveUserDefinedInfo( 13 )( 1 ).resize( 1 );
//        tConstitutiveUserDefinedInfo( 13 )( 1 )( 0 ) = tStrucLinIso;
//
//        // create a list of active block-sets
//        // create a list of active block-sets
//        std::string tDblInterfaceSideSetName01 = tEnrIntegMesh.get_dbl_interface_side_set_name(0,1);
//        std::string tDblInterfaceSideSetName02 = tEnrIntegMesh.get_dbl_interface_side_set_name(0,2);
//        std::string tDblInterfaceSideSetName13 = tEnrIntegMesh.get_dbl_interface_side_set_name(1,3);
//        std::string tDblInterfaceSideSetName23 = tEnrIntegMesh.get_dbl_interface_side_set_name(2,3);
//
//        moris::Cell< moris_index >  tSetList = { tEnrIntegMesh.get_set_index_by_name("HMR_dummy_c_p0"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p0"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_c_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p1"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_c_p2"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p2"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_c_p3"),
//                                                 tEnrIntegMesh.get_set_index_by_name("HMR_dummy_n_p3"),
//                                                 tEnrIntegMesh.get_set_index_by_name("SideSet_4_n_p2"),
//                                                 tEnrIntegMesh.get_set_index_by_name("SideSet_2_n_p3"),
//                                                 tEnrIntegMesh.get_set_index_by_name(tDblInterfaceSideSetName01),
//                                                 tEnrIntegMesh.get_set_index_by_name(tDblInterfaceSideSetName02),
//                                                 tEnrIntegMesh.get_set_index_by_name(tDblInterfaceSideSetName13),
//                                                 tEnrIntegMesh.get_set_index_by_name(tDblInterfaceSideSetName23)};
//
//        moris::Cell< fem::Element_Type > tSetTypeList = { fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::BULK,
//                                                          fem::Element_Type::SIDESET,
//                                                          fem::Element_Type::SIDESET,
//                                                          fem::Element_Type::DOUBLE_SIDESET,
//                                                          fem::Element_Type::DOUBLE_SIDESET,
//                                                          fem::Element_Type::DOUBLE_SIDESET,
//                                                          fem::Element_Type::DOUBLE_SIDESET };
//
//        uint tBSplineMeshIndex = 0;
//        // create model
//        mdl::Model * tModel = new mdl::Model( &tMeshManager,
//                tBSplineMeshIndex,
//                tSetList, tSetTypeList,
//                tIWGUserDefinedInfo,
//                tPropertyUserDefinedInfo,
//                tConstitutiveUserDefinedInfo );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 1: create linear solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//        dla::Solver_Factory  tSolFactory;
//        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::AZTEC_IMPL );
//        //            std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( SolverType::PETSC );
//
//        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;
//        tLinearSolverAlgorithm->set_param("AZ_max_iter") = 5000;
//        tLinearSolverAlgorithm->set_param("AZ_solver") = AZ_gmres_condnum;
//        tLinearSolverAlgorithm->set_param("AZ_subdomain_solve") = AZ_ilu;
//        tLinearSolverAlgorithm->set_param("AZ_ilut_fill") = 10.0;
//        tLinearSolverAlgorithm->set_param("rel_residual") = 1e-8;
//
////            tLinearSolverAlgorithm->set_param("Use_ML_Prec") = true;
//
//        dla::Linear_Solver tLinSolver;
//
//        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );
//
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        // STEP 2: create nonlinear solver and algorithm
//        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        NLA::Nonlinear_Problem * tNonlinearProblem =  new NLA::Nonlinear_Problem( tModel->get_solver_interface(), 0, true, MapType::Epetra );
//        //    NLA::Nonlinear_Problem * tNonlinearProblem =  new NLA::Nonlinear_Problem( tModel->get_solver_interface(), 0, true, MapType::Petsc );
//
//        NLA::Nonlinear_Solver_Factory tNonlinFactory;
//        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
//
//        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 2;
//        tNonlinearSolverAlgorithm->set_param("NLA_rel_residual") = 1e-4;
//        //        tNonlinearSolverAlgorithm->set_param("NLA_hard_break") = false;
//        //        tNonlinearSolverAlgorithm->set_param("NLA_max_lin_solver_restarts") = 2;
//        //        tNonlinearSolverAlgorithm->set_param("NLA_rebuild_jacobian") = true;
//
//        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );
//
//        NLA::Nonlinear_Solver tNonlinearSolver;
//
//        tNonlinearSolver.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );
//
//        tNonlinearSolver.solve( tNonlinearProblem );
//
//        std::cout<<" Solution Vector "<<std::endl;
//        tNonlinearProblem->get_full_vector()->print();
//
//        // output solution and meshes
//        xtk::Output_Options tOutputOptions;
//        tOutputOptions.mAddNodeSets = false;
//        tOutputOptions.mAddSideSets = false;
//        tOutputOptions.mAddClusters = false;
//
//        // add solution field to integration mesh
//        std::string tIntegSolFieldNameUX = "UX";
//        std::string tIntegSolFieldNameUY = "UY";
//        std::string tIntegSolFieldNameUZ = "UZ";
//        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldNameUX, tIntegSolFieldNameUY, tIntegSolFieldNameUZ};
//
//        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);
//
//        // Write to Integration mesh for visualization
//        Matrix<DDRMat> tIntegSolUX = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UX );
//        Matrix<DDRMat> tIntegSolUY = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UY );
//        Matrix<DDRMat> tIntegSolUZ = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UZ );
//
//        Matrix<DDRMat> tSTKIntegSolUX(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//        Matrix<DDRMat> tSTKIntegSolUY(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//        Matrix<DDRMat> tSTKIntegSolUZ(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//
//        for(moris::uint i = 0; i < tIntegMesh1->get_num_entities(EntityRank::NODE); i++)
//        {
//            moris::moris_id tID = tIntegMesh1->get_glb_entity_id_from_entity_loc_index(i,EntityRank::NODE);
//            tSTKIntegSolUX(i) = tIntegSolUX(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//            tSTKIntegSolUY(i) = tIntegSolUY(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//            tSTKIntegSolUZ(i) = tIntegSolUY(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//        }
//
//        // add solution field to integration mesh
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUX,EntityRank::NODE,tSTKIntegSolUX);
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUY,EntityRank::NODE,tSTKIntegSolUY);
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUZ,EntityRank::NODE,tSTKIntegSolUZ);
//
//        std::string tMeshOutputFile = "./mdl_exo/hmr_xtk_linear_elastic_3D.e";
//
//        tIntegMesh1->create_output_mesh(tMeshOutputFile);
//
//        delete tIntegMesh1;
//
//        delete tModel;
//    }
//}
}


