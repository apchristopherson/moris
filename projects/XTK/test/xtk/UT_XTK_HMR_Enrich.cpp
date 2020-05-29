/*
 * UT_XTK_HMR_Enrich.cpp
 *
 *  Created on: Sep 30, 2019
 *      Author: doble
 */

#include "catch.hpp"

#include "cl_XTK_Model.hpp"
#include "typedefs.hpp"
#include "cl_MTK_Mesh_Manager.hpp"

#include "cl_MTK_Vertex.hpp"    //MTK
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh.hpp"

#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Integration_Mesh_STK.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Writer_Exodus.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"

#include "cl_Matrix.hpp"        //LINALG
#include "linalg_typedefs.hpp"
#include "fn_equal_to.hpp" // ALG/src

#include "cl_HMR_Mesh_Interpolation.hpp"
#include "cl_HMR.hpp"
#include "cl_HMR_Background_Mesh.hpp" //HMR/src
#include "cl_HMR_BSpline_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Element.hpp" //HMR/src
#include "cl_HMR_Factory.hpp" //HMR/src
#include "cl_HMR_Field.hpp"
#include "cl_HMR_Lagrange_Mesh_Base.hpp" //HMR/src
#include "cl_PRM_HMR_Parameters.hpp"

#include "cl_GEN_Geometry_Field_HMR.hpp"
#include "fn_norm.hpp"

namespace xtk
{
moris::real
MultiCircle(const moris::Matrix< moris::DDRMat > & aPoint )
{

    moris::real mXCenter = 0.3333;
    moris::real mYCenter = 0.3333;
    moris::real mRadius = 0.22;


    real val1 =   (aPoint(0) - mXCenter) * (aPoint(0) - mXCenter)
                    + (aPoint(1) - mYCenter) * (aPoint(1) - mYCenter)
                    - (mRadius * mRadius);

    mXCenter = -0.3333;
    mYCenter = -0.3333;

    real val2 = (aPoint(0) - mXCenter) * (aPoint(0) - mXCenter)
                            + (aPoint(1) - mYCenter) * (aPoint(1) - mYCenter)
                            - (mRadius * mRadius);


    moris::real mXC =  0.422;
    moris::real mYC = -0.422;
    moris::real mNx = 1.0;
    moris::real mNy = -1.0;
    moris::real val3 = -(mNx*(aPoint(0)-mXC) + mNy*(aPoint(1)-mYC));

    mXC = -0.422;
    mYC = 0.422;
    mNx = 1.0;
    mNy = -1.0;
    moris::real val4 = (mNx*(aPoint(0)-mXC) + mNy*(aPoint(1)-mYC));

    return std::min(val1,std::min(val2,std::min(val3,val4)));

}

TEST_CASE("2D XTK WITH HMR No truncation enrichment","[XTK_HMR_ENR_2D]")
{
    if(par_size()<=1)
    {
        std::string tFieldName = "Cylinder";

         moris::uint tLagrangeMeshIndex = 0;
         moris::uint tBSplineMeshIndex = 0;

         moris::hmr::Parameters tParameters;

         tParameters.set_number_of_elements_per_dimension( { {3}, {3}} );
         tParameters.set_domain_dimensions({ {2}, {2} });
         tParameters.set_domain_offset({ {-1.0}, {-1.0} });
         tParameters.set_bspline_truncation( true );

         tParameters.set_output_meshes( { {0} } );

         tParameters.set_lagrange_orders  ( { {2} });
         tParameters.set_lagrange_patterns({ {0} });

         tParameters.set_bspline_orders   ( { {3} } );
         tParameters.set_bspline_patterns ( { {0} } );

         tParameters.set_side_sets({{1},{2},{3},{4} });

         tParameters.set_union_pattern( 2 );
         tParameters.set_working_pattern( 3 );

         tParameters.set_refinement_buffer( 1 );
         tParameters.set_staircase_buffer( 1 );

         Cell< Matrix< DDSMat > > tLagrangeToBSplineMesh( 1 );
         tLagrangeToBSplineMesh( 0 ) = { {0} };

         tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

         hmr::HMR tHMR( tParameters );

         std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

         // create field
         std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );

         tField->evaluate_scalar_function( MultiCircle );

         for( uint k=0; k<2; ++k )
         {
             tHMR.flag_surface_elements_on_working_pattern( tField );
             tHMR.perform_refinement_based_on_working_pattern( 0 );
             tField->evaluate_scalar_function( MultiCircle );
         }

         tHMR.finalize();

         tHMR.save_to_exodus( 0, "./xtk_exo/xtk_hmr_2d_enr_ip.e" );

         hmr::Interpolation_Mesh_HMR * tInterpMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex  );

