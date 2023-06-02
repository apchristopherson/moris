/*
 * Copyright (c) 2022 University of Colorado
 * Licensed under the MIT license. See LICENSE.txt file in the MORIS root for details.
 *
 *------------------------------------------------------------------------------------
 *
 * cl_FEM_CM_Struc_Nonlinear_Isotropic_Neo_Hookean.hpp
 *
 */

#ifndef SRC_FEM_CL_FEM_CM_STRUC_NONLINEAR_ISOTROPIC_NEO_HOOKEAN_HPP_
#define SRC_FEM_CL_FEM_CM_STRUC_NONLINEAR_ISOTROPIC_NEO_HOOKEAN_HPP_

#include <map>

#include "typedefs.hpp"                     //MRS/COR/src
#include "cl_Cell.hpp"                      //MRS/CNT/src

#include "cl_Matrix.hpp"                    //LINALG/src
#include "linalg_typedefs.hpp"              //LINALG/src

#include "cl_FEM_Field_Interpolator.hpp"    //FEM/INT/src
#include "cl_FEM_Constitutive_Model.hpp"    //FEM/INT/src
#include "cl_FEM_CM_Struc_Nonlinear_Isotropic.hpp"    //FEM/INT/src

namespace moris
{
    namespace fem
    {
        //--------------------------------------------------------------------------------------------------------------

        // Neo-Hookean constitutive model based on
        // [Bonet & Wood 2008 - Nonlinear Continuum Mechanics for Finite Element Analysis 2nd Edition]

        class CM_Struc_Nonlinear_Isotropic_Neo_Hookean : public CM_Struc_Nonlinear_Isotropic
        {
            protected:

            private:

                // default dof
                MSI::Dof_Type mDofDispl    = MSI::Dof_Type::UX;
                MSI::Dof_Type mDofTemp     = MSI::Dof_Type::UNDEFINED;
                MSI::Dof_Type mDofPressure = MSI::Dof_Type::UNDEFINED;

                // property type for CM
                enum class CM_Property_Type
                {
                        EMOD,
                        NU,
                        MAX_ENUM
                };

                // function pointers
                void ( CM_Struc_Nonlinear_Isotropic_Neo_Hookean:: * m_eval_d1PKStressdDOF )(
                        const Cell< MSI::Dof_Type > & aDofTypes ) = nullptr;

                void ( CM_Struc_Nonlinear_Isotropic_Neo_Hookean:: * m_eval_symbolic_traction_derivs )(
                        const Matrix< DDRMat > & aDisplGrad,
                        const real & aMu,
                        const real & aLam,
                        const Matrix< DDRMat > & aNorm ) = nullptr;

                void ( CM_Struc_Nonlinear_Isotropic_Neo_Hookean:: * m_proj_jump )(
                        const Matrix< DDRMat > & aVector,
                        Matrix< DDRMat >       & aProjMatrix ) = nullptr;

                void ( CM_Struc_Nonlinear_Isotropic_Neo_Hookean:: * mConstFunc )(
                        const real &,
                        const real &) = &CM_Struc_Nonlinear_Isotropic_Neo_Hookean::full_3d;

                // flag for the derivatives of the traction evaluation - based on a routine symbolically generated in MATLAB
                bool m2PKTraction_symEval = true;

                // storage for derivatives of traction  - based on a routine symbolically generated in MATLAB
                moris::Cell< Matrix< DDRMat > > m2PKTraction_sym;

                //--------------------------------------------------------------------------------------------------------------
            public:
                /*
                 * trivial constructor
                 */
                CM_Struc_Nonlinear_Isotropic_Neo_Hookean(
                        enum Constitutive_Type aConstType = Constitutive_Type::STRUC_NON_LIN_ISO_NEO_HOOKEAN );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~CM_Struc_Nonlinear_Isotropic_Neo_Hookean(){};

                //------------------------------------------------------------------------------
                /*
                 * @return constitutive_type
                 */
                Constitutive_Type
                get_constitutive_type() const
                {
                    return Constitutive_Type::STRUC_NON_LIN_ISO_NEO_HOOKEAN;
                }

                //------------------------------------------------------------------------------
                /**
                 * reset local evaluation flags
                 */
                void reset_eval_flags();

                //------------------------------------------------------------------------------
                /**
                 * create a global dof type list including local constitutive dependencies
                 */
                void build_global_dof_type_list();

                //------------------------------------------------------------------------------
                /**
                 * set local properties
                 */
                void set_local_properties();

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * set spatial dimensions. Modified from base to set function pointers to the appropriate eval_const()
                 * @param[ in ] aSpaceDim a spatial dimension
                 */
                void set_space_dim( uint aSpaceDim );

