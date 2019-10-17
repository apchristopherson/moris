/*
 * cl_FEM_CM_Diffusion_Linear_Isotropic.hpp
 *
 *  Created on: Sep 17, 2019
 *      Author: noel
 */

#ifndef SRC_FEM_CL_FEM_CM_DIFFUSION_LINEAR_ISOTROPIC_HPP_
#define SRC_FEM_CL_FEM_CM_DIFFUSION_LINEAR_ISOTROPIC_HPP_

#include "typedefs.hpp"                     //MRS/COR/src
#include "cl_Cell.hpp"                      //MRS/CON/src

#include "cl_Matrix.hpp"                    //LINALG/src
#include "linalg_typedefs.hpp"              //LINALG/src

#include "cl_FEM_Field_Interpolator.hpp"    //FEM/INT/src
#include "cl_FEM_Constitutive_Model.hpp"    //FEM/INT/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

        class CM_Diffusion_Linear_Isotropic : public Constitutive_Model
        {

//------------------------------------------------------------------------------
        public:
//------------------------------------------------------------------------------
            /*
             * trivial constructor
             */
            CM_Diffusion_Linear_Isotropic()
            {
                // set the constitutive type
                mConstitutiveType = fem::Constitutive_Type::DIFF_LIN_ISO;
            };

//------------------------------------------------------------------------------
            /**
             * trivial destructor
             */
            ~CM_Diffusion_Linear_Isotropic(){};

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model flux
             */
            void eval_flux();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model test flux
             * flux ( mSpaceDim x 1)
             */
            void eval_testFlux();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model traction
             * @param[ in ] aNormal normal
             * traction ( 1 x 1 )
             */
            void eval_traction( const Matrix< DDRMat > & aNormal );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model test traction
             * @param[ in ] aNormal normal
             * test traction ( numDof x 1 )
             */
            void eval_testTraction( const Matrix< DDRMat > & aNormal );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model strain
             * strain ( mSpaceDim x 1 )
             */
            void eval_strain();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model test strain
             * test strain ( mSpaceDim x numDof  )
             */
            void eval_testStrain();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model matrix
             * constitutive matrix ( mSpaceDim x mSpaceDim )
             */
            void eval_const();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model flux derivative wrt to a dof type
             * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
             * dFluxdDOF ( mSpaceDim x numDerDof )
             */
            void eval_dFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model traction derivative wrt to a dof type
             * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
             * @param[ in ] aNormal   normal
             * dTractiondDOF ( 1 x numDerDof )
             */
            void eval_dTractiondDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes,
                                     const Matrix< DDRMat >             & aNormal );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model test traction derivative wrt to a dof type
             * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
             * @param[ in ] aNormal   normal
             * dTestTractiondDOF ( numDof x numDerDof )
             */
            void eval_dTestTractiondDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes,
                                         const Matrix< DDRMat >             & aNormal );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model strain derivative wrt to a dof type
             * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
             * dStraindDOF ( mSpaceDim x numDerDof )
             */
            void eval_dStraindDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model matrix derivative wrt to a dof type
             * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
             * dConstdDOF ( 1 x numDerDof )
             */
            void eval_dConstdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model flux derivative wrt to a dv type
             * @param[ in ] aDvTypes a dv type wrt which the derivative is evaluated
             * dFluxdDV ( mSpaceDim x numDerDv )
             */
            void eval_dFluxdDV( const moris::Cell< MSI::Dv_Type > & aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model strain derivative wrt to a dv type
             * @param[ in ] aDvTypes a dv type wrt which the derivative is evaluated
             * dStraindDV ( mSpaceDim x numDerDV )
             */
            void eval_dStraindDV( const moris::Cell< MSI::Dv_Type > & aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model matrix derivative wrt to a dv type
             * @param[ in ] aDvTypes   a dv type wrt which the derivative is evaluated
             * dConstdDV ( 1 x numDerDv )
             */
            void eval_dConstdDV( const moris::Cell< MSI::Dv_Type > & aDvTypes );

//------------------------------------------------------------------------------
        };
//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_CM_DIFFUSION_LINEAR_ISOTROPIC_HPP_ */