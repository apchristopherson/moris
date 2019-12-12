/*
 * cl_VIS_Factory.hpp
 *
 *  Created on: Dez 02, 2019
 *      Author: schmidt
 */
#ifndef SRC_FEM_CL_VIS_FACTORY_HPP_
#define SRC_FEM_CL_VIS_FACTORY_HPP_

#include "cl_Cell.hpp"
#include "cl_Communication_Tools.hpp"
#include "cl_Communication_Manager.hpp"

#include "cl_VIS_Vertex_Visualization.hpp"
#include "cl_VSI_Cell_Visualization.hpp"
#include "cl_VIS_Cell_Cluster_Visualization.hpp"
#include "cl_VIS_Visualization_Mesh.hpp"
#include "cl_VIS_Factory.hpp"
#include "cl_VIS_Output_Enums.hpp"

namespace moris
{
    namespace mtk
    {
        class Set;
        class Cluster;
        class Cell;
        class Vertex;
        class Mesh_Manager;
    }

    namespace vis
    {
        struct Output_Data;

        class VIS_Factory
        {
        private:
            moris::Cell< moris::mtk::Set * > mListofBlocks;

            moris::Cell< moris::Cell< mtk::Cluster * > >        mClustersOnBlock;   //FIXME delete can be used temporary
            moris::Cell< moris::Cell< const mtk::Cluster * > >  mClustersOnBlock_1;
            moris::Cell< moris::Cell< mtk::Cell * > >   mCellsOnBlock;
            moris::Cell< moris::Cell< mtk::Vertex * > > mVerticesOnBlock;

            moris::Cell< Matrix< DDSMat > >             mVertexMapOnBlock;
            moris::Cell< Matrix< DDSMat > >             mCellMapOnBlock;

            uint mNumRequestedBlocks;
            bool mOnlyPrimaryCells = false;

            mtk::Mesh_Manager* mMesh = nullptr;
            const uint         mMeshPairIndex;

            moris::Cell< std::string > tRequestedSetNames;


        public:
            VIS_Factory( mtk::Mesh_Manager* aMesh,
                     const uint         aMeshPairIndex ) : mMesh( aMesh ),
                                                           mMeshPairIndex( aMeshPairIndex )
            {};

//-----------------------------------------------------------------------------------------------------------

            ~VIS_Factory(){};

//-----------------------------------------------------------------------------------------------------------

            mtk::Mesh * create_visualization_mesh( moris::vis::Output_Data & aOutputData );

//-----------------------------------------------------------------------------------------------------------

            void create_visualization_vertices();

//-----------------------------------------------------------------------------------------------------------

            void create_visualization_cells();

//-----------------------------------------------------------------------------------------------------------

            void create_visualization_clusters();

//-----------------------------------------------------------------------------------------------------------

            void create_visualization_blocks();

//-----------------------------------------------------------------------------------------------------------


        };
    } /* namespace VIS */
} /* namespace moris */

#endif /* SRC_FEM_CL_VIS_FACTORY_HPP_ */
