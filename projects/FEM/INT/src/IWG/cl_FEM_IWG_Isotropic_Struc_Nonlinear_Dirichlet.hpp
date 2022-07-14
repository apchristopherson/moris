/*
 * cl_FEM_IWG_Isotropic_Struc_Nonlinear_Dirichlet.hpp
 *
 *  Created on: May 13, 2022
 *      Author: hermann
 */

#ifndef SRC_FEM_CL_FEM_IWG_ISOTROPIC_STRUC_NONLINEAR_DIRICHLET_HPP_
#define SRC_FEM_CL_FEM_IWG_ISOTROPIC_STRUC_NONLINEAR_DIRICHLET_HPP_

#include <map>

#include "typedefs.hpp"                     //MRS/COR/src
#include "cl_Cell.hpp"                      //MRS/CON/src

#include "cl_Matrix.hpp"                    //LINALG/src
#include "linalg_typedefs.hpp"              //LINALG/src

#include "cl_FEM_IWG.hpp"                   //FEM/INT/src

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        class IWG_Isotropic_Struc_Nonlinear_Dirichlet : public IWG
        {

                //------------------------------------------------------------------------------
            public:

                // sign for symmetric/unsymmetric Nitsche
                sint mBeta = 1;

                // stress and strain type to evaluate the IWG
                enum CM_Function_Type mStressType;
                enum CM_Function_Type mStrainType;

                enum class IWG_Property_Type
                {
                    DIRICHLET,
                    SELECT,
                    THICKNESS,
                    MAX_ENUM
                };

                enum class IWG_Constitutive_Type
                {
                    ELAST_LIN_ISO,
                    MAX_ENUM
                };

                enum class IWG_Stabilization_Type
                {
                    DIRICHLET_NITSCHE,
                    MAX_ENUM
                };

                //------------------------------------------------------------------------------
                /*
                 * constructor
                 */
                IWG_Isotropic_Struc_Nonlinear_Dirichlet(
                		enum CM_Function_Type aStressType,
        				enum CM_Function_Type aStrainType,
        				sint aBeta );

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~IWG_Isotropic_Struc_Nonlinear_Dirichlet(){};

                //------------------------------------------------------------------------------
                /**
                 * computes the residual
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_residual( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * computes the jacobian
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_jacobian( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * computes the residual and the jacobian
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_jacobian_and_residual( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the residual wrt design variables
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_dRdp( real aWStar );

                //------------------------------------------------------------------------------
        };
        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_IWG_ISOTROPIC_STRUC_NONLINEAR_DIRICHLET_HPP_ */