        moris::Cell< std::shared_ptr<moris::ge::Geometry> > tGeometryVector(1);
        tGeometryVector(0) = std::make_shared<moris::ge::Geometry_Field_HMR>(tField);

         size_t tModelDimension = 2;
         moris::ge::Phase_Table tPhaseTable (1, moris::ge::Phase_Table_Structure::EXP_BASE_2);
         moris::ge::Geometry_Engine tGeometryEngine(tGeometryVector,tPhaseTable,tModelDimension);
         Model tXTKModel(tModelDimension,tInterpMesh,&tGeometryEngine);
         tXTKModel.mVerbose  =  false;

        //Specify decomposition Method and Cut Mesh ---------------------------------------
        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
        tXTKModel.decompose(tDecompositionMethods);

        tXTKModel.perform_basis_enrichment(EntityRank::BSPLINE,0);

        Enrichment const & tEnrichment = tXTKModel.get_basis_enrichment();

        // Declare the fields related to enrichment strategy in output options
        Cell<std::string> tEnrichmentFieldNames = tEnrichment.get_cell_enrichment_field_names();

        xtk::Output_Options tOutputOptions;
        tOutputOptions.mAddNodeSets = false;
        tOutputOptions.mAddSideSets = true;
        tOutputOptions.mAddClusters = false;
        tOutputOptions.mRealElementExternalFieldNames = tEnrichmentFieldNames;

        // add solution field to integration mesh
        std::string tIntegSolFieldName = "solution";
        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldName};

        moris::mtk::Mesh* tCutMeshData = tXTKModel.get_output_mesh(tOutputOptions);

        tEnrichment.write_cell_enrichment_to_fields(tEnrichmentFieldNames,tCutMeshData);

        std::string tMeshOutputFile ="./xtk_exo/xtk_hmr_2d_enr_trun_ig.e";
        tCutMeshData->create_output_mesh(tMeshOutputFile);

        delete tCutMeshData;
        delete tInterpMesh;
    }
}


moris::real
CircleMultiMat(const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real mXCenter = 0.01;
    moris::real mYCenter = 0.01;
    moris::real mRadius = 0.61;


    return  (aPoint(0) - mXCenter) * (aPoint(0) - mXCenter)
                    + (aPoint(1) - mYCenter) * (aPoint(1) - mYCenter)
                    - (mRadius * mRadius);
}

moris::real
PlaneMultiMat(const moris::Matrix< moris::DDRMat > & aPoint )
{
    moris::real mXC = 0.01;
    moris::real mYC = 0.01;
    moris::real mNx = 1.0;
    moris::real mNy = 0.0;
    return (mNx*(aPoint(0)-mXC) + mNy*(aPoint(1)-mYC));
}

