/*
 * cl_FEM_Integration_Coeffs_Tet_1.hpp
 *
 *  Created on: Apr 05, 2019
 *      Author: noel
 */

#ifndef SRC_FEM_CL_FEM_INTEGRATION_COEFFS_TET_1_HPP_
#define SRC_FEM_CL_FEM_INTEGRATION_COEFFS_TET_1_HPP_

//MRS/COR/src
#include "typedefs.hpp"
//LINALG/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
//FEM/INT/src
#include "cl_FEM_Enums.hpp"
#include "cl_FEM_Integration_Coeffs.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        template<>
        uint
        Integration_Coeffs<
        Integration_Type::GAUSS,
        Integration_Order::TET_1>::get_number_of_dimensions()
        {
            return 4;
        }

        //------------------------------------------------------------------------------

        template<>
        uint
        Integration_Coeffs<
        Integration_Type::GAUSS,
        Integration_Order::TET_1>::get_number_of_points()
        {
            return 1;
        }

        //------------------------------------------------------------------------------

        template<>
        void
        Integration_Coeffs<
        Integration_Type::GAUSS,
        Integration_Order::TET_1>::get_points( Matrix< DDRMat > & aIntegrationPoints )
        {
            aIntegrationPoints =
            {
                    {
                            0.250000000000000
                    },
                    {
                            0.250000000000000
                    },
                    {
                            0.250000000000000
                    },
                    {
                            0.250000000000000
                    }
            };
        }

        //------------------------------------------------------------------------------

        template<>
        void
        Integration_Coeffs<
        Integration_Type::GAUSS,
        Integration_Order::TET_1 >::get_weights( Matrix< DDRMat > & aIntegrationWeights )
        {
            aIntegrationWeights.set_size( 1, 1, 1.0 );
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
#endif /* SRC_FEM_CL_FEM_INTEGRATION_COEFFS_TET_1_HPP_ */