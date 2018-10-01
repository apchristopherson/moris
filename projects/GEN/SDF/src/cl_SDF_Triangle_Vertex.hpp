/*
 * cl_SDF_Vertex.hpp
 *
 *  Created on: Sep 30, 2018
 *      Author: messe
 */

#ifndef PROJECTS_GEN_SDF_SRC_CL_SDF_TRIANGLE_VERTEX_HPP_
#define PROJECTS_GEN_SDF_SRC_CL_SDF_TRIANGLE_VERTEX_HPP_

#include "assert.hpp"
#include "typedefs.hpp"
#include "cl_MTK_Vertex.hpp"

namespace moris
{
    namespace sdf
    {
//-------------------------------------------------------------------------------

        class Triangle_Vertex : public mtk::Vertex
        {

            const moris_index       mIndex;

//-------------------------------------------------------------------------------

            const Matrix< DDRMat >  mNodeCoords;

//-------------------------------------------------------------------------------
        public:
//-------------------------------------------------------------------------------

            Triangle_Vertex(
                    const moris_index        aIndex,
                    const Matrix< DDRMat > & aNodeCoords );

//-------------------------------------------------------------------------------

            ~Triangle_Vertex(){};

//-------------------------------------------------------------------------------
// MTK API functions
//-------------------------------------------------------------------------------

            Matrix< DDRMat >
            get_coords() const
            {
                return mNodeCoords;
            }

//-------------------------------------------------------------------------------

            moris_id
            get_id() const
            {
                return mIndex + 1;
            }

//-------------------------------------------------------------------------------

            moris_index
            get_index() const
            {
                return mIndex;
            }

//-------------------------------------------------------------------------------

            moris_id
            get_owner() const
            {
                return 0;
            }

//-------------------------------------------------------------------------------

            mtk::Vertex_Interpolation *
            get_interpolation()
            {
                MORIS_ERROR( false,
                        "get_interpolation() is not available for an SDF Vertex");
                return nullptr;
            }

//-------------------------------------------------------------------------------

            const mtk::Vertex_Interpolation *
            get_interpolation() const
            {
                MORIS_ERROR( false,
                        "get_interpolation() is not available for an SDF Vertex");
                return nullptr;
            }

//-------------------------------------------------------------------------------
        };
//-------------------------------------------------------------------------------
    } /* namespace sdf */
} /* namespace moris */

#endif /* PROJECTS_GEN_SDF_SRC_CL_SDF_TRIANGLE_VERTEX_HPP_ */