TEST_CASE("2D XTK WITH HMR Multi-Mat","[XTK_HMR_MULTI_2D]")
{
    if(par_size()<=1)
    {
        std::string tFieldName = "Geometry";

         moris::uint tLagrangeMeshIndex = 0;
         moris::uint tBSplineMeshIndex = 0;

         moris::hmr::Parameters tParameters;

         tParameters.set_number_of_elements_per_dimension( { {3}, {1}} );
         tParameters.set_domain_dimensions({ {6}, {2} });
         tParameters.set_domain_offset({ {-3.0}, {-1.0} });
         tParameters.set_bspline_truncation( true );

         tParameters.set_output_meshes( { {0} } );

         tParameters.set_lagrange_orders  ( { {2} });
         tParameters.set_lagrange_patterns({ {0} });

         tParameters.set_bspline_orders   ( { {3} } );
         tParameters.set_bspline_patterns ( { {0} } );

         tParameters.set_side_sets({{1},{2},{3},{4} });
         tParameters.set_max_refinement_level( 2 );
         tParameters.set_union_pattern( 2 );
         tParameters.set_working_pattern( 3 );

         tParameters.set_refinement_buffer( 2 );
         tParameters.set_staircase_buffer( 2 );

         Cell< Matrix< DDSMat > > tLagrangeToBSplineMesh( 1 );
         tLagrangeToBSplineMesh( 0 ) = { {0} };

         tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

         hmr::HMR tHMR( tParameters );

         std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

         // create field
         std::shared_ptr< moris::hmr::Field > tPlaneField  = tMesh->create_field( tFieldName, tLagrangeMeshIndex );

         tPlaneField->evaluate_scalar_function( PlaneMultiMat );

         for( uint k=0; k<2; ++k )
         {
             tHMR.flag_surface_elements_on_working_pattern( tPlaneField );
             tHMR.perform_refinement_based_on_working_pattern( 0 );
             tPlaneField->evaluate_scalar_function( PlaneMultiMat );
         }

         std::shared_ptr< moris::hmr::Field > tCircleField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );
         tCircleField->evaluate_scalar_function( CircleMultiMat );
         for( uint k=0; k<2; ++k )
         {
             tHMR.flag_surface_elements_on_working_pattern( tCircleField );
             tHMR.perform_refinement_based_on_working_pattern( 0 );
             tCircleField->evaluate_scalar_function( CircleMultiMat );

         }

         tPlaneField->evaluate_scalar_function( PlaneMultiMat );
         tCircleField->evaluate_scalar_function( CircleMultiMat );
         tHMR.finalize();

         tHMR.save_to_exodus( 0, "./xtk_exo/xtk_hmr_2d_enr_ip2.e" );

         hmr::Interpolation_Mesh_HMR * tInterpMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex  );

        moris::Cell< std::shared_ptr<moris::ge::Geometry> > tGeometryVector(2);
        tGeometryVector(0) = std::make_shared<moris::ge::Geometry_Field_HMR>(tCircleField);
        tGeometryVector(1) = std::make_shared<moris::ge::Geometry_Field_HMR>(tPlaneField);

         size_t tModelDimension = 2;
         moris::ge::Phase_Table tPhaseTable (2, moris::ge::Phase_Table_Structure::EXP_BASE_2);
         moris::ge::Geometry_Engine tGeometryEngine(tGeometryVector,tPhaseTable,tModelDimension);
         Model tXTKModel(tModelDimension,tInterpMesh,&tGeometryEngine);
         tXTKModel.mVerbose  =  false;

        //Specify decomposition Method and Cut Mesh ---------------------------------------
        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
        tXTKModel.decompose(tDecompositionMethods);

        tXTKModel.perform_basis_enrichment(EntityRank::BSPLINE,0);

        Enrichment const & tEnrichment = tXTKModel.get_basis_enrichment();

        Enriched_Integration_Mesh & tEnrInteg = tXTKModel.get_enriched_integ_mesh(0);

        // Declare the fields related to enrichment strategy in output options
        Cell<std::string> tEnrichmentFieldNames = tEnrichment.get_cell_enrichment_field_names();

        xtk::Output_Options tOutputOptions;
        tOutputOptions.mAddNodeSets = false;
        tOutputOptions.mAddSideSets = true;
        tOutputOptions.mAddClusters = false;
        tOutputOptions.mRealElementExternalFieldNames = tEnrichmentFieldNames;

        // add solution field to integration mesh
        std::string tIntegSolFieldName = "solution";
        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldName};

        moris::mtk::Mesh* tCutMeshData = tXTKModel.get_output_mesh(tOutputOptions);

        tEnrichment.write_cell_enrichment_to_fields(tEnrichmentFieldNames,tCutMeshData);

        std::string tMeshOutputFile ="./xtk_exo/xtk_hmr_2d_enr_trun_ig.e";
        tCutMeshData->create_output_mesh(tMeshOutputFile);

        delete tCutMeshData;
        delete tInterpMesh;
    }
}

