/*
 * cl_FEM_IWG_Compressible_NS_Advective_Momentum_Flux_Boundary.cpp
 *
 *  Created on: Aug 26, 2020
 *      Author: wunsch
 */

#include "cl_FEM_IWG_Compressible_NS_Advective_Momentum_Flux_Boundary.hpp"

#include "cl_FEM_Set.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

#include "fn_trans.hpp"
#include "fn_norm.hpp"
#include "fn_eye.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::IWG_Compressible_NS_Advective_Momentum_Flux_Boundary()
        {
            // set size for the constitutive model pointer cell
            mMasterCM.resize( static_cast< uint >( IWG_Constitutive_Type::MAX_ENUM ), nullptr );

            // populate the constitutive map
            mConstitutiveMap[ "Fluid" ] = IWG_Constitutive_Type::FLUID;
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_property(
                std::shared_ptr< Property > aProperty,
                std::string                 aPropertyString,
                mtk::Master_Slave           aIsMaster )
        {
            // check that aPropertyString makes sense
            std::string tErrMsg =
                    std::string( "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_property - Unknown aPropertyString: " ) +
                    aPropertyString;
            MORIS_ERROR( mPropertyMap.find( aPropertyString ) != mPropertyMap.end(), tErrMsg.c_str() );

            // check no slave allowed
            MORIS_ERROR( aIsMaster == mtk::Master_Slave::MASTER,
                    "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_property - No slave allowed." );

            // set the property in the property cell
            this->get_properties( aIsMaster )( static_cast< uint >( mPropertyMap[ aPropertyString ] ) ) = aProperty;
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_constitutive_model(
                std::shared_ptr< Constitutive_Model > aConstitutiveModel,
                std::string                           aConstitutiveString,
                mtk::Master_Slave                     aIsMaster  )
        {
            // check that aConstitutiveString makes sense
            std::string tErrMsg =
                    std::string( "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_constitutive_model - Unknown aConstitutiveString: " ) +
                    aConstitutiveString;
            MORIS_ERROR( mConstitutiveMap.find( aConstitutiveString ) != mConstitutiveMap.end(), tErrMsg.c_str() );

            // check no slave allowed
            MORIS_ERROR( aIsMaster == mtk::Master_Slave::MASTER,
                    "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_constitutive_model - No slave allowed." );

            // set the constitutive model in the constitutive model cell
            this->get_constitutive_models( aIsMaster )( static_cast< uint >( mConstitutiveMap[ aConstitutiveString ] ) ) = aConstitutiveModel;
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_stabilization_parameter(
                std::shared_ptr< Stabilization_Parameter > aStabilizationParameter,
                std::string                                aStabilizationString )
        {
            MORIS_ERROR( false, "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::set_stabilization_parameter - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_residual( real aWStar )
        {
            // check master field interpolators
#ifdef DEBUG
            this->check_field_interpolators();
#endif

            // get master index for residual dof type (here velocity), indices for assembly
            uint tMasterDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterResStartIndex = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 0 );
            uint tMasterResStopIndex  = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 1 );

            // get the field interpolators
            Field_Interpolator * tDensityFI = mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );
            Field_Interpolator * tVelocityFI = mMasterFIManager->get_field_interpolators_for_type( mDofVelocity );

            // build dyadic product of velocity vectors
            Matrix< DDRMat > tUiUj;
            this->compute_uiuj( tUiUj );

            // build flattened normal
            Matrix< DDRMat > tNormalMatrix;
            this->compute_normal_matrix( tNormalMatrix );

            // compute the residual weak form
            mSet->get_residual()( 0 )( { tMasterResStartIndex, tMasterResStopIndex }, { 0, 0 } ) += aWStar * (
                    trans( tVelocityFI->N() ) * tDensityFI->val() * tNormalMatrix * tUiUj );

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_residual()( 0 ) ),
                    "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_residual - Residual contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------
        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_jacobian( real aWStar )
        {
            // check master field interpolators
#ifdef DEBUG
            this->check_field_interpolators();
#endif

            // get master index for residual dof type (here velocity), indices for assembly
            uint tMasterDofIndex      = mSet->get_dof_index_for_type( mResidualDofType( 0 ), mtk::Master_Slave::MASTER );
            uint tMasterResStartIndex = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 0 );
            uint tMasterResStopIndex  = mSet->get_res_dof_assembly_map()( tMasterDofIndex )( 0, 1 );

            // get the FIs
            Field_Interpolator * tFIVelocity =  mMasterFIManager->get_field_interpolators_for_type( mResidualDofType( 0 ) );
            Field_Interpolator * tFIDensity =  mMasterFIManager->get_field_interpolators_for_type( mDofDensity );

            // compute the jacobian for dof dependencies
            uint tNumDofDependencies = mRequestedMasterGlobalDofTypes.size();
            for( uint iDOF = 0; iDOF < tNumDofDependencies; iDOF++ )
            {
                // get the treated dof type
                Cell< MSI::Dof_Type > & tDofType = mRequestedMasterGlobalDofTypes( iDOF );

                // get the index for dof type, indices for assembly
                sint tDofDepIndex         = mSet->get_dof_index_for_type( tDofType( 0 ), mtk::Master_Slave::MASTER );
                uint tMasterDepStartIndex = mSet->get_jac_dof_assembly_map()( tMasterDofIndex )( tDofDepIndex, 0 );
                uint tMasterDepStopIndex  = mSet->get_jac_dof_assembly_map()( tMasterDofIndex )( tDofDepIndex, 1 );

                // if dof type is velocity, add diagonal term (velocity-velocity DoF types)
                if( tDofType( 0 ) == mDofDensity )
                {
                    // build dyadic product of velocity vectors
                    Matrix< DDRMat > tUiUj;
                    this->compute_uiuj( tUiUj );

                    // build flattened normal
                    Matrix< DDRMat > tNormalMatrix;
                    this->compute_normal_matrix( tNormalMatrix );

                    // add contribution
                    mSet->get_jacobian()(
                            { tMasterResStartIndex, tMasterResStopIndex },
                            { tMasterDepStartIndex, tMasterDepStopIndex } ) += aWStar * (
                                    trans( tFIVelocity->N() ) * tNormalMatrix * tUiUj * tFIDensity->N() );
                }

                // if dof type is velocity, add diagonal term (velocity-velocity DoF types)
                if( tDofType( 0 ) == mResidualDofType( 0 ) )
                {
                    // build derivative of dyadic product of velocity vectors wrt to dofs
                    Matrix< DDRMat > tdUiUjdDOF;
                    this->compute_duiujdDOF( tdUiUjdDOF );

                    // build flattened normal
                    Matrix< DDRMat > tNormalMatrix;
                    this->compute_normal_matrix( tNormalMatrix );

                    // add contribution
                    mSet->get_jacobian()(
                            { tMasterResStartIndex, tMasterResStopIndex },
                            { tMasterDepStartIndex, tMasterDepStopIndex } ) += aWStar * (
                                    trans( tFIVelocity->N() ) * tFIDensity->val() * tNormalMatrix * tdUiUjdDOF );
                }
            }

            // check for nan, infinity
            MORIS_ASSERT( isfinite( mSet->get_jacobian() ) ,
                    "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_jacobian - Jacobian contains NAN or INF, exiting!");
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_jacobian_and_residual( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators
            this->check_field_interpolators();
#endif

            MORIS_ERROR( false, "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_jacobian_and_residual - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_dRdp( real aWStar )
        {
#ifdef DEBUG
            // check master field interpolators, properties and constitutive models
            this->check_field_interpolators();
#endif

            MORIS_ERROR( false, "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_dRdp - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_residual_strong_form(
                Matrix< DDRMat > & aRM,
                real             & aRC )
        {
            MORIS_ERROR( false, "IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_residual_strong_form - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_jacobian_strong_form(
                moris::Cell< MSI::Dof_Type >   aDofTypes,
                Matrix< DDRMat >             & aJM,
                Matrix< DDRMat >             & aJC )
        {
            MORIS_ERROR( false, "IWG_Compressible_NS_Advective_Mass_Flux_Boundary::compute_jacobian_strong_form - Not implemented." );
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_uiuj(Matrix< DDRMat > & auiuj)
        {
            // get the velocity vector
            Field_Interpolator * tFIVelocity =  mMasterFIManager->get_field_interpolators_for_type( mDofVelocity );

            // get the velocity vector
            Matrix< DDRMat > tVelocityVec = tFIVelocity->val();

            // assembly into flattened tensor
            // for 2D
            if( tFIVelocity->get_number_of_fields() == 2 )
            {
                auiuj.set_size( 3, 1, 0.0 );

                auiuj( 0 ) = std::pow( tVelocityVec( 0 ), 2.0 );
                auiuj( 1 ) = std::pow( tVelocityVec( 1 ), 2.0 );
                auiuj( 2 ) = tVelocityVec( 0 ) * tVelocityVec( 1 );

            }
            // for 3D
            else
            {
                auiuj.set_size( 6, 1, 0.0 );

                auiuj( 0 ) = std::pow( tVelocityVec( 0 ), 2.0 );
                auiuj( 1 ) = std::pow( tVelocityVec( 1 ), 2.0 );
                auiuj( 2 ) = std::pow( tVelocityVec( 2 ), 2.0 );
                auiuj( 3 ) = tVelocityVec( 1 ) * tVelocityVec( 2 );
                auiuj( 4 ) = tVelocityVec( 0 ) * tVelocityVec( 2 );
                auiuj( 5 ) = tVelocityVec( 0 ) * tVelocityVec( 1 );
            }
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_duiujdDOF(Matrix< DDRMat > & aduiujdDOF)
        {
            // get the velocity vector
            Field_Interpolator * tFIVelocity =  mMasterFIManager->get_field_interpolators_for_type( mDofVelocity );

            // get the velocity vector and shape functions
            Matrix< DDRMat > tUvec = tFIVelocity->val();
            Matrix< DDRMat > tNmat = tFIVelocity->N();

            // get number of bases
            uint tNumBases = mMasterFIManager->get_field_interpolators_for_type( mDofVelocity )->get_number_of_space_time_bases();

            // assembly
            // for 2D
            if( tFIVelocity->get_number_of_fields() == 2 )
            {
                // initialize
                aduiujdDOF.set_size( 3, tNumBases * 2, 0.0 );

                // fill
                aduiujdDOF( { 0, 0 }, { 0, tNumBases - 1 } )             = 2.0 * tUvec( 0 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 1, 1 }, { tNumBases, 2 * tNumBases - 1 } ) = 2.0 * tUvec( 1 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 2, 2 }, { 0, tNumBases - 1 } )             = tUvec( 1 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 2, 2 }, { tNumBases, 2 * tNumBases - 1 } ) = tUvec( 0 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
            }
            // for 3D
            else
            {
                // initialize
                aduiujdDOF.set_size( 6, tNumBases * 3, 0.0 );

                // fill
                aduiujdDOF( { 0, 0 }, { 0, tNumBases - 1 } )                 = 2.0 * tUvec( 0 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 1, 1 }, { tNumBases, 2 * tNumBases - 1 } )     = 2.0 * tUvec( 1 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 2, 2 }, { 2 * tNumBases, 3 * tNumBases - 1 } ) = 2.0 * tUvec( 2 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );

                aduiujdDOF( { 4, 4 }, { tNumBases, 2 * tNumBases - 1 } )     = tUvec( 2 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 4, 4 }, { 2 * tNumBases, 3 * tNumBases - 1 } ) = tUvec( 1 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );

                aduiujdDOF( { 4, 4 }, { 0, tNumBases - 1 } )                 = tUvec( 2 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 4, 4 }, { 2 * tNumBases, 3 * tNumBases - 1 } ) = tUvec( 0 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );

                aduiujdDOF( { 5, 5 }, { 0, tNumBases - 1 } )                 = tUvec( 1 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
                aduiujdDOF( { 5, 5 }, { tNumBases, 2 * tNumBases - 1 } )     = tUvec( 0 ) * tNmat( { 0, 0 }, { 0, tNumBases - 1 } );
            }
        }

        //------------------------------------------------------------------------------

        void IWG_Compressible_NS_Advective_Momentum_Flux_Boundary::compute_normal_matrix( Matrix< DDRMat > & aNormalMatrix )
        {
            // assembly into matrix
            // for 2D
            if( mMasterFIManager->get_field_interpolators_for_type( mDofVelocity )->get_number_of_fields() == 2 )
            {
                aNormalMatrix = {
                        { mNormal( 0 ),         0.0 , mNormal( 1 ) },
                        {         0.0 , mNormal( 1 ), mNormal( 0 ) } };
            }
            // for 3D
            else
            {
                aNormalMatrix = {
                        { mNormal( 0 ),         0.0 ,         0.0 ,         0.0 , mNormal( 2 ), mNormal( 1 ) },
                        {         0.0 , mNormal( 1 ),         0.0 , mNormal( 2 ),         0.0 , mNormal( 0 ) },
                        {         0.0 ,         0.0 , mNormal( 2 ), mNormal( 1 ), mNormal( 0 ),         0.0  } };
            }
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
