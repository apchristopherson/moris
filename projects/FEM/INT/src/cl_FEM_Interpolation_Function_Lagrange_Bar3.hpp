/*
 * cl_FEM_Interpolation_Function_Lagrange_Bar3.hpp
 *
 *  Created on: Jul 13, 2018
 *      Author: messe
 */

#ifndef SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_BAR3_HPP_
#define SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_BAR3_HPP_

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
        Interpolation_Function< mtk::Geometry_Type::LINE, Interpolation_Type::LAGRANGE, 1, 3 >::get_number_of_param_dimensions() const
        {
            return 1;
        }

//------------------------------------------------------------------------------

        template<>
        mtk::Interpolation_Order
        Interpolation_Function< mtk::Geometry_Type::LINE, Interpolation_Type::LAGRANGE, 1, 3 >::get_interpolation_order()  const
        {
            return mtk::Interpolation_Order::QUADRATIC;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::LINE, Interpolation_Type::LAGRANGE, 1, 3 >::get_param_coords() const
        {
            Matrix< DDRMat > tXiHat(1,3);
            tXiHat( 0 ) = -1.000000;
            tXiHat( 1 ) =  1.000000;
            tXiHat( 2 ) =  0.000000;
            return tXiHat;
        }

//------------------------------------------------------------------------------

        template<>
        void
        Interpolation_Function< mtk::Geometry_Type::LINE, Interpolation_Type::LAGRANGE, 1, 3 >::eval_N( const Matrix< DDRMat > & aXi,
                                                                                                              Matrix< DDRMat > & aNXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 1, "LINE3 - eval_N: aXi not allocated or hat wrong size." );

            real xi = aXi( 0 );
            real xi2 = std::pow( xi , 2 );

            aNXi.set_size( 1, 3 );
            aNXi( 0 ) = 0.5 * ( xi2 - xi );
            aNXi( 1 ) = 0.5 * ( xi2 + xi );
            aNXi( 2 ) = 1.0 - xi2;
        }

//------------------------------------------------------------------------------

        template<>
        void
        Interpolation_Function< mtk::Geometry_Type::LINE, Interpolation_Type::LAGRANGE, 1, 3 >::eval_dNdXi( const Matrix< DDRMat > & aXi,
                                                                                                                  Matrix< DDRMat > & adNdXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 1, "LINE3 - eval_dNdXi: aXi not allocated or hat wrong size." );

            // set adNdXi
            real xi = aXi( 0 );
            adNdXi.set_size( 1, 3 );
            adNdXi( 0 ) =   xi - 0.5;
            adNdXi( 1 ) =   xi + 0.5;
            adNdXi( 2 ) = - 2.0 * xi;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::LINE, Interpolation_Type::LAGRANGE, 1, 3 >::eval_d2NdXi2( const Matrix< DDRMat > & aXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 1,
                          "LINE3 - eval_d2NdXi2: aXi not allocated or hat wrong size." );

            Matrix< DDRMat > td2NdXi2(1,3);
            td2NdXi2( 0 ) =   1.0;
            td2NdXi2( 1 ) =   1.0;
            td2NdXi2( 2 ) =  -2.0;
            return td2NdXi2;
        }

//------------------------------------------------------------------------------

        template<>
        Matrix< DDRMat >
        Interpolation_Function< mtk::Geometry_Type::LINE, Interpolation_Type::LAGRANGE, 1, 3 >::eval_d3NdXi3( const Matrix< DDRMat > & aXi ) const
        {
            // make sure that input is correct
            MORIS_ASSERT( aXi.length() >= 1,
                          "LINE3 - eval_d3NdXi3: aXi not allocated or hat wrong size." );

            Matrix< DDRMat > td3NdXi3(1,3,0.0);
            return td3NdXi3;
        }

//------------------------------------------------------------------------------

    } /* namespace fem */
} /* namespace moris */
//------------------------------------------------------------------------------
#endif /* SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_LAGRANGE_BAR3_HPP_ */
