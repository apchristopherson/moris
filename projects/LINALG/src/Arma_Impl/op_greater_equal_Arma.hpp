/*
 * op_greater_equal_Arma.hpp
 *
 *  Created on: Aug 31, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_OP_GREATER_EQUAL_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_OP_GREATER_EQUAL_ARMA_HPP_

namespace moris
{
template< typename Matrix_Type, typename ET >
auto
operator>=( const ET &  aA,
           Matrix< Matrix_Type > & aB )
->decltype( aA >= aB.matrix_data() )
{
    return  aA >= aB.matrix_data();
}

template< typename Matrix_Type, typename ET >
auto
operator>=( Matrix< Matrix_Type > & aA,
           const ET &  aB)
->decltype( aA.matrix_data()  >= aB )
{
    return  aA.matrix_data() >= aB;
}



}



#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_OP_GREATER_EQUAL_ARMA_HPP_ */
