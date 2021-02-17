#ifndef MORIS_CL_GEN_GRAIN_HPP
#define MORIS_CL_GEN_GRAIN_HPP

#include "cl_GEN_Geometry.hpp"
#include "cl_GEN_Field_Analytic.hpp"
#include "cl_Library_IO.hpp"

namespace moris
{
    namespace ge
    {
        class Single_Grain : public Geometry, public Field_Analytic
        {

        private:

            std::shared_ptr<Geometry> mVoxelGeometry = nullptr;
            uint                      mIndex = MORIS_UINT_MAX;

        public:

            /**
             * Constructor, sets the pointers to advs and constant parameters for evaluations.
             *
             * @tparam Vector_Type Type of vector where ADVs are stored
             * @param aADVs ADV vector
             * @param aGeometryVariableIndices Indices of geometry variables to be filled by the ADVs
             * @param aADVIndices The indices of the ADV vector to fill in the geometry variables
             * @param aConstants The constant field variables not filled by ADVs
             * @param aName Name of this field for identification
             * @param aNumRefinements The number of refinement steps to use for this field
             * @param aRefinementMeshIndices Indices of meshes to perform refinement on
             * @param aRefinementFunctionIndex The index of a user-defined refinement function (-1 = {} refinement)
             */
            template <typename Vector_Type>
            Single_Grain(
                    Vector_Type&              aADVs,
                    Matrix<DDUMat>            aGeometryVariableIndices,
                    Matrix<DDUMat>            aADVIndices,
                    Matrix<DDRMat>            aConstants,
                    std::shared_ptr<Geometry> aVoxelGeometry,
                    uint                      aIndex,
                    Geometry_Field_Parameters aParameters = {})
                    : Field(aADVs, aGeometryVariableIndices, aADVIndices, aConstants, aParameters)
                    , Geometry(aParameters)
                    , mVoxelGeometry(aVoxelGeometry)
                    , mIndex(aIndex)
            {
            }

            /**
             * Constructor with only constant parameters
             *
             * @param aConstants The constant field variables not filled by ADVs
             * @param aParameters Additional parameters
             */
            Single_Grain(
                    Matrix<DDRMat>            aConstants,
                    std::shared_ptr<Geometry> aVoxelGeometry,
                    uint                      aIndex,
                    Geometry_Field_Parameters aParameters = {});

            /**
             * Given a node coordinate, returns the field value.
             *
             * @param aCoordinates Coordinate values
             * @return Distance to this geometry
             */
            real get_field_value(const Matrix<DDRMat>& aCoordinates);

            /**
             * Given a node coordinate, evaluates the sensitivity of the geometry field with respect to all of the
             * geometry variables.
             *
             * @param aCoordinates Coordinate values
             * @return Vector of sensitivities
             */
            const Matrix<DDRMat>& get_field_sensitivities(const Matrix<DDRMat>& aCoordinates);

        };
    }
}

#endif //MORIS_CL_GEN_VOXEL_HPP
