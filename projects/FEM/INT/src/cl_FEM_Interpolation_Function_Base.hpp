/*
 * cl_FEM_Interpolation_Function.hpp
 *
 *  Created on: Jul 9, 2018
 *      Author: messe
 */

#ifndef SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_BASE_HPP_
#define SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_BASE_HPP_

#include "cl_FEM_Interpolation_Matrix.hpp"
#include "cl_MTK_Enums.hpp" //MTK/src
#include "cl_FEM_Enums.hpp" //FEM/INT/src

namespace moris
{
    namespace fem
    {

//------------------------------------------------------------------------------

        /**
         * shape function base class
         */
        class Interpolation_Function_Base
        {

//------------------------------------------------------------------------------
        public:
//------------------------------------------------------------------------------

            /**
             * trivial constructor
             */
            Interpolation_Function_Base(){};

//------------------------------------------------------------------------------

            /**
             * trivial destructor
             */
            virtual ~Interpolation_Function_Base(){};

//------------------------------------------------------------------------------
            /**
             * evaluates the shape function at a given point
             *
             * @param[ out ] aN  shape function as
             *                   ( 1 x <number of nodes> )
             *
             * @param[ in ]  aXi parameter coordinates
             *                   ( <number of dimensions>  x 1 )
             */
            virtual void
            eval_N(        Interpolation_Matrix  & aN,
                     const Mat<real> & aXi  ) const = 0;

//------------------------------------------------------------------------------

            /**
             * calculates the first derivative of the shape function
             * in parameter space
             *
             * @param[ out ] adNdXi ( <number of dimensions> x <number of nodes> )
             *
             * @param[ in ] aXi    point where function is evaluated
             *                     ( <number of dimensions>  x 1 )
             *
             */
            virtual void
            eval_dNdXi (         Interpolation_Matrix & adNdXi,
                          const Mat<real> & aXi
                         ) const = 0;

//------------------------------------------------------------------------------

            /**
             * calculates the second derivative of the shape function
             * in parameter space
             *
             * @param[ out ] ad2NdXi2 ( <number of dimensions> x <number of nodes> )
             *
             * @param[ in ] aXi    point where function is evaluated
             *                     ( <number of dimensions>  x 1 )
             *
             */
            virtual void
            eval_d2NdXi2 (         Interpolation_Matrix & ad2NdXi2,
                            const Mat<real> & aXi ) const = 0;

//------------------------------------------------------------------------------

            /**
             * returns a matrix containing the parameter coordinates
             * < number of dimensions * number of basis >
             */
            virtual void
            get_param_coords( Mat< real > & aXihat ) const = 0;

//------------------------------------------------------------------------------

            /**
             * returns the number of basis for this shape function
             */
            virtual uint
            get_number_of_basis() const = 0;

//------------------------------------------------------------------------------

            /**
             * returns the number of dimensions for this shape function
             */
            virtual uint
            get_number_of_dimensions() const = 0;

//------------------------------------------------------------------------------

            /**
             * returns the interpolation order
             */
            virtual mtk::Interpolation_Order
            get_interpolation_order() const = 0;

//------------------------------------------------------------------------------

            /**
             * returns the interpolation type
             */
            virtual Interpolation_Type
            get_interpolation_type() const = 0;

//------------------------------------------------------------------------------

            /**
             * creates am interpolation matrix
             *
             * @param[ in ]  aDerivativeInSpace, 0, 1 or 2
             * @param[ in ]  aDerivativeInTime   0, 1 or 2
             * @param[ in ]  aCoeffsSwitch :
             *                      0: evaluated value
             *                      1: vector N, N_x or N_x2
             */
            virtual Interpolation_Matrix
            create_matrix( const uint & aNumberOfFields,
                           const uint & aDerivativeInSpace,
                           const uint & aDerivativeInTime,
                           const uint & aCoeffsSwitch ) const = 0;

//------------------------------------------------------------------------------

            /**
             * creates a pointer to a new interpolation matrix
             *
             * @param[ in ]  aDerivativeInSpace, 0, 1 or 2
             * @param[ in ]  aDerivativeInTime   0, 1 or 2
             * @param[ in ]  aCoeffsSwitch :
             *                      0: evaluated value
             *                      1: vector N, N_x or N_x2
             */
            virtual Interpolation_Matrix *
            create_matrix_pointer(
                    const uint & aNumberOfFields,
                    const uint & aDerivativeInSpace,
                    const uint & aDerivativeInTime,
                    const uint & aCoeffsSwitch ) const = 0;

//------------------------------------------------------------------------------
        };

//------------------------------------------------------------------------------

    } /* namespace fem */
} /* namespace moris */



#endif /* SRC_FEM_CL_FEM_INTERPOLATION_FUNCTION_BASE_HPP_ */