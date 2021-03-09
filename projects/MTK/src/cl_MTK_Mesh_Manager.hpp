/*
 * cl_MTK_Mesh_Manager.hpp
 *
 *  Created on: Apr 15, 2019
 *      Author: doble
 */

#ifndef PROJECTS_MTK_SRC_CL_MTK_MESH_MANAGER_HPP_
#define PROJECTS_MTK_SRC_CL_MTK_MESH_MANAGER_HPP_

#include "typedefs.hpp"
#include "cl_Cell.hpp"

namespace moris
{
    namespace mtk
    {
        class Mesh_Pair;
        class Interpolation_Mesh;
        class Integration_Mesh;

        class Mesh_Manager : public std::enable_shared_from_this< Mesh_Manager >
        {
        private:
            moris::Cell<Mesh_Pair> mMeshPairs;

        public:

            Mesh_Manager();

            //--------------------------------------------------------------------

            ~Mesh_Manager();

            //--------------------------------------------------------------------

            uint register_mesh_pair(
                    Interpolation_Mesh* aInterpolationMesh,
                    Integration_Mesh*   aIntegrationMesh,
                    bool                aIsOwned = false );

            //--------------------------------------------------------------------

            /**
             * Register a mesh pair with the mesh manager.
             *
             * @param aMeshPair Mesh pair
             * @return Mesh pair index
             */
            uint register_mesh_pair(Mesh_Pair& aMeshPair);

            /**
             * Gets a mesh pair.
             *
             * @param aPairIndex Mesh pair index
             * @return Mesh pair
             */
            const Mesh_Pair& get_mesh_pair(moris_index aPairIndex);

            Mesh_Pair * get_mesh_pair_pointer(moris_index aPairIndex);

            void
            get_mesh_pair(
                    moris_index            aPairIndex,
                    Interpolation_Mesh * & aInterpolationMesh,
                    Integration_Mesh   * & aIntegrationMesh);

            //--------------------------------------------------------------------

            Interpolation_Mesh*
            get_interpolation_mesh(moris_index aMeshIndex);

            //--------------------------------------------------------------------

            Integration_Mesh*
            get_integration_mesh(moris_index aMeshIndex);
            
            //--------------------------------------------------------------------

            std::shared_ptr< Mesh_Manager > get_pointer()
            {
                return shared_from_this();
            }
        };
    }
}
#endif /* PROJECTS_MTK_SRC_CL_MTK_MESH_MANAGER_HPP_ */
