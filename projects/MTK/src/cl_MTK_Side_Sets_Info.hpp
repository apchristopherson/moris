/*
 * cl_MTK_Side_Sets_Info.hpp
 *
 *  Created on: Sep 17, 2018
 *      Author: doble
 */

#ifndef PROJECTS_MTK_SRC_CL_MTK_SIDE_SETS_INFO_HPP_
#define PROJECTS_MTK_SRC_CL_MTK_SIDE_SETS_INFO_HPP_

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_Cell.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_Mesh_Enums.hpp"

namespace moris
{
namespace mtk
{
/////////////////////////
// STRUC FOR SIDE SET  //
/////////////////////////
/*
 * To declare a side set in the mesh the following information
 * is needed:
 *  mElemIdsAndSideOrds - Element Id and the side ordinal of this element
 *                         col 0 - Element Id
 *                         col 1 - Side ordinal
 *
 *  mSideSetName        - Name of the side set
 */

struct MtkSideSetInfo
{
    Matrix< IdMat > * mElemIdsAndSideOrds;
    std::string       mSideSetName;
    bool              mParallelConsistencyReq = true;
    enum CellTopology mSideTopology;

    MtkSideSetInfo():
        mElemIdsAndSideOrds(),
        mSideSetName(),
        mSideTopology(CellTopology::INVALID){}

    bool
    sideset_has_name()
    {
        return !mSideSetName.empty();
    }
};

std::ostream &
operator<<(std::ostream & os, mtk::MtkSideSetInfo const * const & dt)
{
    os<<"Side Set Name: "<< dt->mSideSetName << " | Number of Sides: "<<dt->mElemIdsAndSideOrds->n_rows()<<" | Side Topology: "<< get_enum_str(dt->mSideTopology) <<"  | Parallel Consistent: "<<dt->mParallelConsistencyReq;

    return os;
}
}
}



#endif /* PROJECTS_MTK_SRC_CL_MTK_SIDE_SETS_INFO_HPP_ */
