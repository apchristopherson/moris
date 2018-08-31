/*
 * op_minus.hpp
 *
 *  Created on: Aug 29, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_OP_MINUS_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_OP_MINUS_ARMA_HPP_

#include "cl_Matrix.hpp"
#include <armadillo>


namespace moris
{
template< typename T1, typename T2, typename ET >
auto
operator-( const ET &  aA,
           Matrix< T1, T2 > & aB )
->decltype( aA - aB.matrix_data() )
{
    return  aA - aB.matrix_data();
}

template< typename T1, typename T2, typename ET >
auto
operator-( Matrix< T1, T2 > & aA,
           const ET &  aB)
->decltype( aA.matrix_data() - aB )
{
    return  aA.matrix_data() - aB;
}

}



#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_OP_MINUS_ARMA_HPP_ */
