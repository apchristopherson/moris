
#include "cl_FEM_IWG_Ghost_Normal_Field.hpp"
#include "cl_FEM_Set.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

#include "fn_trans.hpp"
#include "fn_norm.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        IWG_Ghost_Normal_Field::IWG_Ghost_Normal_Field()
        {
            // set size for the stabilization parameter pointer cell
            mStabilizationParam.resize( static_cast< uint >( IWG_Stabilization_Type::MAX_ENUM ), nullptr );

            // populate the stabilization map
            mStabilizationMap[ "GhostSP" ] = IWG_Stabilization_Type::GHOST_SP;
        }

        //------------------------------------------------------------------------------

        void IWG_Ghost_Normal_Field::set_stabilization_parameter(
                std::shared_ptr< Stabilization_Parameter > aStabilizationParameter,
                std::string                                aStabilizationString )
        {
            // check that aStabilizationString makes sense
            std::string tErrMsg =
                    std::string( "IWG_Ghost_Normal_Field::set_stabilization_parameter - Unknown aStabilizationString: " ) +
                    aStabilizationString;
            MORIS_ERROR( mStabilizationMap.find( aStabilizationString ) != mStabilizationMap.end(), tErrMsg.c_str() );

            // set the stabilization parameter in the stabilization parameter cell
            this->get_stabilization_parameters()( static_cast< uint >( mStabilizationMap[ aStabilizationString ] ) ) = aStabilizationParameter;
        }

        //------------------------------------------------------------------------------

        void IWG_Ghost_Normal_Field::compute_residual( real aWStar )
        {
#ifdef DEBUG
            // check master and slave field interpolators
            this->check_field_interpolators( mtk::Master_Slave::MASTER );
            this->check_field_interpolators( mtk::Master_Slave::SLAVE );
#endif

            // set interpolation order
            IWG::set_interpolation_order();

            // get master index for residual dof type, indices for assembly
            uint tMasterDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterResStartIndex = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 0 );
            uint tMasterResStopIndex  = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 1 );

            // get slave index for residual dof type, indices for assembly
            uint tSlaveDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::SLAVE );
            uint tSlaveResStartIndex = mSet->get_res_dof_assembly_map()( tSlaveDofIndex )( 0, 0 );
            uint tSlaveResStopIndex  = mSet->get_res_dof_assembly_map()( tSlaveDofIndex )( 0, 1 );

            // get the master field interpolator for residual dof type
            Field_Interpolator * tFIMaster =
                    mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get the slave field interpolator for residual dof type
            Field_Interpolator * tFISlave  =
                    mSlaveFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get the stabilization parameter
            std::shared_ptr< Stabilization_Parameter > tSP =
                    mStabilizationParam( static_cast< uint >( IWG_Stabilization_Type::GHOST_SP ) );

            // loop over the interpolation order
            for ( uint iOrder = 1; iOrder <= mOrder; iOrder++ )
            {
                // set the order for stabilization parameters
                tSP->set_interpolation_order( iOrder );

                // get normal matrix
                Matrix< DDRMat > tFlatNormal;
                this->get_flat_normal_matrix( tFlatNormal, iOrder );

                // premultiply common terms
                Matrix< DDRMat > tPreMultiply = tSP->val()( 0 ) * tFlatNormal * ( tFIMaster->gradx( iOrder ) - tFISlave->gradx( iOrder ) ) ;
                tPreMultiply = reshape( tPreMultiply , tPreMultiply.numel(), 1 );

                // get flattened directional derivatives for master and slave
                Matrix< DDRMat > tMasterdNdxFlat;
                this->compute_flat_dnNdxn( tMasterdNdxFlat, iOrder, mtk::Master_Slave::MASTER );
                Matrix< DDRMat > tSlavedNdxFlat;
                this->compute_flat_dnNdxn( tSlavedNdxFlat, iOrder, mtk::Master_Slave::SLAVE );

                // compute master residual
                mSet->get_residual()( 0 )(
                        { tMasterResStartIndex, tMasterResStopIndex },
                        { 0, 0 } ) += aWStar * ( tMasterdNdxFlat * tPreMultiply );

                // compute slave residual
                mSet->get_residual()( 0 )(
                        { tSlaveResStartIndex, tSlaveResStopIndex },
                        { 0, 0 } ) -= aWStar * ( tSlavedNdxFlat * tPreMultiply );
            }
        }

        //------------------------------------------------------------------------------

        void IWG_Ghost_Normal_Field::compute_jacobian( real aWStar )
        {
#ifdef DEBUG
            // check master and slave field interpolators
            this->check_field_interpolators( mtk::Master_Slave::MASTER );
            this->check_field_interpolators( mtk::Master_Slave::SLAVE );
#endif

            // get master index for residual dof type, indices for assembly
            uint tMasterDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterResStartIndex = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 0 );
            uint tMasterResStopIndex  = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 1 );

            // get slave index for residual dof type, indices for assembly
            uint tSlaveDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::SLAVE );
            uint tSlaveResStartIndex = mSet->get_res_dof_assembly_map()( tSlaveDofIndex )( 0, 0 );
            uint tSlaveResStopIndex  = mSet->get_res_dof_assembly_map()( tSlaveDofIndex )( 0, 1 );

            // set interpolation order
            IWG::set_interpolation_order();

            // get the master field interpolator for residual dof type
            Field_Interpolator * tFIMaster =
                    mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get the slave field interpolator for residual dof type
            Field_Interpolator * tFISlave  =
                    mSlaveFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get the stabilization parameter
            std::shared_ptr< Stabilization_Parameter > tSP =
                    mStabilizationParam( static_cast< uint >( IWG_Stabilization_Type::GHOST_SP ) );

            // get number of master and slave dependencies
            uint tMasterNumDofDependencies = mRequestedMasterGlobalDofTypes.size();
            uint tSlaveNumDofDependencies  = mRequestedSlaveGlobalDofTypes.size();

            // loop over the interpolation orders
            for ( uint iOrder = 1; iOrder <= mOrder; iOrder++ )
            {
                // set the order for the stabilization parameter
                tSP->set_interpolation_order( iOrder );

                // get flattened normal
                Matrix< DDRMat > tFlatNormal;
                this->get_flat_normal_matrix( tFlatNormal, iOrder );

                // get flattened directional derivatives for master and slave
                Matrix< DDRMat > tMasterdNdxFlat;
                this->compute_flat_dnNdxn( tMasterdNdxFlat, iOrder, mtk::Master_Slave::MASTER );
                Matrix< DDRMat > tSlavedNdxFlat;
                this->compute_flat_dnNdxn( tSlavedNdxFlat, iOrder, mtk::Master_Slave::SLAVE );

                // compute the jacobian for indirect dof dependencies through master
                for( uint iDOF = 0; iDOF < tMasterNumDofDependencies; iDOF++ )
                {
                    // get the dof type
                    Cell< MSI::Dof_Type > tDofType = mRequestedMasterGlobalDofTypes( iDOF );

                    // get the index for the dof type
                    sint tDofDepIndex         = mSet->get_dof_index_for_type( tDofType( 0 ), mtk::Master_Slave::MASTER );
                    uint tMasterDepStartIndex = mSet->get_jac_dof_assembly_map()( tMasterDofIndex )( tDofDepIndex, 0 );
                    uint tMasterDepStopIndex  = mSet->get_jac_dof_assembly_map()( tMasterDofIndex )( tDofDepIndex, 1 );

                    // compute jacobian direct dependencies
                    if ( tDofType( 0 ) == mResidualDofType( 0 ) )
                    {
                        // dRM/dM
                        mSet->get_jacobian()(
                                { tMasterResStartIndex, tMasterResStopIndex },
                                { tMasterDepStartIndex, tMasterDepStopIndex } ) += aWStar * (
                                        tMasterdNdxFlat * tSP->val()( 0 ) * trans( tMasterdNdxFlat ) );

                        // dRS/dM
                        mSet->get_jacobian()(
                                { tSlaveResStartIndex,  tSlaveResStopIndex },
                                { tMasterDepStartIndex, tMasterDepStopIndex } ) -= aWStar * (
                                        tSlavedNdxFlat * tSP->val()( 0 ) * trans( tMasterdNdxFlat ) );
                    }

                    // if stabilization parameter dependency on the dof type
                    if ( tSP->check_dof_dependency( tDofType ) )
                    {
                        // premultiply common terms
                        Matrix< DDRMat > tPreMultiply =
                                tFlatNormal * ( tFIMaster->gradx( iOrder ) - tFISlave->gradx( iOrder ) );
                        tPreMultiply = reshape( tPreMultiply , tPreMultiply.numel(), 1 );
                        tPreMultiply = tPreMultiply * tSP->dSPdMasterDOF( tDofType );

                        // add contribution to jacobian
                        mSet->get_jacobian()(
                                { tMasterResStartIndex, tMasterResStopIndex },
                                { tMasterDepStartIndex, tMasterDepStopIndex } ) += aWStar * (
                                        tMasterdNdxFlat * tPreMultiply );

                        mSet->get_jacobian()(
                                { tSlaveResStartIndex,  tSlaveResStopIndex },
                                { tMasterDepStartIndex, tMasterDepStopIndex } ) -= aWStar * (
                                        tSlavedNdxFlat  * tPreMultiply );
                    }
                }

                // compute the jacobian for indirect dof dependencies through slave
                for( uint iDOF = 0; iDOF < tSlaveNumDofDependencies; iDOF++ )
                {
                    // get the dof type
                    Cell< MSI::Dof_Type > tDofType = mRequestedSlaveGlobalDofTypes( iDOF );

                    // get the index for the dof type
                    sint tDofDepIndex        = mSet->get_dof_index_for_type( tDofType( 0 ), mtk::Master_Slave::SLAVE );
                    uint tSlaveDepStartIndex = mSet->get_jac_dof_assembly_map()( tSlaveDofIndex )( tDofDepIndex, 0 );
                    uint tSlaveDepStopIndex  = mSet->get_jac_dof_assembly_map()( tSlaveDofIndex )( tDofDepIndex, 1 );

                    // compute jacobian direct dependencies
                    if ( tDofType( 0 ) == mResidualDofType( 0 ) )
                    {
                        // dRM/dS
                        mSet->get_jacobian()(
                                { tMasterResStartIndex, tMasterResStopIndex },
                                { tSlaveDepStartIndex,  tSlaveDepStopIndex } ) -= aWStar * (
                                        tMasterdNdxFlat * tSP->val()( 0 ) * trans( tSlavedNdxFlat ) );

                        // dRS/dS
                        mSet->get_jacobian()(
                                { tSlaveResStartIndex, tSlaveResStopIndex },
                                { tSlaveDepStartIndex, tSlaveDepStopIndex } ) += aWStar * (
                                        tSlavedNdxFlat * tSP->val()( 0 ) * trans( tSlavedNdxFlat ) );
                    }
                }
            }
        }

        //------------------------------------------------------------------------------

        void IWG_Ghost_Normal_Field::compute_jacobian_and_residual( real aWStar )
        {
            MORIS_ERROR( false, "IWG_Ghost_Normal_Field::compute_jacobian_and_residual - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Ghost_Normal_Field::compute_dRdp( real aWStar )
        {
            MORIS_ERROR( false, "IWG_Ghost_Normal_Field::compute_dRdp - This function does nothing.");
        }

        //------------------------------------------------------------------------------

        void IWG_Ghost_Normal_Field::compute_flat_dnNdxn(
                Matrix< DDRMat >  & aFlatdnNdxn,
                uint                aOrder,
                mtk::Master_Slave   aIsMaster )
        {
            // get flattened normal
            Matrix< DDRMat > tFlatNormal;
            this->get_flat_normal_matrix( tFlatNormal, aOrder );

            // get the residual dof type FI
            Field_Interpolator * tFIResidual =
                    this->get_field_interpolator_manager( aIsMaster )->
                    get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get number of fields
            uint tNumFields = tFIResidual->get_number_of_fields();

            // get flat dnNdxn (dnNdxn . normal)
            Matrix< DDRMat > tdnNdxn = trans( tFIResidual->dnNdxn( aOrder ) ) * trans( tFlatNormal );
            uint tNumRows = tdnNdxn.n_rows();
            uint tNumCols = tdnNdxn.n_cols();

            // set size for block flat dnNdxn (dnNdxn . normal)
            aFlatdnNdxn.set_size( tNumFields * tNumRows, tNumFields * tNumCols, 0.0 );

            // loop over the number of fields
            for( uint iField = 0; iField < tNumFields; iField++ )
            {
                // fill block flat dnNdxn
                aFlatdnNdxn(
                        { iField * tNumRows, ( iField + 1 ) * tNumRows - 1 },
                        { iField * tNumCols, ( iField + 1 ) * tNumCols - 1 } ) =
                                tdnNdxn.matrix_data();
            }
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
