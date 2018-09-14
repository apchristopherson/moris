/*
 * fn_unique_Eigen.hpp
 *
 *  Created on: Sep 5, 2018
 *      Author: messe
 */
#ifndef PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_UNIQUE_EIGEN_HPP_
#define PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_UNIQUE_EIGEN_HPP_
#include <Eigen/Dense>

#include "assert.hpp"
#include "typedefs.hpp"
#include "cl_Matrix.hpp"

namespace moris
{
    template< typename ET, typename Type, typename Matrix_Type >
    void
    unique( const Eigen::MatrixBase<ET>              & aMatrix,
                  moris::Matrix< Type, Matrix_Type > & aUniqueMatrix )
    {
        // copy expression template to output matrix
        aUniqueMatrix.matrix_data() = aMatrix;

        // make sure that matrix is row or col matrix
        MORIS_ASSERT( aUniqueMatrix.n_rows() == 1 || aUniqueMatrix.n_cols() == 1,
                "unique() can only be performed on a vector" );

        // get pointer to raw data
        Type * tData = aUniqueMatrix.data();

        // get length of data
        moris::uint tLength = aUniqueMatrix.numel();

        // sort data
        std::sort( tData, tData+tLength );

        // find positions
        auto tLast  = std::unique( tData, tData+tLength );
        auto tPos   = std::distance( tData, tLast );

        // resize matrix
        if ( aUniqueMatrix.n_rows() == 1)
        {
            aUniqueMatrix.resize( 1, tPos );
        }
        else
        {
            aUniqueMatrix.resize( tPos, 1 );
        }
    }
}
#endif /* PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_UNIQUE_EIGEN_HPP_ */
