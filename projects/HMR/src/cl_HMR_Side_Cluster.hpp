/*
 * cl_HMR_Side_Cluster.hpp
 *
 *  Created on: May 30, 2019
 *      Author: doble
 */

#ifndef PROJECTS_HMR_SRC_CL_HMR_SIDE_CLUSTER_HPP_
#define PROJECTS_HMR_SRC_CL_HMR_SIDE_CLUSTER_HPP_

#include <unordered_map>

#include "cl_MTK_Side_Cluster.hpp"
#include "cl_Matrix.hpp"


namespace moris
{
namespace hmr
{

class Side_Cluster_HMR: public mtk::Side_Cluster
{
private:

    bool                                    mTrivial;
    moris::mtk::Cell const *                mInterpolationCell;
    moris::Cell<moris::mtk::Cell const *>   mIntegrationCells;
    moris::Matrix<moris::IndexMat>          mIntegrationCellSideOrdinals;
    moris::Cell<moris::mtk::Vertex const *> mVerticesInCluster;
    moris::Matrix<moris::DDRMat>            mVertexParamCoords;

    // map from vertex id to local index
    std::unordered_map<moris_index,moris_index> mVertexIdToLocalIndex;


public:
    //----------------------------------------------------------------

    Side_Cluster_HMR();

    //----------------------------------------------------------------


    // trivial constructor
    Side_Cluster_HMR( moris::mtk::Cell const * aInterpCell,
                      moris::mtk::Cell const * aIntegrationCell,
                      moris::Cell<moris::mtk::Vertex const *> const & aVerticesInCluster,
                      moris_index aSideOrdinal);

    //----------------------------------------------------------------


    Side_Cluster_HMR(bool aTrivial,
                     moris::mtk::Cell const *                        aInterpolationCell,
                     moris::Cell<moris::mtk::Cell const *>   const & aIntegrationCells,
                     moris::Matrix<moris::IndexMat>          const & aIntegrationCellSideOrdinals,
                     moris::Cell<moris::mtk::Vertex const *> const & aVerticesInCluster,
                     moris::Matrix<moris::DDRMat> const & aVertexParamCoords);

    //----------------------------------------------------------------

    bool
    is_trivial( const moris::uint aSide = 0 ) const;

    //----------------------------------------------------------------

    moris::mtk::Cell const &
    get_interpolation_cell( const moris::uint aSide = 0) const;

    //----------------------------------------------------------------

    moris::Cell<moris::mtk::Cell const *> const &
    get_cells_in_side_cluster() const;

    //----------------------------------------------------------------

    moris::Matrix<moris::IndexMat>
    get_cell_side_ordinals( const moris::uint aSide = 0) const;

    //----------------------------------------------------------------

    moris_index
    get_cell_side_ordinal(moris::moris_index aCellIndexInCluster,
            const moris::uint aSide = 0) const;

    //----------------------------------------------------------------

    moris::Cell<moris::mtk::Vertex const *> const &
    get_vertices_in_cluster( const moris::uint aSide = 0) const;

    //----------------------------------------------------------------

    moris::Matrix<moris::DDRMat> const &
    get_vertices_local_coordinates_wrt_interp_cell( const moris::uint aSide = 0) const;

    //----------------------------------------------------------------

    moris_index
    get_vertex_cluster_index( moris::mtk::Vertex const * aVertex ) const;

    //----------------------------------------------------------------

    moris::Matrix<moris::DDRMat>
    get_vertex_local_coordinate_wrt_interp_cell( moris::mtk::Vertex const * aVertex,
            const moris::uint aSide = 0) const;

    //----------------------------------------------------------------

    moris_index
    get_dim_of_param_coord( const moris::uint aSide = 0 ) const;

    //----------------------------------------------------------------

    moris_index
    get_vertex_cluster_local_index(moris_id aVertexId) const;

    //----------------------------------------------------------------

    void
    add_vertex_to_map(moris_id aVertexId,
                      moris_index aVertexLocalIndex);

};
}
}



#endif /* PROJECTS_HMR_SRC_CL_HMR_SIDE_CLUSTER_HPP_ */
