/*
 * cl_FEM_IWG_Compressible_NS_Bulk_Flux_Matrices.cpp
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

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------
        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::assemble_variable_set()
        {
            // check if the variable vectors have already been assembled
            if( !mVarSetEval )
            {      
                return;
            }   

            // set the eval flag
            mVarSetEval = false;

            // check residual DoF types
            MORIS_ASSERT( check_residual_dof_types( mResidualDofType ), 
                    "IWG_Compressible_NS_Bulk::Y() - check for residual DoF types failed. See Error message above for more info." );

            // get number of state variable fields
            uint tNumStateVars = this->num_space_dims() + 2;

            uint tNumSecondDerivs = 3 * this->num_space_dims() - 3;

            // initialize Y vectors
            mY.set_size( tNumStateVars, 1, 0.0 );
            mdYdt.set_size( tNumStateVars, 1, 0.0 );
            mdYdx.assign( this->num_space_dims(), mY );
            //mdYdx.set_size( tNumStateVars, this->num_space_dims(), 0.0 );
            md2Ydx2.assign( tNumSecondDerivs, mY );
            //md2Ydx2.set_size( tNumStateVars, 3 * this->num_space_dims() - 3, 0.0 );

            // initialize counter for fill index
            uint iStateVar = 0;
            
            // go through residual dof types and assemble the state variable vector
            for ( uint iResDofType = 0; iResDofType < mResidualDofType.size(); iResDofType++ )
            {
                // get field interpolator
                Field_Interpolator * tFI =  mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( iResDofType ) );

                // get number of fields in FI
                uint tNumFields = tFI->get_number_of_fields(); 

                // get end index for fields
                uint tEndIndex = iStateVar + tNumFields - 1;

                // put state variables into matrices
                mY( { iStateVar, tEndIndex } ) = tFI->val().matrix_data();
                mdYdt( { iStateVar, tEndIndex } ) = trans( tFI->gradt( 1 ) );
                //mdYdx( { iStateVar, tEndIndex }, { 0, this->num_space_dims() - 1 } ) = trans( tFI->gradx( 1 ) );
                //md2Ydx2( { iStateVar, tEndIndex }, { 0, 3 * this->num_space_dims() - 4 } ) = trans( tFI->gradx( 2 ) );

                // fill dYdx 
                for ( uint iDim = 0; iDim < this->num_space_dims(); iDim++ )
                {
                    mdYdx( iDim )( { iStateVar, tEndIndex } ) = 
                            trans( tFI->gradx( 1 )( { iDim, iDim }, { 0, tNumFields - 1 } ) );
                }

                // fill d2Ydx2
                for ( uint iRow = 0; iRow < tNumSecondDerivs; iRow++ )
                {
                    md2Ydx2( iRow )( { iStateVar, tEndIndex } ) =
                            trans( tFI->gradx( 2 )( { iRow, iRow }, { 0, tNumFields - 1 } ) );
                }

                // increment fill index
                iStateVar += tNumFields;
            }
        }

        //------------------------------------------------------------------------------
        
        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::Y()
        {
            // check if the variable vectors have already been assembled
            if( !mVarSetEval )
            {      
                return mY;
            }   

            // assemble variable set
            this->assemble_variable_set();

            // return Y
            return mY;
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::dYdt()
        {
            // check if the variable vectors have already been assembled
            if( !mVarSetEval )
            {      
                return mdYdt;
            }   

            // assemble variable set
            this->assemble_variable_set();

            // return dYdt
            return mdYdt;
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::dYdx( const uint aSpatialDirection )
        {
            // check if the variable vectors have already been assembled
            if( !mVarSetEval )
            {      
                return mdYdx( aSpatialDirection );
            }   

            // assemble variable set
            this->assemble_variable_set();

            // return dYdx
            return mdYdx( aSpatialDirection );
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::d2Ydx2( const uint aI, const uint aJ )
        {
            // convert the two indices into one for condensed tensor
            uint tFlatIndex = convert_index_pair_to_flat( aI, aJ, this->num_space_dims() );
            
            // check if the variable vectors have already been assembled
            if( !mVarSetEval )
            {      
                return md2Ydx2( tFlatIndex );
            }   

            // assemble variable set
            this->assemble_variable_set();

            // return value
            return md2Ydx2( tFlatIndex );
        }

        //------------------------------------------------------------------------------
        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::assemble_test_function_set()
        {  
            // check if the test funcitons have already been assembled
            if( !mTestFuncSetEval )
            {
                return;
            }    
            
            // set the eval flag
            mTestFuncSetEval = false;

            // check residual DoF types
            MORIS_ASSERT( check_residual_dof_types( mResidualDofType ), 
                    "IWG_Compressible_NS_Bulk::assemble_test_function_set() - check for residual DoF types failed." );

            // get number of state variable fields
            uint tNumStateVars = this->num_space_dims() + 2;
            uint tNumSecondSpaceDerivs = 3 * this->num_space_dims() - 3;
            
            // initialize Y vectors
            mW.set_size( tNumStateVars, tNumStateVars * this->num_bases(), 0.0 );
            mdWdt.set_size( tNumStateVars, tNumStateVars * this->num_bases(), 0.0 );
            mdWdx.assign( this->num_space_dims(), mW );
            md2Wdx2.assign( tNumSecondSpaceDerivs, mW );    

            // get representative values for the different basis function vectors
            // NOTE: only works under the assumption that all state variable fields are interpolated on the same mesh
            Field_Interpolator * tFI =  mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );
            Matrix< DDRMat > tN = tFI->N()( { 0, 0 }, { 0, this->num_bases() - 1 } );
            Matrix< DDRMat > tdNdt = tFI->dnNdtn( 1 )( { 0, 0 }, { 0, this->num_bases() - 1 } );
            Matrix< DDRMat > tdNdx = tFI->dnNdxn( 1 );
            Matrix< DDRMat > td2Ndx2 = tFI->dnNdxn( 2 );
            
            // go through residual dof types and assemble the state variable vector
            for ( uint iVar = 0; iVar < tNumStateVars; iVar++ )
            {
                // put test functions into matrices
                mW( { iVar, iVar  }, { iVar * this->num_bases(), ( iVar + 1 ) * this->num_bases() - 1 } ) = 
                        tN.matrix_data();
                mdWdt( { iVar, iVar }, { iVar * this->num_bases(), ( iVar + 1 ) * this->num_bases() - 1 } ) = 
                        tdNdt.matrix_data(); 

                // fill dWdx 
                for ( uint iDim = 0; iDim < this->num_space_dims(); iDim++ )
                {
                    mdWdx( iDim )( { iVar, iVar }, { iVar * this->num_bases(), ( iVar + 1 ) * this->num_bases() - 1 } ) = 
                            tdNdx( { iDim, iDim }, { 0, this->num_bases() - 1 } );
                }

                // fill d2Wdx2
                for ( uint iRow = 0; iRow < tNumSecondSpaceDerivs; iRow++ )
                {
                    md2Wdx2( iRow )( { iVar, iVar }, { iVar * this->num_bases(), ( iVar + 1 ) * this->num_bases() - 1 } ) = 
                            td2Ndx2( { iRow, iRow }, { 0, this->num_bases() - 1 } );
                }
            }
        }

        //------------------------------------------------------------------------------
        
        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::W()
        {
            // check if the variable vectors have already been assembled
            if( !mTestFuncSetEval )
            {      
                return mW;
            }   

            // assemble variable set
            this->assemble_test_function_set();

            // return value
            return mW;
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::dWdt()
        {
            // check if the variable vectors have already been assembled
            if( !mTestFuncSetEval )
            {      
                return mdWdt;
            }   

            // assemble variable set
            this->assemble_test_function_set();

            // return value
            return mdWdt;
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::dWdx( const uint aSpatialDirection )
        {
            // check if the variable vectors have already been assembled
            if( !mTestFuncSetEval )
            {      
                return mdWdx( aSpatialDirection );
            }   

            // assemble variable set
            this->assemble_test_function_set();

            // return value
            return mdWdx( aSpatialDirection );
        }

        //------------------------------------------------------------------------------
        
        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::d2Wdx2( const uint aI, const uint aJ )
        {
            // convert the two indices into one for condensed tensor
            uint tFlatIndex = convert_index_pair_to_flat( aI, aJ, this->num_space_dims() );
            
            // check if the variable vectors have already been assembled
            if( !mTestFuncSetEval )
            {      
                return md2Wdx2( tFlatIndex );
            }   

            // assemble variable set
            this->assemble_test_function_set();

            // return value
            return md2Wdx2( tFlatIndex );
        }

        //------------------------------------------------------------------------------
        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::A( const uint aK )
        {
            // check that indices are not out of bounds
            MORIS_ASSERT( ( aK >= 0 ) and ( aK <= this->num_space_dims() ), 
                    "IWG_Compressible_NS_Bulk::A() - index out of bounds." );

            // 
            this->eval_A_matrices();

            // return requested value
            return mA( aK );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::eval_A_matrices()
        {
            // check if the variable vectors have already been assembled
            if( !mFluxAMatEval )
            {
                return;
            }  
            
            // set the eval flag
            mFluxAMatEval = false;          

            // get the material and constitutive models
            std::shared_ptr< Material_Model > tMM = mMasterMM( static_cast< uint >( IWG_Material_Type::FLUID_MM ) );
            std::shared_ptr< Constitutive_Model > tCM = mMasterCM( static_cast< uint >( IWG_Constitutive_Type::FLUID_CM ) );

            // evaluate A-matrices and store them
            eval_A( tMM, tCM, mMasterFIManager, mResidualDofType, mA );  
        }

       //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Bulk::eval_A_DOF_matrices()
        {
            // check if the A-Dof flux matrices have already been assembled
            if( !mFluxADofMatEval )
            {
                return;
            }
            
            // set the eval flag
            mFluxADofMatEval = false;

            // get the material and constitutive models
            std::shared_ptr< Material_Model > tMM = mMasterMM( static_cast< uint >( IWG_Material_Type::FLUID_MM ) );
            std::shared_ptr< Constitutive_Model > tCM = mMasterCM( static_cast< uint >( IWG_Constitutive_Type::FLUID_CM ) );

            // get the velocity FI
            Field_Interpolator * tFIVelocity =  mMasterFIManager->get_field_interpolators_for_type( mDofVelocity );
            
            // get number of Space dimensions
            uint tNumSpaceDims = tFIVelocity->get_number_of_fields();

            // get number of bases for the elements used
            uint tNumBases = tFIVelocity->get_number_of_space_time_bases();

            // create standard empty matrix
            const Matrix< DDRMat > tEmptyADof( tNumSpaceDims + 2, ( tNumSpaceDims + 2 ) * tNumBases, 0.0 );

            // initialize derivatives of A-matrices
            mADOF.resize( tNumSpaceDims + 1 );
            for ( uint iMat = 0; iMat < tNumSpaceDims + 1; iMat++)
            {
                mADOF( iMat ).assign( tNumSpaceDims + 2, tEmptyADof );
            } 

            // check Dof dependencies
            MORIS_ASSERT( check_dof_dependencies( mSet, mResidualDofType, mRequestedMasterGlobalDofTypes ),
                    "IWG_Compressible_NS_Bulk::eval_A_DOF_matrices() - List of Dof Dependencies not supported. See error messages above." );     

            // evaluate the derivatives for each of the matrices and store them
            eval_A0_DOF( tMM, tCM, mMasterFIManager, mResidualDofType, mADOF( 0 ) ); 
            eval_A1_DOF( tMM, tCM, mMasterFIManager, mResidualDofType, mADOF( 1 ) );  
            eval_A2_DOF( tMM, tCM, mMasterFIManager, mResidualDofType, mADOF( 2 ) ); 
            if ( tNumSpaceDims == 3 )
            {
                eval_A3_DOF( tMM, tCM, mMasterFIManager, mResidualDofType, mADOF( 3 ) );  
            } 
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::K( const uint aI, const uint aJ )
        {
            // check that indices are not out of bounds
            MORIS_ASSERT( ( aI >= 0 ) and ( aI < this->num_space_dims() ) and ( aJ >= 0 ) and ( aJ < this->num_space_dims() ), 
                    "IWG_Compressible_NS_Bulk::K() - indices out of bounds." );

            // check if K matrices have already been evaluated
            if ( !mFluxKMatEval )
            {
                return mK( aI )( aJ );
            }

            // set the eval flag
            mFluxKMatEval = false;            

            // get the viscosity
            std::shared_ptr< Property > tPropDynamicViscosity = mMasterProp( static_cast< uint >( IWG_Property_Type::DYNAMIC_VISCOSITY ) );
            std::shared_ptr< Property > tPropThermalConductivity = mMasterProp( static_cast< uint >( IWG_Property_Type::THERMAL_CONDUCTIVITY ) );

            // eval K matrices and store them
            eval_K( tPropDynamicViscosity, tPropThermalConductivity, mMasterFIManager, mK );

            // return requested K matrix
            return mK( aI )( aJ );
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::Kiji( const uint aJ )
        {
            // check that indices are not out of bounds
            MORIS_ASSERT( ( aJ >= 0 ) and ( aJ < this->num_space_dims() ), 
                    "IWG_Compressible_NS_Bulk::Kiji() - index out of bounds." );

            // check if Kiji matrices have already been evaluated
            if ( !mKijiEval )
            {
                return mKiji( aJ );
            }

            // set the eval flag
            mKijiEval = false;            

            // get the viscosity
            std::shared_ptr< Property > tPropDynamicViscosity = mMasterProp( static_cast< uint >( IWG_Property_Type::DYNAMIC_VISCOSITY ) );
            std::shared_ptr< Property > tPropThermalConductivity = mMasterProp( static_cast< uint >( IWG_Property_Type::THERMAL_CONDUCTIVITY ) );

            // eval spatial derivatives of K matrices and store them
            eval_dKijdxi( tPropDynamicViscosity, tPropThermalConductivity, mMasterFIManager, mKiji );

            // return requested Kiji matrix
            return mKiji( aJ );
        } 

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::LY()
        {
            // check if LY already been evaluated
            if ( !mLYEval )
            {
                return mLY;
            }

            // set the eval flag
            mLYEval = false;  

            // evaluate LY
            mLY = this->A( 0 ) * this->dYdt();

            // get subview for += operations
            auto tLY = mLY( { 0, mLY.n_rows() - 1 }, { 0, mLY.n_cols() - 1 } );

            // 
            for ( uint iDim = 0; iDim < this->num_space_dims(); iDim++ )
            {
                tLY += ( this->A( iDim + 1 ) - this->Kiji( iDim ) ) * this->dYdx( iDim ); 

                for ( uint jDim = 0; jDim < this->num_space_dims(); jDim++ )
                {
                    tLY -= this->K( iDim, jDim ) * this->d2Ydx2( iDim, jDim );
                }
            }

            // return value
            return mLY;
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::dLdDofY()
        {
            // check if LY already been evaluated
            if ( !mLDofYEval )
            {
                return mdLdDofY;
            }

            // set the eval flag
            mLDofYEval = false;  

            // get the material and constitutive models
            std::shared_ptr< Material_Model > tMM = mMasterMM( static_cast< uint >( IWG_Material_Type::FLUID_MM ) );
            std::shared_ptr< Constitutive_Model > tCM = mMasterCM( static_cast< uint >( IWG_Constitutive_Type::FLUID_CM ) );

            // get the properties
            std::shared_ptr< Property > tPropMu = mMasterProp( static_cast< uint >( IWG_Property_Type::DYNAMIC_VISCOSITY ) );
            std::shared_ptr< Property > tPropKappa = mMasterProp( static_cast< uint >( IWG_Property_Type::THERMAL_CONDUCTIVITY ) );

            // initialize cell containing A-matrices pre-multiplied with the state variable vector
            moris::Cell< Matrix< DDRMat > > tdAjdY_Yj( this->num_space_dims() + 1 );

            // initialize cell containing Kij,i-matrices pre-multiplied with the state variable vector
            moris::Cell< moris::Cell< Matrix< DDRMat > > > tdKijidY_Yj( this->num_space_dims() );

            // initialize cell containing Kij,i-matrices pre-multiplied with the state variable vector
            moris::Cell< moris::Cell< Matrix< DDRMat > > > tdKijdY_Yij( this->num_space_dims() );

            // get dA0/dY * Y,t
            eval_dAdY_VR( tMM, tCM, mMasterFIManager, mResidualDofType, this->dYdt(), 0, tdAjdY_Yj( 0 ) );

            // compute A(0) term
            mdLdDofY = tdAjdY_Yj( 0 ) * this->W();

            // get subview of matrix for += operations
            auto tdLdDofY = mdLdDofY( { 0, mdLdDofY.n_rows() - 1 }, { 0, mdLdDofY.n_cols() - 1 } );

            // go over all Aj*Y,j and Kij,i*Y,j and Kij*Y,ij terms and add up
            for ( uint iDim = 0; iDim < this->num_space_dims(); iDim++ )
            {
                // get dAj/dY * Y,j
                eval_dAdY_VR( tMM, tCM, mMasterFIManager, mResidualDofType, this->dYdx( iDim ), iDim + 1, tdAjdY_Yj( iDim + 1 ) );

                // add contributions from A-matrices
                tdLdDofY += tdAjdY_Yj( iDim + 1 )  * this->W();

                // get dKij,i/dY * Y,j
                eval_dKijidY_VR( tPropMu, tPropKappa, mMasterFIManager, this->dYdx( iDim ), iDim, tdKijidY_Yj( iDim ) );

                // add contributions from Kij,i-matrices
                // tdLdDofY -= tdKijdY_Yij( iDim )( 0 ) * this->W();

                for ( uint jDim = 0; jDim < this->num_space_dims(); jDim++ )
                {
                    // add contributions from Kij,i-matrices
                    tdLdDofY -= tdKijdY_Yij( iDim )( jDim + 1 ) * this->dWdx( jDim );

                    // get dKij/dY * Y,ij
                    eval_dKdY_VR( tPropMu, tPropKappa, mMasterFIManager, this->d2Ydx2( iDim, jDim ), iDim, jDim, tdKijdY_Yij( iDim )( jDim ) );
                    
                    // add contributions from K-matrices
                    tdLdDofY -= tdKijdY_Yij( iDim )( jDim ) * this->W();
                }
            }

            // return value
            return mdLdDofY;
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::LW()
        {
            // check if LY already been evaluated
            if ( !mLWEval )
            {
                return mLW;
            }

            // set the eval flag
            mLWEval = false;  

            // evaluate LY
            mLW = this->A( 0 ) * this->dWdt();

            // get subview for += operations
            auto tLW = mLW( { 0, mLW.n_rows() - 1 }, { 0, mLW.n_cols() - 1 } );

            // 
            for ( uint iDim = 0; iDim < this->num_space_dims(); iDim++ )
            {
                tLW += ( this->A( iDim + 1 ) - this->Kiji( iDim ) ) * this->dWdx( iDim ); 

                for ( uint jDim = 0; jDim < this->num_space_dims(); jDim++ )
                {
                    tLW -= this->K( iDim, jDim ) * this->d2Wdx2( iDim, jDim );
                }
            }

            // return value
            return mLW;
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & IWG_Compressible_NS_Bulk::dLdDofW(  const Matrix< DDRMat > & aVL  )
        {
            // get the material and constitutive models
            std::shared_ptr< Material_Model > tMM = mMasterMM( static_cast< uint >( IWG_Material_Type::FLUID_MM ) );
            std::shared_ptr< Constitutive_Model > tCM = mMasterCM( static_cast< uint >( IWG_Constitutive_Type::FLUID_CM ) );

            // get the properties
            std::shared_ptr< Property > tPropMu = mMasterProp( static_cast< uint >( IWG_Property_Type::DYNAMIC_VISCOSITY ) );
            std::shared_ptr< Property > tPropKappa = mMasterProp( static_cast< uint >( IWG_Property_Type::THERMAL_CONDUCTIVITY ) );

            // initialize cell containing A-matrices pre-multiplied with VL
            moris::Cell< Matrix< DDRMat > > tVLdAjdY( this->num_space_dims() + 1 );

            // initialize cell containing Kij,i-matrices pre-multiplied with VL
            moris::Cell< moris::Cell< Matrix< DDRMat > > > tVLdKijidY( this->num_space_dims() );

            // initialize cell containing Kij,i-matrices pre-multiplied with VL
            moris::Cell< moris::Cell< Matrix< DDRMat > > > tVLdKijdY( this->num_space_dims() );

            // get VL * dA0/dY
            eval_VL_dAdY( tMM, tCM, mMasterFIManager, mResidualDofType, aVL, 0, tVLdAjdY( 0 ) );

            // compute A(0) term
            mdLdDofW = trans( this->dWdt() ) * tVLdAjdY( 0 ) * this->W();

            // get subview of matrix for += operations
            auto tdLdDofW = mdLdDofW( { 0, mdLdDofW.n_rows() - 1 }, { 0, mdLdDofW.n_cols() - 1 } );

            // go over all VL*Aj and VL*Kij,i and VL*Kij terms and add up
            for ( uint iDim = 0; iDim < this->num_space_dims(); iDim++ )
            {
                // get VL * dAj/dY
                eval_VL_dAdY( tMM, tCM, mMasterFIManager, mResidualDofType, aVL, iDim + 1, tVLdAjdY( iDim + 1 ) );

                // add contributions from A-matrices
                tdLdDofW += trans( this->dWdx( iDim ) ) * tVLdAjdY( iDim + 1 )  * this->W();

                // get VL * dKij,i/dY
                eval_VL_dKijidY( tPropMu, tPropKappa, mMasterFIManager, this->dYdx( iDim ), iDim, tVLdKijidY( iDim ) );

                // add contributions from Kij,i-matrices
                // tdLdDofW -= trans( this->dWdx( iDim ) ) * tVLdKijidY( iDim )( 0 ) * this->W();

                for ( uint jDim = 0; jDim < this->num_space_dims(); jDim++ )
                {
                    // add contributions from Kij,i-matrices
                    tdLdDofW -= trans( this->dWdx( iDim ) ) * tVLdKijidY( iDim )( jDim + 1 ) * this->dWdx( jDim );

                    // get VL * dKij/dY
                    eval_VL_dKdY( tPropMu, tPropKappa, mMasterFIManager, aVL, iDim, jDim, tVLdKijdY( iDim )( jDim ) );

                    // add contributions from K-matrices
                    tdLdDofW -= trans( this->d2Wdx2( iDim, jDim ) ) * tVLdKijdY( iDim )( jDim ) * this->W();
                }
            }

            // return value
            return mdLdDofW;
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
