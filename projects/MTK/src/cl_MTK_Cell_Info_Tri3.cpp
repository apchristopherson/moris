/*
 * cl_MTK_Cell_Info_Tri3.cpp
 *
 *  Created on: Sep 26, 2019
 *      Author: doble
 */




#include "cl_MTK_Cell_Info_Tri3.hpp"
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Vertex.hpp"
#include "fn_det.hpp"
#include "fn_dot.hpp"
#include "fn_sum.hpp"
#include "fn_trans.hpp"
#include "op_div.hpp"
#include "op_times.hpp"

namespace moris
{
namespace mtk
{
// ----------------------------------------------------------------------------------
enum Geometry_Type
Cell_Info_Tri3::get_cell_geometry() const
{
    return Geometry_Type::TRI;
}
// ----------------------------------------------------------------------------------
enum Interpolation_Order
Cell_Info_Tri3::get_cell_interpolation_order() const
{
    return Interpolation_Order::LINEAR;
}
// ----------------------------------------------------------------------------------
uint
Cell_Info_Tri3::get_num_verts() const
{
    return 3;
}
// ----------------------------------------------------------------------------------
uint
Cell_Info_Tri3::get_num_facets() const
{
    return 3;
}
// ----------------------------------------------------------------------------------
uint
Cell_Info_Tri3::get_num_verts_per_facet() const
{
    return 2;
}
// ----------------------------------------------------------------------------------
moris::Matrix<moris::IndexMat>
Cell_Info_Tri3::get_node_to_face_map() const
{
    MORIS_ERROR(0,"Elements have no faces in 2D. Check the MTK mesh class to get nodes connected to an element.");
    return moris::Matrix<moris::IndexMat>(0,0);
}
// ----------------------------------------------------------------------------------
moris::Matrix<moris::IndexMat>
Cell_Info_Tri3::get_node_to_edge_map() const
{
    return {{0,1}, {1,2}, {2,0}};
}
// ----------------------------------------------------------------------------------
moris::Matrix<moris::IndexMat>
Cell_Info_Tri3::get_node_to_facet_map() const
{
    return this->get_node_to_edge_map();
}
// ----------------------------------------------------------------------------------
moris::Matrix<moris::IndexMat>
Cell_Info_Tri3::get_node_to_face_map(moris::uint aSideOrdinal) const
{
    MORIS_ERROR(0,"Elements have no faces in 2D. Check the MTK mesh class to get nodes connected to an element.");
    return moris::Matrix<moris::IndexMat>(0,0);
}
// ----------------------------------------------------------------------------------
moris::Matrix<moris::IndexMat>
Cell_Info_Tri3::get_node_to_edge_map(moris::uint aEdgeOrdinal) const
{
    switch (aEdgeOrdinal)
    {
        case(0):{ return {{0, 1}}; break; }
        case(1):{ return {{1, 2}}; break; }
        case(2):{ return {{2, 0}}; break; }
        default:
        {
            MORIS_ASSERT(0,"Invalid edge ordinal specified");
            return moris::Matrix<moris::IndexMat>(0,0);
            break;
        }
    }
}
// ----------------------------------------------------------------------------------
moris::Matrix<moris::IndexMat>
Cell_Info_Tri3::get_node_to_facet_map(moris::uint aSideOrdinal) const
{
    return this->get_node_to_edge_map(aSideOrdinal);
}
// ----------------------------------------------------------------------------------
moris::Matrix<moris::IndexMat>
Cell_Info_Tri3::get_node_map_outward_normal(moris::uint aSideOrdinal) const
{
    switch (aSideOrdinal)
    {
        case(0):{ return {{1,0}}; break; }
        case(1):{ return {{2,1}}; break; }
        case(2):{ return {{0,2}}; break; }
        default:
        {
            MORIS_ERROR(0,"Invalid side ordinal specified");
            return moris::Matrix<moris::IndexMat>(0,0);
            break;
        }
    }
}
// ----------------------------------------------------------------------------------
moris::real
Cell_Info_Tri3::compute_cell_size( moris::mtk::Cell const * aCell ) const
{
    // cell coordinates
    moris::Cell< Vertex* > tVertices = aCell->get_vertex_pointers();

    Matrix<DDRMat> tNodeCoords0 = tVertices(0)->get_coords();
    Matrix<DDRMat> tNodeCoords1 = tVertices(1)->get_coords();
    Matrix<DDRMat> tNodeCoords2 = tVertices(2)->get_coords();

    // Doing it this way insures we do not assume the structure of node coords
    Matrix<DDRMat> tXHat(2,3);
    tXHat(0,0) = tNodeCoords0(0);
    tXHat(1,0) = tNodeCoords0(1);
    tXHat(0,1) = tNodeCoords1(0);
    tXHat(1,1) = tNodeCoords1(1);
    tXHat(0,2) = tNodeCoords2(0);
    tXHat(1,2) = tNodeCoords2(1);


    // populate output matrix
    Matrix< DDRMat > tdNdXi(3,3);
    tdNdXi( 0, 0 ) = 1.0;
    tdNdXi( 0, 1 ) = 0.0;
    tdNdXi( 0, 2 ) = 0.0;

    tdNdXi( 1, 0 ) = 0.0;
    tdNdXi( 1, 1 ) = 1.0;
    tdNdXi( 1, 2 ) = 0.0;

    tdNdXi( 2, 0 ) = 0.0;
    tdNdXi( 2, 1 ) = 0.0;
    tdNdXi( 2, 2 ) = 1.0;

    Matrix<DDRMat> tSpaceJT = tdNdXi * tXHat;

    Matrix< DDRMat > tSpaceJt2( 3, 3, 1.0 );
    tSpaceJt2({ 1, 2 },{ 0, 2 }) = trans( tSpaceJT );
    return det( tSpaceJt2 ) / 2.0;
}
// ----------------------------------------------------------------------------------
}
}