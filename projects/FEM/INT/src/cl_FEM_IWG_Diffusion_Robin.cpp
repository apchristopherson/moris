
#include "cl_FEM_IWG_Diffusion_Robin.hpp"
#include "cl_FEM_Set.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

#include "fn_trans.hpp"
#include "fn_eye.hpp"
#include "fn_dot.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        IWG_Diffusion_Robin::IWG_Diffusion_Robin()
        {
            // set size for the property pointer cell
            mMasterProp.resize( static_cast< uint >( IWG_Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "HeatTransferCoefficient" ] = static_cast< uint >( IWG_Property_Type::HEAT_TRANSFER_COEFFICIENT );
            mPropertyMap[ "AmbientTemperature" ] = static_cast< uint >( IWG_Property_Type::AMBIENT_TEMP );
        }

        //------------------------------------------------------------------------------

        void IWG_Diffusion_Robin::compute_residual( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties, constitutive models
            this->check_field_interpolators();
#endif
            // get heat transfer property
            const std::shared_ptr< Property > & tPropHeatTransfer =
                     mMasterProp(static_cast< uint >( IWG_Property_Type::HEAT_TRANSFER_COEFFICIENT ));

            // get ambient temperature property
            const std::shared_ptr< Property > & tPropAmbientTemp =
                      mMasterProp(static_cast< uint >( IWG_Property_Type::AMBIENT_TEMP ));

            // get index for residual dof type, indices for assembly
            uint tDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tResStartIndex = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 );
            uint tResStopIndex  = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 );

            // get filed interpolator for residual dof type
            Field_Interpolator * tFI = mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // compute the residual
            // N * a * (T - T_ref)
            mSet->get_residual()( 0 )(
                    { tResStartIndex, tResStopIndex }) += aWStar * (
                            tPropHeatTransfer->val() *
                            ( tFI->val() - tPropAmbientTemp->val() ) * trans( tFI->N() ) );

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_residual()( 0 ) ),
                    "IWG_Diffusion_Robin::compute_residual - Residual contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------

        void IWG_Diffusion_Robin::compute_jacobian( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties, constitutive models
            this->check_field_interpolators();
#endif
            // get heat transfer property
            const std::shared_ptr< Property > & tPropHeatTransfer =
                     mMasterProp(static_cast< uint >( IWG_Property_Type::HEAT_TRANSFER_COEFFICIENT ));

            // get ambient temperature property
            const std::shared_ptr< Property > & tPropAmbientTemp =
                      mMasterProp(static_cast< uint >( IWG_Property_Type::AMBIENT_TEMP ));

            // get index for residual dof type, indices for assembly
            uint tDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tResStartIndex = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 );
            uint tResStopIndex  = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 );

            // get field interpolator for residual dof type
            Field_Interpolator * tFI = mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // compute the Jacobian for dof dependencies
            for( uint iDOF = 0; iDOF < mRequestedMasterGlobalDofTypes.size(); iDOF++ )
            {
                // get dof type
                const Cell< MSI::Dof_Type > & tDepDofType = mRequestedMasterGlobalDofTypes( iDOF );

                // get the dof type indices for assembly
                uint tDepDofIndex   = mSet->get_dof_index_for_type( tDepDofType( 0 ), mtk::Master_Slave::MASTER );
                uint tDepStartIndex = mSet->get_jac_dof_assembly_map()( tDofIndex )( tDepDofIndex, 0 );
                uint tDepStopIndex  = mSet->get_jac_dof_assembly_map()( tDofIndex )( tDepDofIndex, 1 );

                // get sub-matrix
                auto tJac = mSet->get_jacobian()(
                        { tResStartIndex, tResStopIndex },
                        { tDepStartIndex, tDepStopIndex } );

                // if dof type is residual dof type
                if( tDepDofType( 0 ) == mResidualDofType( 0 ) )
                {
                    tJac += aWStar *
                            tPropHeatTransfer->val() * trans( tFI->N() ) *  tFI->N();
                }

                // if dependency of heat transfer coefficient on dof type
                if ( tPropHeatTransfer->check_dof_dependency( tDepDofType ) )
                {
                    // add contribution to Jacobian
                    tJac += aWStar * (tFI->val() - tPropAmbientTemp->val() ) *
                                    trans( tFI->N() ) * tPropHeatTransfer->dPropdDOF( tDepDofType );
                }
            }

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_jacobian() ) ,
                    "IWG_Diffusion_Robin::compute_jacobian - Jacobian contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------

        void IWG_Diffusion_Robin::compute_jacobian_and_residual( real aWStar )
        {
            MORIS_ERROR( false, " IWG_Diffusion_Robin::compute_jacobian_and_residual - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Diffusion_Robin::compute_dRdp( real aWStar )
        {
            MORIS_ERROR( false, "IWG_Diffusion_Robin::compute_dRdp - Not implemented.");
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
