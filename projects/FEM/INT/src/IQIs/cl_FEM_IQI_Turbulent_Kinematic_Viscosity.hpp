/*
 * cl_FEM_IQI_Turbulent_Kinematic_Viscosity.hpp
 *
 *  Created on: Jul 20, 2020
 *      Author: noel
 */

#ifndef PROJECTS_FEM_INT_SRC_CL_FEM_IQI_TURBULENT_KINMATIC_VISCOSITY_HPP_
#define PROJECTS_FEM_INT_SRC_CL_FEM_IQI_TURBULENT_KINMATIC_VISCOSITY_HPP_

#include <map>

#include "typedefs.hpp"                     //MRS/COR/src
#include "cl_Cell.hpp"                      //MRS/CON/src

#include "cl_Matrix.hpp"                    //LINALG/src
#include "linalg_typedefs.hpp"              //LINALG/src

#include "cl_FEM_IQI.hpp"                   //FEM/INT/src

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        class IQI_Turbulent_Kinematic_Viscosity : public IQI
        {
            private:

                // viscosity dof type (default)
                MSI::Dof_Type mMasterDofViscosity = MSI::Dof_Type::VISCOSITY;

                // property type for IQI
                enum class Property_Type
                {
                        DYNAMIC_VISCOSITY, // fluid dynamic viscosity
                        DENSITY,           // fluid density
                        MAX_ENUM
                };

                // Spalart-Allmaras turbulence model constants
                real mCv1 = 7.1;

                //------------------------------------------------------------------------------

            public:
                //------------------------------------------------------------------------------
                /*
                 * constructor
                 */
                IQI_Turbulent_Kinematic_Viscosity();

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~IQI_Turbulent_Kinematic_Viscosity(){};

                //------------------------------------------------------------------------------
                /**
                 * set dof type list for IQI
                 * @param[ in ] aDofTypes   list of group of dof types
                 * @param[ in ] aDofStrings list of names for group of dof types
                 */
                void set_dof_type_list(
                        moris::Cell< moris::Cell< MSI::Dof_Type > > & aDofTypes,
                        moris::Cell< std::string >                  & aDofStrings,
                        mtk::Master_Slave                             aIsMaster = mtk::Master_Slave::MASTER );

            private:

                //------------------------------------------------------------------------------
                /**
                 * compute the quantity of interest
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_QI( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * Evaluate the quantity of interest and fill aQI with value
                 * @param[ in ] aQI IQI value at evaluation point
                 */
                void compute_QI( Matrix< DDRMat > & aQI );

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the quantity of interest wrt dof types
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_dQIdu( real aWStar )
                {
                    MORIS_ERROR( false, "IQI_Turbulent_Kinematic_Viscosity::compute_dQIdu - not implemented." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the quantity of interest wrt dof types
                 * @param[ in ] aDofType group of dof types wrt which derivatives are evaluated
                 * @param[ in ] adQIdu   derivative of quantity of interest matrix to fill
                 */
                void compute_dQIdu(
                        moris::Cell< MSI::Dof_Type > & aDofType,
                        Matrix< DDRMat >             & adQIdu )
                {
                    MORIS_ERROR( false, "IQI_Turbulent_Kinematic_Viscosity::compute_dQIdu() - not implemented for a drag/lift coefficient IQI.");
                }

                //------------------------------------------------------------------------------
                /**
                 * compute fv1 = chi³ / ( chi³ + cv1³)
                 * @param[ out ] fv1
                 */
                real compute_fv1();

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of fv1 wrt to a dof type
                 * @param[ in ] aDofTypes  a list of dof type wrt which
                 *                         the derivative is requested
                 * @param[ in ] adfv1du    a matrix to fill with dfv1du
                 */
                void compute_dfv1du(
                        const moris::Cell< MSI::Dof_Type > & aDofTypes,
                        Matrix< DDRMat >                   & adfv1du );

                //------------------------------------------------------------------------------
                /**
                 * compute chi = viscosityDof / viscosityProp
                 * @param[ out ] chi
                 */
                real compute_chi();

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of chi wrt to a dof type
                 * @param[ in ] aDofTypes  a list of dof type wrt which
                 *                         the derivative is requested
                 * @param[ in ] adchidu    a matrix to fill with dchidu
                 */
                void compute_dchidu(
                        const moris::Cell< MSI::Dof_Type > & aDofTypes,
                        Matrix< DDRMat >                   & adchidu );

        };
    }/* end namespace fem */
} /* end namespace moris */

#endif /* PROJECTS_FEM_INT_SRC_CL_FEM_IQI_TURBULENT_KINEMATIC_VISCOSITY_HPP_ */