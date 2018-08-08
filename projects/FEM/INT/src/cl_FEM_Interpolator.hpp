/*
 * cl_FEM_Interpolator.hpp
 *
 *  Created on: Jul 13, 2018
 *      Author: messe
 */

#ifndef SRC_FEM_CL_FEM_INTERPOLATOR_HPP_
#define SRC_FEM_CL_FEM_INTERPOLATOR_HPP_

#include "cl_FEM_Interpolation_Function_Base.hpp"
#include "typedefs.hpp" //MRS/COR/src
#include "cl_Mat.hpp" //LNA/src

#include "cl_FEM_Interpolation_Rule.hpp" //FEM/INT/src
#include "cl_FEM_Interpolation_Matrix.hpp" //FEM/INT/src
#include "cl_FEM_Geometry_Interpolator.hpp" //FEM/INT/src
#include "cl_FEM_Integrator.hpp" //FEM/INT/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        // forward declaration of element
        class Element;

//------------------------------------------------------------------------------

        class Interpolator
        {
            //! how many fields are to be interpolated
            const uint mNumberOfFields;

            //! pointer to space interpolation object
            Interpolation_Function_Base * mSpaceInterpolation = nullptr;

            //! pointer to time interpolation object
            Interpolation_Function_Base * mTimeInterpolation  = nullptr;

            //! pointer to space time interpolation object
            Interpolation_Function_Base * mSpaceTimeInterpolation  = nullptr;

            //! pointer to function that creates matrices
            Interpolation_Function_Base * mMatrixCreator = nullptr;

            //! geometry interpolation object
            Geometry_Interpolator       * mGeometryInterpolator = nullptr;

            //! pointer to integrator object (must be pointer because may be not assigned
            //                                  in alternative constructor )
            Integrator                  * mIntegrator = nullptr;

            //! container for node coordinates
            Mat< real > mNodeCoords;

            //! container for integration points
            Mat< real > mIntegrationPoints;

            //! container for integration weights
            Mat< real > mIntegrationWeights;

            //! flag telling if element is isoparametric
            bool mIsoparametricFlag;

            //! transposed of Geometry Jacobi
            Mat< real > mJt;

            //! last point for which mJt was evaluated
            Mat< real > mLastPointJt;

            //! derivative matrix for the geometry
            Interpolation_Matrix * mGdNdXi = nullptr;

//------------------------------------------------------------------------------
        public:
//------------------------------------------------------------------------------

            /**
             * constructor with two interpolation rules and integrator
             *
             * @param[ in ] aSpaceInterpolationRule   pointer to space interpolation rule
             * @param[ in ] aTimeInterpolationRule    pointer to time interpolation rule
             * @param[ in ] aGeometryInterpolator     pointer to geometry interpolation object
             * @param[ in ] aIntegrator               pointer to integration object
             *
             */
            Interpolator(
                          Element               * aElement,
                    const uint                  & aNumberOfFields,
                    const Interpolation_Rule    & aFieldInterpolationRule,
                    const Interpolation_Rule    & aGeometryInterpolationRule,
                    const Integration_Rule      & aIntegrationRule );

//------------------------------------------------------------------------------

            /**
             * default constructor
             */
            ~Interpolator();

//------------------------------------------------------------------------------


            /**
             * a constructor for the interpolation matrix object
             *
             * @param[ in ] derivative in space
             * @param[ in ] derivative in time  ( currently unused )
             * @param[ in ] type of coeffs  :
             *               0 : Interpolated values
             *               1 : shape function matrix
             *               2 : geometry jacobian
             */
            Interpolation_Matrix
            create_matrix(
                    const uint & aDerivativeInSpace,
                    const uint & aDerivativeInTime,
                    const uint & aCoeffsType  ) const;

//------------------------------------------------------------------------------

            /**
             * returns the number of DOFs of this interpolation
             */
            uint
            get_number_of_dofs();

//------------------------------------------------------------------------------

            /**
             * evaluates a matrix according to a given point
             */
            void
            evaluate_matrix(
                            Interpolation_Matrix & aMatrix,
                            const Mat< real >    & aPoint );

//------------------------------------------------------------------------------

            /**
             * evaluates a matrix according to am integration point index
             */
            void
            evaluate_matrix(
                            Interpolation_Matrix & aMatrix,
                            const uint           & aPoint );

//------------------------------------------------------------------------------

            /**
             * returns the number of points of the integrator
             */
            uint
            get_number_of_integration_points();

//------------------------------------------------------------------------------

            /**
             * returns the integration weight of this point
             */
            real
            get_integration_weight( const uint & aPoint );

//------------------------------------------------------------------------------

            /**
             * returns the determinatnt of the geometry Jacobian by index
             */
            real
            get_det_J( const uint & aPoint );

//------------------------------------------------------------------------------

            /**
             * returns the determinatnt of the geometry Jacobian by point
             */
            real
            get_det_J( const Mat< real > & aPoint );

//------------------------------------------------------------------------------
        private:
//------------------------------------------------------------------------------

            void
            eval_dNdx( Interpolation_Matrix & aMatrix,
                       const Mat< real >    & aPoint );
        };

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_INTERPOLATOR_HPP_ */