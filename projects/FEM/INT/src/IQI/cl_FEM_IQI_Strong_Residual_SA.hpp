/*
 * cl_FEM_IQI_STRONG_RESIDUAL_SA.hpp
 *
 *  Created on: Feb 17, 2022
 *      Author: noel
 */

#ifndef PROJECTS_FEM_INT_SRC_CL_FEM_IQI_STRONG_RESIDUAL_SA_HPP_
#define PROJECTS_FEM_INT_SRC_CL_FEM_IQI_STRONG_RESIDUAL_SA_HPP_

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

        class IQI_Strong_Residual_SA : public IQI
        {
                //------------------------------------------------------------------------------

                // local constitutive enums
                enum class IQI_Constitutive_Type
                {
                        TURBULENCE,
                        MAX_ENUM
                };

                real mCb2 = 0.6220;
                real mSigma = 2.0/3.0;

            public:
                //------------------------------------------------------------------------------
                /*
                 * constructor
                 */
                IQI_Strong_Residual_SA();

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~IQI_Strong_Residual_SA(){};

                //------------------------------------------------------------------------------

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
                void compute_dQIdu( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the quantity of interest wrt dof types
                 * @param[ in ] aDofType group of dof types wrt which derivatives are evaluated
                 * @param[ in ] adQIdu   derivative of quantity of interest matrix to fill
                 */
                void compute_dQIdu(
                        moris::Cell< MSI::Dof_Type > & aDofType,
                        Matrix< DDRMat >             & adQIdu );

                //------------------------------------------------------------------------------
        };
    }/* end namespace fem */
} /* end namespace moris */

#endif /* PROJECTS_FEM_INT_SRC_CL_FEM_IQI_STRONG_RESIDUAL_SA_HPP_ */
