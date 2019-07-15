/*
 * cl_FEM_Interpolation_Function_Tri6.hpp
 *
 *  Created on: Apr 03, 2019
 *      Author: noel
 */

#ifndef SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_TRI6_HPP_
#define SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_TRI6_HPP_

#include "assert.h"
#include "typedefs.hpp" //MRS/COR/src
#include "cl_FEM_Enums.hpp" //FEM/INT/src
#include "cl_FEM_Interpolation_Function.hpp" //FEM/INT/src
#include "op_times.hpp"

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        template<>
        uint
        Interpolation_Function< mtk::Geometry_Type::TRI, Interpolation_Type::LAGRANGE, 2, 6 >::get_number_of_param_dimensions() const
        {
            return 3;
        }

//------------------------------------------------------------------------------

        template<>
        mtk::Interpolation_Order
        Interpolation_Function< mtk::Geometry_Type::TRI, Interpolation_Type::LAGRANGE, 2, 6 >::get_interpolation_order() const
        {
            return mtk::Interpolation_Order::QUADRATIC;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TRI, Interpolation_Type::LAGRANGE, 2, 6 >::get_param_coords() const
        {
            Matrix< DDRMat > tXiHat =
            {
                { 1.000000000000000, 0.000000000000000, 0.000000000000000,
                  0.500000000000000, 0.000000000000000, 0.500000000000000 },
                { 0.000000000000000, 1.000000000000000, 0.000000000000000,
                  0.500000000000000, 0.500000000000000, 0.000000000000000 },
                { 0.000000000000000, 0.000000000000000, 1.000000000000000,
                  0.000000000000000, 0.500000000000000, 0.500000000000000 }
            };
            return tXiHat;
        }

//------------------------------------------------------------------------------

        template<>
        void
        Interpolation_Function< mtk::Geometry_Type::TRI, Interpolation_Type::LAGRANGE, 2, 6 >::eval_N( const Matrix< DDRMat > & aXi,
                                                                                                             Matrix< DDRMat > & aNXi) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 3, "TRI6 - eval_N: aXi not allocated or hat wrong size." );

            // unpack  the triangular coordinates input vector
            real zeta1 = aXi( 0 );
            real zeta2 = aXi( 1 );
            real zeta3 = aXi( 2 );

            // populate matrix with values
            aNXi.set_size( 1, 6 );
            aNXi( 0 ) = zeta1 * ( 2.0 * zeta1 - 1.0 );
            aNXi( 1 ) = zeta2 * ( 2.0 * zeta2 - 1.0 );
            aNXi( 2 ) = zeta3 * ( 2.0 * zeta3 - 1.0 );
            aNXi( 3 ) = 4.0 * zeta1 * zeta2;
            aNXi( 4 ) = 4.0 * zeta2 * zeta3;
            aNXi( 5 ) = 4.0 * zeta3 * zeta1;
        }

//------------------------------------------------------------------------------

        template<>
        void
        Interpolation_Function< mtk::Geometry_Type::TRI, Interpolation_Type::LAGRANGE, 2, 6 >::eval_dNdXi( const Matrix< DDRMat > & aXi,
                                                                                                                 Matrix< DDRMat > & adNdXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 2, "TRI6 - eval_dNdXi: aXi not allocated or hat wrong size." );

            // unpack  the triangular coordinates input vector
            real zeta1 = aXi( 0 );
            real zeta2 = aXi( 1 );
            real zeta3 = aXi( 2 );

            // populate output matrix
            adNdXi.set_size( 3, 6 );
            adNdXi( 0, 0 ) = 4 * zeta1 - 1.0;
            adNdXi( 0, 1 ) = 0.0;
            adNdXi( 0, 2 ) = 0.0;
            adNdXi( 0, 3 ) = 4 * zeta2;
            adNdXi( 0, 4 ) = 0.0;
            adNdXi( 0, 5 ) = 4 * zeta3;

            adNdXi( 1, 0 ) = 0.0;
            adNdXi( 1, 1 ) = 4 * zeta2 - 1.0;
            adNdXi( 1, 2 ) = 0.0;
            adNdXi( 1, 3 ) = 4 * zeta1;
            adNdXi( 1, 4 ) = 4 * zeta3;
            adNdXi( 1, 5 ) = 0.0;

            adNdXi( 2, 0 ) = 0.0;
            adNdXi( 2, 1 ) = 0.0;
            adNdXi( 2, 2 ) = 4 * zeta3 - 1.0;
            adNdXi( 2, 3 ) = 0.0;
            adNdXi( 2, 4 ) = 4 * zeta2;
            adNdXi( 2, 5 ) = 4 * zeta1;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TRI, Interpolation_Type::LAGRANGE, 2, 6 >::eval_d2NdXi2( const Matrix< DDRMat > & aXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 3, "TRI6 - eval_d2NdXi2: aXi not allocated or hat wrong size." );

            // populate output matrix
            Matrix< DDRMat > td2NdZeta2( 6, 6, 0.0 );
            td2NdZeta2( 0, 0 ) = 4.0;
            td2NdZeta2( 1, 1 ) = 4.0;
            td2NdZeta2( 2, 2 ) = 4.0;
            td2NdZeta2( 5, 3 ) = 4.0;
            td2NdZeta2( 3, 4 ) = 4.0;
            td2NdZeta2( 4, 5 ) = 4.0;

            return td2NdZeta2;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TRI, Interpolation_Type::LAGRANGE, 2, 6 >::eval_d3NdXi3( const Matrix< DDRMat > & aXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( false , "TRI6 - eval_d3NdXi3: 3rd order derivatives not implemented for this element." );

            Matrix< DDRMat > td3NdXi3(1,6,0.0);
            return td3NdXi3;
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
#endif /* SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_TRI6_HPP_ */
