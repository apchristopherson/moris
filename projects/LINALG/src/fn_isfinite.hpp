/*
 * fn_isfinite.hpp
 *
 *  Created on: Aug 29, 2018
 *      Author: schmidt
 */
#ifndef PROJECTS_LINALG_SRC_FN_ISFINITE_HPP_
#define PROJECTS_LINALG_SRC_FN_ISFINITE_HPP_

// MORIS library header files.
#include "cl_Matrix.hpp"

#ifdef MORIS_USE_EIGEN
#include "Eigen_Impl/fn_isfinite_Eigen.hpp"
#endif

#ifdef MORIS_USE_ARMA
#include "Arma_Impl/fn_isfinite_Arma.hpp"
#endif

namespace moris
{
/**
 * @brief Checks if an object is finite
 *
 * @param[in] aA object to be checked.
 *
 * @return Checks if an object only contains finite entries or not,
 *         @f$ \mathbf{A}_{ii} = finite@f$\n
 *         Returns true if all elements of the object are finite.\n
 *         Returns false if at least one of the elements of the object
 *         is non-finite (±infinity or NaN).
 */
    template< typename Type, typename Matrix_Type >
    auto
    isfinite( Matrix< Type, Matrix_Type  > const & aA )
    -> decltype( isfinite( aA.matrix_data() ) )
    {
        return isfinite( aA.matrix_data() );
    }
}

#endif /* PROJECTS_LINALG_SRC_FN_ISfinite_HPP_ */
