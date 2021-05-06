/*
 * fn_eye_Eigen.hpp
 *
 *  Created on: Aug 29, 2018
 *      Author: doble
 */

#ifndef PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_EYE_EIGEN_HPP_
#define PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_EYE_EIGEN_HPP_

#include "cl_Matrix.hpp"
#include "Eigen/Dense"

namespace moris
{
    template<typename ET>
    inline
    void
    eye(
            moris::size_t const   & aNumRows,
            moris::size_t const   & aNumCols,
            Eigen::MatrixBase<ET> & aEyeMat )
    {
        aEyeMat = Eigen::MatrixXd::Identity( aNumRows, aNumCols );
    }

    namespace linalg_internal
    {
        inline
        auto
        eye(
                moris::size_t const   & aNumRows,
                moris::size_t const   & aNumCols)
        ->decltype( Eigen::MatrixXd::Identity( aNumRows, aNumCols ) )
        {
            return Eigen::MatrixXd::Identity( aNumRows, aNumCols );
        }
    }
}

#endif /* PROJECTS_LINALG_SRC_EIGEN_IMPL_FN_EYE_EIGEN_HPP_ */
