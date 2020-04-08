/*
 * UT_MDL_XTK_HMR_Linear_Struc_Contact_2d.cpp
 *
 *  Created on: Jan 25, 2020
 *      Author: doble
 */

#include "catch.hpp"

#include "cl_XTK_Model.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"
#include "cl_XTK_Enriched_Interpolation_Mesh.hpp"
#include "cl_XTK_Ghost_Stabilization.hpp"
#include "cl_Geom_Field.hpp"
#include "typedefs.hpp"


#include "cl_MTK_Vertex.hpp"    //MTK
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Writer_Exodus.hpp"

#include "cl_Matrix.hpp"        //LINALG
#include "linalg_typedefs.hpp"
#include "fn_equal_to.hpp" // ALG/src

#include "cl_FEM_Element_Factory.hpp"          //FEM/INT/src
#include "cl_FEM_IWG_Factory.hpp"              //FEM/INT/src
#include "cl_FEM_CM_Factory.hpp"              //FEM/INT/src
#include "cl_FEM_SP_Factory.hpp"              //FEM/INT/src
#include "cl_FEM_Set_User_Info.hpp"              //FEM/INT/src

#include "cl_MDL_Model.hpp"

#include "cl_HMR_Mesh_Interpolation.hpp"
#include "cl_HMR.hpp"
#include "cl_HMR_Element.hpp" //HMR/src
#include "cl_HMR_Factory.hpp" //HMR/src
#include "cl_HMR_Field.hpp"
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
#include "cl_SOL_Warehouse.hpp"
#include "fn_norm.hpp"

#include "cl_GEN_Circle.hpp"
#include "cl_GEN_Geometry.hpp"

#include "cl_Plane.hpp"


#include "cl_PRM_HMR_Parameters.hpp"


#include <functional>

#include "cl_GEN_Geom_Field_HMR.hpp"

