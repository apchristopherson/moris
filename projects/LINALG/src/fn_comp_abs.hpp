/*
 * fn_comp_abs.hpp
 *
 *  Created on: Aug 27, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_FN_COMP_ABS_HPP_
#define PROJECTS_LINALG_SRC_FN_COMP_ABS_HPP_

// MORIS library header files.
#include "cl_Matrix.hpp"

#ifdef MORIS_USE_EIGEN
#include "Eigen_Impl/fn_comp_abs_Eigen.hpp"
#endif

#ifdef MORIS_USE_ARMA
#include "fn_comp_abs_Arma.hpp"
#endif

namespace moris
{

template<typename Matrix_Type >
auto
comp_abs( Matrix< Matrix_Type > & aA )
-> decltype( comp_abs(aA.matrix_data()) )
{
    return comp_abs(aA.matrix_data());
}

template< typename Matrix_Type >
auto
comp_abs( Matrix< Matrix_Type > const & aA )
-> decltype( comp_abs(aA.matrix_data()) )
{
    return comp_abs(aA.matrix_data());
}

}



#endif /* PROJECTS_LINALG_SRC_FN_COMP_ABS_HPP_ */
