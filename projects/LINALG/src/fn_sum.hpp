/*
 * fn_sum.hpp
 *
 *  Created on: Aug 30, 2018
 *      Author: sonne
 */

#ifndef PROJECTS_LINALG_SRC_FN_SUM_HPP_
#define PROJECTS_LINALG_SRC_FN_SUM_HPP_

// MORIS library header files.
#include "cl_Matrix.hpp"

#ifdef MORIS_USE_EIGEN
#include "Eigen_Impl/fn_sum_Eigen.hpp"
#endif

#ifdef MORIS_USE_ARMA
#include "Arma_Impl/fn_sum_Arma.hpp"
#endif

namespace moris
{
    /**
     * @brief Calculate the sum of a matrix.
     *
     *@param[in] aA A given matrix
     *
     * Example:
     * @include LNA/src/fn_sum.inc
     *
     */
    template< typename Matrix_Type >
    auto
    sum( const Matrix< Matrix_Type > & aA )
        -> decltype( sum( aA.matrix_data() ) )
    {
        return sum( aA.matrix_data() );
    }

    /*
    template< typename Matrix_Type >
    auto
    sum( Matrix< Matrix_Type > & aA )
        -> decltype( sum( aA.matrix_data() ) )
    {
        return sum( aA.matrix_data() );
    }*/


}




#endif /* PROJECTS_LINALG_SRC_FN_SUM_HPP_ */