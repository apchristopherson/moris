/*
 * fn_sqrtmat_Arma.hpp
 *
 *  Created on: Feb 01, 2020
 *      Author: maute
 */

#ifndef PROJECTS_LINALG_SRC_ARMA_IMPL_FN_SYLVESTER_ARMA_HPP_
#define PROJECTS_LINALG_SRC_ARMA_IMPL_FN_SYLVESTER_ARMA_HPP_
#define ARMA_ALLOW_FAKE_GCC
#include <armadillo>

namespace moris
{
    /**
     * @brief Solve the Sylvester equation, i.e. A*X + X*B + C = 0, where X is unknown.
     *
     * @param[in] A matrix
     * @param[in] B matrix
     * @param[in] C matrix
     *
     * @return solution of system
     *
     */

    template< typename ET >
    auto
    sylvester(
            ET const & aA,
            ET const & aB,
            ET const & aC )
    -> decltype( arma::syl( aA, aB, aC) )
    {
        return arma::syl(aA, aB, aC);
    }

    /**
     * @brief Solve the Sylvester equation, i.e. A*X + X*B + C = 0, where X is unknown.
     *
     * @param[out] X matrix
     * @param[in] A matrix
     * @param[in] B matrix
     * @param[in] C matrix
     *
     */

    template< typename ET >
    void
    sylvester(
            ET const & aA,
            ET const & aB,
            ET const & aC,
            ET       & aX)
    {
        arma::syl(aX, aA, aB, aC);
    }
}

#endif /* PROJECTS_LINALG_SRC_ARMA_IMPL_FN_SYLVESTER_ARMA_HPP_ */
