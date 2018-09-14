/*
 * fn_cond_Eigen.hpp
 *
 *  Created on: Sep 12, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_COND_EIGEN_HPP_
#define PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_COND_EIGEN_HPP_

#include "cl_Matrix.hpp"
#include "Eigen/Dense"
#include "fn_svd.hpp"
namespace moris
{
    template< typename ET>
    real
    cond(const Eigen::MatrixBase<ET> & aA)
    {
        Eigen::Matrix<real,   Eigen::Dynamic, Eigen::Dynamic> tS;
        moris::svd( tS, aA );

        size_t lowerEigvalInd = 0;
        size_t upperEigvalInd = tS.rows()-1;

        return tS( lowerEigvalInd, 0 ) / tS( upperEigvalInd, 0 );
    }
}

#endif /* PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_COND_EIGEN_HPP_ */
