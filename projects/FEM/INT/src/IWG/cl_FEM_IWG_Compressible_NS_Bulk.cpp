/*
 * cl_FEM_IWG_Compressible_NS_Bulk.cpp
 *
 *  Created on: Feb 10, 2021
 *      Author: wunsch
 */

#include "cl_FEM_Set.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_IWG_Compressible_NS_Bulk.hpp"
#include "fn_FEM_IWG_Compressible_NS.hpp"

#include "fn_trans.hpp"
#include "fn_norm.hpp"
#include "fn_eye.hpp"

// debug - output to hdf5
#include "paths.hpp"
#include "HDF5_Tools.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        IWG_Compressible_NS_Bulk::IWG_Compressible_NS_Bulk()
        {
            // set size for the property pointer cell
            mMasterProp.resize( static_cast< uint >( IWG_Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "DynamicViscosity" ]     = static_cast< uint >( IWG_Property_Type::DYNAMIC_VISCOSITY );
            mPropertyMap[ "ThermalConductivity" ]  = static_cast< uint >( IWG_Property_Type::THERMAL_CONDUCTIVITY );
            mPropertyMap[ "BodyForce" ]            = static_cast< uint >( IWG_Property_Type::BODY_FORCE );
            mPropertyMap[ "BodyHeatLoad" ]         = static_cast< uint >( IWG_Property_Type::BODY_HEAT_LOAD );

            // set size for the material model pointer cell
            mMasterMM.resize( static_cast< uint >( IWG_Material_Type::MAX_ENUM ), nullptr );

            // populate the material map
            mMaterialMap[ "FluidMM" ] = static_cast< uint >( IWG_Material_Type::FLUID_MM );

            // set size for the constitutive model pointer cell
            mMasterCM.resize( static_cast< uint >( IWG_Constitutive_Type::MAX_ENUM ), nullptr );

            // populate the constitutive map
            mConstitutiveMap[ "FluidCM" ] = static_cast< uint >( IWG_Constitutive_Type::FLUID_CM );

            // set size for the stabilization parameter pointer cell
            mStabilizationParam.resize( static_cast< uint >( IWG_Constitutive_Type::MAX_ENUM ), nullptr );

            // populate the stabilization parameter map
            mStabilizationMap[ "GenericSP" ] = static_cast< uint >( IWG_Stabilization_Type::GENERIC );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::reset_spec_eval_flags()
        {
            // reset eval flags
            mSpaceDimEval = true;
            mNumBasesEval = true;

            mVarSetEval = true;
            mTestFuncSetEval = true;

            mFluxAMatEval = true;
            mFluxADofMatEval = true;
            mFluxKMatEval = true;
            mKijiEval = true;
            mFluxKDofMatEval = true;

            mLYEval = true;
            mLWEval = true;
            mLDofYEval = true;

            mA0invEval = true;
            mGEval = true;
            mMEval = true;
            mMinvEval = true;
            mTauEval = true;
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::compute_residual( real aWStar )
        {
            // check master field interpolators
#ifdef DEBUG
            this->check_field_interpolators();
#endif
            // check residual dof types
            MORIS_ASSERT( check_residual_dof_types( mResidualDofType ), 
                    "IWG_Compressible_NS_Bulk::compute_jacobian() - Only pressure or density primitive variables supported for now." );


            // get indeces for residual dof types, indices for assembly (FIXME: assembly only for primitive vars)
            uint tMasterDof1Index      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterDof3Index      = mSet->get_dof_index_for_type( mResidualDofType( 2 ), mtk::Master_Slave::MASTER );
            uint tMasterRes1StartIndex = mSet->get_res_dof_assembly_map()( tMasterDof1Index )( 0, 0 );
            uint tMasterRes3StopIndex  = mSet->get_res_dof_assembly_map()( tMasterDof3Index )( 0, 1 );

            // get number of space dimensions
            uint tNumSpaceDims = this->num_space_dims();

            // get the material and constitutive models
            std::shared_ptr< Material_Model > tMM = mMasterMM( static_cast< uint >( IWG_Material_Type::FLUID_MM ) );
            std::shared_ptr< Constitutive_Model > tCM = mMasterCM( static_cast< uint >( IWG_Constitutive_Type::FLUID_CM ) );

            // get subview for complete residual
            auto tRes = mSet->get_residual()( 0 )( { tMasterRes1StartIndex, tMasterRes3StopIndex }, { 0, 0 } );

            // A0 matrix contribution
            tRes += aWStar * trans( this->W() ) * this->A( 0 ) * this->dYdt();

            // loop over A-Matrices
            for ( uint iA = 1; iA < tNumSpaceDims + 1; iA++ )
            {
                // compute residual
                tRes += aWStar * trans( this->W() ) * this->A( iA ) * this->dYdx( iA - 1 );
            }

            // loop over K-matrices
            for ( uint iDim = 0; iDim < tNumSpaceDims; iDim++ )
            {
                for ( uint jDim = 0; jDim < tNumSpaceDims; jDim++ )
                {
                    // compute residual
                    tRes += aWStar * trans( this->dWdx( iDim ) ) * this->K( iDim, jDim ) * this->dYdx( jDim );
                }
            }

            // get the Stabilization Parameter
            const std::shared_ptr< Stabilization_Parameter > & tSP = mStabilizationParam( static_cast< uint >( IWG_Stabilization_Type::GENERIC ) );

            // add contribution of stabilization term if stabilization parameter has been set
            if ( tSP != nullptr )
            {
                // stabilization term for testing
                tRes += aWStar * tSP->val()( 0 ) * trans( this->W() ) * this->LY();

                // stabilization term
                //tRes += aWStar * tSP->val()( 0 ) * trans( this->LW() ) * this->Tau() * this->LY();
            }

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_residual()( 0 ) ),
                    "IWG_Compressible_NS_Bulk::compute_residual - Residual contains NAN or INF, exiting!");                                 
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::compute_jacobian( real aWStar )
        {
            // check master field interpolators
#ifdef DEBUG
            this->check_field_interpolators();
#endif
            // check residual dof types
            MORIS_ASSERT( check_residual_dof_types( mResidualDofType ), 
                    "IWG_Compressible_NS_Bulk::compute_jacobian() - Only pressure or density primitive variables supported for now." );

            // get indeces for residual dof types, indices for assembly (FIXME: assembly only for primitive vars)
            uint tMasterDof1Index      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterDof3Index      = mSet->get_dof_index_for_type( mResidualDofType( 2 ), mtk::Master_Slave::MASTER );
            uint tMasterRes1StartIndex = mSet->get_res_dof_assembly_map()( tMasterDof1Index )( 0, 0 );
            uint tMasterRes3StopIndex  = mSet->get_res_dof_assembly_map()( tMasterDof3Index )( 0, 1 );

            // get number of space dimensions
            uint tNumSpaceDims = this->num_space_dims();                   

            // get the material and constitutive models
            std::shared_ptr< Material_Model > tMM = mMasterMM( static_cast< uint >( IWG_Material_Type::FLUID_MM ) );
            std::shared_ptr< Constitutive_Model > tCM = mMasterCM( static_cast< uint >( IWG_Constitutive_Type::FLUID_CM ) );

            // check DoF dependencies
            MORIS_ASSERT( check_dof_dependencies( mSet, mResidualDofType, mRequestedMasterGlobalDofTypes ), 
                    "IWG_Compressible_NS_Bulk::compute_jacobian - Set of DoF dependencies not suppported." );

            // get the indeces for assembly
            sint tDofFirstDepIndex     = mSet->get_dof_index_for_type( mRequestedMasterGlobalDofTypes( 0 )( 0 ), mtk::Master_Slave::MASTER );
            sint tDofThirdDepIndex     = mSet->get_dof_index_for_type( mRequestedMasterGlobalDofTypes( 2 )( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterDep1StartIndex = mSet->get_jac_dof_assembly_map()( tMasterDof1Index )( tDofFirstDepIndex, 0 );
            uint tMasterDep3StopIndex  = mSet->get_jac_dof_assembly_map()( tMasterDof3Index )( tDofThirdDepIndex, 1 );                
           
            // get subview of jacobian for += operations   
            auto tJac = mSet->get_jacobian()( { tMasterRes1StartIndex, tMasterRes3StopIndex }, { tMasterDep1StartIndex, tMasterDep3StopIndex } );  

            // add contribution from d(A0)/dDof * Y,t
            Matrix< DDRMat > tdAdY;
            eval_dAdY_VR( tMM, tCM, mMasterFIManager, mResidualDofType, this->dYdt(), 0, tdAdY );
            tJac += aWStar * trans( this->W() ) * ( tdAdY * this->W() + this->A( 0 ) * this->dWdt() ); 

            // loop over contributions from A-matrices
            for ( uint iDim = 0; iDim < tNumSpaceDims; iDim++ )
            {
                // evaluate d(Ai)/dDof * Y,i
                eval_dAdY_VR( tMM, tCM, mMasterFIManager, mResidualDofType, this->dYdx( iDim ), iDim + 1, tdAdY );

                // add contribution
                tJac += aWStar * trans( this->W() ) * ( tdAdY * this->W() + this->A( iDim + 1 ) * this->dWdx( iDim ) );

            }

            // get properties for K-matrices
            std::shared_ptr< Property > tPropMu    = mMasterProp( static_cast< uint >( IWG_Property_Type::DYNAMIC_VISCOSITY ) );
            std::shared_ptr< Property > tPropKappa = mMasterProp( static_cast< uint >( IWG_Property_Type::THERMAL_CONDUCTIVITY ) );

            // loop over contributions from K-matrices
            Matrix< DDRMat > dKdY;
            for ( uint iDim = 0; iDim < tNumSpaceDims; iDim++ )
            {
                for ( uint jDim = 0; jDim < tNumSpaceDims; jDim++ )
                {
                    // get dKij/dY * Y,ij
                    eval_dKdY_VR( tPropMu, tPropKappa, mMasterFIManager, this->dYdx( jDim ), iDim, jDim, dKdY );
                                
                    // add contributions from K-matrices
                    tJac += aWStar * trans( this->dWdx( iDim ) ) * ( dKdY * this->W() + K( iDim, jDim ) * this->dWdx( jDim ) );
                }
            }

            // get the Stabilization Parameter
            const std::shared_ptr< Stabilization_Parameter > & tSP = mStabilizationParam( static_cast< uint >( IWG_Stabilization_Type::GENERIC ) );

            // add contribution of stabilization term if stabilization parameter has been set
            if ( tSP != nullptr )
            {
                // stabilization term for testing
                tJac += aWStar * tSP->val()( 0 ) * trans( this->W() ) * ( this->LW() + this->dLdDofY() );
            }

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_jacobian() ) ,
                    "IWG_Compressible_NS_Bulk::compute_jacobian - Jacobian contains NAN or INF, exiting!");                     
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::compute_jacobian_and_residual( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators
            this->check_field_interpolators();
#endif

            MORIS_ERROR( false, "IWG_Compressible_NS_Bulk::compute_jacobian_and_residual - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::compute_dRdp( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties and constitutive models
            this->check_field_interpolators();
#endif

            MORIS_ERROR( false, "IWG_Compressible_NS_Bulk::compute_dRdp - Not implemented." );
        }

        //------------------------------------------------------------------------------

        uint IWG_Compressible_NS_Bulk::num_space_dims()
        {
            // check if number of spatial dimensions is known
            if ( !mSpaceDimEval )
            {
                return mNumSpaceDims;
            }

            // set eval flag
            mSpaceDimEval = false;
            
            // get CM
            std::shared_ptr< Constitutive_Model > tCM = 
                    mMasterCM( static_cast< uint >( IWG_Constitutive_Type::FLUID_CM ) );

            // get number of spatial dimensions from CM
            mNumSpaceDims = tCM->get_num_space_dims();

            // return
            return mNumSpaceDims;
        }

        //------------------------------------------------------------------------------

        uint IWG_Compressible_NS_Bulk::num_bases()
        {
            // check if number of spatial dimensions is known
            if ( !mNumBasesEval )
            {
                return mNumBasesPerField;
            }

            // set eval flag
            mNumBasesEval = false;
            
            // get first FI
            Field_Interpolator * tFI =  mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );

            // get number of spatial dimensions from CM
            mNumBasesPerField = tFI->get_number_of_space_time_bases();

            // return
            return mNumBasesPerField;
        }

        //------------------------------------------------------------------------------

        // FIXME provided directly by the field interpolator?
        void IWG_Compressible_NS_Bulk::compute_dnNdtn(
                Matrix< DDRMat > & adnNdtn )
        {
            // get the residual dof type FI (here velocity)
            Field_Interpolator * tVelocityFI =
                    mMasterFIManager->get_field_interpolators_for_type( mDofVelocity );                 

            // init size for dnNdtn
            uint tNumRowt = tVelocityFI->get_number_of_fields();
            uint tNumColt = tVelocityFI->dnNdtn( 1 ).n_cols();
            adnNdtn.set_size( tNumRowt, tNumRowt * tNumColt , 0.0 );

            // loop over the fields
            for( uint iField = 0; iField < tNumRowt; iField++ )
            {
                // fill the matrix for each dimension
                adnNdtn( { iField, iField }, { iField * tNumColt, ( iField + 1 ) * tNumColt - 1 } ) =
                        tVelocityFI->dnNdtn( 1 ).matrix_data();
            }
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::MultipMat()
        {
            //build multiplication matrix
            //for 2D
            if( mMasterFIManager->get_field_interpolators_for_type( mDofVelocity )->get_number_of_fields() == 2 )
            {
                return mMultipMat2D;
            }
            // for 3D
            else
            {
                return mMultipMat3D;
            }
        }  

        //------------------------------------------------------------------------------

    } /* namespace fem */
} /* namespace moris */
