/*
 * cl_MTK_Cell_Info_Tet4.cpp
 *
 *  Created on: Sep 26, 2019
 *      Author: doble
 */

#include "cl_MTK_Cell_Info_Tet4.hpp"
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Vertex.hpp"

#include "fn_norm.hpp"
#include "fn_cross.hpp"
#include "fn_dot.hpp"

namespace moris
{
    namespace mtk
    {
        // ----------------------------------------------------------------------------------

        enum Geometry_Type
        Cell_Info_Tet4::get_cell_geometry() const
        {
            return Geometry_Type::TET;
        }

        // ----------------------------------------------------------------------------------

        enum Interpolation_Order
        Cell_Info_Tet4::get_cell_interpolation_order() const
        {
            return Interpolation_Order::LINEAR;
        }

        // ----------------------------------------------------------------------------------

        enum Integration_Order
        Cell_Info_Tet4::get_cell_integration_order() const
        {
            return Integration_Order::TET_11;
        }

        // ----------------------------------------------------------------------------------

        uint
        Cell_Info_Tet4::get_num_verts() const
        {
            return 4;
        }

        // ----------------------------------------------------------------------------------

        uint
        Cell_Info_Tet4::get_num_facets() const
        {
            return 4;
        }

        // ----------------------------------------------------------------------------------

        uint
        Cell_Info_Tet4::get_num_verts_per_facet() const
        {
            return 3;
        }

        // ----------------------------------------------------------------------------------

        uint
        Cell_Info_Tet4::get_loc_coord_dim() const
        {
            return 4;
        }

        // ----------------------------------------------------------------------------------

        moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_node_to_face_map() const
        {
            return {{0, 1, 3}, {1, 2, 3}, {0, 3, 2}, {0, 2, 1}};
        }

        // ----------------------------------------------------------------------------------

        moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_node_to_facet_map() const
        {
            return this->get_node_to_face_map();
        }

        // ----------------------------------------------------------------------------------

        moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_node_to_facet_map(moris::uint aSideOrdinal) const
        {
            return this->get_node_to_face_map(aSideOrdinal);
        }

        // ----------------------------------------------------------------------------------
        moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_node_to_edge_map() const
        {
            return {{0, 1}, {1, 2}, {0, 2}, {0, 3}, {1, 3}, {2, 3}};
        }

        // ----------------------------------------------------------------------------------

        moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_node_to_edge_map(moris::uint aEdgeOrdinal) const
        {
            switch (aEdgeOrdinal)
            {
            case 0:
            {
                return {{0, 1}};
                break;
            }
            case 1:
            {
                return {{1, 2}};
                break;
            }
            case 2:
            {
                return {{0, 2}};
                break;
            }
            case 3:
            {
                return {{0, 3}};
                break;
            }
            case 4:
            {
                return {{1, 3}};
                break;
            }
            case 5:
            {
                return {{2, 3}};
                break;
            }
            default:
            {
                MORIS_ASSERT(0, "Invalid edge ordinal specified");
            }

                return moris::Matrix<moris::IndexMat>(0, 0);
            }
        }

        // ----------------------------------------------------------------------------------

        moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_node_to_face_map(moris::uint aSideOrdinal) const
        {
            switch (aSideOrdinal)
            {
            case 0:
            {
                return {{0, 1, 3}};
                break;
            }
            case 1:
            {
                return {{1, 2, 3}};
                break;
            }
            case 2:
            {
                return {{0, 3, 2}};
                break;
            }
            case 3:
            {
                return {{0, 2, 1}};
                break;
            }
            default:
            {
                MORIS_ASSERT(0, "Invalid side ordinal specified");
            }

                return moris::Matrix<moris::IndexMat>(0, 0);
            }
        }

        // ----------------------------------------------------------------------------------

        inline moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_node_map_outward_normal(moris::uint aSideOrdinal) const
        {
            switch (aSideOrdinal)
            {
            case 0:
            {
                return {{0, 1}, {1, 3}};
                break;
            }
            case 1:
            {
                return {{1, 2}, {2, 3}};
                break;
            }
            case 2:
            {
                return {{0, 3}, {3, 2}};
                break;
            }
            case 3:
            {
                return {{0, 2}, {2, 1}};
                break;
            }
            default:
            {
                MORIS_ERROR(0, "Invalid side ordinal specified");
            }

                return moris::Matrix<moris::IndexMat>(0, 0);
            }
        }

