/*
 * cl_FEM_IQI_Max_Stress.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: wunsch
 */
#include "cl_FEM_Set.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_IQI_Max_Stress.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        IQI_Max_Stress::IQI_Max_Stress( enum Stress_Type aStressType )
        {
            // assign stress type to evaluate
            mStressType = aStressType;

            // set IQI type
            mIQIType = vis::Output_Type::MAX_STRESS;

            // set FEM IQI type
            mFEMIQIType = fem::IQI_Type::MAX_STRESS;

            // set the property pointer cell size
            mMasterProp.resize( static_cast< uint >( IQI_Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "ReferenceValue" ]    = IQI_Property_Type::REFERENCE_VALUE;
            mPropertyMap[ "Exponent" ]          = IQI_Property_Type::EXPONENT;

            // set size for the constitutive model pointer cell
            mMasterCM.resize( static_cast< uint >( IQI_Constitutive_Type::MAX_ENUM ), nullptr );

            // populate the constitutive map
            mConstitutiveMap[ "ElastLinIso" ] = IQI_Constitutive_Type::ELAST_LIN_ISO;
        }

        //------------------------------------------------------------------------------

        void IQI_Max_Stress::set_property( std::shared_ptr< Property > aProperty,
                std::string                 aPropertyString,
                mtk::Master_Slave           aIsMaster)
        {
            // can only be master
            MORIS_ERROR( aIsMaster == mtk::Master_Slave::MASTER,
                    "IQI_Max_Von_Mises_Stress::set_property - can only be master." );

            // check that aPropertyString makes sense
            if ( mPropertyMap.find( aPropertyString ) == mPropertyMap.end() )
            {
                std::string tErrMsg =
                        std::string( "CM_Diffusion_Linear_Isotropic_Phase_Change::set_property - Unknown aPropertyString : ") +
                        aPropertyString;

                MORIS_ERROR( false , tErrMsg.c_str() );
            }

            // set the property in the property cell
            this->get_properties( aIsMaster )( static_cast< uint >( mPropertyMap[ aPropertyString ] ) ) = aProperty;
        }

        //------------------------------------------------------------------------------

        void IQI_Max_Stress::set_constitutive_model(
                std::shared_ptr< Constitutive_Model > aConstitutiveModel,
                std::string                           aConstitutiveString,
                mtk::Master_Slave                     aIsMaster)
        {
            // can only be master
            MORIS_ERROR( aIsMaster == mtk::Master_Slave::MASTER,
                    "IQI_Max_Von_Mises_Stress::set_constitutive model - can only be master." );

            // check that aPropertyString makes sense
            if ( mConstitutiveMap.find( aConstitutiveString ) == mConstitutiveMap.end() )
            {
                std::string tErrMsg =
                        std::string( "IQI_Max_Von_Mises_Stress::set_constitutive_model - Unknown aConstitutiveString : ") +
                        aConstitutiveString;

                MORIS_ERROR( false , tErrMsg.c_str() );
            }

            // set the constitutive model in the constitutive model cell
            this->get_constitutive_models( aIsMaster )( static_cast< uint >( mConstitutiveMap[ aConstitutiveString ] ) ) =
                    aConstitutiveModel;
        }

        //------------------------------------------------------------------------------

        void IQI_Max_Stress::compute_QI( Matrix< DDRMat > & aQI )
        {
            // check if dof index was set
            if( ( mMasterDofTypes( 0 ).size() > 1 ) &&
                    ( mStressType != Stress_Type::VON_MISES_STRESS ) )
            {
                MORIS_ERROR( mIQITypeIndex != -1,
                        "IQI_Max_Stress::compute_QI - mIQITypeIndex not set, but is needed for principal, normal and shear stresses." );

                MORIS_ERROR( mIQITypeIndex > 2,
                        "IQI_Max_Stress::compute_QI - mIQITypeIndex out of bounds, must be 0, 1, or 2 for the three spatial dimensions." );
            }

            // initialize stress value
            real tStressValue;

            // switch for different stress types
            switch (mStressType)
            {
            case Stress_Type::VON_MISES_STRESS:
                tStressValue = this->eval_Von_Mises_stress();
                break;

            case Stress_Type::PRINCIPAL_STRESS:
                tStressValue = this->eval_principal_stress( mIQITypeIndex + 1);
                break;

            case Stress_Type::NORMAL_STRESS:
                tStressValue = this->eval_normal_stress( mIQITypeIndex + 1);
                break;

            case Stress_Type::SHEAR_STRESS:
                tStressValue = this->eval_shear_stress( mIQITypeIndex + 1);
                break;

            default:
                MORIS_ERROR( false, "IQI_Max_Stress::compute_QI - Unknown Stress Type." );
            }

            // check if properties set
            MORIS_ERROR(mMasterProp( static_cast< uint >( IQI_Property_Type::REFERENCE_VALUE ) ) != nullptr,
                    "IQI_Max_Dof - no reference value set");

            MORIS_ERROR(mMasterProp( static_cast< uint >( IQI_Property_Type::EXPONENT ) ) != nullptr,
                    "IQI_Max_Dof - no exponent set");

            // get property values
            real tRefValue = mMasterProp( static_cast< uint >( IQI_Property_Type::REFERENCE_VALUE ) )->val()( 0 );
            real tExponent = mMasterProp( static_cast< uint >( IQI_Property_Type::EXPONENT ) )->val()( 0 );

            // evaluate the QI
            aQI = {{ std::pow( 1/tRefValue * tStressValue - 1.0, tExponent ) }};
        }

        //------------------------------------------------------------------------------

        void IQI_Max_Stress::compute_QI( moris::real aWStar )
        {
            // get index for QI
            sint tQIIndex = mSet->get_QI_assembly_index( mName );

            // compute von mises stress value
            real tStressValue = this->eval_Von_Mises_stress();

            // check if properties are set
            MORIS_ERROR(mMasterProp( static_cast< uint >( IQI_Property_Type::REFERENCE_VALUE ) ) != nullptr,
                    "IQI_Max_Dof - no reference value set");

            MORIS_ERROR(mMasterProp( static_cast< uint >( IQI_Property_Type::EXPONENT ) ) != nullptr,
                    "IQI_Max_Dof - no exponent set");

            // get property values
            real tRefValue = mMasterProp( static_cast< uint >( IQI_Property_Type::REFERENCE_VALUE ) )->val()( 0 );
            real tExponent = mMasterProp( static_cast< uint >( IQI_Property_Type::EXPONENT ) )->val()( 0 );

            // evaluate the QI
            mSet->get_QI()( tQIIndex ).matrix_data() += { aWStar *  std::pow( 1/tRefValue * tStressValue - 1.0, tExponent ) };
        }

        //------------------------------------------------------------------------------

        void IQI_Max_Stress::compute_dQIdDof( Matrix< DDRMat > & adQIdDof )
        {
            MORIS_ERROR(0,"IQI_Max_Von_Mises_Stress::compute_dQIdDof - Derivatives of stress wrt dof not implemented");
        }

        //------------------------------------------------------------------------------

        real IQI_Max_Stress::eval_Von_Mises_stress()
        {
            // get standardized stress vector
            Matrix< DDRMat > tStressVector = this->get_stress_vector();

            // compute contributions to von mises stress
            real tNormalStressContribution =
                            std::pow( tStressVector( 0 ) - tStressVector( 1 ), 2.0 ) +
                            std::pow( tStressVector( 1 ) - tStressVector( 2 ), 2.0 ) +
                            std::pow( tStressVector( 2 ) - tStressVector( 0 ), 2.0 );
            real tShearStressContribution =
                            std::pow( tStressVector( 3 ), 2.0 ) +
                            std::pow( tStressVector( 4 ), 2.0 ) +
                            std::pow( tStressVector( 5 ), 2.0 );

            // compute von mises stress value
            return std::sqrt( 0.5 * tNormalStressContribution + 3.0 * tShearStressContribution );
        }

        //------------------------------------------------------------------------------

        real IQI_Max_Stress::eval_principal_stress( uint aPrincipalStressIndex )
        {
            // get stress vector
            Matrix< DDRMat > tStressVector = this->get_stress_vector();

            // initialize stress value
            real tStressValue = ( tStressVector( 0 ) + tStressVector( 1 ) ) / 2.0;

            // get max shear stress
            real tMaxShearStress = std::sqrt( std::pow( ( tStressVector( 0 ) - tStressVector( 1 ) ) / 2.0, 2.0 ) +
                    std::pow( tStressVector( 2 ), 2.0 ) )  ;

            // switch between the 2D case and the 3D case
            // for 2D
            if ( mMasterFIManager->get_IP_geometry_interpolator()->get_number_of_space_dimensions() == 2 )
            {
                switch ( aPrincipalStressIndex )
                {
                    case 1 :
                        tStressValue += tMaxShearStress;
                        break;

                    case 2 :
                        tStressValue -= tMaxShearStress;
                        break;

                    default :
                        MORIS_ERROR( false ,
                                "IQI_Max_Stress::eval_principal_stress - Only 1st and 2nd principal stresses known for 2D.");
                }
            }

            // for 3D
            else
            {
                // compute invariants
                real tI1 = tStressVector( 0 ) + tStressVector( 1 ) + tStressVector( 2 );
                real tI2 = tStressVector( 0 ) * tStressVector( 1 ) +
                        tStressVector( 1 ) * tStressVector( 2 ) +
                        tStressVector( 2 ) * tStressVector( 0 ) -
                        std::pow( tStressVector( 3 ), 2.0 ) -
                        std::pow( tStressVector( 4 ), 2.0 ) -
                        std::pow( tStressVector( 5 ), 2.0 ) ;
                real tI3 = tStressVector( 0 ) * tStressVector( 1 ) * tStressVector( 2 ) -
                        tStressVector( 0 ) * std::pow( tStressVector( 3 ), 2.0 ) -
                        tStressVector( 1 ) * std::pow( tStressVector( 4 ), 2.0 ) -
                        tStressVector( 3 ) * std::pow( tStressVector( 5 ), 2.0 ) +
                        2.0 * tStressVector( 3 ) * tStressVector( 4 ) * tStressVector( 5 );

                // help values
                real tQ = ( 1 / 9 ) * ( std::pow( tI1, 2.0 ) - 3.0 * tI2 );
                real tR = ( 1 / 54 ) * ( 2.0 * std::pow( tI1, 3.0 ) - 9.0 * tI1 * tI2 + 27.0 * tI3 );
                real tT = std::acos( tR / std::sqrt( std::pow( tQ, 3.0 ) ) );

                // compute principal values
                real tE1 = 2.0 * std::sqrt( tQ ) * std::cos( ( tT              ) / 3.0 ) + tI1 / 3.0;
                real tE2 = 2.0 * std::sqrt( tQ ) * std::cos( ( tT + 2.0 * M_PI ) / 3.0 ) + tI1 / 3.0;
                real tE3 = 2.0 * std::sqrt( tQ ) * std::cos( ( tT + 4.0 * M_PI ) / 3.0 ) + tI1 / 3.0;

                // figure out minimum and maximum values
                switch ( aPrincipalStressIndex )
                {
                    case 1 :
                        tStressValue = std::max( std::max( tE1, tE2 ), std::max( tE2, tE3 ) );
                        break;

                    case 2 :
                        tStressValue = std::max( std::min( tE1, tE2 ), std::min( std::max( tE1, tE2 ), tE3 ) );
                        break;

                    case 3 :
                        tStressValue = std::min( std::min( tE1, tE2 ), std::min( tE2, tE3 ) );
                        break;

                    default :
                        MORIS_ERROR( false ,
                                "IQI_Max_Stress::eval_principal_stress - Only 1st, 2nd, and 3rd principal stresses known for 3D.");
                }
            }

            // return stress value
            return tStressValue;
        }

        //------------------------------------------------------------------------------

        real IQI_Max_Stress::eval_normal_stress( uint aStressIndex )
        {
            // get stress vector
            Matrix< DDRMat > tStressVector = this->get_stress_vector();

            // pick stress value
            return tStressVector( aStressIndex - 1 );
        }

        //------------------------------------------------------------------------------

        real IQI_Max_Stress::eval_shear_stress( uint aStressIndex )
        {
            // get stress vector
            Matrix< DDRMat > tStressVector = this->get_stress_vector();

            // pick stress value
            return tStressVector( aStressIndex + 2 );
        }

        //------------------------------------------------------------------------------

        Matrix< DDRMat > IQI_Max_Stress::get_stress_vector()
        {
            // create stress vector
            Matrix< DDRMat > tStressVector( 6, 1, 0.0 );

            // get stress vector from Constitutive model
            Matrix< DDRMat > tCMStress =
                    mMasterCM( static_cast< uint >( IQI_Constitutive_Type::ELAST_LIN_ISO ) )->flux();

            // pull apart stress vector into components
            uint tNumStressComponents = tCMStress.length();
            switch  (tNumStressComponents)
            {
                // 2D plane stress
                case 3:
                    tStressVector( 0 ) = tCMStress( 0 );
                    tStressVector( 1 ) = tCMStress( 1 );
                    tStressVector( 5 ) = tCMStress( 2 );
                    break;

                    // 2D plane strain
                case 4:
                    tStressVector( 0 ) = tCMStress( 0 );
                    tStressVector( 1 ) = tCMStress( 1 );
                    tStressVector( 2 ) = tCMStress( 2 );
                    tStressVector( 5 ) = tCMStress( 3 );
                    break;

                    // 3D
                case 6:
                    tStressVector = tCMStress;
                    break;

                    // Unknown size - error
                default:
                    MORIS_ERROR( false,
                            "IQI_Max_Von_Mises_Stress::get_stress_vector - CM stress vector of unknown size; 3, 4 or 6 components expected." );
            }

            // return filled stress vector
            return tStressVector;
        }

        //------------------------------------------------------------------------------

    }/* end_namespace_fem */
}/* end_namespace_moris */



