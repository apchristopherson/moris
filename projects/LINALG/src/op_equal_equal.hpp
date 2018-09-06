/*
 * op_equal_equal.hpp
 *
 *  Created on: Aug 31, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_OP_EQUAL_EQUAL_HPP_
#define PROJECTS_LINALG_OP_EQUAL_EQUAL_HPP_

#ifdef MORIS_USE_EIGEN
#include "Eigen_Impl/op_equal_equal_Eigen.hpp"
#endif

#ifdef MORIS_USE_ARMA
#include "Arma_Impl/op_equal_equal_Arma.hpp"
#endif

namespace moris
{

/**
 * @brief Determine element-wise equality of two arrays
 *
 * @param[in] aA Input matrix
 * @param[in] aB Input matrix
 *
 * This function checks if @f$ \mathbf{A}_{ij} = \mathbf{B}_{ij} @f$. It returns a
 * logical matrix with elements set to logical 1 (true) where matrices A and B are
 * equal; otherwise, it returns logical 0 (false).
 *
 * Example:
 * @include LNA/src/op_equal_equal.inc
 *
 */
template< typename Type, typename Matrix_Type >
auto
operator==(
        Matrix< Type, Matrix_Type > const & aA,
        Matrix< Type, Matrix_Type > const & aB )
        ->decltype( operator==( aA.matrix_data(), aB.matrix_data() ) )
        {
    return operator==( aA.matrix_data(), aB.matrix_data() );
        }

template< typename Type, typename Matrix_Type >
auto
operator==(
        Matrix< Type, Matrix_Type > & aA,
        Matrix< Type, Matrix_Type > & aB )
        ->decltype( operator==( aA.matrix_data(), aB.matrix_data() ) )
        {
    return operator==( aA.matrix_data(), aB.matrix_data() );
        }

template< typename Type, typename Matrix_Type >
auto
operator==( Type aA,
        Matrix< Type, Matrix_Type > & aB )
        ->decltype( operator==( aA, aB.matrix_data() ) )
        {
    return operator==( aA, aB.matrix_data() );
        }

template< typename Type, typename Matrix_Type >
auto
operator==( Matrix< Type, Matrix_Type > & aA,
        Type                          aB)
        ->decltype( operator==( aA.matrix_data(), aB ) )
        {

    return operator==( aA.matrix_data(), aB );
        }
}



#endif /* PROJECTS_LINALG_OP_EQUAL_EQUAL_HPP_ */
