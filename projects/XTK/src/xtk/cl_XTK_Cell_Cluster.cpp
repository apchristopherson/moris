/*
 * cl_XTK_Cell_Cluster.cpp
 *
 *  Created on: Jul 22, 2019
 *      Author: doble
 */

#include "cl_XTK_Cell_Cluster.hpp"
#include "cl_XTK_Interpolation_Cell_Unzipped.hpp"
#include "cl_XTK_Child_Mesh.hpp"
namespace xtk
{
//----------------------------------------------------------------
Cell_Cluster::Cell_Cluster():
    mTrivial(true),
    mInterpolationCell(nullptr),
    mPrimaryIntegrationCells(0,nullptr),
    mVoidIntegrationCells(0,nullptr),
    mVerticesInCluster(0,nullptr)
{

}
//----------------------------------------------------------------
Cell_Cluster::~Cell_Cluster()
{

}
//----------------------------------------------------------------
bool
Cell_Cluster::is_trivial( const mtk::Master_Slave aIsMaster ) const
{
    return mTrivial;
}
//----------------------------------------------------------------
moris::Cell<moris::mtk::Cell const *> const &
Cell_Cluster::get_primary_cells_in_cluster( const mtk::Master_Slave aIsMaster ) const
{
    return mPrimaryIntegrationCells;
}
//----------------------------------------------------------------
moris::Cell<moris::mtk::Cell const *> const &
Cell_Cluster::get_void_cells_in_cluster() const
{
    return mVoidIntegrationCells;
}
//----------------------------------------------------------------
moris::mtk::Cell const &
Cell_Cluster::get_interpolation_cell( const mtk::Master_Slave aIsMaster ) const
{
    return *mInterpolationCell;
}
//----------------------------------------------------------------
moris::Cell<moris::mtk::Vertex const *> const &
Cell_Cluster::get_vertices_in_cluster( const mtk::Master_Slave aIsMaster ) const
{
    MORIS_ASSERT(!mTrivial,"Trying to access vertices in cluster on a trivial cluster");
    return mVerticesInCluster;
}
//----------------------------------------------------------------
moris::Matrix<moris::DDRMat> const &
Cell_Cluster::get_vertices_local_coordinates_wrt_interp_cell( const mtk::Master_Slave aIsMaster )  const
{
    MORIS_ASSERT(!mTrivial,"Accessing local coordinates on a trivial cell cluster is not allowed");

    return mChildMesh->get_parametric_coordinates();
}
//----------------------------------------------------------------
moris::Matrix<moris::DDRMat>
Cell_Cluster::get_vertex_local_coordinate_wrt_interp_cell( moris::mtk::Vertex const * aVertex,
                                                           const mtk::Master_Slave aIsMaster ) const
{
    MORIS_ERROR(!mTrivial,"Accessing local coordinates on a trivial cell cluster is not allowed");

    return mChildMesh->get_parametric_coordinates(aVertex->get_index());
}
//----------------------------------------------------------------
moris_index
Cell_Cluster::get_dim_of_param_coord( const mtk::Master_Slave aIsMaster) const
{
    return this->get_vertices_local_coordinates_wrt_interp_cell(aIsMaster).n_cols();
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------


}

