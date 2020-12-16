/*
 * cl_FEM_SP_Compressible_Velocity_Dirichlet_Nitsche.hpp
 *
 *  Created on: Dec 15, 2020
 *  Author: wunsch
 */

#ifndef SRC_FEM_CL_FEM_SP_COMPRESSIBLE_VELOCITY_DIRICHLET_NITSCHE_HPP_
#define SRC_FEM_CL_FEM_SP_COMPRESSIBLE_VELOCITY_DIRICHLET_NITSCHE_HPP_

#include <map>
//MRS/CON/src
#include "typedefs.hpp"
#include "cl_Cell.hpp"
//LINALG/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
//FEM/INT/src
#include "cl_FEM_Field_Interpolator.hpp"
#include "cl_FEM_Constitutive_Model.hpp"
#include "cl_FEM_Stabilization_Parameter.hpp"
#include "cl_FEM_Cluster.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------
        /*
         * Stabilization parameter for Dirichlet BC on velocity for fluid problem
         * applied with Nitsche's formulation
         * gamma_N = alpha_N * ( viscosity / h + density * norm_inf( u ) / 6.0 )
         * from Schott et al. (2015)
         */
        class SP_Compressible_Velocity_Dirichlet_Nitsche : public Stabilization_Parameter
        {

                //------------------------------------------------------------------------------
            private:

                // populate the dof map (default)
                MSI::Dof_Type mMasterDofVelocity = MSI::Dof_Type::VX;

                // element size
                real mElementSize = 1.0;

                // property type for the SP
                enum class Property_Type
                {
                    VISCOSITY,  // fluid dynamic viscosity
                    MAX_ENUM
                };

                /*
                 * Rem: mParameters( 0 ) - alpha_N
                 */

            public:
                //------------------------------------------------------------------------------
                /*
                 * constructor
                 */
                SP_Compressible_Velocity_Dirichlet_Nitsche();

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~SP_Compressible_Velocity_Dirichlet_Nitsche(){};

                //------------------------------------------------------------------------------
                /**
                 * reset the cluster measures required for this SP
                 */
                void reset_cluster_measures();

                //------------------------------------------------------------------------------
                /**
                 * set dof types
                 * @param[ in ] aDofTypes a cell of cell of dof types
                 * @param[ in ] aDofStrings list of strings describing the dof types
                 * @param[ in ] aIsMaster enum for master or slave
                 */
                void set_dof_type_list(
                        moris::Cell< moris::Cell< MSI::Dof_Type > > & aDofTypes,
                        moris::Cell< std::string >                  & aDofStrings,
                        mtk::Master_Slave                             aIsMaster = mtk::Master_Slave::MASTER );

                //------------------------------------------------------------------------------
                /**
                 * set dv types
                 * @param[ in ] aDvTypes   a cell of group of dv types
                 * @param[ in ] aDvStrings list of strings describing the dv types
                 * @param[ in ] aIsMaster enum for master or slave
                 */
                void set_dv_type_list(
                        moris::Cell< moris::Cell< PDV_Type > > & aDvTypes,
                        moris::Cell< std::string >             & aDvStrings,
                        mtk::Master_Slave                        aIsMaster = mtk::Master_Slave::MASTER )
                {
                    Stabilization_Parameter::set_dv_type_list( aDvTypes, aIsMaster );
                }

                //------------------------------------------------------------------------------
                /**
                 * evaluate the stabilization parameter value
                 */
                void eval_SP();

                //------------------------------------------------------------------------------
                /**
                 * evaluate the stabilization parameter derivative wrt to a master dof type
                 * @param[ in ] aDofTypes a dof type wrt which the derivative is evaluated
                 */
                void eval_dSPdMasterDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes );

                //------------------------------------------------------------------------------
                /**
                 * evaluate the penalty parameter derivative wrt to a master dv type
                 * @param[ in ] aDvTypes a dv type wrt which the derivative is evaluated
                 */
                void eval_dSPdMasterDV( const moris::Cell< PDV_Type > & aDvTypes )
                {
                    MORIS_ERROR( false, "SP_Compressible_Velocity_Dirichlet_Nitsche::eval_dSPdMasterDV - not implemented." );
                }

                //------------------------------------------------------------------------------
        };
        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_SP_COMPRESSIBLE_VELOCITY_DIRICHLET_NITSCHE_HPP_ */
