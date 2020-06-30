/*
 * cl_FEM_IWG_Spalart_Allmaras_Turbulence_Interface.hpp
 *
 *  Created on: Jun 02, 2020
 *      Author: noel
 */

#ifndef SRC_FEM_CL_FEM_IWG_SPALART_ALLMARAS_TURBULENCE_INTERFACE_HPP_
#define SRC_FEM_CL_FEM_IWG_SPALART_ALLMARAS_TURBULENCE_INTERFACE_HPP_
//MRS/COR/src
#include <map>
#include "typedefs.hpp"
#include "cl_Cell.hpp"
//LINALG/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
//FEM/INT/src
#include "cl_FEM_IWG.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        class IWG_Spalart_Allmaras_Turbulence_Interface : public IWG
        {

                //------------------------------------------------------------------------------
            public:

                // local property enums
                enum class IWG_Property_Type
                {
                    VISCOSITY,
                    MAX_ENUM
                };

                // local string to property enum map
                std::map< std::string, IWG_Property_Type > mPropertyMap;

                // local stabilization parameter enums
                enum class IWG_Stabilization_Type
                {
                        NITSCHE_INTERFACE,
                        MASTER_WEIGHT_INTERFACE,
                        SLAVE_WEIGHT_INTERFACE,
                        MAX_ENUM
                };

                // local string to constitutive enum map
                std::map< std::string, IWG_Stabilization_Type > mStabilizationMap;

                // FIXME temp all the constants
                real mSigma = 2.0/3.0;

                //------------------------------------------------------------------------------
                /*
                 *  constructor
                 */
                IWG_Spalart_Allmaras_Turbulence_Interface();

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~IWG_Spalart_Allmaras_Turbulence_Interface(){};

                //------------------------------------------------------------------------------
                /**
                 * set property
                 * @param[ in ] aProperty       a property pointer
                 * @param[ in ] aPropertyString a string defining the property
                 * @param[ in ] aIsMaster       an enum for master or slave
                 */
                void set_property(
                        std::shared_ptr< Property > aProperty,
                        std::string                 aPropertyString,
                        mtk::Master_Slave           aIsMaster = mtk::Master_Slave::MASTER );

                //------------------------------------------------------------------------------
                /**
                 * set stabilization parameter
                 * @param[ in ] aStabilizationParameter a stabilization parameter pointer
                 * @param[ in ] aStabilizationString    a string defining the stabilization parameter
                 */
                void set_stabilization_parameter(
                        std::shared_ptr< Stabilization_Parameter > aStabilizationParameter,
                        std::string                                aStabilizationString );

                //------------------------------------------------------------------------------
                /**
                 * compute the residual
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_residual( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the jacobian
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_jacobian( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the residual and the jacobian
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_jacobian_and_residual( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the residual wrt design variables
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_dRdp( real aWStar );

            private:

                //------------------------------------------------------------------------------
                /**
                 * compute the traction = ( v + vtilde ) * grad vtilde
                 * @param[ in ] aTraction a matrix to fill with traction
                 */
                void compute_traction(
                        Matrix< DDRMat > & aTraction,
                        mtk::Master_Slave  aIsMaster = mtk::Master_Slave::MASTER );

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the traction = ( v + vtilde ) * grad vtilde
                 * wrt dof type aDofTypes
                 * @param[ in ] aDofTypes    group of dervative dof types
                 * @param[ in ] adtractiondu a matrix to fill with dtractiondu
                 */
                void compute_dtractiondu(
                        moris::Cell< MSI::Dof_Type > & aDofTypes,
                        Matrix< DDRMat >             & adtractiondu,
                        mtk::Master_Slave              aIsMaster = mtk::Master_Slave::MASTER );

                //------------------------------------------------------------------------------
                /**
                 * compute the test traction = delta ( ( v + vtilde ) * grad vtilde )
                 * @param[ in ] aTestDofTypes group of test dof types
                 * @param[ in ] aTestTraction a matrix to fill with test traction
                 */
                void compute_testtraction(
                        moris::Cell< MSI::Dof_Type> & aTestDofTypes,
                        Matrix< DDRMat >            & aTestTraction,
                        mtk::Master_Slave             aIsMaster = mtk::Master_Slave::MASTER );

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the test traction
                 * = delta ( ( v + vtilde ) * grad vtilde )
                 * @param[ in ] aTestDofTypes    group of test dof types
                 * @param[ in ] aDofTypes        group of derivative dof types
                 * @param[ in ] adtesttractiondu a matrix to fill with test traction
                 */
                void compute_dtesttractiondu(
                        moris::Cell< MSI::Dof_Type> & aTestDofTypes,
                        moris::Cell< MSI::Dof_Type> & aDofTypes,
                        Matrix< DDRMat >            & adtesttractiondu,
                        mtk::Master_Slave             aIsMaster = mtk::Master_Slave::MASTER );

        };
        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_IWG_SPALART_ALLMARAS_TURBULENCE_INTERFACE_HPP_ */