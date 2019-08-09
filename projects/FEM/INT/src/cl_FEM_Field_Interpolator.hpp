/*
 * cl_FEM_Field_Interpolator.hpp
 *
 *  Created on: Jan 31, 2019
 *      Author: noel
 */

#ifndef SRC_FEM_CL_FEM_FIELD_INTERPOLATOR_HPP_
#define SRC_FEM_CL_FEM_FIELD_INTERPOLATOR_HPP_

#include "typedefs.hpp" //MRS/COR/src
#include "cl_Matrix.hpp" //LNA/src
#include "cl_Cell.hpp"
#include "linalg_typedefs.hpp" //LNA/src

#include "cl_FEM_Interpolation_Rule.hpp" //FEM/INT/src
#include "cl_FEM_Geometry_Interpolator.hpp" //FEM/INT/src
//#include "cl_FEM_Property.hpp" //FEM/INT/src
#include "cl_MSI_Dof_Type_Enums.hpp"     //FEM/MSI/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------
class Property;

        class Field_Interpolator
        {
            // how many fields are to be interpolated
            const uint mNumberOfFields;

            // pointer to space and time interpolation objects
            Interpolation_Function_Base * mSpaceInterpolation = nullptr;
            Interpolation_Function_Base * mTimeInterpolation  = nullptr;

            // space and time geometry interpolator
            const Geometry_Interpolator * mGeometryInterpolator = nullptr;

            // space, time, and space time number of bases
            uint mNSpaceBases;
            uint mNTimeBases;
            uint mNFieldBases;

            // space time number of coefficients
            uint mNFieldCoeff;

            // space, time dimensions
            uint mNSpaceDim;
            uint mNTimeDim;

            // space parametric dimensions
            uint mNSpaceParamDim;

            // parametric point where field is interpolatedctest
            Matrix< DDRMat > mXi;
            Matrix< DDRMat > mTau;

            // matrix of field coefficients uHat
            Matrix < DDRMat > mUHat;

            // field interpolator dof type
            MSI::Dof_Type mDofType;

            // field interpolator property
            const moris::fem::Property* mProperty = nullptr;

            // field interpolator property type
            fem::Property_Type mPropertyType;

            // storage
            Matrix< DDRMat > mN;

//------------------------------------------------------------------------------
        public:
//------------------------------------------------------------------------------
            /**
             * constructor
             * @param[ in ] aNumberOfFields           number of interpolated fields
             * @param[ in ] aFieldInterpolationRule   field interpolation rule
             * @param[ in ] aGeometryInterpolator     pointer to geometry interpolator object
             * @param[ in ] aDofType                  dof type for the interpolated fields
             *
             */
            Field_Interpolator( const uint                   & aNumberOfFields,
                                const Interpolation_Rule     & aFieldInterpolationRule,
                                const Geometry_Interpolator*   aGeometryInterpolator,
                                const MSI::Dof_Type            aDofType = MSI::Dof_Type::UNDEFINED );

            Field_Interpolator( const uint                   & aNumberOfFields,
                                const Interpolation_Rule     & aFieldInterpolationRule,
                                const Geometry_Interpolator*   aGeometryInterpolator,
                                const fem::Property*           aProperty,
                                const fem::Property_Type       aPropertyType = fem::Property_Type::UNDEFINED );
            /**
             * trivial constructor
             */
            Field_Interpolator( const uint & aNumberOfFields) : mNumberOfFields( aNumberOfFields ){};

//------------------------------------------------------------------------------
            /**
             * default constructor
             */
            ~Field_Interpolator();

//------------------------------------------------------------------------------
            /**
             * get dof type
             */
            MSI::Dof_Type get_dof_type() const
            {
                return mDofType;
            }

//------------------------------------------------------------------------------
            /**
             * get the number of fields
             */
            uint get_number_of_fields() const
            {
                return mNumberOfFields;
            }

//------------------------------------------------------------------------------
            /**
             * get the number of space bases
             */
            uint get_number_of_space_bases() const
            {
                return mNSpaceBases;
            }

//------------------------------------------------------------------------------
            /**
             * get the number of time bases
             */
            uint get_number_of_time_bases() const
            {
                return mNTimeBases;
            }

//------------------------------------------------------------------------------
             /**
              * get the number of space time bases
              */
             uint get_number_of_space_time_bases() const
             {
                 return mNFieldBases;
             }

//------------------------------------------------------------------------------
             /**
              * get the number of space time coefficients
              */
             uint get_number_of_space_time_coefficients() const
             {
                 return mNFieldCoeff;
             }
//------------------------------------------------------------------------------
            /**
             * set the parametric point where field is interpolated
             * @param[ in ] aParamPoint evaluation point in space and time
             */
            void set_space_time( const Matrix< DDRMat > & aParamPoint );

//------------------------------------------------------------------------------
             /**
              * get the parametric point in space where field is interpolated
              */
              Matrix< DDRMat > get_space() const
              {
                  return mXi;
              }

//------------------------------------------------------------------------------
            /**
             * get the parametric point in time where field is interpolated of tau
             */
            Matrix< DDRMat > get_time() const
            {
                return mTau;
            }

//------------------------------------------------------------------------------
             /**
              * set the coefficients of the field uHat
              * @param[ in ] aUHat coefficients
              */
             void set_coeff( const Matrix< DDRMat > & aUHat );

//------------------------------------------------------------------------------
             /**
             * get the coefficients of the field uHat
             * @param[ out ] mUHat coefficients
             */
              Matrix< DDRMat > get_coeff() const
              {
                  return mUHat;
              }

//------------------------------------------------------------------------------
            /**
             * evaluates the space time shape functions
             * @param[ out ] shape functions matrix
             *               ( 1 x <number of basis> )
             */
            Matrix < DDRMat > N();

            void eval_N();

//------------------------------------------------------------------------------
            /**
             * evaluates the first derivatives of the space time shape functions
             * wrt space x
             * @param[ out ] dNdx
             *               ( < number of space dimensions > x <number of space time basis > )
             */
             Matrix< DDRMat > Bx();

//------------------------------------------------------------------------------
            /**
             * evaluates the second derivatives of the space time shape functions
             * wrt space x
             * @param[ out ] d2Ndx2
             *               ( < 1D:1, 2D:3, 3D:6 > x <number of space time basis > )
             */
            Matrix< DDRMat > eval_d2Ndx2();

//------------------------------------------------------------------------------
            /**
             * evaluates the third derivatives of the space time shape functions
             * wrt space x
             * @param[ out ] d3Ndx3
             *               ( < 1D:1, 2D:4, 3D:10 > x <number of space time basis > )
             */
            Matrix< DDRMat > eval_d3Ndx3();

//------------------------------------------------------------------------------
            /**
             * evaluates the first derivative of the space time shape functions
             * wrt time t
             * @param[ out ] dNdt
             *               ( < number of time dimensions > x <number of space time basis > )
             */
            Matrix< DDRMat > Bt();

//------------------------------------------------------------------------------
            /**
            * evaluates the second derivative of the space time shape functions
            * wrt time t
            * @param[ out ] d2Ndt2
            *               ( < number of time dimensions > x <number of space time basis > )
            */
            Matrix< DDRMat > eval_d2Ndt2();

//------------------------------------------------------------------------------
            /**
            * evaluates the field at given space and time Xi, Tau
            * @param[ out ]          interpolated field
            */
            Matrix< DDRMat > val();

//------------------------------------------------------------------------------
            /**
            * evaluates the field space derivatives at given space and time evaluation point
            * @param[ in ]  aDerivativeOrder  order of the required derivatives
            * @param[ out ] gradx             space derivatives
            */
            Matrix< DDRMat > gradx( const uint & aDerivativeOrder );

//------------------------------------------------------------------------------
            /**
             * evaluates the field time derivative at given space and time evaluation point
             * @param[ in ] aDerivativeOrder  order of the required derivative
             * @param[ out ] gradt            time derivatives
             */
            Matrix< DDRMat > gradt( const uint & aDerivativeOrder );

//------------------------------------------------------------------------------
        };

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_FIELD_INTERPOLATOR_HPP_ */
