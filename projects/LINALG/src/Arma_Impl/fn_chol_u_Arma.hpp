/*
 * fn_chol_u_Arma.hpp
 *
 *  Created on: Sep 6, 2018
 *      Author: messe
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_FN_CHOL_U_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_FN_CHOL_U_ARMA_HPP_

#include "cl_Matrix.hpp"
#include <armadillo>

namespace moris
{
    template< typename ET >
    auto
    chol_u(
            const ET & aA,
            const std::string & aString = "upper" )
        -> decltype (  arma::chol( aA, aString.c_str() ) )
    {
       return arma::chol( aA, aString.c_str() );
    }
}

#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_FN_CHOL_U_ARMA_HPP_ */
