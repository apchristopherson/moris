/*
 * fn_sort_Eigen.hpp
 *
 *  Created on: Aug 29, 2018
 *      Author: schmidt
 */

#ifndef PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_SORT_EIGEN_HPP_
#define PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_SORT_EIGEN_HPP_

#include <Eigen/Dense>

#include "assert.hpp"
#include "typedefs.hpp"
#include "cl_Matrix.hpp"


namespace moris
{
    template< typename ET, typename Type, typename Matrix_Type >
    void
    sort( const Eigen::MatrixBase<ET> & aA,
                moris::Matrix< Type, Matrix_Type > & aSorted )
    {
        //MORIS::ERROR(false, "sort in eigen not implemented yet");
        // Eigen does not have an internal sort function
        // 1. Create copy  of input  matrix
        // 2. Sort matrix using std::sort
        aSorted.matrix_data() = aA;
        
		// make sure that matrix is row or col matrix
		MORIS_ASSERT( aSorted.n_rows() == 1 || aSorted.n_cols() == 1,
	                 "sort() can only be performed on a vector" );
                
        Type * tData = aSorted.data();
        moris::uint tLen = aSorted.numel();
        std::sort(tData,tData+tLen);
    }

}

#endif /* PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_SORT_EIGEN_HPP_ */
