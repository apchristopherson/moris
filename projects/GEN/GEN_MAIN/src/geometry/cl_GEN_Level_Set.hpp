#ifndef MORIS_CL_GEN_LEVEL_SET_HPP
#define MORIS_CL_GEN_LEVEL_SET_HPP

#include "cl_GEN_Geometry.hpp"
#include "cl_GEN_Field_Discrete.hpp"
#include "cl_MTK_Mesh_Core.hpp"

namespace moris
{
    namespace ge
    {
        class Level_Set : public Geometry, public Field_Discrete
        {

        private:
            mtk::Mesh* mMesh;
            uint mNumOriginalNodes;
            Cell<std::shared_ptr<Child_Node>> mChildNodes;

        public:
            /**
             * Constructor where ADVs are given directly.
             *
             * @param aADVs Reference to the full advs
             * @param aGeometryVariableIndices Indices of geometry variables to be filled by the ADVs
             * @param aADVIndices The indices of the ADV vector to fill in the geometry variables
             * @param aConstantParameters The constant parameters not filled by ADVs
             * @param aMesh The mesh pointer where the B-spline information can be obtained
             * @param aNumRefinements The number of refinement steps to use for this geometry
             * @param aRefinementFunctionIndex The index of a user-defined refinement function (-1 = default refinement)
             * @param aBSplineMeshIndex The index of a B-spline mesh for level set discretization
             */
            Level_Set(Matrix<DDRMat>& aADVs,
                      Matrix<DDUMat>  aGeometryVariableIndices,
                      Matrix<DDUMat>  aADVIndices,
                      Matrix<DDRMat>  aConstantParameters,
                      mtk::Mesh*      aMesh,
                      sint            aNumRefinements = 0,
                      sint            aRefinementFunctionIndex = -1,
                      uint            aBSplineMeshIndex = 0,
                      real            aLevelSetLowerBound = -1.0,
                      real            aLevelSetUpperBound = 1.0);

            /**
             * Constructor where ADVs are added based on an input field and the B-spline mesh.
             *
             * @param aADVs Reference to the full advs
             * @param aMesh The mesh pointer where the B-spline information can be obtained
             * @param aGeometry Geometry for initializing the B-spline level set discretization
             */
            Level_Set(Matrix<DDRMat>&           aADVs,
                      mtk::Mesh*                aMesh,
                      std::shared_ptr<Geometry> aGeometry);

            /**
             * Given an index, the discrete geometry needs to return a field value.
             *
             * @param aNodeIndex Node index for field evaluation
             * @return field value at the specified index
             */
            real evaluate_field_value(uint aNodeIndex);

            /**
             * Given an index, returns sensitivites of the geometry with respect to input parameters
             *
             * @param aNodeIndex Node index for field evaluation
             * @param aSensitivities Matrix of sensitivities to be returned
             */
            void evaluate_all_sensitivities(uint aNodeIndex, Matrix<DDRMat>& aSensitivities);

            /**
             * Add a new child node for evaluation
             *
             * @param aNodeIndex Index of the child node
             * @param aChildNode Contains information about how the child node was created
             */
            void add_child_node(uint aNodeIndex, std::shared_ptr<Child_Node> aChildNode);

            /**
             * Function for determining if this geometry is to be used for seeding a B-spline level set field.
             *
             * @return false
             */
            bool conversion_to_level_set();

        };
    }
}

#endif //MORIS_CL_GEN_LEVEL_SET_HPP
