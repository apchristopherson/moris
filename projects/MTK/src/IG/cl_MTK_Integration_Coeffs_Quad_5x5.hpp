/*
 * Copyright (c) 2022 University of Colorado
 * Licensed under the MIT license. See LICENSE.txt file in the MORIS root for details.
 *
 *------------------------------------------------------------------------------------
 *
 * cl_MTK_Integration_Coeffs_Quad_5x5.hpp
 *
 */

#ifndef SRC_MTK_CL_MTK_INTEGRATION_COEFFS_QUAD_5X5_HPP_
#define SRC_MTK_CL_MTK_INTEGRATION_COEFFS_QUAD_5X5_HPP_

#include "cl_MTK_Integration_Coeffs.hpp"
#include "typedefs.hpp" //MRS/COR/src
#include "cl_Matrix.hpp" //LNA/src
#include "linalg_typedefs.hpp" //LNA/src
#include "cl_MTK_Enums.hpp" //MTK/src

namespace moris
{
    namespace mtk
    {
//------------------------------------------------------------------------------

        template<>
        uint
        Integration_Coeffs<
            Integration_Type::GAUSS,
            Integration_Order::QUAD_5x5>::get_number_of_dimensions()
        {
            return 2;
        }

//------------------------------------------------------------------------------

        template<>
        uint
        Integration_Coeffs<
            Integration_Type::GAUSS,
            Integration_Order::QUAD_5x5>::get_number_of_points()
            {
                return 25;
            }

//------------------------------------------------------------------------------

        template<>
        void
        Integration_Coeffs<
                Integration_Type::GAUSS,
                Integration_Order::QUAD_5x5>::get_points( Matrix< DDRMat > & aIntegrationPoints )
        {
            aIntegrationPoints =
            {
                {
                    -9.061798459386640e-01,
                    -5.384693101056831e-01,
                    +0.000000000000000e+00,
                    +5.384693101056831e-01,
                    +9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -5.384693101056831e-01,
                    +0.000000000000000e+00,
                    +5.384693101056831e-01,
                    +9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -5.384693101056831e-01,
                    +0.000000000000000e+00,
                    +5.384693101056831e-01,
                    +9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -5.384693101056831e-01,
                    +0.000000000000000e+00,
                    +5.384693101056831e-01,
                    +9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -5.384693101056831e-01,
                    +0.000000000000000e+00,
                    +5.384693101056831e-01,
                    +9.061798459386640e-01
                },
                {
                    -9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -9.061798459386640e-01,
                    -5.384693101056831e-01,
                    -5.384693101056831e-01,
                    -5.384693101056831e-01,
                    -5.384693101056831e-01,
                    -5.384693101056831e-01,
                    +0.000000000000000e+00,
                    +0.000000000000000e+00,
                    +0.000000000000000e+00,
                    +0.000000000000000e+00,
                    +0.000000000000000e+00,
                    +5.384693101056831e-01,
                    +5.384693101056831e-01,
                    +5.384693101056831e-01,
                    +5.384693101056831e-01,
                    +5.384693101056831e-01,
                    +9.061798459386640e-01,
                    +9.061798459386640e-01,
                    +9.061798459386640e-01,
                    +9.061798459386640e-01,
                    +9.061798459386640e-01
                    }
            };
          }

//------------------------------------------------------------------------------

        template<>
        void
        Integration_Coeffs<
            Integration_Type::GAUSS,
            Integration_Order::QUAD_5x5 >::get_weights( Matrix< DDRMat > & aIntegrationWeights )
        {
            aIntegrationWeights =
            {
                    {
                            +5.613434886242864e-02,
                            +1.134000000000000e-01,
                            +1.347850723875209e-01,
                            +1.134000000000000e-01,
                            +5.613434886242864e-02,
                            +1.134000000000000e-01,
                            +2.290854042239911e-01,
                            +2.722865325507507e-01,
                            +2.290854042239911e-01,
                            +1.134000000000000e-01,
                            +1.347850723875209e-01,
                            +2.722865325507507e-01,
                            +3.236345679012347e-01,
                            +2.722865325507507e-01,
                            +1.347850723875209e-01,
                            +1.134000000000000e-01,
                            +2.290854042239911e-01,
                            +2.722865325507507e-01,
                            +2.290854042239911e-01,
                            +1.134000000000000e-01,
                            +5.613434886242864e-02,
                            +1.134000000000000e-01,
                            +1.347850723875209e-01,
                            +1.134000000000000e-01,
                            +5.613434886242864e-02
                    }
            };
        }

//------------------------------------------------------------------------------
    } /* namespace mtk */
} /* namespace moris */
#endif /* SRC_MTK_CL_MTK_INTEGRATION_COEFFS_QUAD_5X5_HPP_ */
