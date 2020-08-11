#include "cl_GEN_Level_Set.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Mesh_Factory.hpp"
#include "cl_MTK_Mapper.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        Level_Set::Level_Set(Matrix<DDRMat>& aADVs,
                             Matrix<DDUMat>  aGeometryVariableIndices,
                             Matrix<DDUMat>  aADVIndices,
                             Matrix<DDRMat>  aConstantParameters,
                             mtk::Mesh*      aMesh,
                             sint            aNumRefinements,
                             sint            aRefinementFunctionIndex,
                             uint            aBSplineMeshIndex,
                             real            aBSplineLowerBound,
                             real            aBSplineUpperBound)
                : Field(aADVs,
                        aGeometryVariableIndices,
                        aADVIndices,
                        aConstantParameters,
                        aNumRefinements,
                        aRefinementFunctionIndex,
                        aBSplineMeshIndex,
                        aBSplineLowerBound,
                        aBSplineUpperBound),
                  mMesh(aMesh),
                  mNumOriginalNodes(mMesh->get_num_nodes())
        {
            // Check that number of variables equals the number of B-spline coefficients
            MORIS_ASSERT(mFieldVariables.size() == mMesh->get_num_coeffs(aBSplineMeshIndex),
                    "There must be a field variable for each B-spline coefficient in a level set geometry.");
        }

        //--------------------------------------------------------------------------------------------------------------

        Level_Set::Level_Set(Matrix<DDRMat>&           aADVs,
                             uint                      aADVIndex,
                             mtk::Interpolation_Mesh*  aMesh,
                             std::shared_ptr<Geometry> aGeometry)
                : Field(aADVs,
                        aADVIndex,
                        aMesh->get_num_coeffs(aGeometry->get_bspline_mesh_index()),
                        aGeometry->get_num_refinements(),
                        aGeometry->get_refinement_function_index(),
                        aGeometry->get_bspline_mesh_index(),
                        aGeometry->get_bspline_lower_bound(),
                        aGeometry->get_bspline_upper_bound()),
                  mMesh(aMesh),
                  mNumOriginalNodes(mMesh->get_num_nodes())
        {
            // Check for L2 needed
            if (mNumOriginalNodes != mMesh->get_num_coeffs(this->get_bspline_mesh_index()))
            {
                // Set values ons source field
                Matrix<DDRMat> tSourceField(mNumOriginalNodes, 1);
                for (uint tNodeIndex = 0; tNodeIndex < mNumOriginalNodes; tNodeIndex++)
                {
                    tSourceField(tNodeIndex) = aGeometry->evaluate_field_value(tNodeIndex, mMesh->get_node_coordinate(tNodeIndex));
                }

                // Create mesh manager
                std::shared_ptr<mtk::Mesh_Manager> tMeshManager = std::make_shared<mtk::Mesh_Manager>();
                tMeshManager->register_mesh_pair(aMesh, create_integration_mesh_from_interpolation_mesh(MeshType::HMR, aMesh));

                // Use mapper
                mapper::Mapper tMapper(tMeshManager, 0, (uint)this->get_bspline_mesh_index());
                Matrix<DDRMat> tTargetField(0, 0);
                tMapper.perform_mapping(tSourceField,
                                        EntityRank::NODE,
                                        tTargetField,
                                        EntityRank::BSPLINE);

                // Assign ADVs
                for (uint tBSplineIndex = 0; tBSplineIndex < mMesh->get_num_coeffs(this->get_bspline_mesh_index()); tBSplineIndex++)
                {
                    aADVs(aADVIndex++) = tTargetField(tBSplineIndex);
                }
            }
            else // Nodal values, no L2
            {
                // Assign ADVs directly
                for (uint tNodeIndex = 0; tNodeIndex < mNumOriginalNodes; tNodeIndex++)
                {
                    aADVs(aADVIndex++) = aGeometry->evaluate_field_value(tNodeIndex, mMesh->get_node_coordinate(tNodeIndex));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        real Level_Set::evaluate_field_value(uint aNodeIndex)
        {
            // Initialize
            real tResult = 0.0;

            // If node is on original interpolation mesh
            if (aNodeIndex < mNumOriginalNodes)
            {
                Matrix<IndexMat> tIndices = mMesh->get_bspline_inds_of_node_loc_ind(aNodeIndex, EntityRank::BSPLINE);
                Matrix<DDRMat> tMatrix = mMesh->get_t_matrix_of_node_loc_ind(aNodeIndex, EntityRank::BSPLINE);
                for (uint tBspline = 0; tBspline < tIndices.length(); tBspline++)
                {
                    tResult += tMatrix(tBspline) * (*mFieldVariables(tIndices(tBspline)));
                }
            }

            // Otherwise, use child nodes
            else
            {
                MORIS_ASSERT((aNodeIndex - mNumOriginalNodes) < mChildNodes.size(),
                             "A level set field value was requested from a node that this geometry doesn't know. "
                             "Perhaps a child node was not added to this field?");
                tResult = mChildNodes(aNodeIndex - mNumOriginalNodes)->interpolate_geometry_field_value(this);
            }

            // Return result
            return tResult;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Level_Set::evaluate_all_sensitivities(uint aNodeIndex, Matrix<DDRMat>& aSensitivities)
        {
            // Initialize
            aSensitivities.set_size(1, mFieldVariables.size(), 0.0);

            // If node is on original interpolation mesh
            if (aNodeIndex < mNumOriginalNodes)
            {
                Matrix<IndexMat> tIndices = mMesh->get_bspline_inds_of_node_loc_ind(aNodeIndex, EntityRank::BSPLINE);
                Matrix<DDRMat> tMatrix = mMesh->get_t_matrix_of_node_loc_ind(aNodeIndex, EntityRank::BSPLINE);
                for (uint tBspline = 0; tBspline < tIndices.length(); tBspline++)
                {
                    aSensitivities(tIndices(tBspline)) = tMatrix(tBspline);
                }
            }

            // Otherwise, use child nodes
            else
            {
                MORIS_ASSERT((aNodeIndex - mNumOriginalNodes) < mChildNodes.size(),
                             "A level set sensitivity was requested from a node that this geometry doesn't know. "
                             "Perhaps a child node was not added to this field?");
                mChildNodes(aNodeIndex - mNumOriginalNodes)->interpolate_geometry_sensitivity(this, aSensitivities);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Level_Set::add_child_node(uint aNodeIndex, std::shared_ptr<Child_Node> aChildNode)
        {
            mChildNodes.push_back(aChildNode);
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Level_Set::conversion_to_bsplines()
        {
            return false;
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}