/*
 * cl_CM_Fluid_Compressible_Van_der_Waals.cpp
 *
 *  Created on: Jul 25, 2020
 *  Author: wunsch
 */

#include "cl_FEM_CM_Fluid_Compressible_Van_der_Waals.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

#include "fn_trans.hpp"
#include "fn_norm.hpp"
#include "fn_eye.hpp"
#include "op_minus.hpp"

namespace moris
{
    namespace fem
    {

        //--------------------------------------------------------------------------------------------------------------

        CM_Fluid_Compressible_Van_der_Waals::CM_Fluid_Compressible_Van_der_Waals()
        {
            // set the property pointer cell size
            mProperties.resize( static_cast< uint >( CM_Property_Type::MAX_ENUM ), nullptr );

            // populate the map
            mPropertyMap[ "IsochoricHeatCapacity" ]  = CM_Property_Type::ISOCHORIC_HEAT_CAPACITY; // constant property
            mPropertyMap[ "SpecificGasConstant" ]    = CM_Property_Type::SPECIFIC_GAS_CONSTANT;   // constant property
            mPropertyMap[ "DynamicViscosity" ]       = CM_Property_Type::DYNAMIC_VISCOSITY;       // may be a fnct. of T
            mPropertyMap[ "ThermalConductivity" ]    = CM_Property_Type::THERMAL_CONDUCTIVITY;    // may be a fnct. of T
            mPropertyMap[ "CapillarityCoefficient" ] = CM_Property_Type::CAPILLARITY_COEFFICIENT; // constant property
            mPropertyMap[ "FirstVdWconstant" ]       = CM_Property_Type::FIRST_VDW_CONSTANT;      // constant property
            mPropertyMap[ "SecondVdWconstant" ]      = CM_Property_Type::SECOND_VDW_CONSTANT;     // constant property
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::set_function_pointers()
        {
            switch ( mSpaceDim )
            {
                case ( 2 ):
                        {
                    m_eval_strain            = &CM_Fluid_Compressible_Van_der_Waals::eval_strain_2d;
                    m_eval_teststrain        = &CM_Fluid_Compressible_Van_der_Waals::eval_teststrain_2d;
                    m_eval_densitystrain     = &CM_Fluid_Compressible_Van_der_Waals::eval_densitystrain_2d;
                    m_eval_densitystraindof  = &CM_Fluid_Compressible_Van_der_Waals::eval_densitystraindof_2d;
                    m_eval_laplacedensity    = &CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensity_2d;
                    m_eval_laplacedensitydof = &CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensitydof_2d;
                    m_eval_velocitymatrix    = &CM_Fluid_Compressible_Van_der_Waals::eval_velocitymatrix_2d;
                    m_unfold_tensor          = &CM_Fluid_Compressible_Van_der_Waals::unfold_2d;
                    mFlatIdentity = { { 1.0 }, { 1.0 }, { 0.0 } };
                    break;
                        }
                case ( 3 ):
                        {
                    m_eval_strain            = &CM_Fluid_Compressible_Van_der_Waals::eval_strain_3d;
                    m_eval_teststrain        = &CM_Fluid_Compressible_Van_der_Waals::eval_teststrain_3d;
                    m_eval_densitystrain     = &CM_Fluid_Compressible_Van_der_Waals::eval_densitystrain_3d;
                    m_eval_densitystraindof  = &CM_Fluid_Compressible_Van_der_Waals::eval_densitystraindof_3d;
                    m_eval_laplacedensity    = &CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensity_3d;
                    m_eval_laplacedensitydof = &CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensitydof_3d;
                    m_eval_velocitymatrix    = &CM_Fluid_Compressible_Van_der_Waals::eval_velocitymatrix_3d;
                    m_unfold_tensor          = &CM_Fluid_Compressible_Van_der_Waals::unfold_3d;
                    mFlatIdentity = { { 1.0 }, { 1.0 }, { 1.0 }, { 0.0 }, { 0.0 }, { 0.0 } };
                    break;
                        }
                default :
                {
                    MORIS_ERROR( false, "CM_Fluid_Compressible_Van_der_Waals::set_function_pointers - this function is currently unused, might be used in the future." );
                    break;
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::set_dof_type_list(
                moris::Cell< moris::Cell< MSI::Dof_Type > > aDofTypes,
                moris::Cell< std::string >                  aDofStrings )
        {
            // set dof type list
            Constitutive_Model::set_dof_type_list( aDofTypes );

            // loop over the provided dof type
            for( uint iDof = 0; iDof < aDofTypes.size(); iDof++ )
            {
                // get dof type string
                std::string tDofString = aDofStrings( iDof );

                // get dof type
                MSI::Dof_Type tDofType = aDofTypes( iDof )( 0 );

                // switch on dof type string
                if( tDofString == "Velocity" )
                {
                    mDofVelocity = tDofType;
                }
                else if( tDofString == "Density" )
                {
                    mDofDensity = tDofType;
                }
                else if( tDofString == "Temperature" )
                {
                    mDofTemperature = tDofType;
                }
                else
                {
                    std::string tErrMsg =
                            std::string( "CM_Fluid_Compressible_Van_der_Waals::set_dof_type_list - Unknown aDofString : ") +
                            tDofString;
                    MORIS_ERROR( false , tErrMsg.c_str() );
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::set_property(
                std::shared_ptr< fem::Property > aProperty,
                std::string                      aPropertyString )
        {
            // check that aPropertyString makes sense
            if ( mPropertyMap.find( aPropertyString ) == mPropertyMap.end() )
            {
                std::string tErrMsg =
                        std::string( "CM_Fluid_Compressible_Van_der_Waals::set_property - Unknown aPropertyString : ") +
                        aPropertyString;

                MORIS_ERROR( false , tErrMsg.c_str() );
            }

            // set the property in the property cell
            mProperties( static_cast< uint >( mPropertyMap[ aPropertyString ] ) ) = aProperty;
        }

        //--------------------------------------------------------------------------------------------------------------

        std::shared_ptr< Property > CM_Fluid_Compressible_Van_der_Waals::get_property(
                std::string aPropertyString )
        {
            // check that aPropertyString makes sense
            if ( mPropertyMap.find( aPropertyString ) == mPropertyMap.end() )
            {
                std::string tErrMsg =
                        std::string( "CM_Fluid_Compressible_Van_der_Waals::get_property - Unknown aPropertyString : ") +
                        aPropertyString;

                MORIS_ERROR( false , tErrMsg.c_str() );
            }

            // get the property in the property cell
            return  mProperties( static_cast< uint >( mPropertyMap[ aPropertyString ] ) );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_flux()
        {
            // get the velocity
            Matrix< DDRMat > tVelocity =  mFIManager->get_field_interpolators_for_type( mDofVelocity )->val();

            // evaluate the thermal flux
            this->eval_thermal_flux();

            // evaluate the velocity matrix
            this->eval_velocityMatrix();

            // compute contribution
            mFlux = mVelocityMatrix * this->stress() -
                    this->Energy() * tVelocity -
                    mThermalFlux;
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_dFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // get the velocity
            Field_Interpolator * tFIVelocity  =  mFIManager->get_field_interpolators_for_type( mDofVelocity );

            // evaluate the velocity matrix
            this->eval_velocityMatrix();

            // unfold the flattened stress tensor
            Matrix< DDRMat > tStressTensor;
            this->unfold( this->stress() , tStressTensor );


            // initialize the matrix
            mdFluxdDof( tDofIndex ).set_size( mSpaceDim,
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->
                    get_number_of_space_time_coefficients(), 0.0 );

            // direct dependency on the density dof type
            if( aDofTypes( 0 ) == mDofDensity )
            {
                // evaluate thermal flux DoF derivative
                this->eval_thermal_dFluxdDOF( aDofTypes );

                // compute contribution
                mdFluxdDof( tDofIndex ).matrix_data() +=
                        mVelocityMatrix * this->dStressdDOF( aDofTypes ) -
                        this->dEnergydDOF( aDofTypes ) * tFIVelocity->val() -
                        mThermalFluxDof;
            }

            // direct dependency on the velocity dof type
            if( aDofTypes( 0 ) == mDofVelocity )
            {
                // compute contribution
                mdFluxdDof( tDofIndex ).matrix_data() +=
                        mVelocityMatrix * this->dStressdDOF( aDofTypes ) +
                        tStressTensor * tFIVelocity->dnNdxn( 1 ) -
                        this->dEnergydDOF( aDofTypes ) * tFIVelocity->val() -
                        this->Energy() * tFIVelocity->dnNdxn( 1 );
            }

            // direct dependency on the temperature dof type
            if( aDofTypes( 0 ) == mDofTemperature )
            {
                // evaluate thermal flux DoF derivative
                this->eval_thermal_dFluxdDOF( aDofTypes );

                // compute contribution
                mdFluxdDof( tDofIndex ).matrix_data() +=
                        mVelocityMatrix * this->dStressdDOF( aDofTypes ) -
                        this->dEnergydDOF( aDofTypes ) * tFIVelocity->val() -
                        mThermalFluxDof;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_thermal_flux()
        {
            // get field interpolators
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );
            std::shared_ptr< Property > tPropThermalConductivity = get_property( "ThermalConductivity" );

            // compute thermal flux q = - k * grad(T)
            mThermalFlux = tPropCapillarityCoefficient->val() * tFIDensity->val() * tFIVelocity->div() * tFIDensity->gradx( 1 ) -
                    tPropThermalConductivity->val() * tFITemp->gradx( 1 );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_thermal_dFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get field interpolators
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );
            std::shared_ptr< Property > tPropThermalConductivity = get_property( "ThermalConductivity" );

            // initialize the matrix
            mThermalFluxDof.set_size( mSpaceDim,
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->
                    get_number_of_space_time_coefficients(), 0.0 );

            // direct dependency on the density dof type
            if( aDofTypes( 0 ) == mDofDensity )
            {
                // compute contribution
                mThermalFluxDof.matrix_data() +=
                        tPropCapillarityCoefficient->val() * tFIVelocity->div() * tFIDensity->gradx( 1 ) * tFIDensity->N() +
                        tPropCapillarityCoefficient->val() * tFIVelocity->div() * tFIDensity->val() * tFIDensity->dnNdxn( 1 ) ;
            }

            // direct dependency on the velocity dof type
            if( aDofTypes( 0 ) == mDofVelocity )
            {
                // compute contribution
                mThermalFluxDof.matrix_data() +=
                        tPropCapillarityCoefficient->val() * tFIDensity->val() * tFIDensity->gradx( 1 ) * tFIVelocity->div_operator();
            }

            // direct dependency on the temperature dof type
            if( aDofTypes( 0 ) == mDofTemperature )
            {
                // compute contribution
                mThermalFluxDof.matrix_data() +=
                        -1.0 * tPropThermalConductivity->val() * tFITemp->dnNdxn( 1 );
            }

            // if indirect dependency of capillarity
            if ( tPropCapillarityCoefficient->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mThermalFluxDof.matrix_data() +=
                        tFIDensity->val() * tFIVelocity->div() * tFIDensity->gradx( 1 ) *
                        tPropCapillarityCoefficient->dPropdDOF( aDofTypes );
            }

            // if indirect dependency of conductivity on the dof type
            if ( tPropThermalConductivity->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mThermalFluxDof.matrix_data() +=
                        -1.0 * tFITemp->gradx( 1 ) * tPropThermalConductivity->dPropdDOF( aDofTypes );
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_Energy()
        {
            // get field interpolators
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropIsochoricHeatCapacity = get_property( "IsochoricHeatCapacity" );
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );
            std::shared_ptr< Property > tPropFirstVdWconstant = get_property( "FirstVdWconstant" );

            // compute energy
            mEnergy = tPropIsochoricHeatCapacity->val() * tFIDensity->val() * tFITemp->val() -
                    tPropFirstVdWconstant->val() * tFIDensity->val() * tFIDensity->val() +
                    0.5 * tFIDensity->val() * trans( tFIVelocity->val() ) * tFIVelocity->val() +
                    0.5 * tPropCapillarityCoefficient->val() * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->gradx( 1 );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_dEnergydDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // get the FIs
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropIsochoricHeatCapacity = get_property( "IsochoricHeatCapacity" );
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );
            std::shared_ptr< Property > tPropFirstVdWconstant = get_property( "FirstVdWconstant" );

            // initialize the matrix
            mEnergyDof( tDofIndex ).set_size( 1,
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->
                    get_number_of_space_time_coefficients(), 0.0 );

            // direct dependency on the density dof type
            if( aDofTypes( 0 ) == mDofDensity )
            {
                // compute contribution
                mEnergyDof( tDofIndex ).matrix_data() +=
                        tPropIsochoricHeatCapacity->val() * tFITemp->val() * tFIDensity->N() +
                        0.5 * trans( tFIVelocity->val() ) * tFIVelocity->val() * tFIDensity->N() -
                        2.0 * tPropFirstVdWconstant->val() * tFIDensity->val() * tFIDensity->N() -
                        tPropCapillarityCoefficient->val() * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->dnNdxn( 1 );
            }

            // direct dependency on the velocity dof type
            if( aDofTypes( 0 ) == mDofVelocity )
            {
                // compute contribution
                mEnergyDof( tDofIndex ).matrix_data() +=
                        tFIDensity->val() * trans( tFIVelocity->val() ) * tFIVelocity->N();
            }

            // direct dependency on the temperature dof type
            if( aDofTypes( 0 ) == mDofTemperature )
            {
                // compute contribution
                mEnergyDof( tDofIndex ).matrix_data() +=
                        tPropIsochoricHeatCapacity->val() * tFIDensity->val() * tFITemp->N();
            }

            // if indirect dependency of viscosity
            if ( tPropCapillarityCoefficient->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mEnergyDof( tDofIndex ).matrix_data() +=
                        0.5 * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->gradx( 1 ) *
                        tPropCapillarityCoefficient->dPropdDOF( aDofTypes );
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_EnergyDot()
        {
            // get field interpolators
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropIsochoricHeatCapacity = get_property( "IsochoricHeatCapacity" );
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );
            std::shared_ptr< Property > tPropFirstVdWconstant = get_property( "FirstVdWconstant" );

            // compute total energy density
            mEnergyDot =
                    tPropIsochoricHeatCapacity->val() * tFIDensity->val() * tFITemp->gradt( 1 ) +
                    tPropIsochoricHeatCapacity->val() * tFITemp->val() * tFIDensity->gradt( 1 ) +
                    2.0 * tPropFirstVdWconstant->val() * tFIDensity->val() * tFIDensity->gradt( 1 ) +
                    0.5 * trans( tFIVelocity->val() ) * tFIVelocity->val() * tFIDensity->gradt( 1 ) +
                    tFIDensity->val() * trans( tFIVelocity->val() ) * tFIVelocity->gradt( 1 ) +
                    tPropCapillarityCoefficient->val() * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->gradxt();
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_dEnergyDotdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // get the FIs
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropIsochoricHeatCapacity = get_property( "IsochoricHeatCapacity" );
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );
            std::shared_ptr< Property > tPropFirstVdWconstant = get_property( "FirstVdWconstant" );

            // initialize the matrix
            mEnergyDotDof( tDofIndex ).set_size( 1,
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->
                    get_number_of_space_time_coefficients(), 0.0 );

            // direct dependency on the density dof type
            if( aDofTypes( 0 ) == mDofDensity )
            {
                // compute contribution
                mEnergyDotDof( tDofIndex ).matrix_data() +=
                        tPropIsochoricHeatCapacity->val() * tFITemp->val() * tFIDensity->dnNdtn( 1 ) +
                        tPropIsochoricHeatCapacity->val() * tFITemp->gradt( 1 ) * tFIDensity->N() -
                        2.0 *  tPropFirstVdWconstant->val() * tFIDensity->val() * tFIDensity->dnNdtn( 1 ) -
                        2.0 *  tPropFirstVdWconstant->val() * tFIDensity->gradt( 1 ) * tFIDensity->N() +
                        0.5 * trans( tFIVelocity->val() ) * tFIVelocity->val() * tFIDensity->dnNdtn( 1 ) +
                        trans( tFIVelocity->val() ) * tFIVelocity->gradt( 1 ) * tFIDensity->N() +
                        tPropCapillarityCoefficient->val() * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->d2Ndxt() +
                        tPropCapillarityCoefficient->val() * trans( tFIDensity->gradxt() ) * tFIDensity->dnNdxn( 1 ) ;
            }

            // direct dependency on the velocity dof type
            if( aDofTypes( 0 ) == mDofVelocity )
            {
                // compute contribution
                mEnergyDotDof( tDofIndex ).matrix_data() +=
                        tFIDensity->gradt( 1 ) * trans( tFIVelocity->val() ) * tFIVelocity->N() +
                        tFIDensity->val() * trans( tFIVelocity->val() ) * tFIVelocity->d2Ndxt() +
                        tFIDensity->val() * trans( tFIVelocity->gradt( 1 ) ) * tFIVelocity->dnNdxn( 1 ) ;
            }

            // direct dependency on the temperature dof type
            if( aDofTypes( 0 ) == mDofTemperature )
            {
                // compute contribution
                mEnergyDotDof( tDofIndex ).matrix_data() +=
                        tPropIsochoricHeatCapacity->val() * tFIDensity->val() * tFITemp->dnNdtn( 1 ) +
                        tPropIsochoricHeatCapacity->val() * tFIDensity->gradt( 1 ) * tFITemp->N() ;
            }

            // if indirect dependency of viscosity
            if ( tPropCapillarityCoefficient->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mEnergyDotDof( tDofIndex ).matrix_data() +=
                        trans( tFIDensity->gradx( 1 ) ) * tFIDensity->gradxt() *
                        tPropCapillarityCoefficient->dPropdDOF( aDofTypes );
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_stress()
        {
            // get the FIs
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );

            // get the properties
            std::shared_ptr< Property > tPropDynamicViscosity = get_property( "DynamicViscosity" );
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );

            // evaluate density strain
            this->eval_densityStrain();

            // evaluate Laplace(density)
            this->eval_laplaceDensity();

            // compute Stress
            mStress = 2.0 * tPropDynamicViscosity->val() *
                    ( this->strain() -  ( 1 / 3 ) * tFIVelocity->div() * mFlatIdentity  ) +
                    ( tFIDensity->val() * tFIDensity->val() * mLaplaceDensity +
                            0.5 * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->gradx( 1 ) ) *
                    tPropCapillarityCoefficient->val() * mFlatIdentity -
                    tPropCapillarityCoefficient->val() * mDensityStrain -
                    this->pressure() * mFlatIdentity ;
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_dStressdDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // get the FIs
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFIVelocity = mFIManager->get_field_interpolators_for_type( mDofVelocity );

            // get the properties
            std::shared_ptr< Property > tPropDynamicViscosity = get_property( "DynamicViscosity" );
            std::shared_ptr< Property > tPropCapillarityCoefficient = get_property( "CapillarityCoefficient" );

            // initialize the matrix
            mdStressdDof( tDofIndex ).set_size( ( mSpaceDim - 1 ) * 3,
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) )->
                    get_number_of_space_time_coefficients(), 0.0 );

            // direct dependency on the density dof type
            if( aDofTypes( 0 ) == mDofDensity )
            {
                // evaluate the (dof derivs of the) laplacian of the density
                this->eval_laplaceDensity();
                this->eval_laplaceDensityDof();

                // evaluate the dof deriv of the density strain
                this->eval_densityStrainDof();

                // compute contribution
                mdStressdDof( tDofIndex ).matrix_data() +=
                        mFlatIdentity * this->dPressuredDOF( aDofTypes ) +
                        tFIDensity->val() * mFlatIdentity * mLaplaceDensityDof +
                        mLaplaceDensity * mFlatIdentity * tFIDensity->N() +
                        mFlatIdentity * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->dnNdxn( 1 ) -
                        mLaplaceDensityDof;
            }

            // direct dependency on the velocity dof type
            if( aDofTypes( 0 ) == mDofVelocity )
            {
                // compute contribution
                mdStressdDof( tDofIndex ).matrix_data() +=
                        2.0 * tPropDynamicViscosity->val() *
                        ( this->dStraindDOF( aDofTypes ) - ( 1 / 3 ) * mFlatIdentity * tFIVelocity->div_operator() );
            }

            // direct dependency on the temperature dof type
            if( aDofTypes( 0 ) == mDofTemperature )
            {
                // compute contribution
                mdStressdDof( tDofIndex ).matrix_data() +=
                        mFlatIdentity * this->dPressuredDOF( aDofTypes );
            }

            // if indirect dependency of viscosity
            if ( tPropDynamicViscosity->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mdStressdDof( tDofIndex ).matrix_data() +=
                        2.0 * ( this->strain() - ( 1 / 3 ) * tFIVelocity->div() * mFlatIdentity ) *
                        tPropDynamicViscosity->dPropdDOF( aDofTypes );
            }

            // if indirect dependency of capillarity
            if ( tPropCapillarityCoefficient->check_dof_dependency( aDofTypes ) )
            {
                // compute derivative with indirect dependency through properties
                mdStressdDof( tDofIndex ).matrix_data() +=
                        ( tFIDensity->val() * tFIDensity->val() * mLaplaceDensity +
                                0.5 * trans( tFIDensity->gradx( 1 ) ) * tFIDensity->gradx( 1 ) ) *
                        mFlatIdentity  * tPropCapillarityCoefficient->dPropdDOF( aDofTypes ) -
                        mDensityStrain * tPropCapillarityCoefficient->dPropdDOF( aDofTypes ) ;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_dTractiondDOF(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                const Matrix< DDRMat >             & aNormal )
        {
            // Function is not implemented
            MORIS_ERROR( false, "CM_Fluid_Compressible_Van_der_Waals::eval_dTractiondDOF - not implemented yet." );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_traction( const Matrix< DDRMat > & aNormal )
        {
            // Function is not implemented
            MORIS_ERROR( false, "CM_Fluid_Compressible_Van_der_Waals::eval_traction - not implemented yet." );
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix< DDRMat > & CM_Fluid_Compressible_Van_der_Waals::pressure()
        {
            // get field interpolators
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropSpecificGasConstant = get_property( "SpecificGasConstant" );
            std::shared_ptr< Property > tPropFirstVdWconstant = get_property( "FirstVdWconstant" );
            std::shared_ptr< Property > tPropSecondVdWconstant = get_property( "SecondVdWconstant" );

            // return the pressure
            mPressure = ( 1 / tPropSecondVdWconstant->val()( 0 ) - tFIDensity->val()( 0 ) ) *
                    tPropSpecificGasConstant->val() * tPropSecondVdWconstant->val() * tFIDensity->val() * tFITemp->val() -
                    std::pow( tFIDensity->val()( 0 ), 2.0 ) * tPropFirstVdWconstant->val();

            return mPressure;
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix< DDRMat > & CM_Fluid_Compressible_Van_der_Waals::dPressuredDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get field interpolators
            Field_Interpolator * tFIDensity = mFIManager->get_field_interpolators_for_type( mDofDensity );
            Field_Interpolator * tFITemp = mFIManager->get_field_interpolators_for_type( mDofTemperature );

            // get the properties
            std::shared_ptr< Property > tPropSpecificGasConstant = get_property( "SpecificGasConstant" );
            std::shared_ptr< Property > tPropFirstVdWconstant = get_property( "FirstVdWconstant" );
            std::shared_ptr< Property > tPropSecondVdWconstant = get_property( "SecondVdWconstant" );

            // if Density DoF
            if( aDofTypes( 0 ) == mDofDensity )
            {
                mPressureDof = ( std::pow( tPropSecondVdWconstant->val()( 0 ), 2.0 ) /
                        std::pow( tPropSecondVdWconstant->val()( 0 ) - tFIDensity->val()( 0 ), 2.0 ) ) *
                        tPropSpecificGasConstant->val() * tFITemp->val() * tFIDensity->N() -
                        2.0 * tPropFirstVdWconstant->val() * tFIDensity->val() * tFIDensity->N();
            }

            // if Temperature DoF
            if( aDofTypes( 0 ) == mDofTemperature )
            {
                // compute derivative with direct dependency
                mPressureDof = ( 1 / tPropSecondVdWconstant->val()( 0 ) - tFIDensity->val()( 0 ) ) *
                        tPropSpecificGasConstant->val() * tPropSecondVdWconstant->val() * tFIDensity->val() * tFITemp->N();
            }

            // return the pressure DoF deriv
            return mPressureDof;
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_strain_2d()
        {
            // get the velocity spatial gradient from velocity FI
            Matrix< DDRMat > tVelocityGradx = mFIManager->get_field_interpolators_for_type( mDofVelocity )->gradx( 1 );

            // evaluate the strain
            mStrain.set_size( 3, 1, 0.0 );
            mStrain( 0, 0 ) = tVelocityGradx( 0, 0 );
            mStrain( 1, 0 ) = tVelocityGradx( 1, 1 );
            mStrain( 2, 0 ) = 0.5 * ( tVelocityGradx( 1, 0 ) + tVelocityGradx( 0, 1 ) );
        }

        void CM_Fluid_Compressible_Van_der_Waals::eval_strain_3d()
        {
            // get the velocity spatial gradient from velocity FI
            Matrix< DDRMat > tVelocityGradx = mFIManager->get_field_interpolators_for_type( mDofVelocity )->gradx( 1 );

            // evaluate the strain
            mStrain.set_size( 6, 1, 0.0 );
            mStrain( 0, 0 ) = tVelocityGradx( 0, 0 );
            mStrain( 1, 0 ) = tVelocityGradx( 1, 1 );
            mStrain( 2, 0 ) = tVelocityGradx( 2, 2 );
            mStrain( 3, 0 ) = 0.5 * ( tVelocityGradx( 1, 2 ) + tVelocityGradx( 2, 1 ) );
            mStrain( 4, 0 ) = 0.5 * ( tVelocityGradx( 0, 2 ) + tVelocityGradx( 2, 0 ) );
            mStrain( 5, 0 ) = 0.5 * ( tVelocityGradx( 0, 1 ) + tVelocityGradx( 1, 0 ) );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_densitystrain_2d()
        {
            // get the density gradient from density FI
            Matrix< DDRMat > tDensityGradx = mFIManager->get_field_interpolators_for_type( mDofDensity)->gradx( 1 );

            // evaluate the strain
            mDensityStrain.set_size( 3, 1, 0.0 );
            mDensityStrain( 0, 0 ) = std::pow( tDensityGradx( 0 ), 2.0 );
            mDensityStrain( 1, 0 ) = std::pow( tDensityGradx( 1 ), 2.0 );
            mDensityStrain( 2, 0 ) = tDensityGradx( 0 ) * tDensityGradx( 1 ) ;
        }

        void CM_Fluid_Compressible_Van_der_Waals::eval_densitystrain_3d()
        {
            // get the density spatial gradient from density FI
            Matrix< DDRMat > tDensityGradx = mFIManager->get_field_interpolators_for_type( mDofDensity )->gradx( 1 );

            // evaluate the strain
            mDensityStrain.set_size( 6, 1, 0.0 );
            mDensityStrain( 0, 0 ) = std::pow( tDensityGradx( 0 ), 2.0 );
            mDensityStrain( 1, 0 ) = std::pow( tDensityGradx( 1 ), 2.0 );
            mDensityStrain( 2, 0 ) = std::pow( tDensityGradx( 2 ), 2.0 );
            mDensityStrain( 3, 0 ) = tDensityGradx( 1 ) * tDensityGradx( 2 );
            mDensityStrain( 4, 0 ) = tDensityGradx( 0 ) * tDensityGradx( 2 );
            mDensityStrain( 5, 0 ) = tDensityGradx( 0 ) * tDensityGradx( 1 );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_densitystraindof_2d()
        {
            // get the density gradient and shape function derivatives from density FI
            Matrix< DDRMat > tNx = mFIManager->get_field_interpolators_for_type( mDofDensity)->dnNdxn( 1 );
            Matrix< DDRMat > tGradX = mFIManager->get_field_interpolators_for_type( mDofDensity)->gradx( 1 );

            // get number of bases
            uint tNumBases = mFIManager->get_field_interpolators_for_type( mDofDensity )->get_number_of_space_time_bases();

            // evaluate the strain
            mDensityStrainDof.set_size( 3, tNumBases, 0.0 );
            mDensityStrainDof( { 0, 0 }, { 0, tNumBases - 1 } ) = tGradX( 0 ) * tNx( { 0, 0 }, { 0, tNumBases - 1 } );
            mDensityStrainDof( { 1, 1 }, { 0, tNumBases - 1 } ) = tGradX( 1 ) * tNx( { 1, 1 }, { 0, tNumBases - 1 } );
            mDensityStrainDof( { 2, 2 }, { 0, tNumBases - 1 } ) = 0.5 * (
                    tGradX( 1 ) * tNx( { 0, 0 }, { 0, tNumBases - 1 } ) +
                    tGradX( 0 ) * tNx( { 1, 1 }, { 0, tNumBases - 1 } ) );
        }

        void CM_Fluid_Compressible_Van_der_Waals::eval_densitystraindof_3d()
        {
            // get the density gradient and shape function derivatives from density FI
            Matrix< DDRMat > tNx = mFIManager->get_field_interpolators_for_type( mDofDensity)->dnNdxn( 1 );
            Matrix< DDRMat > tGradX = mFIManager->get_field_interpolators_for_type( mDofDensity)->gradx( 1 );

            // get number of bases
            uint tNumBases = mFIManager->get_field_interpolators_for_type( mDofDensity )->get_number_of_space_time_bases();

            // evaluate the strain
            mDensityStrainDof.set_size( 6, tNumBases, 0.0 );
            mDensityStrainDof( { 0, 0 }, { 0, tNumBases - 1 } ) = tGradX( 0 ) * tNx( { 0, 0 }, { 0, tNumBases - 1 } );
            mDensityStrainDof( { 1, 1 }, { 0, tNumBases - 1 } ) = tGradX( 1 ) * tNx( { 1, 1 }, { 0, tNumBases - 1 } );
            mDensityStrainDof( { 2, 2 }, { 0, tNumBases - 1 } ) = tGradX( 2 ) * tNx( { 2, 2 }, { 0, tNumBases - 1 } );

            mDensityStrainDof( { 3, 3 }, { 0, tNumBases - 1 } ) = 0.5 * (
                    tGradX( 1 ) * tNx( { 2, 2 }, { 0, tNumBases - 1 } ) +
                    tGradX( 2 ) * tNx( { 1, 1 }, { 0, tNumBases - 1 } ) );
            mDensityStrainDof( { 4, 4 }, { 0, tNumBases - 1 } ) = 0.5 * (
                    tGradX( 0 ) * tNx( { 2, 2 }, { 0, tNumBases - 1 } ) +
                    tGradX( 2 ) * tNx( { 0, 0 }, { 0, tNumBases - 1 } ) );
            mDensityStrainDof( { 5, 5 }, { 0, tNumBases - 1 } ) = 0.5 * (
                    tGradX( 1 ) * tNx( { 0, 0 }, { 0, tNumBases - 1 } ) +
                    tGradX( 0 ) * tNx( { 1, 1 }, { 0, tNumBases - 1 } ) );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_teststrain_2d()
        {
            // compute displacement gradient
            Matrix< DDRMat > tdnNdxn =  mFIManager->get_field_interpolators_for_type( mDofVelocity )->dnNdxn( 1 );

            // get number of bases for velocity in one spatial dimension
            uint tNumBases = mFIManager->get_field_interpolators_for_type( mDofVelocity )->get_number_of_space_time_bases();

            // build the test strain
            mTestStrain.set_size( 3, tNumBases * 2, 0.0 );
            mTestStrain( { 0, 0 }, { 0, tNumBases - 1 } ) = tdnNdxn( { 0, 0 }, { 0, tNumBases - 1 } );
            mTestStrain( { 2, 2 }, { 0, tNumBases - 1 } ) = 0.5 * tdnNdxn( { 1, 1 }, { 0, tNumBases - 1 } );

            mTestStrain( { 1, 1 }, { tNumBases, 2 * tNumBases - 1 } ) = tdnNdxn( { 1, 1 }, { 0, tNumBases - 1 } );
            mTestStrain( { 2, 2 }, { tNumBases, 2 * tNumBases - 1 } ) = 0.5 * tdnNdxn( { 0, 0 }, { 0, tNumBases - 1 } );
        }

        void CM_Fluid_Compressible_Van_der_Waals::eval_teststrain_3d()
        {
            // compute displacement gradient
            Matrix< DDRMat > tdnNdxn =
                    mFIManager->get_field_interpolators_for_type( mDofVelocity )->dnNdxn( 1 );

            // get number of bases for velocity in one spatial dimension
            uint tNumBases = mFIManager->get_field_interpolators_for_type( mDofVelocity )->get_number_of_space_time_bases();

            // build the test strain
            mTestStrain.set_size( 6, tNumBases * 3, 0.0 );
            mTestStrain( { 0, 0 }, { 0, tNumBases - 1 } ) = tdnNdxn( { 0, 0 }, { 0, tNumBases - 1 } );
            mTestStrain( { 4, 4 }, { 0, tNumBases - 1 } ) = 0.5 * tdnNdxn( { 2, 2 }, { 0, tNumBases - 1 } );
            mTestStrain( { 5, 5 }, { 0, tNumBases - 1 } ) = 0.5 * tdnNdxn( { 1, 1 }, { 0, tNumBases - 1 } );

            mTestStrain( { 1, 1 }, { tNumBases, 2 * tNumBases - 1 } ) = tdnNdxn( { 1, 1 }, { 0, tNumBases - 1 } );
            mTestStrain( { 3, 3 }, { tNumBases, 2 * tNumBases - 1 } ) = 0.5 * tdnNdxn( { 2, 2 }, { 0, tNumBases - 1 } );
            mTestStrain( { 5, 5 }, { tNumBases, 2 * tNumBases - 1 } ) = 0.5 * tdnNdxn( { 0, 0 }, { 0, tNumBases - 1 } );

            mTestStrain( { 2, 2 }, { 2 * tNumBases, 3 * tNumBases - 1 } ) = tdnNdxn( { 2, 2 }, { 0, tNumBases - 1 } );
            mTestStrain( { 3, 3 }, { 2 * tNumBases, 3 * tNumBases - 1 } ) = 0.5 * tdnNdxn( { 1, 1 }, { 0, tNumBases - 1 } );
            mTestStrain( { 4, 4 }, { 2 * tNumBases, 3 * tNumBases - 1 } ) = 0.5 * tdnNdxn( { 0, 0 }, { 0, tNumBases - 1 } );
        }

        //--------------------------------------------------------------------------------------------------------------

        void CM_Fluid_Compressible_Van_der_Waals::eval_dStraindDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type as a uint
            uint tDofType = static_cast< uint >( aDofTypes( 0 ) );

            // get the dof FI
            Field_Interpolator * tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get the dof type index
            uint tDofIndex = mGlobalDofTypeMap( tDofType );

            // init mdStraindDof
            mdStraindDof( tDofIndex ).set_size( ( mSpaceDim - 1 ) * 3, tFI->get_number_of_space_time_coefficients(), 0.0 );

            // if velocity dof
            if( aDofTypes( 0 ) == mDofVelocity )
            {
                // compute derivative
                mdStraindDof( tDofIndex ).matrix_data() += this->testStrain().matrix_data();
            }
        }

        //--------------------------------------------------------------------------------------------------------------
        void CM_Fluid_Compressible_Van_der_Waals::eval_velocitymatrix_2d()
        {
            // get velocity vector
            Matrix< DDRMat > tU = mFIManager->get_field_interpolators_for_type( mDofVelocity )->val();

            // assemble matrix
            mVelocityMatrix = {
                    { tU( 0 ),    0.0 , tU( 1 ) },
                    {    0.0 , tU( 1 ), tU( 0 ) } };
        }

        void CM_Fluid_Compressible_Van_der_Waals::eval_velocitymatrix_3d()
        {
            // get velocity vector
            Matrix< DDRMat > tU = mFIManager->get_field_interpolators_for_type( mDofVelocity )->val();

            // assemble matrix
            mVelocityMatrix = {
                    { tU( 0 ),    0.0 ,  0.0 ,    0.0 , tU( 2 ), tU( 1 ) },
                    {    0.0 , tU( 1 ),  0.0 , tU( 2 ),    0.0 , tU( 0 ) },
                    {    0.0 ,  0.0 , tU( 2 ), tU( 1 ), tU( 0 ),    0.0  } };
        }

        //--------------------------------------------------------------------------------------------------------------
        void CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensity_2d()
        {
            // get second derivatives for density
            Matrix< DDRMat > td2Rhodx2 = mFIManager->get_field_interpolators_for_type( mDofDensity )->gradx( 2 );

            // do summation of Laplace operator
            mLaplaceDensity = {{ td2Rhodx2(0) + td2Rhodx2(1) }};
        }

        void CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensity_3d()
        {
            // get second derivatives for density
            Matrix< DDRMat > td2Rhodx2 = mFIManager->get_field_interpolators_for_type( mDofDensity )->gradx( 2 );

            // do summation of Laplace operator
            mLaplaceDensity = {{ td2Rhodx2(0) + td2Rhodx2(1) + td2Rhodx2(2) }};
        }

        //--------------------------------------------------------------------------------------------------------------
        void CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensitydof_2d()
        {
            // get second derivatives shape functions of density field
            Matrix< DDRMat > td2Ndx2 = mFIManager->get_field_interpolators_for_type( mDofDensity )->dnNdxn( 2 );

            // get number of bases
            uint tNumBases = mFIManager->get_field_interpolators_for_type( mDofDensity )->get_number_of_space_time_bases();

            // initialize vector
            mLaplaceDensityDof.set_size( 1, tNumBases );

            // FIXME: for-loop = slow?
            // fill vector
            for ( uint iBase = 0; iBase < tNumBases; iBase++ )
            {
                mLaplaceDensityDof( { 0, 0 }, { iBase, iBase } ) = sum( td2Ndx2( { 0, 2 }, { iBase, iBase } ) );
            }
        }

        void CM_Fluid_Compressible_Van_der_Waals::eval_laplacedensitydof_3d()
        {
            // get second derivatives shape functions of density field
            Matrix< DDRMat > td2Ndx2 = mFIManager->get_field_interpolators_for_type( mDofDensity )->dnNdxn( 2 );

            // get number of bases
            uint tNumBases = mFIManager->get_field_interpolators_for_type( mDofDensity )->get_number_of_space_time_bases();

            // initialize vector
            mLaplaceDensityDof.set_size( 1, tNumBases );

            // FIXME: for-loop = slow?
            // fill vector
            for ( uint iBase = 0; iBase < tNumBases; iBase++ )
            {
                mLaplaceDensityDof( { 0, 0 }, { iBase, iBase } ) = sum( td2Ndx2( { 0, 5 }, { iBase, iBase } ) );
            }
        }

        //--------------------------------------------------------------------------------------------------------------
        void CM_Fluid_Compressible_Van_der_Waals::unfold_2d(
                const Matrix< DDRMat > & aFlattenedTensor,
                Matrix< DDRMat > & aExpandedTensor)
        {
            aExpandedTensor = {
                    { aFlattenedTensor( 0 ), aFlattenedTensor( 2 ) },
                    { aFlattenedTensor( 2 ), aFlattenedTensor( 1 ) } };
        }

        void CM_Fluid_Compressible_Van_der_Waals::unfold_3d(
                const Matrix< DDRMat > & aFlattenedTensor,
                Matrix< DDRMat > & aExpandedTensor)
        {
            aExpandedTensor = {
                    { aFlattenedTensor( 0 ), aFlattenedTensor( 5 ), aFlattenedTensor( 4 ) },
                    { aFlattenedTensor( 5 ), aFlattenedTensor( 1 ), aFlattenedTensor( 3 ) },
                    { aFlattenedTensor( 4 ), aFlattenedTensor( 3 ), aFlattenedTensor( 2 ) } };
        }

        //--------------------------------------------------------------------------------------------------------------

    } /* namespace fem */
} /* namespace moris */
