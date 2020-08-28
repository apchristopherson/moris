/*
 * cl_FEM_IWG_Compressible_NS_Mass_Flux_Neumann.cpp
 *
 *  Created on: Aug 26, 2020
 *      Author: wunsch
 */

#include "cl_FEM_IWG_Compressible_NS_Mass_Flux_Neumann.hpp"
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

        IWG_Compressible_NS_Mass_Flux_Neumann::IWG_Compressible_NS_Mass_Flux_Neumann()
        {
            // set size for the property pointer cell
            mMasterProp.resize( static_cast< uint >( IWG_Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "MassFlux" ] = IWG_Property_Type::MASS_FLUX;
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Mass_Flux_Neumann::set_property(
                std::shared_ptr< Property > aProperty,
                std::string                 aPropertyString,
                mtk::Master_Slave           aIsMaster )
        {
            // check that aPropertyString makes sense
            std::string tErrMsg =
                    std::string( "IWG_Compressible_NS_Mass_Flux_Neumann::set_property - Unknown aPropertyString: " ) +
                    aPropertyString;
            MORIS_ERROR( mPropertyMap.find( aPropertyString ) != mPropertyMap.end(), tErrMsg.c_str() );

            // check no slave allowed
            MORIS_ERROR( aIsMaster == mtk::Master_Slave::MASTER,
                    "IWG_Compressible_NS_Mass_Flux_Neumann::set_property - No slave allowed." );

            // set the property in the property cell
            this->get_properties( aIsMaster )( static_cast< uint >( mPropertyMap[ aPropertyString ] ) ) = aProperty;
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Mass_Flux_Neumann::compute_residual( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties, constitutive models
            this->check_field_interpolators();
#endif

            // get index for residual dof type, indices for assembly
            uint tDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tResStartIndex = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 );
            uint tResStopIndex  = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 );

            // get field interpolator for residual dof type (density)
            Field_Interpolator * tFIDensity =
                    mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get indices for SP, CM, properties
            uint tPropMassFluxIndex = static_cast< uint >( IWG_Property_Type::MASS_FLUX );

            // compute the residual
            mSet->get_residual()( 0 )(
                    { tResStartIndex, tResStopIndex },
                    { 0, 0 } ) += aWStar * ( trans( tFIDensity->N() ) * mMasterProp( tPropMassFluxIndex )->val() );

            // check for nan, infinity
            MORIS_ERROR( isfinite( mSet->get_residual()( 0 ) ),
                    "IWG_Compressible_NS_Mass_Flux_Neumann::compute_residual - Residual contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Mass_Flux_Neumann::compute_jacobian( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties, constitutive models
            this->check_field_interpolators();
#endif

            // get index for residual dof type, indices for assembly
            uint tDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tResStartIndex = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 0 );
            uint tResStopIndex  = mSet->get_res_dof_assembly_map()( tDofIndex )( 0, 1 );

            // get field interpolator for residual dof type (density)
            Field_Interpolator * tFIDensity =
                    mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get indices for SP, CM, properties
            uint tPropMassFluxIndex = static_cast< uint >( IWG_Property_Type::MASS_FLUX );

            // compute the jacobian for dof dependencies
            for( uint iDOF = 0; iDOF < mRequestedMasterGlobalDofTypes.size(); iDOF++ )
            {
                // get dof type
                Cell< MSI::Dof_Type > tDepDofType = mRequestedMasterGlobalDofTypes( iDOF );

                // get the dof type indices for assembly
                uint tDepDofIndex   = mSet->get_dof_index_for_type( tDepDofType( 0 ), mtk::Master_Slave::MASTER );
                uint tDepStartIndex = mSet->get_jac_dof_assembly_map()( tDofIndex )( tDepDofIndex, 0 );
                uint tDepStopIndex  = mSet->get_jac_dof_assembly_map()( tDofIndex )( tDepDofIndex, 1 );

                // if dependency in the dof type
                if ( mMasterProp( tPropMassFluxIndex )->check_dof_dependency( tDepDofType ) )
                {
                    // add contribution to jacobian
                    mSet->get_jacobian()(
                            { tResStartIndex, tResStopIndex },
                            { tDepStartIndex, tDepStopIndex } ) += aWStar * (
                                    trans( tFIDensity->N() ) * mMasterProp( tPropMassFluxIndex )->dPropdDOF( tDepDofType ) );
                }
            }

            // check for nan, infinity
            MORIS_ERROR(  isfinite( mSet->get_jacobian() ) ,
                    "IWG_Compressible_NS_Mass_Flux_Neumann::compute_jacobian - Jacobian contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Mass_Flux_Neumann::compute_jacobian_and_residual( real aWStar )
        {
            MORIS_ERROR( false, " IWG_Compressible_NS_Mass_Flux_Neumann::compute_jacobian_and_residual - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Mass_Flux_Neumann::compute_dRdp( real aWStar )
        {
            MORIS_ERROR( false, "IWG_Compressible_NS_Mass_Flux_Neumann::compute_dRdp - Not implemented.");
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
