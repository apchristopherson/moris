/*
 * cl_FEM_Interpolation_Function_Tet4.hpp
 *
 *  Created on: Apr 04, 2019
 *      Author: noel
 */

#ifndef SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_TET4_HPP_
#define SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_TET4_HPP_

#include "assert.h"
#include "typedefs.hpp" //MRS/COR/src
#include "cl_FEM_Enums.hpp" //FEM/INT/src
#include "cl_FEM_Interpolation_Function.hpp" //FEM/INT/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        template<>
        uint
        Interpolation_Function< mtk::Geometry_Type::TET, Interpolation_Type::LAGRANGE, 3, 4 >::get_number_of_param_dimensions() const
        {
            return 4;
        }

//------------------------------------------------------------------------------

        template<>
        mtk::Interpolation_Order
        Interpolation_Function< mtk::Geometry_Type::TET, Interpolation_Type::LAGRANGE, 3, 4 >::get_interpolation_order() const
        {
            return mtk::Interpolation_Order::LINEAR;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TET, Interpolation_Type::LAGRANGE, 3, 4 >::get_param_coords() const
        {
            Matrix< DDRMat > tXiHat =
                { { 1.000000000000000, 0.000000000000000, 0.000000000000000, 0.000000000000000 },
                  { 0.000000000000000, 1.000000000000000, 0.000000000000000, 0.000000000000000 },
                  { 0.000000000000000, 0.000000000000000, 1.000000000000000, 0.000000000000000 },
                  { 0.000000000000000, 0.000000000000000, 0.000000000000000, 1.000000000000000 }
                };
            return tXiHat;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TET, Interpolation_Type::LAGRANGE, 3, 4 >::eval_N( const Matrix< DDRMat > & aXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 4, "TET4 - eval_N: aXi not allocated or hat wrong size." );

            // unpack zeta1, zeta2, zeta3, zeta4 from input vector
            real zeta1 = aXi( 0 );
            real zeta2 = aXi( 1 );
            real zeta3 = aXi( 2 );
            real zeta4 = aXi( 3 );

            // populate matrix with values
            Matrix< DDRMat > tN( 1, 4 );
            tN( 0 ) = zeta1;
            tN( 1 ) = zeta2;
            tN( 2 ) = zeta3;
            tN( 3 ) = zeta4;
            return tN;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TET, Interpolation_Type::LAGRANGE, 3, 4 >::eval_dNdXi( const Matrix< DDRMat > & aXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 4, "TET4 - eval_dNdXi: aXi not allocated or hat wrong size." );

            // populate output matrix
            Matrix< DDRMat > tdNdXi( 4, 4, 0.0 );
            tdNdXi( 0, 0 ) = 1.0;
            tdNdXi( 1, 1 ) = 1.0;
            tdNdXi( 2, 2 ) = 1.0;
            tdNdXi( 3, 3 ) = 1.0;
            return tdNdXi;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TET, Interpolation_Type::LAGRANGE, 3, 4 >::eval_d2NdXi2( const Matrix< DDRMat > & aXi ) const
        {

            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 4, "TET4 - eval_d2NdXi2: aXi not allocated or hat wrong size." );

            // populate output matrix
            Matrix< DDRMat > td2NdXi2( 10, 4, 0.0 );
            return td2NdXi2;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::TET, Interpolation_Type::LAGRANGE, 3, 4 >::eval_d3NdXi3( const Matrix< DDRMat > & aXi ) const
        {

            // make sure that input is correct
            MORIS_ASSERT( false, "TET4 - eval_d3NdXi3: 3rd order derivatives not implemented for this element." );

            Matrix< DDRMat > td3NdXi3(1,4,0.0);
            return td3NdXi3;

        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
#endif /* SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_TET4_HPP_ */