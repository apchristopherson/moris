
#include "cl_FEM_IWG_Isotropic_Spatial_Diffusion_Neumann.hpp"
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
        IWG_Isotropic_Spatial_Diffusion_Neumann::IWG_Isotropic_Spatial_Diffusion_Neumann()
        {
            // set size for the property pointer cell
            mMasterProp.resize( static_cast< uint >( IWG_Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "Neumann" ] = IWG_Property_Type::NEUMANN;
        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Neumann::compute_residual( real tWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties, constitutive models
            this->check_field_interpolators();
#endif

            // get index for residual dof type, indices for assembly
            uint tDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tResStartIndex = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 );
            uint tResStopIndex  = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 );

            // get filed interpolator for residual dof type
            Field_Interpolator * tFI = mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get indices for SP, CM, properties
            uint tNeumannIndex = static_cast< uint >( IWG_Property_Type::NEUMANN );

            // compute the residual
            mSet->get_residual()( { tResStartIndex, tResStopIndex }, { 0, 0 } )
            += - trans( tFI->N() ) * mMasterProp( tNeumannIndex )->val() * tWStar;
        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Neumann::compute_jacobian( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties, constitutive models
            this->check_field_interpolators();
#endif

            // get index for residual dof type, indices for assembly
            uint tDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tResStartIndex = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 );
            uint tResStopIndex  = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 );

            // get field interpolator for residual dof type
            Field_Interpolator * tFI = mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get indices for SP, CM, properties
            uint tNeumannIndex = static_cast< uint >( IWG_Property_Type::NEUMANN );

            // compute the jacobian for direct IWG dof dependencies
            // None

            // compute the jacobian for indirect IWG dof dependencies through properties
            for( uint iDOF = 0; iDOF < mRequestedMasterGlobalDofTypes.size(); iDOF++ )
            {
                // get dof type
                Cell< MSI::Dof_Type > tDepDofType = mRequestedMasterGlobalDofTypes( iDOF );

                // get the dof type indices for assembly
                uint tDepDofIndex   = mSet->get_dof_index_for_type( tDepDofType( 0 ), mtk::Master_Slave::MASTER );
                uint tDepStartIndex = mSet->get_jac_dof_assembly_map()( tDofIndex )( tDepDofIndex, 0 );
                uint tDepStopIndex  = mSet->get_jac_dof_assembly_map()( tDofIndex )( tDepDofIndex, 1 );

                // if dependency in the dof type
                if ( mMasterProp( tNeumannIndex )->check_dof_dependency( tDepDofType ) )
                {
                    // add contribution to jacobian
                    mSet->get_jacobian()( { tResStartIndex, tResStopIndex }, { tDepStartIndex, tDepStopIndex } )
                    += - trans( tFI->N() ) * mMasterProp( tNeumannIndex )->dPropdDOF( tDepDofType ) * aWStar;
                }
            }
        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Neumann::compute_jacobian_and_residual( real aWStar )
        {
            MORIS_ERROR( false, " IWG_Isotropic_Spatial_Diffusion_Neumann::compute_jacobian_and_residual - Not implemented." );
        }

//------------------------------------------------------------------------------
        void IWG_Isotropic_Spatial_Diffusion_Neumann::compute_drdpdv( real aWStar )
        {
            MORIS_ERROR( false, "IWG_Isotropic_Spatial_Diffusion_Neumann::compute_drdpdv - Not implemented.");
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
