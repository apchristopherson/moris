
//FEM/INT/src
#include "cl_FEM_SP_Compressible_Velocity_Dirichlet_Nitsche.hpp"
#include "cl_FEM_Cluster.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------
        
        SP_Compressible_Velocity_Dirichlet_Nitsche::SP_Compressible_Velocity_Dirichlet_Nitsche()
        {
            // set the property pointer cell size
            mMasterProp.resize( static_cast< uint >( Property_Type::MAX_ENUM ), nullptr );

            // populate the map
            mPropertyMap[ "DynamicViscosity" ] = static_cast< uint >( Property_Type::VISCOSITY );
        }

        //------------------------------------------------------------------------------

        void SP_Compressible_Velocity_Dirichlet_Nitsche::reset_cluster_measures()
        {
            // evaluate element size from the cluster
            mElementSize = mCluster->compute_cluster_cell_length_measure(
                    mtk::Primary_Void::PRIMARY,
                    mtk::Master_Slave::MASTER );
        }

        //------------------------------------------------------------------------------

        void SP_Compressible_Velocity_Dirichlet_Nitsche::set_dof_type_list(
                moris::Cell< moris::Cell< MSI::Dof_Type > > & aDofTypes,
                moris::Cell< std::string >                  & aDofStrings,
                mtk::Master_Slave                             aIsMaster )
        {
            // switch on master slave
            switch ( aIsMaster )
            {
                case mtk::Master_Slave::MASTER :
                {
                    // set dof type list
                    mMasterDofTypes = aDofTypes;

                    // loop on dof type
                    for( uint iDof = 0; iDof < aDofTypes.size(); iDof++ )
                    {
                        // get dof string
                        std::string tDofString = aDofStrings( iDof );

                        // get dof type
                        MSI::Dof_Type tDofType = aDofTypes( iDof )( 0 );

                        // if velocity
                        if( tDofString == "Velocity" )
                        {
                            mMasterDofVelocity = tDofType;
                        }
                        else
                        {
                            // create error message
                            std::string tErrMsg =
                                    std::string( "SP_Velocity_Dirichlet_Nitsche::set_dof_type_list - Unknown aDofString : ") +
                                    tDofString;
                            MORIS_ERROR( false , tErrMsg.c_str() );
                        }
                    }
                    break;
                }

                case mtk::Master_Slave::SLAVE :
                {
                    // set dof type list
                    mSlaveDofTypes = aDofTypes;
                    break;
                }

                default:
                    MORIS_ERROR( false, "SP_Compressible_Velocity_Dirichlet_Nitsche::set_dof_type_list - unknown master slave type." );
            }
        }

        //------------------------------------------------------------------------------

        void SP_Compressible_Velocity_Dirichlet_Nitsche::eval_SP()
        {
            // get the viscosity and density property
            std::shared_ptr< Property > & tPropViscosity =
                    mMasterProp( static_cast< uint >( Property_Type::VISCOSITY ) );

            // compute stabilization parameter value
            mPPVal = mParameters( 0 ) * ( tPropViscosity->val()( 0 ) / mElementSize );
        }

        //------------------------------------------------------------------------------

        void SP_Compressible_Velocity_Dirichlet_Nitsche::eval_dSPdMasterDOF(
                const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type index
            uint tDofIndex = mMasterGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the dof type FI
            Field_Interpolator * tFIDerivative =
                    mMasterFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // set size for dSPdMasterDof
            mdPPdMasterDof( tDofIndex ).set_size( 1, tFIDerivative->get_number_of_space_time_coefficients(), 0.0 );

            // get the viscosity property
            std::shared_ptr< Property > & tPropViscosity =
                    mMasterProp( static_cast< uint >( Property_Type::VISCOSITY ) );

            // if viscosity depends on dof type
            if( tPropViscosity->check_dof_dependency( aDofTypes ) )
            {
                // compute contribution from viscosity
                mdPPdMasterDof( tDofIndex ) +=
                        mParameters( 0 )( 0 ) * tPropViscosity->dPropdDOF( aDofTypes ) / mElementSize;
            }
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

