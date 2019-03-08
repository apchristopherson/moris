/*
 * cl_XTK_Basis_Function.hpp
 *
 *  Created on: Jul 17, 2017
 *      Author: ktdoble
 */

#ifndef INCLUDE_TOPOLOGY_CL_XTK_BASIS_FUNCTION_HPP_
#define INCLUDE_TOPOLOGY_CL_XTK_BASIS_FUNCTION_HPP_

#include "cl_Matrix.hpp"

namespace xtk
{
class Basis_Function
{
public:
    ~Basis_Function()
    {

    }

    /**
     * Evaluates the values of the basis functions at a give local or parametric coordinate [-1,1]
     */
    virtual void evaluate_basis_function(moris::Matrix< moris::DDRMat > const & aLocalCoordinate,
                                         moris::Matrix< moris::DDRMat > & aBasisFunctionValues) const = 0;
};

}


#endif /* INCLUDE_TOPOLOGY_CL_XTK_BASIS_FUNCTION_HPP_ */