/*
 * cl_MTK_Cell_Cluster_Proxy.hpp
 *
 *  Created on: Apr 26, 2019
 *      Author: doble
 */

#ifndef PROJECTS_MTK_TEST_CL_MTK_CELL_CLUSTER_PROXY_HPP_
#define PROJECTS_MTK_TEST_CL_MTK_CELL_CLUSTER_PROXY_HPP_

#include "cl_MTK_Cell_Cluster.hpp"
#include "cl_MTK_Cell_Info.hpp"
#include "cl_Matrix.hpp"

namespace moris
{
    class Cell_Cluster_Proxy: public mtk::Cell_Cluster
    {
        public:
            bool                                    mTrivial;
            moris::mtk::Cell*                       mInterpolationCell;
            moris::Cell<moris::mtk::Cell const *>   mPrimaryIntegrationCells;
            moris::Cell<moris::mtk::Cell const *>   mVoidIntegrationCells;
            moris::Cell<moris::mtk::Vertex const *> mVerticesInCluster;
            moris::Matrix<moris::DDRMat>            mVertexParamCoords;
        public:
            Cell_Cluster_Proxy(){};

            bool
            is_trivial( const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const
            {
                return mTrivial;
            }

            moris::Cell<moris::mtk::Cell const *> const &
            get_primary_cells_in_cluster( const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const
            {
                return mPrimaryIntegrationCells;
            }

            moris::Cell<moris::mtk::Cell const *> const &
            get_void_cells_in_cluster() const
            {
                return mVoidIntegrationCells;
            }

            moris::mtk::Cell const &
            get_interpolation_cell( const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER) const
            {
                return *mInterpolationCell;
            }

            moris::Cell<moris::mtk::Vertex const *>
            get_vertices_in_cluster( const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const
            {
                return mVerticesInCluster;
            }

            moris::Matrix<moris::DDRMat>
            get_vertices_local_coordinates_wrt_interp_cell( const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const
            {
                return mVertexParamCoords;
            }

            moris::Matrix<moris::DDRMat>
            get_vertex_local_coordinate_wrt_interp_cell( moris::mtk::Vertex const * aVertex,
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER) const
            {
                MORIS_ERROR(0,"get_vertex_local_coordinate_wrt_interp_cell not implemented in proxy cell cluster");
                return moris::Matrix<moris::DDRMat>(0,0);
            }

            moris_index
            get_dim_of_param_coord( const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const
            {
                return mVertexParamCoords.n_cols();
            }

            moris::real
            compute_cluster_cell_measure(const mtk::Primary_Void aPrimaryOrVoid = mtk::Primary_Void::PRIMARY,
                    const mtk::Master_Slave aIsMaster      = mtk::Master_Slave::MASTER) const
            {
                moris::real tVolume = 0.0;

                moris::Cell<moris::mtk::Cell const *> const* tCells = nullptr;

                if(aPrimaryOrVoid == mtk::Primary_Void::PRIMARY)
                {
                    tCells = &this->get_primary_cells_in_cluster();
                }
                else
                {
                    tCells = & this->get_void_cells_in_cluster();
                }

                for(auto iC = tCells->cbegin(); iC < tCells->cend(); iC++)
                {
                    tVolume += (*iC)->compute_cell_measure();
                }

                return tVolume;
            }

            moris::real
            compute_cluster_cell_side_measure(const mtk::Primary_Void aPrimaryOrVoid = mtk::Primary_Void::PRIMARY,
                    const mtk::Master_Slave aIsMaster      = mtk::Master_Slave::MASTER) const
            {
                MORIS_ERROR(0,"compute_cluster_cell_side_measure only valid on side clusters");
                return 0;
            }
    };
}
#endif /* PROJECTS_MTK_TEST_CL_MTK_CELL_CLUSTER_PROXY_HPP_ */
