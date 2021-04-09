/*
 * cl_XTK_Side_Cluster.hpp
 *
 *  Created on: Jul 22, 2019
 *      Author: doble
 */

#ifndef PROJECTS_XTK_SRC_XTK_CL_XTK_SIDE_CLUSTER_HPP_
#define PROJECTS_XTK_SRC_XTK_CL_XTK_SIDE_CLUSTER_HPP_

#include "cl_MTK_Side_Cluster.hpp"
#include <unordered_map>
using namespace moris;

namespace moris
{
    namespace mtk
    {
        class Cell_Cluster;
    }
}

namespace xtk
{
    class Interpolation_Cell_Unzipped;
    class Child_Mesh;

    class Side_Cluster : public mtk::Side_Cluster
    {
        public:

            //---------------------------------------------------------------------------------------

            Side_Cluster();

            //---------------------------------------------------------------------------------------

            bool is_trivial( const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const ;

            //---------------------------------------------------------------------------------------

            moris::mtk::Cell const & get_interpolation_cell(
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const;

            //---------------------------------------------------------------------------------------

            moris::Cell<mtk::Cell const *> const &  get_cells_in_side_cluster() const;

            //---------------------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat> get_cell_side_ordinals(
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const ;

            //---------------------------------------------------------------------------------------

            moris_index get_cell_side_ordinal(
                    moris::moris_index       aCellIndexInCluster,
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const;

            //---------------------------------------------------------------------------------------

            moris::Cell<moris::mtk::Vertex const *> get_vertices_in_cluster(
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const;

            //---------------------------------------------------------------------------------------

            moris::Matrix<moris::DDRMat> get_vertices_local_coordinates_wrt_interp_cell(
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const;

            //---------------------------------------------------------------------------------------

            moris::moris_index get_vertex_cluster_index(
                    const moris::mtk::Vertex * aVertex,
                    const mtk::Master_Slave    aIsMaster = mtk::Master_Slave::MASTER) const;

            //---------------------------------------------------------------------------------------

            moris_index get_vertex_ordinal_on_facet(
                    moris_index                aCellIndexInCluster,
                    moris::mtk::Vertex const * aVertex ) const;

            //---------------------------------------------------------------------------------------

            moris::Matrix<moris::DDRMat> get_vertex_local_coordinate_wrt_interp_cell(
                    const moris::mtk::Vertex * aVertex,
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER) const ;

            //---------------------------------------------------------------------------------------

            moris_index get_dim_of_param_coord(
                    const mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER ) const ;

            //---------------------------------------------------------------------------------------

            moris::real compute_cluster_cell_measure(
                    const mtk::Primary_Void aPrimaryOrVoid = mtk::Primary_Void::PRIMARY,
                    const mtk::Master_Slave aIsMaster      = mtk::Master_Slave::MASTER) const;

            //---------------------------------------------------------------------------------------

            Matrix<DDRMat> compute_cluster_ig_cell_measures(
                    const mtk::Primary_Void aPrimaryOrVoid = mtk::Primary_Void::PRIMARY,
                    const mtk::Master_Slave aIsMaster      = mtk::Master_Slave::MASTER) const;

            //---------------------------------------------------------------------------------------

            moris::real compute_cluster_cell_measure_derivative(
                    const Matrix< DDRMat >  & aPerturbedVertexCoords, uint aDirection,
                    const mtk::Primary_Void   aPrimaryOrVoid,
                    const mtk::Master_Slave   aIsMaster) const;

            //---------------------------------------------------------------------------------------

            // memory
            size_t
            capacity();

            //---------------------------------------------------------------------------------------

            void print_vertex_map() const;

            //---------------------------------------------------------------------------------------

            friend class Enriched_Integration_Mesh;
            friend class Ghost_Stabilization;

        protected:

            bool                                        mTrivial;
            Interpolation_Cell_Unzipped const *         mInterpolationCell;
            Child_Mesh const *                          mChildMesh;
            moris::Cell<moris::mtk::Cell const *>       mIntegrationCells;
            moris::Matrix<moris::IndexMat>              mIntegrationCellSideOrdinals;
            moris::Cell<moris::mtk::Vertex const *>     mVerticesInCluster;
            std::unordered_map<moris_index,moris_index> mVertexIdToLocalIndex;
            moris::Cell<moris::Matrix<moris::DDRMat>>   mVertexLocalCoords;
            moris::Matrix<moris::DDRMat>                mVertexLocalCoordsMat; /*FIXME: get rid of mVertexLocalCoords*/
            moris::mtk::Cell_Cluster const *            mAssociatedCellCluster; /* Associated cell cluster (needed for volume computations in nitsche).*/

            //---------------------------------------------------------------------------------------

            void
            finalize_setup();

            //---------------------------------------------------------------------------------------

            void
            add_vertex_to_map(
                    moris_id    aVertexId,
                    moris_index aVertexLocalIndex);

            //---------------------------------------------------------------------------------------
    };
}

#endif /* PROJECTS_XTK_SRC_XTK_CL_XTK_SIDE_CLUSTER_HPP_ */
