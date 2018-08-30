/*
 * fn_trans_Eigen.hpp
 *
 *  Created on: Aug 27, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_FN_TRANS_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_FN_TRANS_ARMA_HPP_

#include "cl_Matrix.hpp"
#include <armadillo>


namespace moris
{
template<typename ET >
auto
trans( const ET &  A)
->decltype( arma::strans( A ) )
{
    return arma::strans( A );
}

template<typename ET >
auto
trans( ET & A)
->decltype( arma::strans( A ) )
{
    return arma::strans( A );
}

}



#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_FN_TRANS_ARMA_HPP_ */
