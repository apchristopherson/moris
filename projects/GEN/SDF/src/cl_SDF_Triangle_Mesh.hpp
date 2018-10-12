/*
 * cl_SDF_Mesh.hpp
 *
 *  Created on: Sep 30, 2018
 *      Author: messe
 */

#ifndef PROJECTS_GEN_SDF_SRC_CL_SDF_TRIANGLE_MESH_HPP_
#define PROJECTS_GEN_SDF_SRC_CL_SDF_TRIANGLE_MESH_HPP_

#include <GEN/SDF/src/cl_SDF_Triangle_Vertex.hpp>
#include <string>

#include "typedefs.hpp"
#include "cl_Cell.hpp"
#include "cl_SDF_Triangle.hpp"

namespace moris
{
    namespace sdf
    {
//-------------------------------------------------------------------------------
        class Triangle_Mesh
        {
            const real                        mMeshHighPass=1e-9;
            moris::Cell< Triangle_Vertex * >  mVertices;
            moris::Cell< Triangle * >         mTriangles;

//-------------------------------------------------------------------------------
        public:
//-------------------------------------------------------------------------------

            Triangle_Mesh ( const std::string & aFilePath );

//-------------------------------------------------------------------------------

            ~Triangle_Mesh();

//-------------------------------------------------------------------------------

            moris::Cell< Triangle * > &
            get_triangles()
            {
                return mTriangles;
            }

//-------------------------------------------------------------------------------
// MTK
//-------------------------------------------------------------------------------

            Matrix< IndexMat >
            get_nodes_connected_to_element_loc_inds
                ( moris_index aElementIndex ) const;

//-------------------------------------------------------------------------------
        private:
//-------------------------------------------------------------------------------

            /**
             * loads an ascii file and creates vertex and triangle objects
             */
            void
            load_from_object_file( const std::string& aFilePath );

//-------------------------------------------------------------------------------

            /**
             * loads an ASCII file into a buffer of strings.
             * Called through construction.
             */
            void
            load_ascii_to_buffer( const std::string& aFilePath,
                    moris::Cell<std::string>& aBuffer);

//-------------------------------------------------------------------------------
        };
//-------------------------------------------------------------------------------
    } /* namespace sdf */
} /* namespace moris */

#endif /* PROJECTS_GEN_SDF_SRC_CL_SDF_TRIANGLE_MESH_HPP_ */
