
#include "cl_FEM_IWG_Isotropic_Struc_Linear_Neumann.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_Set.hpp"

#include "fn_trans.hpp"
#include "fn_eye.hpp"
#include "fn_dot.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        IWG_Isotropic_Struc_Linear_Neumann::IWG_Isotropic_Struc_Linear_Neumann()
        {
            // set size for the property pointer cell
            mMasterProp.resize( static_cast< uint >( IWG_Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "Traction" ] = static_cast< uint >( IWG_Property_Type::TRACTION );
            mPropertyMap[ "Pressure" ] = static_cast< uint >( IWG_Property_Type::PRESSURE );
        }

        //------------------------------------------------------------------------------

        void IWG_Isotropic_Struc_Linear_Neumann::compute_residual( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties and constitutive models
            this->check_field_interpolators();
#endif

            // get master index for residual dof type (here displacement), indices for assembly
            uint tMasterDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterResStartIndex = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 0 );
            uint tMasterResStopIndex  = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 1 );

            // get field interpolator for the residual dof type
            Field_Interpolator * tFI =
                    mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get traction load property
            const std::shared_ptr< Property > & tPropTraction =
                    mMasterProp( static_cast< uint >( IWG_Property_Type::TRACTION ) );

            // get traction load property
            const std::shared_ptr< Property > & tPropPressure =
                    mMasterProp( static_cast< uint >( IWG_Property_Type::PRESSURE ) );

            // get sub-vector
            auto tRes =  mSet->get_residual()( 0 )(
                    { tMasterResStartIndex, tMasterResStopIndex } );

            // compute the residual
            if (tPropTraction != nullptr)
            {
                tRes -= aWStar * (
                        trans( tFI->N() ) * tPropTraction->val() );
            }

            if (tPropPressure != nullptr)
            {
                tRes -= aWStar * (
                                trans( tFI->N() ) * mNormal *  tPropPressure->val());
            }

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_residual()( 0 ) ),
                    "IWG_Isotropic_Struc_Linear_Neumann::compute_residual - Residual contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------
        void IWG_Isotropic_Struc_Linear_Neumann::compute_jacobian( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties and constitutive models
            this->check_field_interpolators();
#endif

            // get master index for residual dof type (here displacement), indices for assembly
            uint tMasterDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterResStartIndex = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 0 );
            uint tMasterResStopIndex  = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 1 );

            // get field interpolator for the residual dof type
            Field_Interpolator * tFI =
                    mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get traction load property
            const std::shared_ptr< Property > & tPropTraction =
                    mMasterProp( static_cast< uint >( IWG_Property_Type::TRACTION ) );

            // get traction load property
            const std::shared_ptr< Property > & tPropPressure =
                    mMasterProp( static_cast< uint >( IWG_Property_Type::PRESSURE ) );

            // compute the Jacobian for indirect IWG dof dependencies through properties
            for( uint iDOF = 0; iDOF < mRequestedMasterGlobalDofTypes.size(); iDOF++ )
            {
                // get dof type
                 const Cell< MSI::Dof_Type > & tDofType = mRequestedMasterGlobalDofTypes( iDOF );

                // get the index for the dof type
                sint tDofDepIndex         = mSet->get_dof_index_for_type( tDofType( 0 ), mtk::Master_Slave::MASTER );
                uint tMasterDepStartIndex = mSet->get_jac_dof_assembly_map()( tMasterDofIndex )( tDofDepIndex, 0 );
                uint tMasterDepStopIndex  = mSet->get_jac_dof_assembly_map()( tMasterDofIndex )( tDofDepIndex, 1 );

                auto tJac = mSet->get_jacobian()(
                        { tMasterResStartIndex, tMasterResStopIndex },
                        { tMasterDepStartIndex, tMasterDepStopIndex } );

                // if traction load depends on the dof type
                if ( tPropTraction != nullptr )
                {
                    if ( tPropTraction->check_dof_dependency( tDofType ) )
                    {
                        // add contribution to Jacobian
                        tJac -= aWStar * (
                                trans( tFI->N() ) * tPropTraction->dPropdDOF( tDofType ) );
                    }
                }

                // if pressure depends on the dof type
                if ( tPropPressure != nullptr )
                {
                    if ( tPropPressure->check_dof_dependency( tDofType ) )
                    {
                        // add contribution to Jacobian
                        tJac -= aWStar * (
                                trans( tFI->N() ) * mNormal * tPropPressure->dPropdDOF( tDofType ) );
                    }
                }
            }

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_jacobian() ) ,
                    "IWG_Isotropic_Struc_Linear_Neumann::compute_jacobian - Jacobian contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------

        void IWG_Isotropic_Struc_Linear_Neumann::compute_jacobian_and_residual( real aWStar )
        {
            MORIS_ERROR( false, "IWG_Isotropic_Struc_Linear_Neumann::compute_jacobian_and_residual - Not implemented.");
        }

        //------------------------------------------------------------------------------

        void IWG_Isotropic_Struc_Linear_Neumann::compute_dRdp( real aWStar )
        {
            MORIS_ERROR( false, "IWG_Isotropic_Struc_Linear_Neumann::compute_dRdp - Not implemented.");
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */