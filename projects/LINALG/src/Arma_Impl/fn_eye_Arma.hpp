/*
 * fn_eye_Arma.hpp
 *
 *  Created on: Aug 29, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_FN_EYE_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_FN_EYE_ARMA_HPP_

#include "cl_Matrix.hpp"
#include <armadillo>

namespace moris
{

template<typename ET>
inline
void
eye( moris::size_t const   & aNumRows,
     moris::size_t const   & aNumCols,
     ET & aEyeMat )
{
    aEyeMat =arma::eye( aNumRows, aNumCols );
}

}



#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_FN_EYE_ARMA_HPP_ */
