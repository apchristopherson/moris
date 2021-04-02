/*
 * cl_MTK_Cell_Info_Hex64.hpp
 *
 *  Created on: May 14, 2019
 *      Author: doble
 */

#ifndef PROJECTS_MTK_SRC_CL_MTK_CELL_INFO_HEX64_HPP_
#define PROJECTS_MTK_SRC_CL_MTK_CELL_INFO_HEX64_HPP_

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_MTK_Cell_Info.hpp"

namespace moris
{
    namespace mtk
    {
        class Cell;

        class Cell_Info_Hex64 : public mtk::Cell_Info
        {
        public:
            //-----------------------------------------------------------------------------

            enum Geometry_Type
            get_cell_geometry() const;

            //-----------------------------------------------------------------------------

            enum Interpolation_Order
            get_cell_interpolation_order() const;

            //-----------------------------------------------------------------------------

            enum Integration_Order
            get_cell_integration_order() const;

            //-----------------------------------------------------------------------------

            uint
            get_num_verts() const;

            //-----------------------------------------------------------------------------

            uint
            get_num_facets() const;

            // ----------------------------------------------------------------------------------

            uint
            get_num_edges() const;

            //-----------------------------------------------------------------------------

            uint
            get_num_verts_per_facet() const;

            //-----------------------------------------------------------------------------

            uint
            get_loc_coord_dim() const;

            //----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_node_to_face_map() const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_node_to_face_map(moris::uint aSideOrdinal) const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_node_to_edge_map() const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_node_to_edge_map(moris::uint aEdgeOrdinal) const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_node_to_facet_map() const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_node_to_facet_map(moris::uint aSideOrdinal) const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_geometric_node_to_facet_map() const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_geometric_node_to_facet_map(moris::uint aSideOrdinal) const;

            //-----------------------------------------------------------------------------

            moris::uint
            get_adjacent_side_ordinal(moris::uint aSideOrdinal) const;

            //-----------------------------------------------------------------------------

            Matrix<DDRMat>
            get_vertex_loc_coord(moris_index const &aVertexOrdinal) const;

            //-----------------------------------------------------------------------------

            moris::Matrix<moris::IndexMat>
            get_node_map_outward_normal(moris::uint aSideOrdinal) const;

            //-----------------------------------------------------------------------------

            moris::real
            compute_cell_size_special(moris::mtk::Cell const *aCell) const;

            //-----------------------------------------------------------------------------

            moris::real
            compute_cell_side_size(moris::mtk::Cell const *aCell,
                                   moris_index const &aSideOrd) const;

            // ----------------------------------------------------------------------------------
            void
            eval_N(const Matrix<DDRMat> &aXi,
                   Matrix<DDRMat> &aNXi) const;
        };
    } // namespace mtk
} // namespace moris
#endif /* PROJECTS_MTK_SRC_CL_MTK_CELL_INFO_HEX64_HPP_ */