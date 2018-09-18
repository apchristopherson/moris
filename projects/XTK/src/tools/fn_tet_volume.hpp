/*
 * fn_tet_volume.hpp
 *
 *  Created on: Jun 22, 2017
 *      Author: ktdoble
 */

#ifndef SRC_TOOLS_FN_TET_VOLUME_HPP_
#define SRC_TOOLS_FN_TET_VOLUME_HPP_

#include <memory>

#include "linalg/cl_XTK_Matrix.hpp"
#include "fn_det.hpp"

namespace xtk
{
template<typename Real_Matrix>
typename moris::Matrix< Real_Matrix >::Data_Type
vol_tetrahedron(moris::Matrix< Real_Matrix >  & aCoord)
{
    //explanation: www.colorado.edu/engineering/Aerospace/CAS/courses.d/AFEM.d/AFEM.Ch09.d/AFEM.Ch09.pdf
    moris::Matrix< Real_Matrix > J(4,4,1.0); // 4 Coordinates are needed for the Jacobian-matrix.
    // The order of the coordinates is irrelevant.

    J( 1, 0 ) = aCoord(0,0); J( 1, 1 ) = aCoord(1,0); J( 1, 2 ) = aCoord(2,0); J( 1, 3 ) = aCoord(3,0);
    J( 2, 0 ) = aCoord(0,1); J( 2, 1 ) = aCoord(1,1); J( 2, 2 ) = aCoord(2,1); J( 2, 3 ) = aCoord(3,1);
    J( 3, 0 ) = aCoord(0,2); J( 3, 1 ) = aCoord(1,2); J( 3, 2 ) = aCoord(2,2); J( 3, 3 ) = aCoord(3,2);

    typename moris::Matrix< Real_Matrix >::Data_Type volume = (det(J))/6; // Volume = 1/6*det(J)
    return volume;
}
}

#endif /* SRC_TOOLS_FN_TET_VOLUME_HPP_ */
