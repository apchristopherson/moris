#ifndef MORIS_ST_GEN_GEOMETRY_ENGINE_PARAMETERS_HPP
#define MORIS_ST_GEN_GEOMETRY_ENGINE_PARAMETERS_HPP

#include "cl_Cell.hpp"
#include "cl_Matrix.hpp"
#include "cl_GEN_Geometry.hpp"
#include "cl_GEN_Property.hpp"
#include "cl_GEN_Pdv_Enums.hpp"

namespace moris
{
    namespace ge
    {
        struct Geometry_Engine_Parameters
        {
            /**
             * @var mGeometries Geometries to be used
             * @var mProperties Properties to be used
             * @var mBulkPhases Bulk phases for creating a custom phase table
             * @var mIntersectionMode Intersection mode
             * @var mIsocontourThreshold Isocontour threshold for determining phases
             * @var mIsocontourTolerance Tolerance for determining if a node is on the interface based on geometry value
             * @var mIntersectionTolerance Tolerance for determining if a node is on the interface based on distance
             * @var mRequestedIQIs Requested IQI names
             * @var mGeometryFieldFile File name for writing geometry field values
             * @var mOutputMeshFile File name for writing an exodus mesh
             * @var mTimeOffset Time offset for writing sequential meshes
             */
            Cell<std::shared_ptr<Geometry>> mGeometries = {};
            Cell<std::shared_ptr<Property>> mProperties = {};
            Matrix<DDUMat>                  mBulkPhases = {{}};
            Intersection_Mode               mIntersectionMode = Intersection_Mode::LEVEL_SET;
            real                            mIsocontourThreshold = 0.0;
            real                            mIsocontourTolerance = 0.0;
            real                            mIntersectionTolerance = 0.0;
            Cell<std::string>               mRequestedIQIs = {};
            std::string                     mGeometryFieldFile = "";
            std::string                     mOutputMeshFile = "";
            real                            mTimeOffset = 0.0;
        };
    }
}

#endif //MORIS_ST_GEN_GEOMETRY_ENGINE_PARAMETERS_HPP
