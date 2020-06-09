/*
 * cl_XTK_Interpolation_Vertex_Unzipped.cpp
 *
 *  Created on: Jul 10, 2019
 *      Author: doble
 */


#include "cl_XTK_Interpolation_Vertex_Unzipped.hpp"

#include "cl_MTK_Vertex.hpp"

namespace xtk
{
//------------------------------------------------------------------------------

Interpolation_Vertex_Unzipped::Interpolation_Vertex_Unzipped(mtk::Vertex*               aBaseInterpVertex,
                                                             moris_id                   aVertexId,
                                                             moris_index                aVertexIndex,
                                                             moris_index                aVertexOwner,
                                                             uint                       aInterpolationOrder,
                                                             Vertex_Enrichment* aVertexInterp):
                                                                     mBaseInterpVertex(aBaseInterpVertex),
                                                                     mVertexId(aVertexId),
                                                                     mVertexIndex(aVertexIndex),
                                                                     mVertexOwner(aVertexOwner),
                                                                     mInterpolation(aInterpolationOrder+1,nullptr)
{
    mInterpolation(aInterpolationOrder) = aVertexInterp;
}
//------------------------------------------------------------------------------
Matrix< DDRMat >
Interpolation_Vertex_Unzipped::get_coords() const
{
    return mBaseInterpVertex->get_coords();
}
//------------------------------------------------------------------------------
moris_id
Interpolation_Vertex_Unzipped::get_id() const
{
    return mVertexId;
}
//------------------------------------------------------------------------------
moris_index
Interpolation_Vertex_Unzipped::get_index() const
{
    return mVertexIndex;
}
//------------------------------------------------------------------------------
moris_index
Interpolation_Vertex_Unzipped::get_owner() const
{
    return mVertexOwner;
}
//------------------------------------------------------------------------------
mtk::Vertex_Interpolation *
Interpolation_Vertex_Unzipped::get_interpolation( const uint aOrder )
{
    MORIS_ASSERT(mInterpolation(aOrder) != nullptr,"Accessing vertex interpolation on a vertex which does not have vertex interpolation information.");

    return mInterpolation(aOrder);
}
//------------------------------------------------------------------------------
const mtk::Vertex_Interpolation *
Interpolation_Vertex_Unzipped::get_interpolation( const uint aOrder ) const
{
    MORIS_ASSERT(mInterpolation(aOrder) != nullptr,"Accessing vertex interpolation on a vertex which does not have vertex interpolation information.");
    return mInterpolation(aOrder);
}
//------------------------------------------------------------------------------
Vertex_Enrichment *
Interpolation_Vertex_Unzipped::get_xtk_interpolation( const uint aOrder )
{
    return mInterpolation(aOrder);
}
//------------------------------------------------------------------------------
Vertex_Enrichment const *
Interpolation_Vertex_Unzipped::get_xtk_interpolation( const uint aOrder ) const
{
    return mInterpolation(aOrder);
}
//------------------------------------------------------------------------------
mtk::Vertex const *
Interpolation_Vertex_Unzipped::get_base_vertex(  ) const
{
    return mBaseInterpVertex;
}
//------------------------------------------------------------------------------
mtk::Vertex *
Interpolation_Vertex_Unzipped::get_base_vertex(  )
{
    return mBaseInterpVertex;
}
//------------------------------------------------------------------------------
void
Interpolation_Vertex_Unzipped::add_vertex_interpolation(const uint aOrder,
                                                        Vertex_Enrichment* aVertexInterp)
{
    if(aOrder >= mInterpolation.size())
    {
        mInterpolation.resize(aOrder+1,nullptr);
    }

    MORIS_ASSERT(mInterpolation(aOrder) == nullptr,"Vertex interpolation for this order already set");

    mInterpolation(aOrder) = aVertexInterp;
}
//------------------------------------------------------------------------------

}
