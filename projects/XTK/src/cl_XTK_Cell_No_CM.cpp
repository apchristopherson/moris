/*
 * cl_MTK_Cell_XTK_CM_Impl.cpp
 *
 *  Created on: Feb 19, 2019
 *      Author: doble
 */

#include "cl_XTK_Cell_No_CM.hpp"
#include "cl_XTK_Background_Mesh.hpp"

namespace xtk
{
    // ----------------------------------------------------------------------------------
    // Constructor/Deconstructor Source code
    // ----------------------------------------------------------------------------------
    Cell_XTK_No_CM::Cell_XTK_No_CM(moris::moris_id                 aElementId,
                                   moris::moris_index              aElementIndex,
                                   moris::moris_index              aElementOwner,
                                   std::shared_ptr<mtk::Cell_Info> aCellInfo,
                                   moris::Cell< mtk::Vertex* >     aVertices)
                                   : Cell(aElementId,aElementIndex,aElementOwner, aCellInfo),
                           mCellVertices(aVertices)
                           {}

    // ----------------------------------------------------------------------------------
    // Cell get functions
    // ----------------------------------------------------------------------------------
    Matrix< DDRMat >
    Cell_XTK_No_CM::get_vertex_coords() const
    {
        size_t tNumVertices = this->get_number_of_vertices();
        Matrix< DDRMat > tVertexCoords;
        for(size_t i = 0; i<tNumVertices; i++)
        {
            Matrix<DDRMat> tVertCoord = mCellVertices(i)->get_coords();

            if(i == 0 )
            {
                tVertexCoords.resize(tNumVertices,tVertCoord.numel());
            }

            tVertexCoords.set_row(i,tVertCoord);
        }
        return tVertexCoords;
    }


    moris::real
    Cell_XTK_No_CM::compute_cell_side_measure_deriv(moris_index const & aSideOrdinal, uint aLocalVertexID, uint aDirection) const
    {
        return mCellInfo->compute_cell_side_size_deriv(this, aSideOrdinal, aLocalVertexID, aDirection);
    }

}



