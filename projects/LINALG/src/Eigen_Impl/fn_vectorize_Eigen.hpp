/*
 * fn_vectorize_Eigen.hpp
 *
 *  Created on: May 17, 2021
 *      Author: maute
 */

#ifndef PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_VECTORIZE_EIGEN_HPP_
#define PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_VECTORIZE_EIGEN_HPP_

#include <Eigen/Dense>
#include "cl_Matrix.hpp"

namespace moris
{
    template<  typename TYPE, typename INT >
    Eigen::Map<Eigen::Matrix<TYPE,   Eigen::Dynamic, Eigen::Dynamic>>
    vectorize(
            Eigen::Matrix<TYPE, Eigen::Dynamic, Eigen::Dynamic>  & aA,
            bool                                                   aToRowVector)
    {
        if (aToRowVector)
        {
            Eigen::Map<Eigen::Matrix<TYPE,   Eigen::Dynamic, Eigen::Dynamic>> tM2(aA.data(), 1, aA.size() );
            return tM2;
        }

        Eigen::Map<Eigen::Matrix<TYPE,   Eigen::Dynamic, Eigen::Dynamic>> tM2(aA.data(), aA.size(), 1 );
        return tM2;
    }
}

#endif /* PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_VECTORIZE_EIGEN_HPP_ */
