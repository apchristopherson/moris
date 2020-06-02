/*
 * UT_XTK_Parameters.cpp
 *
 *  Created on: Mar 26, 2020
 *      Author: doble
 */

#include <memory>
#include "catch.hpp"
#include "cl_XTK_Model.hpp"
#include "cl_XTK_Enums.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"
#include "xtk/cl_XTK_Enrichment.hpp"
#include "xtk/fn_write_element_ownership_as_field.hpp"

#include "topology/cl_XTK_Hexahedron_8_Basis_Function.hpp"

#include "cl_MTK_Visualization_STK.hpp"
#include "Child_Mesh_Verification_Utilities.hpp"

#include "cl_GEN_Sphere.hpp"

// Linalg includes
#include "cl_Matrix.hpp"
#include "fn_all_true.hpp"
#include "op_equal_equal.hpp"
#include "fn_norm.hpp"
#include "op_minus.hpp"

#include "cl_PRM_XTK_Parameters.hpp"
#include "cl_Param_List.hpp"


namespace xtk
{

TEST_CASE("XTK Parameter List","[PARAM]")
                {
    // Geometry Engine Setup ---------------------------------------------------------
    // Using a Levelset Sphere as the Geometry

    real tRadius  = 0.25;
    real tXCenter = 1.0;
    real tYCenter = 1.0;
    real tZCenter = 0.0;

    Cell<std::shared_ptr<moris::ge::Geometry>> tGeometry(1);
    tGeometry(0) = std::make_shared<moris::ge::Sphere>(tXCenter, tYCenter, tZCenter, tRadius);

    moris::ge::Phase_Table tPhaseTable (1, moris::ge::Phase_Table_Structure::EXP_BASE_2);
    moris::ge::Geometry_Engine tGeometryEngine(tGeometry, tPhaseTable);


    // Create Mesh --------------------------------------------------------------------
    std::string tMeshFileName = "generated:1x1x4";
    moris::mtk::Interpolation_Mesh* tMeshData = moris::mtk::create_interpolation_mesh( MeshType::STK, tMeshFileName, NULL );
    std::string tBMOutputFile ="./xtk_exo/xtk_test_output_conformal_bm.e";
    tMeshData->create_output_mesh(tBMOutputFile);

    ParameterList tXTKParams = prm::create_xtk_parameter_list();

    // set enrichment
    tXTKParams.set( "enrich", true );
    tXTKParams.set( "enrich_mesh_indices",std::string("0"));
    tXTKParams.set( "high_to_low_dbl_side_sets",true);

    // set ghost
    tXTKParams.set( "ghost_stab", false );

    std::shared_ptr< mtk::Mesh_Manager > tMeshManager = std::make_shared< mtk::Mesh_Manager >();

    Model tXTKModel(tXTKParams);
    tXTKModel.mVerbose = false;
    tXTKModel.set_geometry_engine(&tGeometryEngine);
    tXTKModel.set_mtk_background_mesh(tMeshData);
    tXTKModel.set_output_performer(tMeshManager);

    tXTKModel.perform();

    // Access the Cut Mesh-------------------------------------------------------------
    Cut_Mesh const & tCutMesh = tXTKModel.get_cut_mesh();

    // Do some testing
    size_t tNumNodesAfterDecompositionSTK    = tXTKModel.get_background_mesh().get_num_entities(EntityRank::NODE);
    size_t tNumElementsAfterDecompositionSTK = tXTKModel.get_background_mesh().get_num_entities(EntityRank::ELEMENT);
    size_t tNumNodesAfterDecompositionXTK    = tCutMesh.get_num_entities(EntityRank::NODE);
    size_t tNumElementsAfterDecompositionXTK = tCutMesh.get_num_entities(EntityRank::ELEMENT);

    if(par_size() == 1)
    {
        CHECK(tNumNodesAfterDecompositionXTK == 22 );
        CHECK(tNumElementsAfterDecompositionXTK == 42);

        CHECK(tNumElementsAfterDecompositionSTK == 46);
        // But it does add nodes because this is where the coordinates are stored
        CHECK(tNumNodesAfterDecompositionSTK == 34);
    }

    moris::Matrix< moris::DDRMat > tNodeCoordinates = tXTKModel.get_background_mesh().get_all_node_coordinates_loc_inds();

    // verify ancestry of child mesh
    verify_child_mesh_ancestry(tXTKModel.get_background_mesh(),
                               tCutMesh);

    // Verify parametric coordinates reproduce the same results as tabulated in global node coordinates

    // Basis function to use for hex8
    Hexahedron_8_Basis_Function tHex8Basis;

    // Allocate a basis function weight matrix
    moris::Matrix< moris::DDRMat > tBasisWeights(1,8);

    // tolerance for difference between coordinates
    real tTol = 1e-12;

    // Iterate over child meshes
    for(size_t iCM = 0; iCM < tCutMesh.get_num_child_meshes(); iCM++)
    {
        // Get reference to child mesh
        Child_Mesh const & tChildMesh = tCutMesh.get_child_mesh(iCM);

        // iterate over nodes
        size_t tNumNodes = tChildMesh.get_num_entities(EntityRank::NODE);

        // Get child node indices from the child mesh (note these aren't guaranteed to be monotonically increasing)
        moris::Matrix<moris::IndexMat> const & tNodeIndicesOfCM = tChildMesh.get_node_indices();

        // Parent element index
        moris::moris_index tParentIndex = tChildMesh.get_parent_element_index();

        // Nodes attached to parent element
        moris::Matrix< moris::IndexMat > tNodesAttachedToParentElem = tMeshData->get_entity_connected_to_entity_loc_inds(tParentIndex,moris::EntityRank::ELEMENT,moris::EntityRank::NODE);

        // Get the node coordinates
        moris::Matrix< moris::DDRMat > tHex8NodeCoords = tXTKModel.get_background_mesh().get_selected_node_coordinates_loc_inds(tNodesAttachedToParentElem);

        for(size_t i= 0; i<tNumNodes; i++)
        {
            // Node index
            moris::moris_index tNodeIndex = tNodeIndicesOfCM(i);

            // Get the nodes parametric coordinate
            moris::Matrix<moris::DDRMat> tNodeParamCoord = tChildMesh.get_parametric_coordinates(tNodeIndex);

            // Get the basis function values at this point
            tHex8Basis.evaluate_basis_function(tNodeParamCoord,tBasisWeights);

            // Evaluate the nodes global coordinate from the basis weights
            moris::Matrix<moris::DDRMat> tInterpNodeCoord = tBasisWeights*tHex8NodeCoords;

            // Verify the interpolated coordinate is equal to the node coordinate row
            CHECK(norm(tInterpNodeCoord - tNodeCoordinates.get_row(tNodeIndex)) < tTol);
        }

    }

    // verify the interface nodes have level set values sufficiently close to 0
    // when interpolated to
    moris::Matrix<moris::IndexMat> tInterfaceNodes = tXTKModel.get_background_mesh().get_interface_nodes_loc_inds(0);

    // Iterate over interface nodes
    for(size_t iIN = 0; iIN < tInterfaceNodes.numel(); iIN++)
    {
        // Get the child meshes these nodes belong to
        moris::Matrix<moris::IndexMat> tCMIndices = tXTKModel.get_background_mesh().get_node_child_mesh_assocation(tInterfaceNodes(iIN));

        // Iterate over child meshes this node belongs to
        for(size_t iCM = 0; iCM<tCMIndices.numel(); iCM++)
        {
            // Get reference to child mesh
            Child_Mesh const & tChildMesh = tCutMesh.get_child_mesh(tCMIndices(iCM));

            // Parent element index
            moris::moris_index tParentIndex = tChildMesh.get_parent_element_index();

            // Nodes attached to parent element
            moris::Matrix< moris::IndexMat > tNodesAttachedToParentElem = tMeshData->get_entity_connected_to_entity_loc_inds(tParentIndex,moris::EntityRank::ELEMENT,moris::EntityRank::NODE);

            // Get the level set values of each node on the hex8
            moris::Matrix< moris::DDRMat > tHex8LSVs(8,1);
            for(size_t iNode = 0; iNode<8; iNode++)
            {
                tHex8LSVs(iNode)  = tXTKModel.get_geom_engine()->get_entity_phase_val(tNodesAttachedToParentElem(iNode),0);
            }

            // Get the interface node parametric coordinate in iCM
            moris::Matrix<moris::DDRMat> tNodeParamCoord = tChildMesh.get_parametric_coordinates(tInterfaceNodes(iIN));

            // Get the basis function values at this point
            tHex8Basis.evaluate_basis_function(tNodeParamCoord,tBasisWeights);

            // Evaluate the nodes global coordinate from the basis weights
            moris::Matrix<moris::DDRMat> tInterfaceLSV = tBasisWeights*tHex8LSVs;

            // Verify it is  approximately 0.0
            CHECK(approximate(tInterfaceLSV(0),0.0) );

        }
    }


    delete tMeshData;
                }
}
