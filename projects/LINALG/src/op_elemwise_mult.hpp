/*
 * op_elemwise_mult.hpp
 *
 *  Created on: Sep 6, 2018
 *      Author: messe
 */

#ifndef PROJECTS_LINALG_SRC_OP_ELEMWISE_MULT_HPP_
#define PROJECTS_LINALG_SRC_OP_ELEMWISE_MULT_HPP_

#ifdef MORIS_USE_EIGEN
#include "Eigen_Impl/op_elemwise_mult_Eigen.hpp"
#endif

#ifdef MORIS_USE_ARMA
#include "Arma_Impl/op_elemwise_mult_Arma.hpp"
#endif

#endif /* PROJECTS_LINALG_SRC_OP_ELEMWISE_MULT_HPP_ */
