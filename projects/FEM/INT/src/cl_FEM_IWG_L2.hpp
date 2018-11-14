/*
 * cl_FEM_Interpolator.hpp
 *
 *  Created on: Aug 13, 2018
 *      Author: messe
 */
#ifndef SRC_FEM_CL_FEM_IWG_L2_TEST_HPP_
#define SRC_FEM_CL_FEM_IWG_L2_TEST_HPP_

#include "typedefs.hpp"                     //MRS/COR/src


#include "cl_Cell.hpp"                      //MRS/CON/src
#include "cl_Matrix.hpp"                    //LINALG/src
#include "linalg_typedefs.hpp"              //LINALG/src
#include "cl_FEM_Interpolation_Matrix.hpp"  //FEM/INT/src
#include "cl_FEM_Interpolator.hpp"          //FEM/INT/src
#include "cl_FEM_IWG.hpp"                   //FEM/INT/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        class IWG_L2 : public IWG
        {
            // Alpha-Parameter, for J = M + alpha*K
            const real             mAlpha;

            // pointer to interpolator
            Interpolator         * mInterpolator = nullptr;

            // N-Matrix
            Interpolation_Matrix * mN = nullptr;

            // B-Matrix
            Interpolation_Matrix * mB = nullptr;



            void
            ( IWG_L2:: * mComputeFunction )(
                    Matrix< DDRMat >       & aJacobian,
                    Matrix< DDRMat >       & aResidual,
                    const Matrix< DDRMat > & aNodalDOF,
                    const Matrix< DDRMat > & aNodalWeakBC,
                    const uint             & aPointIndex );

//------------------------------------------------------------------------------
        public:
//------------------------------------------------------------------------------

            /*
             *  constructor
             *
             *  J = N'*N + alpha* B'*B
             */
            IWG_L2( const real aAlpha = 0.0 );

//------------------------------------------------------------------------------

            // trivial destructor
            ~IWG_L2(){};

//------------------------------------------------------------------------------

            /**
             * returns a cell with the dof types, assuming that all nodes
             * have the same type
             */
            Cell< MSI::Dof_Type >
            get_dof_types()
            {
                Cell< MSI::Dof_Type > aDofTypes( 1, MSI::Dof_Type::L2 );

                return aDofTypes;
            }

//------------------------------------------------------------------------------

            void
            create_matrices( Interpolator * aInterpolator );

//------------------------------------------------------------------------------

            void
            delete_matrices();

//------------------------------------------------------------------------------

            void
            compute_jacobian_and_residual(
                    Matrix< DDRMat >       & aJacobian,
                    Matrix< DDRMat >       & aResidual,
                    const Matrix< DDRMat > & aNodalDOF,
                    const Matrix< DDRMat > & aNodalWeakBC,
                    const uint        & aPointIndex );

//------------------------------------------------------------------------------

            /**
             * calculates the square of the error at a given point
             */
            real
            compute_integration_error(
                    const Matrix< DDRMat > & aNodalDOF,
                    real (*aFunction)( const Matrix< DDRMat > & aPoint ) ,
                    const uint        & aPointIndex );
//------------------------------------------------------------------------------
        private:
//------------------------------------------------------------------------------

            /**
             * J = N'*N
             */
            void
            compute_jacobian_and_residual_without_alpha(
                    Matrix< DDRMat >       & aJacobian,
                    Matrix< DDRMat >       & aResidual,
                    const Matrix< DDRMat > & aNodalDOF,
                    const Matrix< DDRMat > & aNodalWeakBC,
                    const uint             & aPointIndex );

//------------------------------------------------------------------------------

            /**
             * J = N'*N + alpha * B'*B
             */
            void
            compute_jacobian_and_residual_with_alpha(
                    Matrix< DDRMat >       & aJacobian,
                    Matrix< DDRMat >       & aResidual,
                    const Matrix< DDRMat > & aNodalDOF,
                    const Matrix< DDRMat > & aNodalWeakBC,
                    const uint        & aPointIndex );


//------------------------------------------------------------------------------

            real
            interpolate_scalar_at_point(
                                const Matrix< DDRMat > & aNodalWeakBC,
                                const uint             & aPointIndex );

//------------------------------------------------------------------------------
        };
//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_IWG_L2_TEST_HPP_ */
