/*
 * cl_FEM_CM_Diffusion_Linear_Isotropic_Phase_Change.hpp
 *
 *  Created on: Apr 18, 2020
 *  Author: wunsch
 */


#ifndef SRC_FEM_CL_FEM_CM_DIFFUSION_LINEAR_ISOTROPIC_PHASE_CHANGE_HPP_
#define SRC_FEM_CL_FEM_CM_DIFFUSION_LINEAR_ISOTROPIC_PHASE_CHANGE_HPP_

#include <map>

#include "typedefs.hpp"                     //MRS/COR/src
#include "cl_Cell.hpp"                      //MRS/CON/src

#include "cl_Matrix.hpp"                    //LINALG/src
#include "linalg_typedefs.hpp"              //LINALG/src

#include "cl_FEM_Field_Interpolator.hpp"    //FEM/INT/src
#include "cl_FEM_Constitutive_Model.hpp"    //FEM/INT/src
#include "cl_FEM_CM_Diffusion_Linear_Isotropic.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        class CM_Diffusion_Linear_Isotropic_Phase_Change : public CM_Diffusion_Linear_Isotropic
        {

                //------------------------------------------------------------------------------
            protected:

                // nothing

            private:

                // default dof type for CM
                MSI::Dof_Type mTempDof = MSI::Dof_Type::TEMP;

                // property type for CM
                enum class Property_Type
                {
                    CONDUCTIVITY,
                    HEAT_CAPACITY,
                    DENSITY,
                    LATENT_HEAT,
                    PC_TEMP,
                    PHASE_STATE_FUNCTION,
                    PHASE_CHANGE_CONST,
                    MAX_ENUM
                };

                // Local string to property enum map
                std::map< std::string, CM_Diffusion_Linear_Isotropic_Phase_Change::Property_Type > mPropertyMap;

                //------------------------------------------------------------------------------
            public:
                /*
                 * trivial constructor
                 */
                CM_Diffusion_Linear_Isotropic_Phase_Change();

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~CM_Diffusion_Linear_Isotropic_Phase_Change(){};

                //------------------------------------------------------------------------------
                /**
                 * set constitutive model dof types
                 * @param[ in ] aDofTypes a list of group of dof types
                 * @param[ in ] aDofStrings a list of strings to describe the dof types
                 */
                void set_dof_type_list(
                        moris::Cell< moris::Cell< MSI::Dof_Type > > aDofTypes,
                        moris::Cell< std::string >                  aDofStrings );

                //------------------------------------------------------------------------------
                /**
                 * set constitutive model dv types
                 * @param[ in ] aDvTypes a list of group of dv types
                 * @param[ in ] aDvStrings a list of strings to describe the dv types
                 */
                void set_dv_type_list(
                        moris::Cell< moris::Cell< PDV_Type > > aDvTypes,
                        moris::Cell< std::string >             aDvStrings )
                {
                    Constitutive_Model::set_dv_type_list( aDvTypes );
                }

                //------------------------------------------------------------------------------
                /**
                 * set a property pointer
                 * @param[ in ] aProperty     a property pointer
                 * @param[ in ] aPropertyType a char
                 */
                void set_property(
                        std::shared_ptr<fem::Property > aProperty,
                        std::string                     aPropertyString );

                //------------------------------------------------------------------------------
                /**
                 * get a property pointer
                 * @param[ in ]  aPropertyType a string defining the property
                 * @param[ out ] aProperty     a property pointer
                 */
                std::shared_ptr< Property > get_property( std::string aPropertyString );

                //------------------------------------------------------------------------------
                /**
                 * evaluates the constitutive model change rate of enthalpy
                 */
                void eval_Hdot();

                //------------------------------------------------------------------------------
                /**
                 * evaluates the constitutive model change rate of spatial gradient of enthalpy (needed for GGLS-stabilization)
                 */
                void eval_gradH();

                //------------------------------------------------------------------------------
                /**
                 * evaluates the constitutive model change rate of spatial gradient of enthalpy (needed for GGLS-stabilization)
                 */
                void eval_gradHdot();

                //------------------------------------------------------------------------------
                /**
                 * evaluate the constitutive model enthalpy change rate wrt to a dof type
                 * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
                 * dHdotdDOF ( 1 x numDerDof )
                 */
                void eval_dHdotdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes );

                //------------------------------------------------------------------------------
                /**
                 * evaluate the constitutive model gradient of enthalpy wrt to a dof type
                 * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
                 * dGradHdDOF ( mSpaceDim x numDerDof )
                 */
                void eval_dGradHdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes );

                //------------------------------------------------------------------------------
                /**
                 * evaluate the constitutive model gradient of enthalpy change rate wrt to a dof type
                 * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
                 * dgradHdotdDOF ( mSpaceDim x numDerDof )
                 */
                void eval_dGradHdotdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes );

        };
        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_CM_DIFFUSION_LINEAR_ISOTROPIC_PHASE_CHANGE_HPP_ */