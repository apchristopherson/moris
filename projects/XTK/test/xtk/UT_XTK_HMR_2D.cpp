/*
 * UT_XTK_HMR_2D.cpp
 *
 *  Created on: Sep 10, 2019
 *      Author: doble
 */

#include "catch.hpp"

#include "cl_XTK_Model.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"

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
#include "cl_HMR_Parameters.hpp" //HMR/src

#include "cl_GEN_Plane.hpp"
#include "cl_GEN_Geometry_Field_HMR.hpp"

#include "fn_norm.hpp"

namespace xtk
{
moris::real
CircleFuncXTKHMR2D(const moris::Matrix< moris::DDRMat > & aPoint )
{

    moris::real mXCenter = 0;
    moris::real mYCenter = 0;
    moris::real mRadius = 1.1;

    return    (aPoint(0) - mXCenter) * (aPoint(0) - mXCenter)
            + (aPoint(1) - mYCenter) * (aPoint(1) - mYCenter)
            - (mRadius * mRadius);
}

TEST_CASE("2D XTK WITH HMR","[XTK_HMR_2D]")
{
    if(par_size()<=2)
    {
        for( moris::uint iOrder = 1; iOrder < 4; iOrder ++)
        {

            std::string tFieldName = "Cylinder";

            moris::uint tLagrangeMeshIndex = 0;
            moris::uint tBSplineMeshIndex = 0;

            moris::hmr::Parameters tParameters;

            tParameters.set_number_of_elements_per_dimension( { {24}, {24}} );
            tParameters.set_domain_dimensions({ {2}, {2} });
            tParameters.set_domain_offset({ {-1.0}, {-1.0} });
            tParameters.set_bspline_truncation( true );

            tParameters.set_output_meshes( { {0} } );

            tParameters.set_lagrange_orders  ( { {iOrder} });
            tParameters.set_lagrange_patterns({ {0} });

            tParameters.set_bspline_orders   ( { {iOrder} } );
            tParameters.set_bspline_patterns ( { {0} } );

            tParameters.set_side_sets({{1},{2},{3},{4} });

            tParameters.set_union_pattern( 2 );
            tParameters.set_working_pattern( 3 );

            tParameters.set_refinement_buffer( 2 );
            tParameters.set_staircase_buffer( 2);
            tParameters.set_number_aura(true);

            Cell< Matrix< DDSMat > > tLagrangeToBSplineMesh( 1 );
            tLagrangeToBSplineMesh( 0 ) = { {0} };

            tParameters.set_lagrange_to_bspline_mesh( tLagrangeToBSplineMesh );

            hmr::HMR tHMR( tParameters );

            std::shared_ptr< moris::hmr::Mesh > tMesh = tHMR.create_mesh( tLagrangeMeshIndex );

            // create field
            std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );

            tField->evaluate_scalar_function( CircleFuncXTKHMR2D );

            for( uint k=0; k<3; ++k )
            {
                tHMR.flag_surface_elements_on_working_pattern( tField );
                tHMR.perform_refinement_based_on_working_pattern( 0 );

                tField->evaluate_scalar_function( CircleFuncXTKHMR2D );
            }

            tHMR.finalize();

//            tHMR.save_to_exodus( 0, "./xtk_exo/xtk_hmr_2d_ip.e" );

            hmr::Interpolation_Mesh_HMR * tInterpMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex  );

            moris::ge::Geometry_Field_HMR tFieldAsGeom(tField);

        moris::Cell< std::shared_ptr<moris::ge::Geometry_Discrete> > tGeometryVector(1);
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


            xtk::Enriched_Integration_Mesh & tEnrIgMesh = tXTKModel.get_enriched_integ_mesh(0);

            tXTKModel.construct_face_oriented_ghost_penalization_cells();


            // output to exodus file ----------------------------------------------------------
//            xtk::Enrichment const & tEnrichment = tXTKModel.get_basis_enrichment();

            // Declare the fields related to enrichment strategy in output options
//            Cell<std::string> tEnrichmentFieldNames = tEnrichment.get_cell_enrichment_field_names();

            // output solution and meshes

            // Write mesh
//            moris::mtk::Writer_Exodus writer(&tEnrIgMesh);
//            writer.write_mesh("", "./xtk_exo/xtk_hmr_2d_ig.exo");

            // Write the fields
//            writer.set_time(0.0);
//            writer.close_file();


            delete tInterpMesh;
        }
    }
}

