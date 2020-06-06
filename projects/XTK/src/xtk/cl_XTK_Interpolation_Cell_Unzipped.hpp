/*
 * cl_XTK_Interpolation_Cell_Unzipped.hpp
 *
 *  Created on: Jul 10, 2019
 *      Author: doble
 */

#ifndef PROJECTS_XTK_SRC_XTK_CL_XTK_INTERPOLATION_CELL_UNZIPPED_HPP_
#define PROJECTS_XTK_SRC_XTK_CL_XTK_INTERPOLATION_CELL_UNZIPPED_HPP_

#include "cl_XTK_Interpolation_Cell.hpp"
#include "cl_XTK_Interpolation_Vertex_Unzipped.hpp"
#include "cl_MTK_Cell.hpp"
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
using namespace moris;

namespace moris
{
namespace mtk
{
class Vertex;
}
}


namespace xtk
{

class Interpolation_Vertex_Unzipped;

class Interpolation_Cell_Unzipped: public Interpolation_Cell
{
public:
    Interpolation_Cell_Unzipped(){};
    Interpolation_Cell_Unzipped(moris::mtk::Cell*      aBaseCell,
                                moris_index            aSubphaseIndex,
                                moris_index            aBulkPhaseIndex,
                                moris_id               aCellId,
                                moris_index            aCellIndex,
                                moris_id               aCellOwner,
                                moris::mtk::Cell_Info* aConnectivity);

    //------------------------------------------------------------------------------
    // MTK Interpolation Cell Implementation
    // see base class for documentation
    //------------------------------------------------------------------------------
    uint                        get_level() const { return mBaseCell->get_level();};
    uint                        get_number_of_vertices() const;
    moris::Cell< mtk::Vertex* > get_vertex_pointers() const;
    Matrix< DDRMat >            get_vertex_coords() const;
    void                        set_vertices(moris::Cell< xtk::Interpolation_Vertex_Unzipped* > const & aVertexPointers);
    moris::mtk::Cell const*     get_base_cell() const;
    moris::mtk::Cell*           get_base_cell();
    //------------------------------------------------------------------------------
    // End Mtk Interpolation Cell Implementation
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    // Accessor functions of XTK specific data structures
    //------------------------------------------------------------------------------
    /*
     * Return the processor local sub phase index of this interpolation cell
     */
    moris_index
    get_subphase_index() const;
    //------------------------------------------------------------------------------
    /*
     * Return the bulk phase index of the subphase
     */
    moris_index
    get_bulkphase_index() const;
    //------------------------------------------------------------------------------
    moris::Cell< xtk::Interpolation_Vertex_Unzipped* > const &
    get_xtk_interpolation_vertices() const;
    //------------------------------------------------------------------------------
protected:
    moris::Cell< xtk::Interpolation_Vertex_Unzipped* > &
    get_xtk_interpolation_vertices();

    // friend class
    friend class Enrichment;
    friend class Ghost_Stabilization;

private:
    moris::mtk::Cell*                                  mBaseCell;
    moris::moris_index                                 mSubPhaseIndex;
    moris::moris_index                                 mBulkPhaseIndex;
    moris::Cell< xtk::Interpolation_Vertex_Unzipped* > mVertices;
    mtk::Cell_Info*                                    mCellInfo;
};


/*
 * Outputting of this data structure
 */
inline
std::ostream &
operator<<(std::ostream & os, const xtk::Interpolation_Cell_Unzipped & dt)
{
    os<<"Cell Id: "         <<std::right<<std::setw(9)<<dt.get_id()
      <<" | Cell Index: "   <<std::setw(9)<<dt.get_index()
      <<" | Base Cell Id: " <<std::setw(9)<<dt.get_base_cell()->get_id()
      <<" | Subphase: "     <<std::setw(9)<<dt.get_subphase_index()
      <<" | Bulkphase: "    <<std::setw(9)<<dt.get_bulkphase_index();

    // vertex interpolation
    std::cout<<"\n Interpolation Cell Vertices:"<<std::endl;
    moris::Cell< xtk::Interpolation_Vertex_Unzipped* > const & tVertices = dt.get_xtk_interpolation_vertices();

    for(moris::uint i = 0; i < tVertices.size(); i++)
    {
        os<<"    "<<*tVertices(i)<<std::endl;
    }

    return os;
}

inline
std::ostream &
operator<<(std::ostream & os, xtk::Interpolation_Cell_Unzipped const * const & dt)
{
    os<<*dt;

    return os;
}


}



#endif /* PROJECTS_XTK_SRC_XTK_CL_XTK_INTERPOLATION_CELL_UNZIPPED_HPP_ */