namespace moris
{

//-------------------------------------------------------------------------------------
// Functions for Parameters in FEM
void ConstFunctionVal
( moris::Matrix< moris::DDRMat >                 & aPropMatrix,
  moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
  moris::fem::Field_Interpolator_Manager         * aFIManager )
{
    aPropMatrix = aParameters( 0 );
}

void tMValFunctionContact
( moris::Matrix< moris::DDRMat >                 & aPropMatrix,
  moris::Cell< moris::Matrix< moris::DDRMat > >  & aParameters,
  moris::fem::Field_Interpolator_Manager         * aFIManager )
{
    aPropMatrix = {{ aParameters( 0 )( 0 ),                   0.0 },
                   { 0.0,                   aParameters( 0 )( 1 ) }};
}

//-------------------------------------------------------------------------------------

TEST_CASE("2D Linear Stuct Contract","[XTK_HMR_LS_Contact_2D]")
{
    if(par_size()<=1)
    {
        // Geometry Parameters
        moris::real tBlockL   =  1.0; /* Length of the block  (m) */
        moris::real tBlockH   =  1.0; /* Length of the block  (m) */
        moris::real tDomainLX = 2.0; /* Length of full domain in x  (m) */
        moris::real tDomainLY = 2.0; /* Length of full domain in y  (m) */
        moris::real tAngle =   0;
        Matrix<DDRMat> tCenterPoint = {{0.01111,0.0111}}; /* Center point of the block (intentionally off 0.0,0.0 to prevent interface at node)*/

        //Material Parameters
        moris::real tEa  = 1e6; // Pa
        moris::real tNua = 0.3;
        moris::real tEb  = 1e12; // Pa
        moris::real tNub = 0.3;

        // Boundary Conditions
        moris::real tDirchletX = 0.0; // m
        moris::real tDirchletY = 0.0; // m
        moris::real tDBCGamma  = 1000.0;

        moris::real tForce =  -10000.0; // N/m (normal to top surface)

        // Mesh Setup
        moris::uint tNumX   = 20; /* Number of elements in x*/
        moris::uint tNumY   = 20; /* Number of elements in y*/
        moris::uint tNumRef = 1;  /* Number of HMR refinements */
        moris::uint tOrder = 1;  /* Lagrange Order and Bspline Order (forced to be same for this example) */

        // Files
        std::string tHMRIPMeshFileName = "./mdl_exo/mdl_xtk_hmr_2d.e";
        std::string tEnrIgMeshFileName = "./mdl_exo/contact_enr_ig.e";

        // flags
        bool tVizGhost = false;
        bool tVerboseGeometry = false;
        bool tVizIGMeshBeforeFEM = false;


        // Construct Left Plane
        Matrix<moris::DDRMat> tLeftCenters = {{ (tCenterPoint(0) + 0.01)  - ( tBlockL / 2 ) * std::cos(tAngle) ,
                                                (tCenterPoint(1) + 0.01) - tBlockH / 2 * std::sin(tAngle) }};
        Matrix<moris::DDRMat> tLeftNormal  = {{-std::cos(tAngle),-std::sin(tAngle)}};
        xtk::Plane<2> tLeftPlane(tLeftCenters,tLeftNormal);
        auto tLeftPlaneFP = [&tLeftPlane] (moris::Matrix< moris::DDRMat > const & aCoordinates) { return tLeftPlane.evaluate_field_value_with_single_coordinate(aCoordinates); }; /*Lambda pointer for class */

        // Construct Right Plane
        Matrix<moris::DDRMat> tRightCenters = {{ (tCenterPoint(0) + 0.01)  + ( tBlockL / 2 ) * std::cos(tAngle) ,
                                                 (tCenterPoint(1) + 0.01) + tBlockH / 2 * std::sin(tAngle) }};
        Matrix<moris::DDRMat> tRightNormal  = {{std::cos(tAngle),std::sin(tAngle)}};
        xtk::Plane<2> tRightPlane(tRightCenters,tRightNormal);
        auto tRightPlaneFP = [&tRightPlane] (moris::Matrix< moris::DDRMat > const & aCoordinates) { return tRightPlane.evaluate_field_value_with_single_coordinate(aCoordinates); }; /*Lambda pointer for class */

        // Construct Mid Plane
        Matrix<moris::DDRMat> tMidCenters = {{ tCenterPoint(0) , tCenterPoint(1)  }};
        Matrix<moris::DDRMat> tMidNormal  = {{std::cos(tAngle) - 1,1-std::sin(tAngle)}};
        xtk::Plane<2> tMidPlane(tMidCenters,tMidNormal);
//        auto tMidPlaneFP = [&tMidPlane] (moris::Matrix< moris::DDRMat > const & aCoordinates) { return tMidPlane.evaluate_field_value_with_single_coordinate(aCoordinates); }; /*Lambda for class */

        // Construct Top Plane
//        Matrix<moris::DDRMat> tTopCenters = {{0, tCenterPoint(1) + 0.01 + tBlockH / 2  }};
        Matrix<moris::DDRMat> tTopCenters = {{(tCenterPoint(0) + 0.01) - tBlockH / 2 * std::sin(tAngle) ,
                                              (tCenterPoint(1) + 0.01) + tBlockH / 2 * std::cos(tAngle)}};
        Matrix<moris::DDRMat> tTopNormal  = {{std::cos(tAngle) - 1,1-std::sin(tAngle)}};
        xtk::Plane<2> tTopPlane(tTopCenters,tTopNormal);
        auto tTopPlaneFP = [&tTopPlane] (moris::Matrix< moris::DDRMat > const & aCoordinates) { return tTopPlane.evaluate_field_value_with_single_coordinate(aCoordinates); }; /*Lambda for class */

        // Construct Bottom Plane
//        Matrix<moris::DDRMat> tBottomCenters = {{0, tCenterPoint(1) + 0.01 - tBlockH / 2  }};
        Matrix<moris::DDRMat> tBottomCenters = {{(tCenterPoint(0) + 0.01) + tBlockH / 2 * std::sin(tAngle) ,
                                                 (tCenterPoint(1) + 0.01) - tBlockH / 2 * std::cos(tAngle)}};

        Matrix<moris::DDRMat> tBottomNormal  = {{1 - std::cos(tAngle),std::sin(tAngle) - 1}};
        xtk::Plane<2> tBottomPlane(tBottomCenters,tBottomNormal);
        auto tBottomPlaneFP = [&tBottomPlane] (moris::Matrix< moris::DDRMat > const & aCoordinates) { return tBottomPlane.evaluate_field_value_with_single_coordinate(aCoordinates); }; /*Lambda for class */

        if(tVerboseGeometry)
        {
            std::cout<<"\nLeft Plane :"<<std::endl;
            std::cout<<"    xc = "<<std::setw(16)<<tLeftCenters(0)<<std::setw(16)<<tLeftCenters(1)<<std::endl;
            std::cout<<"    n  = "<<std::setw(16)<<tLeftNormal(0)<<std::setw(16)<<tLeftNormal(1)<<std::endl;

            std::cout<<"\nRight Plane :"<<std::endl;
            std::cout<<"    xc = "<<std::setw(16)<<tRightCenters(0)<<std::setw(16)<<tRightCenters(1)<<std::endl;
            std::cout<<"    n  = "<<std::setw(16)<<tRightNormal(0)<<std::setw(16)<<tRightNormal(1)<<std::endl;

            std::cout<<"\nBottom Plane :"<<std::endl;
            std::cout<<"    xc = "<<std::setw(16)<<tBottomCenters(0)<<std::setw(16)<<tBottomCenters(1)<<std::endl;
            std::cout<<"    n  = "<<std::setw(16)<<tBottomNormal(0)<<std::setw(16)<<tBottomNormal(1)<<std::endl;

            std::cout<<"\nTop Plane :"<<std::endl;
            std::cout<<"    xc = "<<std::setw(16)<<tTopCenters(0)<<std::setw(16)<<tTopCenters(1)<<std::endl;
            std::cout<<"    n  = "<<std::setw(16)<<tTopNormal(0)<<std::setw(16)<< tTopNormal(1)<<std::endl;

            std::cout<<"\nMid Plane :"<<std::endl;
            std::cout<<"    xc = "<<std::setw(16)<<tMidCenters(0)<<std::setw(16)<<tMidCenters(1)<<std::endl;
            std::cout<<"    n  = "<<std::setw(16)<<tMidNormal(0)<<std::setw(16)<< tMidNormal(1)<<std::endl;
        }

        uint tLagrangeMeshIndex = 0;
        std::string tLeftFieldName   = "LeftPlane";
        std::string tRightFieldName  = "RightPlane";
        std::string tMidFieldName    = "MidPlane";
        std::string tTopFieldName    = "TopPlane";
        std::string tBottomFieldName = "BottomPlane";

        ParameterList tParameters = prm::create_hmr_parameter_list();

        tParameters.set( "number_of_elements_per_dimension", std::to_string(tNumX) + "," + std::to_string(tNumY));
        tParameters.set( "domain_dimensions", std::to_string(tDomainLX) + "," + std::to_string(tDomainLY) );
        tParameters.set( "domain_offset", std::to_string(-tDomainLX/2) + "," + std::to_string(-tDomainLY/2) );
        tParameters.set( "domain_sidesets", std::string("1,2,3,4") );
        tParameters.set( "lagrange_output_meshes", std::string("0") );

        tParameters.set( "lagrange_orders", std::string("1") );
        tParameters.set( "lagrange_pattern", std::string("0") );
        tParameters.set( "bspline_orders", std::string("1") );
        tParameters.set( "bspline_pattern", std::string("0") );

        tParameters.set( "lagrange_to_bspline", std::string("0") );

        tParameters.set( "truncate_bsplines", 1 );
        tParameters.set( "refinement_buffer", 3 );
        tParameters.set( "staircase_buffer", 3 );
        tParameters.set( "initial_refinement", 0 );

        tParameters.set( "use_multigrid", 0 );
        tParameters.set( "severity_level", 2 );
        tParameters.set("use_number_aura",1);

        hmr::HMR tHMR( tParameters );

        //initial refinement
        tHMR.perform_initial_refinement( 0 );

        std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

        //  create field
        std::shared_ptr< moris::hmr::Field > tLeftField   = tMesh->create_field( tLeftFieldName, tLagrangeMeshIndex );
        std::shared_ptr< moris::hmr::Field > tRightField  = tMesh->create_field( tRightFieldName, tLagrangeMeshIndex );
//        std::shared_ptr< moris::hmr::Field > tMidField    = tMesh->create_field( tMidFieldName, tLagrangeMeshIndex );
        std::shared_ptr< moris::hmr::Field > tTopField    = tMesh->create_field( tTopFieldName, tLagrangeMeshIndex );
        std::shared_ptr< moris::hmr::Field > tBottomField = tMesh->create_field( tBottomFieldName, tLagrangeMeshIndex );

        for( uint k=0; k<tNumRef; ++k )
        {
            tLeftField->evaluate_scalar_function( tLeftPlaneFP );
            tRightField->evaluate_scalar_function( tRightPlaneFP );
//            tMidField->evaluate_scalar_function( tMidPlaneFP );
            tTopField->evaluate_scalar_function( tTopPlaneFP );
            tBottomField->evaluate_scalar_function( tBottomPlaneFP );


            tHMR.flag_surface_elements_on_working_pattern( tLeftField );
            tHMR.flag_surface_elements_on_working_pattern( tRightField );
//            tHMR.flag_surface_elements_on_working_pattern( tMidField );
            tHMR.flag_surface_elements_on_working_pattern( tTopField );
            tHMR.flag_surface_elements_on_working_pattern( tBottomField );


            tHMR.perform_refinement_based_on_working_pattern( 0 );

        }

        tLeftField->evaluate_scalar_function( tLeftPlaneFP );
        tRightField->evaluate_scalar_function( tRightPlaneFP );
//        tMidField->evaluate_scalar_function( tMidPlaneFP );
        tTopField->evaluate_scalar_function( tTopPlaneFP );
        tBottomField->evaluate_scalar_function( tBottomPlaneFP );

        tHMR.finalize();

        tHMR.save_to_exodus( 0, tHMRIPMeshFileName );

        moris::hmr::Interpolation_Mesh_HMR * tInterpolationMesh = tHMR.create_interpolation_mesh(tLagrangeMeshIndex);

        //-----------------------------------------------------------------------------------------------
          moris::ge::GEN_Geom_Field_HMR tLeftPlaneForGE(tLeftField);
          moris::ge::GEN_Geom_Field_HMR tRightPlaneForGE(tRightField);
//          moris::ge::GEN_Geom_Field tMidPlaneForGE(tMidField);
          moris::ge::GEN_Geom_Field_HMR tTopPlaneForGE(tTopField);
          moris::ge::GEN_Geom_Field_HMR tBottomPlaneForGE(tBottomField);

          // NOTE the order of this geometry vector is important. If it changes the resulting bulk phase of the output mesh change.
//          moris::Cell<moris::ge::GEN_Geometry*> tGeometryVector = { & tLeftPlaneForGE , & tRightPlaneForGE , &tTopPlaneForGE, &tBottomPlaneForGE, & tMidPlaneForGE};
          moris::Cell<moris::ge::GEN_Geometry*> tGeometryVector = { & tLeftPlaneForGE , & tRightPlaneForGE , &tTopPlaneForGE, &tBottomPlaneForGE};

          size_t tModelDimension = 2;
          moris::ge::GEN_Phase_Table tPhaseTable (tGeometryVector.size(),  Phase_Table_Structure::EXP_BASE_2);
          moris::ge::GEN_Geometry_Engine tGeometryEngine(tGeometryVector,tPhaseTable,tModelDimension);
          xtk::Model tXTKModel(tModelDimension,tInterpolationMesh,&tGeometryEngine);
          tXTKModel.mVerbose = false;

        //Specify decomposition Method and Cut Mesh ---------------------------------------
        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
        tXTKModel.decompose(tDecompositionMethods);

        tXTKModel.perform_basis_enrichment(EntityRank::BSPLINE,0);
//        tXTKModel.construct_face_oriented_ghost_penalization_cells();

        // get meshes for FEM
        xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
        xtk::Enriched_Integration_Mesh   & tEnrIntegMesh = tXTKModel.get_enriched_integ_mesh();

        if(tVizGhost)
        {
            // access ghost
            xtk::Ghost_Stabilization & tGhostStab = tXTKModel.get_ghost_stabilization();

            for(moris_index iG = 0; iG < (moris_index)tPhaseTable.get_num_phases(); iG++)
            {
                tGhostStab.visualize_ghost_on_mesh(iG);
            }
        }


        if(tVizIGMeshBeforeFEM)
        {
            tEnrIntegMesh.deactivate_empty_sets();
            // Write mesh
            moris::mtk::Writer_Exodus writer(&tEnrIntegMesh);
            writer.write_mesh("", tEnrIgMeshFileName);

            // Write the fields
            writer.set_time(0.0);
            writer.close_file();

//            moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh();
//            tIntegMesh1->create_output_mesh(tEnrIgMeshFileName);
//            delete tIntegMesh1;

        }


        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair(&tEnrInterpMesh, &tEnrIntegMesh);

        //------------------------------------------------------------------------------
        // create the properties
        std::shared_ptr< fem::Property > tPropEModA = std::make_shared< fem::Property >();
        tPropEModA->set_parameters( { {{ tEa }} } );
        tPropEModA->set_val_function( ConstFunctionVal );

        std::shared_ptr< fem::Property > tPropEModB = std::make_shared< fem::Property >();
        tPropEModB->set_parameters( { {{ tEb }} } );
        tPropEModB->set_val_function( ConstFunctionVal );

        std::shared_ptr< fem::Property > tPropNua = std::make_shared< fem::Property >();
        tPropNua->set_parameters( { {{ tNua }} } );
        tPropNua->set_val_function( ConstFunctionVal );

        std::shared_ptr< fem::Property > tPropNub = std::make_shared< fem::Property >();
        tPropNub->set_parameters( { {{ tNub }} } );
        tPropNub->set_val_function( ConstFunctionVal );

        std::shared_ptr< fem::Property > tPropDirichlet = std::make_shared< fem::Property >();
        tPropDirichlet->set_parameters( { {{ tDirchletX }, { tDirchletX }} } );
        tPropDirichlet->set_val_function( ConstFunctionVal );

        std::shared_ptr< fem::Property > tPropDirichlet2 = std::make_shared< fem::Property >();
        tPropDirichlet2->set_parameters( { {{ 1.0, 1.0 }} } );
        tPropDirichlet2->set_val_function( tMValFunctionContact );

        std::shared_ptr< fem::Property > tPropNeumann = std::make_shared< fem::Property >();
        tPropNeumann->set_parameters( {{{ tForce*std::sin(tAngle) } , { tForce*std::cos(tAngle) }}} );
        tPropNeumann->set_val_function( ConstFunctionVal );

        // define constitutive models
        fem::CM_Factory tCMFactory;

        std::shared_ptr< fem::Constitutive_Model > tCMStrucLinIso1 = tCMFactory.create_CM( fem::Constitutive_Type::STRUC_LIN_ISO );
        tCMStrucLinIso1->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tCMStrucLinIso1->set_property( tPropEModA, "YoungsModulus" );
        tCMStrucLinIso1->set_property( tPropNua, "PoissonRatio" );
        tCMStrucLinIso1->set_space_dim( 2 );
        tCMStrucLinIso1->set_model_type(fem::Model_Type::PLANE_STRESS);

        std::shared_ptr< fem::Constitutive_Model > tCMStrucLinIso2 = tCMFactory.create_CM( fem::Constitutive_Type::STRUC_LIN_ISO );
        tCMStrucLinIso2->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tCMStrucLinIso2->set_property( tPropEModB, "YoungsModulus" );
        tCMStrucLinIso2->set_property( tPropNub, "PoissonRatio" );
        tCMStrucLinIso2->set_space_dim( 2 );
        tCMStrucLinIso2->set_model_type(fem::Model_Type::PLANE_STRESS);

        // define stabilization parameters
        fem::SP_Factory tSPFactory;
        std::shared_ptr< fem::Stabilization_Parameter > tSPDirichletNitsche = tSPFactory.create_SP( fem::Stabilization_Type::DIRICHLET_NITSCHE );
        tSPDirichletNitsche->set_parameters( { {{ tDBCGamma }} } );
        tSPDirichletNitsche->set_property( tPropEModB, "Material", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::Stabilization_Parameter > tSPNitscheInterface = tSPFactory.create_SP( fem::Stabilization_Type::NITSCHE_INTERFACE );
        tSPNitscheInterface->set_parameters( { {{ 1.0 }} } );
        tSPNitscheInterface->set_property( tPropEModB, "Material", mtk::Master_Slave::MASTER );
        tSPNitscheInterface->set_property( tPropEModA, "Material", mtk::Master_Slave::SLAVE );

        std::shared_ptr< fem::Stabilization_Parameter > tSPMasterWeightInterface = tSPFactory.create_SP( fem::Stabilization_Type::MASTER_WEIGHT_INTERFACE );
        tSPMasterWeightInterface->set_property( tPropEModB, "Material", mtk::Master_Slave::MASTER );
        tSPMasterWeightInterface->set_property( tPropEModA, "Material", mtk::Master_Slave::SLAVE );

        std::shared_ptr< fem::Stabilization_Parameter > tSPSlaveWeightInterface = tSPFactory.create_SP( fem::Stabilization_Type::SLAVE_WEIGHT_INTERFACE );
        tSPSlaveWeightInterface->set_property( tPropEModB, "Material", mtk::Master_Slave::MASTER );
        tSPSlaveWeightInterface->set_property( tPropEModA, "Material", mtk::Master_Slave::SLAVE );

        // define the IWGs
        fem::IWG_Factory tIWGFactory;

        std::shared_ptr< fem::IWG > tIWGBulkA = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_BULK );
        tIWGBulkA->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIWGBulkA->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tIWGBulkA->set_constitutive_model( tCMStrucLinIso1, "ElastLinIso", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGBulkB = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_BULK );
        tIWGBulkB->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIWGBulkB->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tIWGBulkB->set_constitutive_model( tCMStrucLinIso2, "ElastLinIso", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGDirichlet = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_DIRICHLET_SYMMETRIC_NITSCHE );
        tIWGDirichlet->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIWGDirichlet->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tIWGDirichlet->set_stabilization_parameter( tSPDirichletNitsche, "DirichletNitsche" );
        tIWGDirichlet->set_constitutive_model( tCMStrucLinIso1, "ElastLinIso", mtk::Master_Slave::MASTER );
        tIWGDirichlet->set_property( tPropDirichlet, "Dirichlet", mtk::Master_Slave::MASTER );
        tIWGDirichlet->set_property( tPropDirichlet2, "Select", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGNeumann = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_NEUMANN );
        tIWGNeumann->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIWGNeumann->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tIWGNeumann->set_property( tPropNeumann, "Neumann", mtk::Master_Slave::MASTER );

        std::shared_ptr< fem::IWG > tIWGInterface = tIWGFactory.create_IWG( fem::IWG_Type::STRUC_LINEAR_INTERFACE );
        tIWGInterface->set_residual_dof_type( { MSI::Dof_Type::UX, MSI::Dof_Type::UY } );
        tIWGInterface->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }} );
        tIWGInterface->set_dof_type_list( {{ MSI::Dof_Type::UX, MSI::Dof_Type::UY }},mtk::Master_Slave::SLAVE );
        tIWGInterface->set_stabilization_parameter( tSPNitscheInterface, "NitscheInterface" );
        tIWGInterface->set_stabilization_parameter( tSPMasterWeightInterface, "MasterWeightInterface" );
        tIWGInterface->set_stabilization_parameter( tSPSlaveWeightInterface, "SlaveWeightInterface" );
        tIWGInterface->set_constitutive_model( tCMStrucLinIso2, "ElastLinIso", mtk::Master_Slave::MASTER );
        tIWGInterface->set_constitutive_model( tCMStrucLinIso1, "ElastLinIso", mtk::Master_Slave::SLAVE );

        // define set info
        fem::Set_User_Info tSetBulk1;
        tSetBulk1.set_mesh_set_name( "HMR_dummy_c_p0" );
        tSetBulk1.set_IWGs( { tIWGBulkA } );

        fem::Set_User_Info tSetBulk2;
        tSetBulk2.set_mesh_set_name( "HMR_dummy_n_p0" );
        tSetBulk2.set_IWGs( { tIWGBulkA } );

        fem::Set_User_Info tSetBulk3;
        tSetBulk3.set_mesh_set_name( "HMR_dummy_c_p1" );
        tSetBulk3.set_IWGs( { tIWGBulkB } );

        fem::Set_User_Info tSetBulk4;
        tSetBulk4.set_mesh_set_name( "HMR_dummy_n_p1" );
        tSetBulk4.set_IWGs( { tIWGBulkB } );

        fem::Set_User_Info tSetBulk5;
        tSetBulk5.set_mesh_set_name( "HMR_dummy_c_p5" );
        tSetBulk5.set_IWGs( { tIWGBulkB } );

        fem::Set_User_Info tSetBulk6;
        tSetBulk6.set_mesh_set_name( "HMR_dummy_n_p5" );
        tSetBulk6.set_IWGs( { tIWGBulkB } );

        fem::Set_User_Info tSetBulk7;
        tSetBulk7.set_mesh_set_name( "HMR_dummy_c_p9" );
        tSetBulk7.set_IWGs( { tIWGBulkB } );

        fem::Set_User_Info tSetBulk8;
        tSetBulk8.set_mesh_set_name( "HMR_dummy_n_p9" );
        tSetBulk8.set_IWGs( { tIWGBulkB } );

        fem::Set_User_Info tSetDirichlet1;
        tSetDirichlet1.set_mesh_set_name( "SideSet_1_n_p9" );
        tSetDirichlet1.set_IWGs( { tIWGDirichlet } );

        fem::Set_User_Info tSetDirichlet2;
        tSetDirichlet2.set_mesh_set_name( "SideSet_1_c_p9" );
        tSetDirichlet2.set_IWGs( { tIWGDirichlet } );

        fem::Set_User_Info tSetDirichlet3;
        tSetDirichlet3.set_mesh_set_name( "SideSet_1_c_p1" );
        tSetDirichlet3.set_IWGs( { tIWGDirichlet } );

        fem::Set_User_Info tSetDirichlet4;
        tSetDirichlet4.set_mesh_set_name( "SideSet_1_n_p1" );
        tSetDirichlet4.set_IWGs( { tIWGDirichlet } );

        fem::Set_User_Info tSetDirichlet5;
        tSetDirichlet5.set_mesh_set_name( "SideSet_1_c_p5" );
        tSetDirichlet5.set_IWGs( { tIWGDirichlet } );

        fem::Set_User_Info tSetDirichlet6;
        tSetDirichlet6.set_mesh_set_name( "SideSet_1_n_p5" );
        tSetDirichlet6.set_IWGs( { tIWGDirichlet } );

        std::cout<<"tEnrIntegMesh.get_interface_side_set_name(3,9,8) = "<<tEnrIntegMesh.get_interface_side_set_name(3,9,8)<<" Index = "<<tEnrIntegMesh.get_set_index_by_name(tEnrIntegMesh.get_interface_side_set_name(3,9,8))<<std::endl;
        std::cout<<"tEnrIntegMesh.get_interface_side_set_name(3,5,4) = "<<tEnrIntegMesh.get_interface_side_set_name(3,5,4)<<tEnrIntegMesh.get_set_index_by_name(tEnrIntegMesh.get_interface_side_set_name(3,5,4))<<std::endl;

        fem::Set_User_Info tSetNeumann1;
        tSetNeumann1.set_mesh_set_name( tEnrIntegMesh.get_interface_side_set_name(2,0,2) );
        tSetNeumann1.set_IWGs( { tIWGNeumann } );

        fem::Set_User_Info tSetNeumann2;
        tSetNeumann2.set_mesh_set_name( tEnrIntegMesh.get_interface_side_set_name(3,5,4) );
        tSetNeumann2.set_IWGs( { tIWGNeumann } );

        fem::Set_User_Info tSetNeumann3;
        tSetNeumann3.set_mesh_set_name( tEnrIntegMesh.get_interface_side_set_name(3,9,8) );
        tSetNeumann3.set_IWGs( { tIWGNeumann } );

        /* This is the contact interface*/
        fem::Set_User_Info tSetInterface1;
        tSetInterface1.set_mesh_set_name( tEnrIntegMesh.get_dbl_interface_side_set_name(0,1) );
        tSetInterface1.set_IWGs( { tIWGInterface } );


        fem::Set_User_Info tSetInterface2;
        tSetInterface2.set_mesh_set_name( tEnrIntegMesh.get_dbl_interface_side_set_name(1,9) );
        tSetInterface2.set_IWGs( { tIWGInterface } );

        fem::Set_User_Info tSetInterface3;
        tSetInterface3.set_mesh_set_name( tEnrIntegMesh.get_dbl_interface_side_set_name(1,5) );
        tSetInterface3.set_IWGs( { tIWGInterface } );

        // create a cell of set info
        moris::Cell< fem::Set_User_Info > tSetInfo( 20 );
        tSetInfo( 0 )  = tSetBulk1;
        tSetInfo( 1 )  = tSetBulk2;
        tSetInfo( 2 )  = tSetBulk3;
        tSetInfo( 3 )  = tSetBulk4;
        tSetInfo( 4 )  = tSetBulk5;
        tSetInfo( 5 )  = tSetBulk6;
        tSetInfo( 6 )  = tSetBulk7;
        tSetInfo( 7 )  = tSetBulk8;
        tSetInfo( 8 )  = tSetDirichlet1;
        tSetInfo( 9 )  = tSetDirichlet2;
        tSetInfo( 10 )  = tSetDirichlet3;
        tSetInfo( 11 )  = tSetDirichlet4;
        tSetInfo( 12 )  = tSetDirichlet5;
        tSetInfo( 13 )  = tSetDirichlet6;
        tSetInfo( 14 ) = tSetNeumann1;
        tSetInfo( 15 ) = tSetNeumann2;
        tSetInfo( 16 ) = tSetNeumann3;
        tSetInfo( 17 ) = tSetInterface1;
        tSetInfo( 18 ) = tSetInterface2;
        tSetInfo( 19 ) = tSetInterface3;


        // create model
        mdl::Model * tModel = new mdl::Model( &tMeshManager,
                                              0,
                                              tSetInfo,
                                              0, false );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: create linear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        moris::Cell< enum MSI::Dof_Type > tDofTypesU( 2 );
        tDofTypesU( 0 ) = MSI::Dof_Type::UX;
        tDofTypesU( 1 ) = MSI::Dof_Type::UY;

        dla::Solver_Factory  tSolFactory;
        std::shared_ptr< dla::Linear_Solver_Algorithm > tLinearSolverAlgorithm = tSolFactory.create_solver( sol::SolverType::AZTEC_IMPL );

        tLinearSolverAlgorithm->set_param("AZ_diagnostics") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_output") = AZ_none;
        tLinearSolverAlgorithm->set_param("AZ_max_iter") = 10000;
        tLinearSolverAlgorithm->set_param("AZ_solver") = AZ_gmres;
        tLinearSolverAlgorithm->set_param("AZ_subdomain_solve") = AZ_ilu;
        tLinearSolverAlgorithm->set_param("AZ_graph_fill") = 10;
        //        tLinearSolverAlgorithm->set_param("Use_ML_Prec") = true;

        dla::Linear_Solver tLinSolver;
        tLinSolver.set_linear_algorithm( 0, tLinearSolverAlgorithm );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: create nonlinear solver and algorithm
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        NLA::Nonlinear_Solver_Factory tNonlinFactory;
        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithm = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );
        //        std::shared_ptr< NLA::Nonlinear_Algorithm > tNonlinearSolverAlgorithmMonolythicU = tNonlinFactory.create_nonlinear_solver( NLA::NonlinearSolverType::NEWTON_SOLVER );

        tNonlinearSolverAlgorithm->set_param("NLA_max_iter")   = 3;
        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_hard_break") = false;
        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_max_lin_solver_restarts") = 2;
        //        tNonlinearSolverAlgorithmMonolythic->set_param("NLA_rebuild_jacobian") = true;

        tNonlinearSolverAlgorithm->set_linear_solver( &tLinSolver );
        //        tNonlinearSolverAlgorithmMonolythicU->set_linear_solver( &tLinSolver );

        NLA::Nonlinear_Solver tNonlinearSolverMain;
        tNonlinearSolverMain.set_nonlinear_algorithm( tNonlinearSolverAlgorithm, 0 );


        tNonlinearSolverMain.set_dof_type_list( tDofTypesU );

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

        //------------------------------------------------------------------------------
        tTimeSolver.solve();

        // output solution and meshes

