/*
 * cl_MTK_CELL_INFO.hpp
 *
 *  Created on: Jul 25, 2019
 *      Author: ryan
 */

#ifndef PROJECTS_MTK_SRC_CL_MTK_CELL_INFO_HPP_
#define PROJECTS_MTK_SRC_CL_MTK_CELL_INFO_HPP_

#include "cl_Cell.hpp"
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "assert.hpp"
#include "cl_MTK_Enums.hpp"
namespace moris
{
    namespace mtk
    {

        class Cell;

        class Cell_Info
        {
            public:
                Cell_Info(){}

                virtual
                ~Cell_Info(){}
                // ---------------------------------------------------------------------------------
                /*!
                 * @return Cells geometry type, i.e. a hexahedron, tetrahedron
                 */
                virtual
                enum Geometry_Type
                get_cell_geometry() const = 0;

                // ---------------------------------------------------------------------------------

                /*!
                 * @return a cell interpolation order, i.e. linear,quadratic or cubic
                 */
                virtual
                enum Interpolation_Order
                get_cell_interpolation_order() const = 0;

                // ---------------------------------------------------------------------------------

                /*
                 * @return number of vertices attached to cell
                 */
                virtual
                uint
                get_num_verts() const  = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * @return number of vertices attached to a single facet of the cell
                 */
                virtual
                uint
                get_num_verts_per_facet() const = 0;

                // ---------------------------------------------------------------------------------

                /*!
                 * @return number of facets attached to the cell
                 */
                virtual
                uint
                get_num_facets() const = 0;

                // ---------------------------------------------------------------------------------

                /*!
                 * @return Dimension of cell's parametric coordinates
                 */
                virtual
                uint
                get_loc_coord_dim() const = 0;

                // ---------------------------------------------------------------------------------

                /*!
                 * The vertex to facet map is the vertex ordinals on a facet. For 2D cells,
                 * a facet is an edge, for 3D cells a facet is a face
                 * @return Vertex to facet map
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_node_to_facet_map() const = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * The vertex to facet map is the vertex ordinals on a facet. For 2D cells,
                 * a facet is an edge, for 3D cells a facet is a face
                 * @param[in] aSideOrdinal Side Ordinal
                 * @return Vertex to facet map
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_node_to_facet_map(moris::uint aSideOrdinal) const = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * The geometric  vertex to facet map is the vertex ordinals on a facet.
                 * The geometric vertices correspond to the corner points of the cell.
                 * For example, a hex16 geometric vertices would be the onese of a hex8.
                 * @return Geometric vertex to facet map
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_geometric_node_to_facet_map() const;

                // ---------------------------------------------------------------------------------
                /*!
                 * The geometric  vertex to facet map is the vertex ordinals on a facet.
                 * The geometric vertices correspond to the corner points of the cell.
                 * For example, a hex16 geometric vertices would be the onese of a hex8.
                 * @param[in] aSideOrdinal a side ordinal
                 * @return Geometric vertex to facet map
                 */
                moris::Matrix<moris::IndexMat>
                get_geometric_node_to_facet_map(moris::uint aSideOrdinal) const;

                // ---------------------------------------------------------------------------------

                /*!
                 * The vertex to face map is the vertex ordinals on a face. face has dim = 2
                 * and does not exist on 2d cells
                 * @return Vertex to face map
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_node_to_face_map() const = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * The vertex to face map is the vertex ordinals on a face. face has dim = 2
                 * and does not exist on 2d cells
                 * @param[in] aSideOrdinal a side ordinal
                 * @return Vertex to face map on a single side
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_node_to_face_map(moris::uint aSideOrdinal) const  = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * The vertex to face map is the vertex ordinals on an edge. Edge has dim = 1
                 * @return Vertex to edge map
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_node_to_edge_map() const  = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * The vertex to face map is the vertex ordinals on an edge. Edge has dim = 1
                 * and does not exist on 2d cells
                 * @param[in] aEdgeOrdinal Edge ordinal
                 * @return Vertex to edge map on a single side
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_node_to_edge_map(moris::uint aEdgeOrdinal) const  = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * @return Edges attached to facets (col edge ord, row face ord)
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_edge_to_face_map() const;

                // ---------------------------------------------------------------------------------
                /*!
                 * Computes and returns the outward facing normal on a cell
                 * @param[in] aSideOrdinal Side Ordinal
                 * @return Outward pointing normal (unit length)
                 */
                virtual
                moris::Matrix<moris::IndexMat>
                get_node_map_outward_normal(moris::uint aSideOrdinal) const  = 0;

                // ---------------------------------------------------------------------------------
                /*!
                 * x---2---x
                 * |       |
                 * 3       1
                 * |       |
                 * x---0---x
                 *
                 * If aSideOrdinal = 0 this functions returns 2
                 * @param[in] aSideOrdinal Side Ordinal
                 * @return  Adjacent facet ordinal
                 */
                virtual
                moris::uint
                get_adjacent_side_ordinal(moris::uint aSideOrdinal) const;

                // ---------------------------------------------------------------------------------
                /*!
                 * @param[in] aVertexOrdinal Vertex Ordinal
                 * @return Parametric coordinate of the vertex
                 */
                virtual
                Matrix<DDRMat>
                get_vertex_loc_coord(moris_index aVertexOrdinal) const;

                // ---------------------------------------------------------------------------------
                /*!
                 * @param[in] aSideOrdinal Side Ordinal
                 * @param[out] Parametric coordinate of the vertices attached to the side ordinal
                 */
                virtual
                void
                get_loc_coord_on_side_ordinal(
                        moris::uint const & aSideOrdinal,
                        Matrix<DDRMat>    & aXi ) const;

                // ---------------------------------------------------------------------------------
                /*!
                * @param[out] Parametric coordinate of the all the cell's vertices
                */
                virtual
                void
                get_loc_coords_of_cell(Matrix<DDRMat> & aXi) const;

                // ---------------------------------------------------------------------------------
                /*!
                 * Compute the volume of 3D cell or the surface area of 2d cell
                 * @return Cell size
                 */
                virtual
                moris::real
                compute_cell_size( moris::mtk::Cell const * aCell ) const = 0;
                // ---------------------------------------------------------------------------------
                /*!
                 * Compute the side surface area of 3D cell or the side length of 2d cell
                 * @return Cell side size
                 */
                virtual
                moris::real
                compute_cell_side_size( moris::mtk::Cell const * aCell ,
                        moris_index const & aSideOrd) const = 0;
                // ---------------------------------------------------------------------------------
                /*!
                 * Evaluates the basis functions of the cell at local coordinate aXi
                 * @param[in] aXi Local Coordinate Vector
                 * @param[out] aNXi Interpolation function weights at point aXi
                 */
                virtual
                void
                eval_N( const Matrix< DDRMat > & aXi,
                        Matrix< DDRMat > & aNXi ) const;

        };

    }

}



#endif /* PROJECTS_MTK_SRC_CL_MTK_CELL_INFO_HPP_ */
