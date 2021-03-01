#ifndef MORIS_CL_GEN_BSPLINE_GEOMETRY_HPP
#define MORIS_CL_GEN_BSPLINE_GEOMETRY_HPP

#include "cl_GEN_BSpline_Field.hpp"
#include "cl_GEN_Geometry.hpp"

namespace moris
{
    namespace ge
    {
        class BSpline_Geometry : public BSpline_Field, public Geometry
        {
        public:

            /**
             * Constructor where ADVs are added based on an input field and a B-spline mesh.
             *
             * @param aOwnedADVs Pointer to the owned distributed ADVs
             * @param aCoefficientIndices Coefficient indices to be mapped to
             * @param aSharedADVIds All owned and shared ADV IDs for this B-spline field
             * @param aADVOffsetID Offset in the owned ADV IDs for pulling ADV IDs
             * @param aMesh The mesh pointer where the B-spline information can be obtained
             * @param aGeometry Geometry for initializing the B-spline level set discretization
             */
            BSpline_Geometry(
                    sol::Dist_Vector*         aOwnedADVs,
                    const Matrix<DDUMat>&     aCoefficientIndices,
                    const Matrix<DDSMat>&     aSharedADVIds,
                    uint                      aADVOffsetID,
                    mtk::Interpolation_Mesh*  aMesh,
                    std::shared_ptr<Geometry> aGeometry);
        };
    }
}

#endif //MORIS_CL_GEN_BSPLINE_GEOMETRY_HPP