//        xtk::Output_Options tOutputOptions;
//        tOutputOptions.mAddNodeSets = false;
//        tOutputOptions.mAddSideSets = true;
//        tOutputOptions.mAddClusters = false;
//
//        // add solution field to integration mesh
//        std::string tIntegSolFieldNameUX = "UX";
//        std::string tIntegSolFieldNameUY = "UY";
//        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldNameUX, tIntegSolFieldNameUY};
//
//        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);
//
//        // Write to Integration mesh for visualization
//        Matrix<DDRMat> tIntegSolUX = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UX );
//        Matrix<DDRMat> tIntegSolUY = tModel->get_solution_for_integration_mesh_output( MSI::Dof_Type::UY );
//
//        //    print(tIntegSolUX,"tIntegSolUX");
//        //    print(tIntegSolUY,"tIntegSolUY");
//
//        Matrix<DDRMat> tSTKIntegSolUX(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//        Matrix<DDRMat> tSTKIntegSolUY(tIntegMesh1->get_num_entities(EntityRank::NODE),1);
//
//        for(moris::uint i = 0; i < tIntegMesh1->get_num_entities(EntityRank::NODE); i++)
//        {
//            moris::moris_id tID = tIntegMesh1->get_glb_entity_id_from_entity_loc_index(i,EntityRank::NODE);
//            tSTKIntegSolUX(i) = tIntegSolUX(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//            tSTKIntegSolUY(i) = tIntegSolUY(tEnrIntegMesh.get_loc_entity_ind_from_entity_glb_id(tID,EntityRank::NODE));
//        }
//
//        // add solution field to integration mesh
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUX,EntityRank::NODE,tSTKIntegSolUX);
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds(tIntegSolFieldNameUY,EntityRank::NODE,tSTKIntegSolUY);
//
//        //    Matrix<DDRMat> tFullSol;
//        //    tNonlinearSolver.get_full_solution(tFullSol);
//        //
//        //    print(tFullSol,"tFullSol");
//
//        std::string tMeshOutputFile = "./mdl_exo/stk_xtk_linear_struc_2D.e";
//
//        tIntegMesh1->create_output_mesh(tMeshOutputFile);
//
//        delete tIntegMesh1;

        delete tModel;
        delete tInterpolationMesh;
    }
}
}
