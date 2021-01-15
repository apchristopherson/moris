#ifndef MORIS_CL_GEN_GEOMETRY_ENGINE_TEST_HPP
#define MORIS_CL_GEN_GEOMETRY_ENGINE_TEST_HPP

#include "cl_GEN_Geometry_Engine.hpp"

namespace moris
{
    namespace ge
    {
        /**
         * Alternate geometry engine class for testing (provides access to some needed protected members)
         */
        class Geometry_Engine_Test : public Geometry_Engine
        {
        public:
            /**
             * Constructor
             *
             * @param aMesh Mesh for getting B-spline information
             * @param aParameters Optional geometry engine parameters
             */
            Geometry_Engine_Test(
                    mtk::Interpolation_Mesh*   aMesh = nullptr,
                    Geometry_Engine_Parameters aParameters = {});

            /**
             * Gets a geometry
             *
             * @param aGeometryIndex Geometry index
             * @return Geometry
             */
            std::shared_ptr<Geometry> get_geometry(uint aGeometryIndex);
        };
    }
}

#endif //MORIS_CL_GEN_GEOMETRY_ENGINE_TEST_HPP