                //--------------------------------------------------------------------------------------------------------------
            private:

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the first Piola-Kirchhoff stress tensor based on the Neo-Hookean material model
                 */
                void eval_flux_first_piola_kirchhoff();

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the second Piola-Kirchhoff stress tensor based on the Neo-Hookeanmaterial model
                 * [Bonet & Wood 2008 ] eq. 6.28
                 * S = dW/dE = μ * ( I − C−1 ) + λ * ( ln J ) * C − 1
                 */
                void eval_flux_second_piola_kirchhoff();

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the second Piola-Kirchhoff stress tensor based on the Neo-Hookean material model
                 */
                void eval_flux_cauchy();

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the derivative of first Piola-Kirchhoff stress tensor wrt dof
                 * @param[ in ]  aDofTypes     a dof type wrt which the derivative is evaluated
                 */
                void eval_d1PKStressdDOF( const Cell< MSI::Dof_Type > & aDofTypes )
                {
                    ( this->*m_eval_d1PKStressdDOF )( aDofTypes );
                }

                void eval_d1PKStressdDOF_2d( const Cell< MSI::Dof_Type > & aDofTypes );
                void eval_d1PKStressdDOF_3d( const Cell< MSI::Dof_Type > & aDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the derivative of second Piola-Kirchhoff stress tensor wrt dof
                 * @param[ in ]  aDofTypes     a dof type wrt which the derivative is evaluated
                 */
                void eval_d2PKStressdDOF( const Cell< MSI::Dof_Type > & aDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the derivative of cauchy stress tensor wrt dof
                 * @param[ in ]  aDofTypes     a dof type wrt which the derivative is evaluated
                 */
                void eval_dCauchyStressdDOF( const Cell< MSI::Dof_Type > & aDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the constitutive model matrix
                 */
                void eval_const();

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the traction based on first Piola-Kirchhoff stress Tp = P N
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_traction_first_piola_kirchhoff( const Matrix< DDRMat > & aNormal );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the traction based on second Piola-Kirchhoff stress Ts = S N
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_traction_second_piola_kirchhoff( const Matrix< DDRMat > & aNormal );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the traction based on cauchy stress t = sigma n
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_traction_cauchy( const Matrix< DDRMat > & aNormal );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the derivative of traction based on the first Piola-Kirchhoff stress wrt. dof
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_dTractiondDOF_first_piola_kirchhoff(
                        const Matrix< DDRMat > & aNormal,
                        const Cell< MSI::Dof_Type > & aDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the derivative of traction based on the second Piola-Kirchhoff stress wrt. dof
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_dTractiondDOF_second_piola_kirchhoff(
                        const Matrix< DDRMat > & aNormal,
                        const Cell< MSI::Dof_Type > & aDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the derivative of traction based on the Cauchy stress wrt. dof
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_dTractiondDOF_cauchy(
                        const Matrix< DDRMat > & aNormal,
                        const Cell< MSI::Dof_Type > & aDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the test traction based on the first Piola-Kirchhoff stress
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_testTraction_first_piola_kirchhoff(
                        const Matrix< DDRMat >      & aNormal,
                        const Cell< MSI::Dof_Type > & aTestDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the test traction based on the second Piola-Kirchhoff stress
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_testTraction_second_piola_kirchhoff(
                        const Matrix< DDRMat >      & aNormal,
                        const Cell< MSI::Dof_Type > & aTestDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the test traction based on the Cauchy stress
                 * @param[ in ] aNormal normal in the reference configuration
                 */
                void eval_testTraction_cauchy(
                        const Matrix< DDRMat >      & aNormal,
                        const Cell< MSI::Dof_Type > & aTestDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the test traction derivative based on the first Piola-Kirchhoff stress
                 * wrt to a dof type
                 * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
                 * @param[ in ] aNormal   normal in the reference configuration
                 */
                void eval_dTestTractiondDOF_first_piola_kirchhoff(
                        const Cell< MSI::Dof_Type > & aDofTypes,
                        const Matrix< DDRMat >      & aNormal,
                        const Matrix< DDRMat >      & aJump,
                        const Cell< MSI::Dof_Type > & aTestDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the test traction derivative based on the second Piola-Kirchhoff stress
                 * wrt to a dof type
                 * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
                 * @param[ in ] aNormal   normal in the reference configuration
                 */
                void eval_dTestTractiondDOF_second_piola_kirchhoff(
                        const Cell< MSI::Dof_Type > & aDofTypes,
                        const Matrix< DDRMat >      & aNormal,
                        const Matrix< DDRMat >      & aJump,
                        const Cell< MSI::Dof_Type > & aTestDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the test traction derivative based on the cauchy stress
                 * wrt to a dof type
                 * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
                 * @param[ in ] aNormal   normal in the reference configuration
                 */
                void eval_dTestTractiondDOF_cauchy(
                        const Cell< MSI::Dof_Type > & aDofTypes,
                        const Matrix< DDRMat >      & aNormal,
                        const Matrix< DDRMat >      & aJump,
                        const Cell< MSI::Dof_Type > & aTestDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the constitutive model matrix derivative wrt to a dof type
                 * @param[ in ] aDofTypes   a dof type wrt which the derivative is evaluated
                 * @param[ in ] adConstdDOF a matrix to fill with derivative evaluation
                 */
                void eval_dConstdDOF( const Cell< MSI::Dof_Type > & aDofTypes );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * evaluate the constitutive model matrix derivative wrt to a dof type
                 * @param[ in ] aDofTypes   a dof type wrt which the derivative is evaluated
                 * @param[ in ] adConstdDOF a matrix to fill with derivative evaluation
                 */

                const Matrix< DDRMat > & symbolic_traction_derivs(
                        const Matrix< DDRMat > & aDisplGrad,
                        const real & aMu,
                        const real & aLam,
                        const Matrix< DDRMat > & aNorm,
                        uint number);

                void eval_symbolic_traction_derivs(
                        const Matrix< DDRMat > & aDisplGrad,
                        const real & aMu,
                        const real & aLam,
                        const Matrix< DDRMat > & aNorm)
                {
                    ( this->*m_eval_symbolic_traction_derivs )( aDisplGrad, aMu, aLam, aNorm );
                }

                void eval_symbolic_traction_derivs_2d(
                        const Matrix< DDRMat > & aDisplGrad,
                        const real & aMu,
                        const real & aLam,
                        const Matrix< DDRMat > & aNorm);

                void eval_symbolic_traction_derivs_3d(
                        const Matrix< DDRMat > & aDisplGrad,
                        const real & aMu,
                        const real & aLam,
                        const Matrix< DDRMat > & aNorm);

                //--------------------------------------------------------------------------------------------------------------
                /**
                 *  Sets the appropriate function pointers
                 *  based on the current member data
                 *  for spatial dimensions and model types
                 */
                void set_function_pointers();

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * Full plane stress tensor
                 * @param[ in ] aEmod Elastic modulus
                 * @param[ in ] aNu   Poisson ratio
                 */
                void full_plane_stress(
                        const real & aEmod,
                        const real & aNu );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * Deviatoric plane stress tensor
                 * @param aEmod Elastic modulus
                 * @param aNu   Poisson ratio
                 */
                void deviatoric_plane_stress(
                        const real & aEmod,
                        const real & aNu );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * Full plane strain tensor
                 * @param aEmod Elastic modulus
                 * @param aNu   Poisson ratio
                 */
                void full_plane_strain(
                        const real & aEmod,
                        const real & aNu  );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * Deviatoric plane strain tensor
                 * @param aEmod Elastic modulus
                 * @param aNu Poisson ratio
                 */
                void deviatoric_plane_strain(
                        const real & aEmod,
                        const real & aNu  );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * Full 3d nonlinear isotropic tensor
                 * @param aEmod Elastic modulus
                 * @param aNu Poisson ratio
                 */
                void full_3d(
                        const real & aEmod,
                        const real & aNu  );

                //--------------------------------------------------------------------------------------------------------------
                /**
                 * Deviatoric 3d tensor
                 * @param aEmod Elastic modulus
                 * @param aNu Poisson ratio
                 */
                void deviatoric_3d(
                        const real & aEmod,
                        const real & aNu  );

                // FIXME Nils provide headers!!!
                void proj_jump(
                        const Matrix< DDRMat > & aVector,
                        Matrix< DDRMat >       & aProjMatrix )
                {
                    ( this->*m_proj_jump )( aVector, aProjMatrix );
                }

                void proj_jump_2d(
                        const Matrix< DDRMat > & aVector,
                        Matrix< DDRMat >       & aProjMatrix );

                void proj_jump_3d(
                        const Matrix< DDRMat > & aVector,
                        Matrix< DDRMat >       & aProjMatrix );

        };
        //--------------------------------------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_CM_STRUC_NONLINEAR_ISOTROPIC_NEO_HOOKEAN_HPP_ */

