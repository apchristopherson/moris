/*
 * fn_sort_Arma.hpp
 *
 *  Created on: Aug 29, 2018
 *      Author: schmidt
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_FN_SORT_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_FN_SORT_ARMA_HPP_
#define ARMA_ALLOW_FAKE_GCC
#include <armadillo>

namespace moris
{
    template< typename ET, typename Matrix_Type >
    void
    sort(
            ET const                     & aA,
            moris::Matrix< Matrix_Type > & aSorted )
    {
        aSorted = sort( aA );
    }

    template< typename ET, typename Matrix_Type, typename Num_Type >
    void
    sort(
            ET const                     & aA,
            moris::Matrix< Matrix_Type > & aSorted,
            char const                   * aDirection,
            Num_Type                       aDimension )
    {
        aSorted = sort( aA, aDirection, aDimension);
    }
}

#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_FN_SORT_ARMA_HPP_ */
