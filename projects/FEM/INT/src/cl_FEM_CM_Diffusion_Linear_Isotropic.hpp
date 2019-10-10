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
             * @param[ in ] aFlux a matrix to fill with evaluation
             */
            void eval_flux();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model strain
             */
            void eval_strain();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model matrix
             */
            void eval_const();

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model flux derivative wrt to a dof type
             * @param[ in ] aDofTypes  a dof type wrt which the derivative is evaluated
             */
            void eval_dFluxdDOF( moris::Cell< MSI::Dof_Type > aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model strain derivative wrt to a dof type
             * @param[ in ] aDofTypes    a dof type wrt which the derivative is evaluated
             */
            void eval_dStraindDOF( moris::Cell< MSI::Dof_Type > aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model matrix derivative wrt to a dof type
             * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
             */
            void eval_dConstdDOF( moris::Cell< MSI::Dof_Type > aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model flux derivative wrt to a dv type
             * @param[ in ] aDvTypes  a dv type wrt which the derivative is evaluated
             */
            void eval_dFluxdDV( moris::Cell< MSI::Dv_Type >   aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model strain derivative wrt to a dv type
             * @param[ in ] aDvTypes    a dv type wrt which the derivative is evaluated
             */
            void eval_dStraindDV( moris::Cell< MSI::Dv_Type >   aDofTypes );

//------------------------------------------------------------------------------
            /**
             * evaluate the constitutive model matrix derivative wrt to a dv type
             * @param[ in ] aDvTypes   a dv type wrt which the derivative is evaluated
             */
            void eval_dConstdDV( moris::Cell< MSI::Dv_Type >   aDvTypes );

//------------------------------------------------------------------------------
        };
//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_CM_DIFFUSION_LINEAR_ISOTROPIC_HPP_ */