        // ----------------------------------------------------------------------------------

        moris::Matrix<moris::IndexMat>
        Cell_Info_Tet4::get_edge_to_face_map() const
        {
            return {{0, 3}, {1, 3}, {2, 3}, {0, 2}, {0, 1}, {1, 2}};
        }

        // ----------------------------------------------------------------------------------

        moris::real
        Cell_Info_Tet4::compute_cell_size_special(moris::mtk::Cell const *aCell) const
        {
            moris::Cell<Vertex *> tVertices = aCell->get_vertex_pointers();

            const Matrix<DDRMat> tNodeCoords0 = tVertices(0)->get_coords();

            const Matrix<DDRMat> tNodeCoords10 = tVertices(1)->get_coords() - tNodeCoords0;
            const Matrix<DDRMat> tNodeCoords20 = tVertices(2)->get_coords() - tNodeCoords0;
            const Matrix<DDRMat> tNodeCoords30 = tVertices(3)->get_coords() - tNodeCoords0;

            // return 1.0 / 6.0 * std::abs(dot(tNodeCoords10, cross(tNodeCoords20, tNodeCoords30)));
            return 1.0 / 6.0 * dot(tNodeCoords10, cross(tNodeCoords20, tNodeCoords30));
        }

        // ----------------------------------------------------------------------------------

        moris::real
        Cell_Info_Tet4::compute_cell_size_straight( moris::mtk::Cell const * aCell ) const
        {
            return compute_cell_size_special(aCell);
        }

        // ----------------------------------------------------------------------------------

        moris::real
        Cell_Info_Tet4::compute_cell_size_deriv( moris::mtk::Cell const * aCell, uint aLocalVertexID, uint aDirection ) const
        {
            moris::Cell<Vertex *> tVertices = aCell->get_vertex_pointers();

            // permutation vectors
            moris::Matrix<DDUMat> tVertIndexMap = {{3,0,1,2,3,0,1}};
            moris::Matrix<DDUMat> tDirIndexMap = {{1,2,0,1}};

            // getting appropriate edge vectors
            const Matrix<DDRMat> tNodeCoords0 = tVertices( tVertIndexMap(aLocalVertexID) )->get_coords();
            const Matrix<DDRMat> tNodeCoords20 = tVertices( tVertIndexMap(aLocalVertexID + 3) )->get_coords() - tNodeCoords0;
            const Matrix<DDRMat> tNodeCoords30 = tVertices( tVertIndexMap(aLocalVertexID + 2) )->get_coords() - tNodeCoords0;

            MORIS_ASSERT(tNodeCoords0.numel() == 3,"Cell_Info_Tet4::compute_cell_size_deriv only works in 3D.\n");
            MORIS_ASSERT( aDirection < 3,"Cell_Info_Tet4::compute_cell_size_deriv directions can only be 0, 1, or 2.\n");
            MORIS_ASSERT( aLocalVertexID < 4,"Cell_Info_Tet4::compute_cell_size_deriv vertex IDs must be 0, 1, 2, or 3.\n");

            // calculating volume
            return 1.0/6.0 * std::pow(-1.0,aLocalVertexID) * 
                        ( tNodeCoords20( tDirIndexMap(aDirection) ) * tNodeCoords30( tDirIndexMap(aDirection + 1) ) -
                          tNodeCoords20( tDirIndexMap(aDirection + 1) ) * tNodeCoords30( tDirIndexMap(aDirection) ) );

        }

        // ----------------------------------------------------------------------------------

        moris::real
        Cell_Info_Tet4::compute_cell_side_size(
            moris::mtk::Cell const *aCell,
            moris_index const &aSideOrd) const
        {
            // cell coordinates
            moris::Cell<mtk::Vertex const *> tVertices = aCell->get_vertices_on_side_ordinal(aSideOrd);

            const Matrix<DDRMat> tNodeCoords0 = tVertices(0)->get_coords();

            const Matrix<DDRMat> tNodeCoords10 = tVertices(1)->get_coords() - tNodeCoords0;
            const Matrix<DDRMat> tNodeCoords20 = tVertices(2)->get_coords() - tNodeCoords0;

            return norm(cross(tNodeCoords10, tNodeCoords20)) / 2.0;
        }
        // ----------------------------------------------------------------------------------
    } // namespace mtk
} // namespace moris