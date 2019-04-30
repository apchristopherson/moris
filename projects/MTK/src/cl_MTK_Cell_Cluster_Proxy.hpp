/*
 * cl_MTK_Cell_Cluster_Proxy.hpp
 *
 *  Created on: Apr 26, 2019
 *      Author: doble
 */

#ifndef PROJECTS_MTK_TEST_CL_MTK_CELL_CLUSTER_PROXY_HPP_
#define PROJECTS_MTK_TEST_CL_MTK_CELL_CLUSTER_PROXY_HPP_

#include "cl_MTK_Cell_Cluster.hpp"
#include "cl_Matrix.hpp"

namespace moris
{
class Cell_Cluster_Proxy: public mtk::Cell_Cluster
{
public:
    moris::mtk::Cell*                mInterpolationCell;
    moris::Cell<moris::mtk::Cell const *>   mPrimaryIntegrationCells;
    moris::Cell<moris::mtk::Cell const *>   mVoidIntegrationCells;
    moris::Cell<moris::mtk::Vertex const *> mVerticesInCluster;
    moris::Matrix<moris::DDRMat>     mVertexParamCoords;
public:
    Cell_Cluster_Proxy(){};

    moris::Cell<moris::mtk::Cell const *> const &
    get_primary_cells_in_cluster() const
    {
        return mPrimaryIntegrationCells;
    }

    moris::Cell<moris::mtk::Cell const *> const &
    get_void_cells_in_cluster() const
    {
        return mVoidIntegrationCells;
    }

    moris::mtk::Cell const &
    get_interpolation_cell() const
    {
        return *mInterpolationCell;
    }

    moris::Cell<moris::mtk::Vertex const *> const &
    get_vertices_in_cluster() const
    {
        return mVerticesInCluster;
    }

    moris::Matrix<moris::DDRMat> const &
    get_vertices_local_coordinates_wrt_interp_cell() const
    {
        return mVertexParamCoords;
    }
};
}



#endif /* PROJECTS_MTK_TEST_CL_MTK_CELL_CLUSTER_PROXY_HPP_ */