TEST_CASE("2D XTK WITH HMR WEIRD INTERSECTION","[XTK_HMR_2D_WI]")
{
    if(par_size()<=1)
    {
        std::string tFieldName = "Cylinder";

         moris::uint tLagrangeMeshIndex = 0;
         moris::uint tBSplineMeshIndex = 0;

         moris::hmr::Parameters tParameters;

         tParameters.set_number_of_elements_per_dimension( { {2}, {2}} );
         tParameters.set_domain_dimensions({ {2}, {2} });
         tParameters.set_domain_offset({ {-1.0}, {-1.0} });
         tParameters.set_bspline_truncation( true );

         tParameters.set_output_meshes( { {0} } );

         tParameters.set_lagrange_orders  ( { {1} });
         tParameters.set_lagrange_patterns({ {0} });

         tParameters.set_bspline_orders   ( { {1} } );
         tParameters.set_bspline_patterns ( { {0} } );

         tParameters.set_side_sets({{1},{2},{3},{4} });

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
         std::shared_ptr< moris::hmr::Field > tField = tMesh->create_field( tFieldName, tLagrangeMeshIndex );

         tField->evaluate_scalar_function( CircleFuncXTKHMR2D );

         for( uint k=0; k<2; ++k )
         {
             tHMR.flag_surface_elements_on_working_pattern( tField );
             tHMR.perform_refinement_based_on_working_pattern( 0 );

             tField->evaluate_scalar_function( CircleFuncXTKHMR2D );
         }

         tHMR.finalize();

         tHMR.save_to_exodus( 0, "./xtk_exo/xtk_hmr_wi_2d_ip.e" );

         hmr::Interpolation_Mesh_HMR * tInterpMesh = tHMR.create_interpolation_mesh( tLagrangeMeshIndex  );

         // create a plane which intentionally intersects from fine to coarse
         moris::Matrix<moris::DDRMat> tCenters = {{ 0.1,0.1 }};
         moris::Matrix<moris::DDRMat> tNormals = {{ 1.0,0.0 }};
        Cell<std::shared_ptr<moris::ge::Geometry_Analytic>> tGeometry(1);
        tGeometry(0) = std::make_shared<moris::ge::Plane>(tCenters(0), tCenters(1), tNormals(0), tNormals(1));

         size_t tModelDimension = 2;
         moris::ge::Phase_Table tPhaseTable (1, moris::ge::Phase_Table_Structure::EXP_BASE_2);
         moris::ge::Geometry_Engine tGeometryEngine(tGeometry,tPhaseTable,tModelDimension);
         Model tXTKModel(tModelDimension,tInterpMesh,&tGeometryEngine);
         tXTKModel.mVerbose  =  false;

        //Specify decomposition Method and Cut Mesh ---------------------------------------
        Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4, Subdivision_Method::C_TRI3};
        tXTKModel.decompose(tDecompositionMethods);

        tXTKModel.perform_basis_enrichment(EntityRank::BSPLINE,0);

        // output to exodus file ----------------------------------------------------------
        xtk::Enrichment const & tEnrichment = tXTKModel.get_basis_enrichment();


         // Declare the fields related to enrichment strategy in output options
         Cell<std::string> tEnrichmentFieldNames = tEnrichment.get_cell_enrichment_field_names();

        // output solution and meshes
        xtk::Output_Options tOutputOptions;
        tOutputOptions.mAddNodeSets = false;
        tOutputOptions.mAddSideSets = true;
        tOutputOptions.mAddClusters = false;

        // add solution field to integration mesh
        std::string tIntegSolFieldName = "solution";
        tOutputOptions.mRealNodeExternalFieldNames = {tIntegSolFieldName};
        tOutputOptions.mRealElementExternalFieldNames = tEnrichmentFieldNames;

        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);

        tEnrichment.write_cell_enrichment_to_fields(tEnrichmentFieldNames,tIntegMesh1);

        std::string tMeshOutputFile ="./xtk_exo/xtk_hmr_2d_wi_ig.e";
        tIntegMesh1->create_output_mesh(tMeshOutputFile);

        delete tIntegMesh1;
        delete tInterpMesh;
    }
}

}
