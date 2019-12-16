/*
 * cl_General_Functionalities.cpp
 *
 *  Created on: Oct 24, 2019
 *      Author: sonne
 */
#include "catch.hpp"

#include "cl_Matrix.hpp"
#include "fn_all_true.hpp"
#include "op_equal_equal.hpp"

// GE include -----------------------------------
#include "cl_GEN_Enums.hpp"
#include "cl_GEN_Phase_Table.hpp"

#include "cl_GEN_Field_User_Defined.hpp"

#include "cl_GEN_Geometry_Engine.hpp"

#include "cl_GEN_Circle.hpp"

// HMR includes ---------------------------------
#include "cl_HMR.hpp"

// MTK includes ---------------------------------
#include "cl_Mesh_Factory.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Writer_Exodus.hpp"

// XTK include ----------------------------------
#include "cl_XTK_Model.hpp"
#include "cl_XTK_Edge_Topology.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"
#include "cl_XTK_Enriched_Interpolation_Mesh.hpp"
//------------------------------------------------------------------------------

namespace moris
{
namespace ge
{
real tSimpleFunction( Matrix< DDRMat > const & aCoeff,
                      Matrix< DDRMat > const & aParam )
{
    return aCoeff(0)*aParam(0)*aParam(0) - aCoeff(1)*aParam(1);
}


//-----------------------------------------------------------------------------
TEST_CASE("general_test_01","[GE],[field_evaluation]")
{
    if(par_size()<=1)
    {
    /*
     * create 4-element stk mesh
     */
    uint aNumElemTypes = 1;     // quad
    uint aNumDim = 2;           // specify number of spatial dimensions

    Matrix< IdMat > aElementConnQuad = {{ 1, 2, 5, 8 },
                                        { 2, 3, 4, 5 },
                                        { 8, 5, 6, 7 },
                                        { 5, 4, 9, 6 }};        // specify element connectivity of quad for mesh

    Matrix< IdMat > aElemLocalToGlobalQuad = {{ 1, 2, 3, 4 }};      // specify the local to global element map for quads

    Matrix< DDRMat > aCoords = {{ 0.0, 0.0 },
                                { 1.0, 0.0 },
                                { 2.0, 0.0 },
                                { 2.0, 1.0 },
                                { 1.0, 1.0 },
                                { 1.0, 2.0 },
                                { 0.0, 2.0 },
                                { 0.0, 1.0 },
                                { 2.0, 2.0 }};      // Node coordinate matrix

    Matrix< IdMat > aNodeLocalToGlobal = {{ 1, 2, 3, 4, 5, 6, 7, 8, 9 }};       // specify the local to global map
    //------------------------------------------------------------------------------
    // create MORIS mesh using MTK database
    mtk::MtkMeshData tMeshData( aNumElemTypes );
    tMeshData.CreateAllEdgesAndFaces = true;
    tMeshData.SpatialDim = & aNumDim;
    tMeshData.ElemConn(0) = & aElementConnQuad;
    //------------------------------------------------------------------------------
    tMeshData.NodeCoords = & aCoords;
    tMeshData.LocaltoGlobalElemMap(0) = & aElemLocalToGlobalQuad;
    tMeshData.LocaltoGlobalNodeMap = & aNodeLocalToGlobal;
    //------------------------------------------------------------------------------
    // create mesh pair
    mtk::Interpolation_Mesh* tInterpMesh = create_interpolation_mesh( MeshType::STK, tMeshData );
    mtk::Integration_Mesh*   tIntegMesh  = create_integration_mesh_from_interpolation_mesh( MeshType::STK,tInterpMesh );

    // place the pair in mesh manager
    mtk::Mesh_Manager tMeshManager;
    uint tMeshIndex = tMeshManager.register_mesh_pair( tInterpMesh, tIntegMesh );
    //-----------------------------------------------------------------------------
    /*
     * create geometry engine, register mesh and field
     */
    Matrix< DDRMat >tCoeff{ {5, 2} };
    moris::ge::GEN_Field* tUserDefField = new moris::ge::GEN_Field_User_Defined( tSimpleFunction, tCoeff );

    moris::ge::GEN_Geometry_Engine tGeomEng;
    moris_index tFieldIndex  = tGeomEng.register_field( tUserDefField );
    moris_index tGEMeshIndex = tGeomEng.set_mesh( &tMeshManager );
    /*
     * evaluate and check field values at nodes
     */
    Matrix< DDRMat > tNodeVals;
    tGeomEng.calc_field_vals_at_nodes( tGEMeshIndex, tFieldIndex, tNodeVals );

    Matrix< DDRMat > aSolVec{ {0,5,20,18,3,1,-4,-2,16} };

    bool tMatrixMatch = all_true( tNodeVals == aSolVec );
    CHECK( tMatrixMatch );

    //-----------------------------------------------------------------------------
//    std::string tOutputFile = "./general_test.exo";
//    tInterpMesh1->create_output_mesh(tOutputFile);
    //-----------------------------------------------------------------------------
    // clean up
    delete tInterpMesh;
    delete tIntegMesh;
//    delete tUserDefField;
    }
}

//-----------------------------------------------------------------------------
TEST_CASE("general_test_02","[GE],[basic_functions]")
{
    if(par_size()<=1)
    {
        uint tNumElemTypes = 1;     // quad
        uint tNumDim = 2;           // specify number of spatial dimensions

        Matrix< IdMat > tElementConnQuad = {{ 1, 2, 3, 4 }};   // specify element connectivity of quad for mesh

        Matrix< IdMat > tElemLocalToGlobalQuad = {{ 1 }};      // specify the local to global element map for quads

        Matrix< DDRMat > tCoords = {{ 0.0, 0.0 },
                                    { 1.0, 0.0 },
                                    { 1.0, 1.0 },
                                    { 0.0, 1.0 }};             // Node coordinate matrix

        Matrix< IdMat > tNodeLocalToGlobal = {{ 1, 2, 3, 4 }}; // specify the local to global map
        //------------------------------------------------------------------------------
        // create MORIS mesh using MTK database
        mtk::MtkMeshData tMeshData( tNumElemTypes );
        tMeshData.CreateAllEdgesAndFaces = true;
        tMeshData.SpatialDim = & tNumDim;
        tMeshData.ElemConn(0) = & tElementConnQuad;
        //------------------------------------------------------------------------------
        tMeshData.NodeCoords = & tCoords;
        tMeshData.LocaltoGlobalElemMap(0) = & tElemLocalToGlobalQuad;
        tMeshData.LocaltoGlobalNodeMap = & tNodeLocalToGlobal;
        //------------------------------------------------------------------------------
        // declare scalar node field for the circle LS
        moris::mtk::Scalar_Field_Info<DDRMat> tNodeField1;
        std::string tFieldName = "circle";
        tNodeField1.set_field_name(tFieldName);
        tNodeField1.set_field_entity_rank(EntityRank::NODE);

        // initialize field information container
        moris::mtk::MtkFieldsInfo tFieldsInfo;
        // Place the node field into the field info container
        add_field_for_mesh_input(&tNodeField1,tFieldsInfo);

        // declare some supplementary fields
        tMeshData.FieldsInfo = &tFieldsInfo;

        // create mesh pair
        mtk::Interpolation_Mesh* tInterpMesh = create_interpolation_mesh( MeshType::STK, tMeshData );
        mtk::Integration_Mesh*   tIntegMesh  = create_integration_mesh_from_interpolation_mesh( MeshType::STK,tInterpMesh );

        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        uint tMeshIndex = tMeshManager.register_mesh_pair( tInterpMesh, tIntegMesh );
        //------------------------------------------------------------------------------
        real tRadius  = 0.6;
        real tXcenter = 0;
        real tYcenter = 0;
        moris::ge::Circle tCircle( tRadius, tXcenter, tYcenter );

        moris::ge::GEN_Phase_Table      tPhaseTable( 1,  Phase_Table_Structure::EXP_BASE_2 );
        moris::ge::GEN_Geometry_Engine  tGENGeometryEngine( tCircle, tPhaseTable, tNumDim );

        //=================== manual calls to GE (w/out XTK model) =============================
        uint tNumNodes = tInterpMesh->get_num_nodes();
        tGENGeometryEngine.initialize_geometry_objects_for_background_mesh_nodes( tNumNodes );

        Matrix< DDRMat > tLSVals( tNumNodes,1 );
        for( uint i=0; i<tNumNodes; i++ )
        {
            tGENGeometryEngine.initialize_geometry_object_phase_values( tCoords );
            tLSVals(i) = tGENGeometryEngine.get_entity_phase_val( i,0 );
        }
        tInterpMesh->add_mesh_field_real_scalar_data_loc_inds(tFieldName, EntityRank::NODE, tLSVals);

        moris::Matrix< moris::IndexMat > tNodetoEdgeConnectivity(4, 2);
        // Edge 0
        (tNodetoEdgeConnectivity)(0, 0) = 0;
        (tNodetoEdgeConnectivity)(0, 1) = 1;
        // Edge 1
        (tNodetoEdgeConnectivity)(1, 0) = 1;
        (tNodetoEdgeConnectivity)(1, 1) = 2;
        // Edge 2
        (tNodetoEdgeConnectivity)(2, 0) = 2;
        (tNodetoEdgeConnectivity)(2, 1) = 3;
        // Edge 3
        (tNodetoEdgeConnectivity)(3, 0) = 3;
        (tNodetoEdgeConnectivity)(3, 1) = 0;

        Cell< GEN_Geometry_Object > tGeometryObjects;
        tGENGeometryEngine.is_intersected( tCoords, tNodetoEdgeConnectivity, (size_t) 1, tGeometryObjects );

        size_t tNumIntersections = tGeometryObjects.size();
        //======================================
        REQUIRE( tNumIntersections == 2 );  // there should be two intersected edges
        //======================================
        Matrix< DDRMat >   tLclCoord( tNumIntersections,1 );
        Matrix< DDRMat >   tNodeCoords( tNumIntersections,tNumDim );
        for( uint i=0; i<tNumIntersections; i++ )
        {
            tLclCoord(i) = tGeometryObjects(i).get_interface_lcl_coord();
            tNodeCoords.set_row( i,tGeometryObjects(i).get_interface_glb_coord() );
        }
        //======================================
        REQUIRE( tLclCoord(0) =  0.2 );
        REQUIRE( tLclCoord(1) = -0.2 );
        REQUIRE( tNodeCoords(0,0) = 0.6 );
        REQUIRE( tNodeCoords(1,1) = 0.6 );
        //======================================
        Matrix< moris::IndexMat > tEdgeIndices00{{ 0,1 }};
        std::shared_ptr< xtk::Topology > tTop00 = std::make_shared< xtk::Edge_Topology >( tEdgeIndices00 );
        Matrix< moris::IndexMat > tEdgeIndices01{{ 0,1 }};
        std::shared_ptr< xtk::Topology > tTop01 = std::make_shared< xtk::Edge_Topology >( tEdgeIndices01 );
        moris::Cell< xtk::Topology* > tTopCell(2);
        tTopCell(0) = tTop00.get();          tTopCell(1) = tTop01.get();

        moris::Cell< moris_index > tIndices(2);
        tIndices(0) = 78;   tIndices(1) = 99;
        moris::Cell< Matrix<DDRMat> > tLocalCoords(2);
        tLocalCoords(0) = {{0.2}};  tLocalCoords(1) = {{-0.2}};
        moris::Cell< Matrix<DDRMat> > tGlbCoords(2);
        tGlbCoords(0) = tNodeCoords.get_row(0); tGlbCoords(1) = tNodeCoords.get_row(1);

        tGENGeometryEngine.create_new_node_geometry_objects( tIndices, true, tTopCell, tLocalCoords, tGlbCoords );

        Matrix< IndexMat > tInd{{ tIndices(0),tIndices(1) }};
        tGENGeometryEngine.compute_interface_sensitivity( tInd, tNodeCoords, (moris_index)0 );

        GEN_Geometry_Object tGeomObj01 = tGENGeometryEngine.get_geometry_object( tIndices(0) );
        moris::Matrix< moris::DDRMat >tSens01 = tGeomObj01.get_sensitivity_dx_dp();
        //================================== end ===============================================

        //------------------------------------------------------------------------------
//        std::string tOutputFile = "./testing_output.exo";
//        tInterpMesh->create_output_mesh(tOutputFile);
        delete tInterpMesh;
        delete tIntegMesh;
        //------------------------------------------------------------------------------
    }
}

//-----------------------------------------------------------------------------
TEST_CASE("general_test_03","[GE],[sensitivity_check]")
{
    if(par_size()<=1)
    {
        size_t tModelDimension = 2;
        uint tLagrangeMeshIndex = 0;
        //  HMR Parameters setup
        hmr::ParameterList tParameters = hmr::create_hmr_parameter_list();

        tParameters.set( "number_of_elements_per_dimension", "2, 2" );
        tParameters.set( "domain_dimensions",                "4, 4" );
        tParameters.set( "domain_offset",                    "-2, -2" );

//        tParameters.set( "domain_sidesets",      "1, 2, 3, 4, 5, 6" );

        tParameters.set( "truncate_bsplines", 1 );
        tParameters.set( "lagrange_orders", "1" );
        tParameters.set( "lagrange_pattern", "0" );
        tParameters.set( "bspline_orders", "1" );
        tParameters.set( "bspline_pattern", "0" );

        tParameters.set( "lagrange_output_meshes", "0" );
        tParameters.set( "lagrange_input_meshes", "0" );

        tParameters.set( "lagrange_to_bspline", "0" );

        tParameters.set( "use_multigrid", 0 );

        tParameters.set( "refinement_buffer", 1 );
        tParameters.set( "staircase_buffer", 1 );

        tParameters.insert( "initial_refinement", 0 );

        //  HMR Initialization
        moris::hmr::HMR tHMR( tParameters );

        std::shared_ptr< hmr::Database > tDatabase = tHMR.get_database();

        tHMR.perform_initial_refinement( 0 );

        tDatabase->update_bspline_meshes();
        tDatabase->update_lagrange_meshes();

        std::shared_ptr< moris::hmr::Mesh > tHMRMesh = tHMR.create_mesh( tLagrangeMeshIndex );
        tHMR.finalize();

        std::shared_ptr< hmr::Interpolation_Mesh_HMR >      tInterpMesh      = tHMR.create_interpolation_mesh( tLagrangeMeshIndex );
//        std::shared_ptr< moris::hmr::Integration_Mesh_HMR > tIntegrationMesh = tHMR.create_integration_mesh( 1, 0, *tInterpMesh );

        //------------------------------------------------------------------------------
        real tRadius  = 1.0;
        real tXCenter = 0.0;
        real tYCenter = 0.0;

        moris::ge::Circle tCircleGeom( tRadius, tXCenter, tYCenter );

        moris::ge::GEN_Phase_Table      tPhaseTable( 1,  Phase_Table_Structure::EXP_BASE_2 );
        moris::ge::GEN_Geometry_Engine  tGENGeometryEngine( tCircleGeom, tPhaseTable, tModelDimension );

        //------------------------------------------------------------------------------
        xtk::Model tXTKModel( tModelDimension, tInterpMesh.get(), tGENGeometryEngine );
        tXTKModel.mVerbose = false;
        Cell<enum Subdivision_Method> tDecompositionMethods = { Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4 };
        tXTKModel.decompose(tDecompositionMethods);

        tXTKModel.perform_basis_enrichment( EntityRank::BSPLINE_1,0 );

        xtk::Enriched_Interpolation_Mesh & tEnrInterpMesh = tXTKModel.get_enriched_interp_mesh();
        xtk::Enriched_Integration_Mesh   & tEnrIntegMesh  = tXTKModel.get_enriched_integ_mesh();

        // place the pair in mesh manager
        mtk::Mesh_Manager tMeshManager;
        tMeshManager.register_mesh_pair( &tEnrInterpMesh, &tEnrIntegMesh);

        xtk::Output_Options tOutputOptions;
        tOutputOptions.mAddNodeSets = false;
        tOutputOptions.mAddSideSets = false;
        tOutputOptions.mAddClusters = false;

        std::string tCircleFieldName = "circle";
        tOutputOptions.mRealNodeExternalFieldNames = { tCircleFieldName };

        moris::mtk::Integration_Mesh* tIntegMesh1 = tXTKModel.get_output_mesh(tOutputOptions);
        //------------------------------------------------------------------------------
        // get level-set field and add to integration mesh output
        uint tNumNodes = tIntegMesh1->get_num_entities( EntityRank::NODE );

        moris::Cell< moris::mtk::Vertex const * > tAllVerices = tIntegMesh1->get_all_vertices();

        Matrix< DDRMat > tAllCoords( tNumNodes, 2 );
        for(uint i=0; i<tNumNodes; i++ )
        {
            tAllCoords.set_row( i,tAllVerices(i)->get_coords() );
        }

        tGENGeometryEngine.initialize_geometry_objects_for_background_mesh_nodes( tNumNodes );
        tGENGeometryEngine.initialize_geometry_object_phase_values( tAllCoords );

//        Matrix< IndexMat > tNodeToEdgeConnectivity( tNumNodes,2 );
//        Matrix< DDRMat > tLSVals( tNumNodes, 1, -1.0 );
//        for( uint i=0; i<tNumNodes; i++ )
//        {
//            tLSVals(i) = tGENGeometryEngine.get_entity_phase_val( i,0 );
//        }
//        tIntegMesh1->add_mesh_field_real_scalar_data_loc_inds( tCircleFieldName, EntityRank::NODE, tLSVals );
//        //------------------------------------------------------------------------------
//        Matrix< IndexMat > tNodeToEdgeConnectivity =
//
//        Cell< GEN_Geometry_Object > tGeometryObjects;
//        tGENGeometryEngine.is_intersected( tAllCoords, tNodeToEdgeConnectivity, (size_t) 1, tGeometryObjects );
//
//        size_t tNumIntersections = tGeometryObjects.size();
//        std::cout<<"number of intersected edges: "<<tNumIntersections<<std::endl;

        //------------------------------------------------------------------------------
//        std::string tMeshOutputFile = "circleMesh.e";
//        tIntegMesh1->create_output_mesh(tMeshOutputFile);

//        std::string tMorisRoot = std::getenv("MORISROOT");
//        std::string tMeshFilePath = tMorisRoot + "build" ;
//
//        Writer_Exodus tWriterInteg( tIntegMesh1 );
//        tWriterInteg.write_mesh( tMeshFilePath, "testMeshXTK_integ.e" );
//        tWriterInteg.close_file();

        //------------------------------------------------------------------------------
        delete tIntegMesh1;
//        for(uint k=0; k<tAllVerices.size(); k++)
//        {
//            delete tAllVerices(k);
//        }
    }
}
}   // ge namespace
}   // moris namespace
