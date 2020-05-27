/*
 * cl_XTK_Vertex_Enrichment.cpp
 *
 *  Created on: Jul 11, 2019
 *      Author: doble
 */

#include "cl_XTK_Vertex_Enrichment.hpp"
#include "fn_equal_to.hpp"
#include "fn_unique.hpp"

namespace xtk
{
//------------------------------------------------------------------------------
Vertex_Enrichment::Vertex_Enrichment():
 mNodeIndex(MORIS_INDEX_MAX)
{}
//------------------------------------------------------------------------------
Matrix< IdMat >
Vertex_Enrichment::get_ids() const
{
    return mBasisIds;
}
//------------------------------------------------------------------------------
Matrix< IndexMat >
Vertex_Enrichment::get_indices() const
{
    return this->get_basis_indices();
}
//------------------------------------------------------------------------------
const Matrix< DDRMat > *
Vertex_Enrichment::get_weights() const
{
    return &this->get_basis_weights();
}
//------------------------------------------------------------------------------
Matrix< IdMat >
Vertex_Enrichment::get_owners() const
{
    return mBaseVertexInterp->get_owners();
}
//------------------------------------------------------------------------------
void
Vertex_Enrichment::set_node_index(moris::moris_index aNodeIndex)
{
    MORIS_ASSERT(mNodeIndex==MORIS_INDEX_MAX,"Node index already set for Vertex Enrichment");
    mNodeIndex = aNodeIndex;
}
//------------------------------------------------------------------------------
void
Vertex_Enrichment::add_basis_information( moris::Matrix<moris::IndexMat> const & aBasisIndices,
                                          moris::Matrix<moris::IndexMat> const & aBasisId)
{
#ifdef DEBUG
    // since I can't write these functions in one line, need to have ifdef
    moris::Matrix<moris::IndexMat> tUniqueBasis;
    moris::unique(aBasisIndices,tUniqueBasis);

    MORIS_ASSERT(tUniqueBasis.numel() == aBasisIndices.numel(), "duplicate basis indices detected" );
#endif


    // num basis
    moris::uint tNumBasis = aBasisIndices.numel();

    // allocate space
    mBasisIndices.resize(tNumBasis,1);
    mBasisIds.resize(tNumBasis,1);
    mBasisWeights.resize(tNumBasis,1);

    // iterate to store data
    for(moris::uint i = 0; i<aBasisIndices.numel(); i++ )
    {
        moris::uint tBasisLocInd = this->local_basis_index(aBasisIndices(i));
        mBasisIndices(tBasisLocInd)         = aBasisIndices(i);
        mBasisIds(tBasisLocInd) = aBasisId(i);
    }
}
//------------------------------------------------------------------------------
void
Vertex_Enrichment::add_basis_weights(moris::Matrix<moris::IndexMat> const & aBasisIndices,
                                     moris::Matrix<moris::DDRMat>   const & aBasisWeight)
{
    for(moris::uint i = 0; i <aBasisIndices.numel(); i++)
    {
        moris::uint tBasisLocInd = this->local_basis_index(aBasisIndices(i));
        mBasisWeights(tBasisLocInd) = aBasisWeight(i);
    }
}
//------------------------------------------------------------------------------
void
Vertex_Enrichment::add_base_vertex_interpolation(mtk::Vertex_Interpolation * aBaseVertInterp)
{
	mBaseVertexInterp = aBaseVertInterp;
}
//------------------------------------------------------------------------------
mtk::Vertex_Interpolation const *
Vertex_Enrichment::get_base_vertex_interpolation() const
{
	return mBaseVertexInterp;
}
//------------------------------------------------------------------------------
std::unordered_map<moris::moris_index, moris::moris_index> &
Vertex_Enrichment::get_basis_map()
{
    return mBasisMap;
}
//------------------------------------------------------------------------------
moris::uint
Vertex_Enrichment::local_basis_index(moris::uint aBasisIndex)
{
    auto tIter = mBasisMap.find(aBasisIndex);
    MORIS_ASSERT(tIter!=mBasisMap.end(),"Provided basis index not found in map");

    return tIter->second;
}
//------------------------------------------------------------------------------
void
Vertex_Enrichment::condense_out_basis_with_0_weight()
{
    moris::uint tCount = 0;

    for( moris::uint i = 0;  i<mBasisIndices.numel(); i++)
    {
        if(moris::equal_to( mBasisWeights(i), 0))
        {
            mBasisMap.erase(mBasisIndices(i));
        }

        else
        {
            mBasisIndices(tCount) = mBasisIndices(i);
            mBasisWeights(tCount) = mBasisWeights(i);

            // change map index
            mBasisMap[mBasisIndices(i)] = tCount;
            tCount++;
        }
    }

    // remove excess space
    mBasisIndices.resize(tCount,1);
    mBasisWeights.resize(tCount,1);

}
//------------------------------------------------------------------------------
moris::Matrix< moris::IndexMat > const &
Vertex_Enrichment::get_basis_indices() const
{
    return mBasisIndices;
}
//------------------------------------------------------------------------------
moris::Matrix< moris::DDRMat > const &
Vertex_Enrichment::get_basis_weights() const
{
    return mBasisWeights;
}
moris::Matrix< moris::IndexMat > const &
Vertex_Enrichment::get_basis_ids() const
{
    return mBasisIds;
}

//------------------------------------------------------------------------------
moris::Matrix< moris::DDRMat > &
Vertex_Enrichment::get_basis_weights()
{
    return mBasisWeights;
}
//------------------------------------------------------------------------------
bool
Vertex_Enrichment::basis_exists_in_enrichment(moris_index aBasisIndex) const
{


    return mBasisMap.find(aBasisIndex) != mBasisMap.end();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

}


