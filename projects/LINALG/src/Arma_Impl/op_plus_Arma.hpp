/*
 * op_plus_Arma.hpp
 *
 *  Created on: Aug 27, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_OP_PLUS_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_OP_PLUS_ARMA_HPP_

#include "cl_Matrix.hpp"
#define ARMA_ALLOW_FAKE_GCC
#include <armadillo>


namespace moris
{
    template< typename Matrix_Type, typename ET >
    auto
    operator+(
            const ET                    & aA,
            Matrix< Matrix_Type > const & aB )
    ->decltype( aA + aB.matrix_data() )
    {
        return  aA + aB.matrix_data();
    }

    template< typename Matrix_Type, typename ET >
    auto
    operator+(
            Matrix< Matrix_Type > const & aA,
            const ET                    & aB)
    ->decltype( aA.matrix_data() + aB )
    {
        return  aA.matrix_data() + aB;
    }
}

#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_OP_PLUS_ARMA_HPP_ */
