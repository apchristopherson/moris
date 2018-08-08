/*
 * cl_FEM_Interpolation_Function_Lagrange_Bar2.hpp
 *
 *  Created on: Jul 13, 2018
 *      Author: messe
 */

#ifndef SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_BAR2_HPP_
#define SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_BAR2_HPP_

#include "assert.h"

#include "cl_FEM_Interpolation_Matrix.hpp"
#include "typedefs.hpp" //MRS/COR/src
#include "cl_FEM_Enums.hpp" //FEM/INT/src
#include "cl_FEM_Interpolation_Function.hpp" //FEM/INT/src

namespace moris
{
    namespace fem
    {

//------------------------------------------------------------------------------

    template<>
    mtk::Interpolation_Order
    Interpolation_Function< Interpolation_Type::LAGRANGE, 1, 2 >::get_interpolation_order() const
    {
        return mtk::Interpolation_Order::LINEAR;
    }

//------------------------------------------------------------------------------

    template<>
    void
    Interpolation_Function< Interpolation_Type::LAGRANGE, 1, 2  >::get_param_coords(
            Mat<real> & aXihat ) const
    {
        aXihat.set_size( 1, 2 );

        aXihat( 0 ) = -1.000000;
        aXihat( 1 ) =  1.000000;
    }


//------------------------------------------------------------------------------
        template<>
        void
        Interpolation_Function< Interpolation_Type::LAGRANGE, 1, 2  >::eval_N(
                      Interpolation_Matrix  & aN,
                const Mat<real> & aXi
        )  const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 1,
                    "eval_N: aXi not allocated or hat wrong size." );

            // make sure that output array has correct number of columns
            MORIS_ASSERT( aN.n_cols() == 2,
                    "eval_N: aN not allocated or hat wrong size." );

            // make sure that output array has correct number of rows
            MORIS_ASSERT( aN.n_rows() == 1,
                    "eval_N: aN not allocated or hat wrong size." );

            auto xi = aXi( 0 );

            aN( 0 ) = 0.5*(1.0-xi);
            aN( 1 ) = 0.5*(1.0+xi);
        }

//------------------------------------------------------------------------------

        template<>
        void
        Interpolation_Function< Interpolation_Type::LAGRANGE, 1, 2  >::eval_dNdXi(
                      Interpolation_Matrix  & adNdXi,
                const Mat<real> & aXi
        )  const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 1,
                    "eval_dNdXi: aXi not allocated or hat wrong size." );

            // make sure that output array has correct number of columns
            MORIS_ASSERT( adNdXi.n_cols() == 2,
                    "eval_dNdXi: aN not allocated or hat wrong size." );

            // make sure that output array has correct number of rows
            MORIS_ASSERT( adNdXi.n_rows() == 1,
                    "eval_dNdXi: aN not allocated or hat wrong size." );


            adNdXi( 0 ) = -0.5;
            adNdXi( 1 ) =  0.5;

        }

//------------------------------------------------------------------------------

        template<>
        void
        Interpolation_Function< Interpolation_Type::LAGRANGE, 1, 2  >::eval_d2NdXi2(
                      Interpolation_Matrix  & ad2NdXi2,
                const Mat<real> & aXi
        )  const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 1,
                    "ad2NdXi2: aXi not allocated or hat wrong size." );

            // make sure that output array has correct number of columns
            MORIS_ASSERT( ad2NdXi2.n_cols() == 2,
                    "ad2NdXi2: aN not allocated or hat wrong size." );

            // make sure that output array has correct number of rows
            MORIS_ASSERT( ad2NdXi2.n_rows() == 1,
                    "ad2NdXi2: aN not allocated or hat wrong size." );

            ad2NdXi2( 0 ) =  0.0;
            ad2NdXi2( 1 ) =  0.0;
        }
//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

//------------------------------------------------------------------------------
#endif /* SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_BAR2_HPP_ */