TEST_CASE("2D XTK WITH HMR Multiple Order Enrichment","[XTK_HMR_ENR_2D_MO]")
{
    if(par_size()<=1)
    {
        std::string tFieldName = "Geometry";

         moris::uint tLagrangeMeshIndex = 0;
         moris::uint tBSplineMeshIndex = 0;

         ParameterList tParameters = prm::create_hmr_parameter_list();

         tParameters.set( "number_of_elements_per_dimension", std::string("3, 1"));
         tParameters.set( "domain_dimensions",                std::string("6,2"));
         tParameters.set( "domain_offset",                    std::string("-3,-1") );
         tParameters.set( "domain_sidesets",                  std::string("1,2,3,4"));
         tParameters.set( "lagrange_output_meshes",           std::string("0") );
         tParameters.set( "lagrange_orders",                  std::string("2"));
         tParameters.set( "lagrange_pattern",                 std::string("0") );
         tParameters.set( "bspline_orders",                   std::string("1,2"));
         tParameters.set( "bspline_pattern",                 std::string("0,0"));
         tParameters.set( "lagrange_to_bspline",              std::string("0,1") );
         tParameters.set( "max_refinement_level",             3);
         tParameters.set( "truncate_bsplines", 1 );
         tParameters.set( "refinement_buffer", 2 );
         tParameters.set( "staircase_buffer", 2 );
         tParameters.set( "initial_refinement", 0 );

         tParameters.set( "use_multigrid", 0 );
         tParameters.set( "severity_level", 2 );
         tParameters.set( "use_number_aura", 1);


         hmr::HMR tHMR( tParameters );

         std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

         // create field
         std::shared_ptr< moris::hmr::Field > tPlaneField  = tMesh->create_field( tFieldName, tLagrangeMeshIndex );

         tPlaneField->evaluate_scalar_function( PlaneMultiMat );

         for( uint k=0; k<2; ++k )
         {
             tHMR.flag_surface_elements_on_working_pattern( tPlaneField );
             tHMR.perform_refinement_based_on_working_pattern( 0 );
             tPlaneField->evaluate_scalar_function( PlaneMultiMat );
         }

         std::shared_ptr< moris::hmr::Field > tCircleField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );
         tCircleField->evaluate_scalar_function( CircleMultiMat );
         for( uint k=0; k<2; ++k )
         {
             tHMR.flag_surface_elements_on_working_pattern( tCircleField );
             tHMR.perform_refinement_based_on_working_pattern( 0 );
             tCircleField->evaluate_scalar_function( CircleMultiMat );

         }

         tPlaneField->evaluate_scalar_function( PlaneMultiMat );
         tCircleField->evaluate_scalar_function( CircleMultiMat );
         tHMR.finalize();

         tHMR.save_to_exodus( 0, "./xtk_exo/xtk_hmr_2d_enr_ip2.e" );

         hmr::Interpolation_Mesh_HMR * tInterpMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex  );

        moris::Cell< std::shared_ptr<moris::ge::Geometry> > tGeometryVector(2);
        tGeometryVector(0) = std::make_shared<moris::ge::Geometry_Field_HMR>(tCircleField);
        tGeometryVector(1) = std::make_shared<moris::ge::Geometry_Field_HMR>(tPlaneField);

         size_t tModelDimension = 2;
         moris::ge::Phase_Table tPhaseTable (2, moris::ge::Phase_Table_Structure::EXP_BASE_2);
         moris::ge::Geometry_Engine tGeometryEngine(tGeometryVector,tPhaseTable,tModelDimension);
         Model tXTKModel(tModelDimension,tInterpMesh,&tGeometryEngine);
         tXTKModel.mVerbose  =  false;

        //Specify decomposition Method and Cut Mesh ---------------------------------------
        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
        tXTKModel.decompose(tDecompositionMethods);

        // enrich using basis functions in b-spline mesh 0 and b-spline mesh 1
        tXTKModel.perform_basis_enrichment(EntityRank::BSPLINE, {{0,1}});

        Enrichment const & tEnrichment = tXTKModel.get_basis_enrichment();

        Enriched_Integration_Mesh & tEnrInteg = tXTKModel.get_enriched_integ_mesh(0);
        Enriched_Interpolation_Mesh & tEnrIP = tXTKModel.get_enriched_interp_mesh(0);

//        moris::Cell<std::string> tBasisSupportFields = tEnrInteg.create_basis_support_fields();

        std::string tEnrIgMeshFileName = "./xtk_exo/multiple_order_ig_mesh.exo";

        mtk::Writer_Exodus writer(&tEnrInteg);
        writer.write_mesh("", tEnrIgMeshFileName);
        writer.set_time(0.0);
//        writer.set_nodal_fields(tBasisSupportFields);
//
//        for(moris::uint i = 0; i < tBasisSupportFields.size(); i++)
//        {
//            moris_index tFieldIndex = tEnrInteg.get_field_index(tBasisSupportFields(i),EntityRank::NODE);
//            writer.write_nodal_field(tBasisSupportFields(i), tEnrInteg.get_field_data(tFieldIndex,EntityRank::NODE));
//        }

        // Write the fields
        writer.close_file();


        // Declare the fields related to enrichment strategy in output options
        Cell<std::string> tEnrichmentFieldNames = tEnrichment.get_cell_enrichment_field_names();

        xtk::Output_Options tOutputOptions;
        tOutputOptions.mAddNodeSets = false;
        tOutputOptions.mAddSideSets = true;
        tOutputOptions.mAddClusters = false;
        tOutputOptions.mRealElementExternalFieldNames = tEnrichmentFieldNames;

        // add solution field to integration mesh
        std::string tIntegSolFieldName = "solution";
        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldName};

        moris::mtk::Mesh* tCutMeshData = tXTKModel.get_output_mesh(tOutputOptions);

        tEnrichment.write_cell_enrichment_to_fields(tEnrichmentFieldNames,tCutMeshData);

        std::string tMeshOutputFile ="./xtk_exo/xtk_hmr_2d_enr_trun_ig.e";
        tCutMeshData->create_output_mesh(tMeshOutputFile);

        delete tCutMeshData;
        delete tInterpMesh;
    }
}

}

