/*
 * cl_FEM_IQI_Latent_Heat_Absorption.cpp
 *
 *  Created on: Jul 10, 2020
 *      Author: wunsch
 */

#include "cl_FEM_Set.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"
#include "cl_FEM_IQI_Latent_Heat_Absorption.hpp"
#include "fn_FEM_CM_Phase_State_Functions.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        IQI_Latent_Heat_Absorption::IQI_Latent_Heat_Absorption()
        {
            // set fem IQI type
            mFEMIQIType = fem::IQI_Type::LATENT_HEAT_ABSORPTION;

            // set the property pointer cell size
            mMasterProp.resize( static_cast< uint >( IQI_Property_Type::MAX_ENUM ), nullptr );

            // populate the property map
            mPropertyMap[ "Density" ]             = static_cast< uint >( IQI_Property_Type::DENSITY );
            mPropertyMap[ "LatentHeat" ]          = static_cast< uint >( IQI_Property_Type::LATENT_HEAT );
            mPropertyMap[ "PCTemp" ]              = static_cast< uint >( IQI_Property_Type::PC_TEMP );
            mPropertyMap[ "PhaseStateFunction" ]  = static_cast< uint >( IQI_Property_Type::PHASE_STATE_FUNCTION );
            mPropertyMap[ "PhaseChangeConst" ]    = static_cast< uint >( IQI_Property_Type::PHASE_CHANGE_CONST );
        }

        //------------------------------------------------------------------------------

        void IQI_Latent_Heat_Absorption::compute_QI( Matrix< DDRMat > & aQI )
        {
            moris::real tDensity = mMasterProp( static_cast< uint >( IQI_Property_Type::DENSITY ) )->val()( 0 );
            moris::real tLatHeat = mMasterProp( static_cast< uint >( IQI_Property_Type::LATENT_HEAT ) )->val()( 0 );

            // get the temperature FI
            Field_Interpolator * tFITemp =
                    mMasterFIManager->get_field_interpolators_for_type( MSI::Dof_Type::TEMP );

            // compute derivative of Phase State Function
            // real tdfdT = this->eval_dFdTemp();
            real tdfdT = eval_dFdTemp(
                    mMasterProp( static_cast< uint >( IQI_Property_Type::PC_TEMP ) )->val()( 0 ),
                    mMasterProp( static_cast< uint >( IQI_Property_Type::PHASE_CHANGE_CONST ) )->val()( 0 ),
                    mMasterProp( static_cast< uint >( IQI_Property_Type::PHASE_STATE_FUNCTION ) )->val()( 0 ),
                    tFITemp );

            // evaluate the QI
            aQI = tDensity * tLatHeat * tdfdT * tFITemp->gradt( 1 );
        }

        //------------------------------------------------------------------------------

        void IQI_Latent_Heat_Absorption::compute_dQIdu(
                moris::Cell< MSI::Dof_Type > & aDofType,
                Matrix< DDRMat >             & adQIdu )
        {
            // get properties
            std::shared_ptr< Property > & tPropDensity = mMasterProp( static_cast< uint >( IQI_Property_Type::DENSITY ) );
            std::shared_ptr< Property > & tPropLatHeat = mMasterProp( static_cast< uint >( IQI_Property_Type::LATENT_HEAT ) );
            std::shared_ptr< Property > & tPropPCtemp  = mMasterProp( static_cast< uint >( IQI_Property_Type::PC_TEMP ) );
            std::shared_ptr< Property > & tPropPCconst = mMasterProp( static_cast< uint >( IQI_Property_Type::PHASE_CHANGE_CONST ) );
            std::shared_ptr< Property > & tPropPSfunct = mMasterProp( static_cast< uint >( IQI_Property_Type::PHASE_STATE_FUNCTION ) );

            // get the temperature FI
            Field_Interpolator * tFITemp =
                    mMasterFIManager->get_field_interpolators_for_type( MSI::Dof_Type::TEMP );

            // compute derivative of Phase State Function
            // real tdfdT = this->eval_dFdTemp();
            real tdfdT = eval_dFdTemp(
                    tPropPCtemp->val()( 0 ),
                    tPropPCconst->val()( 0 ),
                    tPropPSfunct->val()( 0 ),
                    tFITemp );

            // if direct dependency on the dof type
            if( aDofType( 0 ) == MSI::Dof_Type::TEMP )
            {
                // compute Dof derivative of phase state function
                const moris::Matrix<DDRMat> dfdDof = eval_dFdTempdDOF(
                        tPropPCtemp->val()(0),
                        tPropPCconst->val()(0),
                        tPropPSfunct->val()(0),
                        tFITemp);

                // compute derivative with direct dependency
                adQIdu =
                        tPropDensity->val()(0) * tPropLatHeat->val()(0) * tdfdT * tFITemp->dnNdtn(1) +
                        tPropDensity->val()(0) * tPropLatHeat->val()(0) * tFITemp->gradt(1) * dfdDof;
            }

            // if density property depends on the dof type
            if ( tPropDensity->check_dof_dependency( aDofType ) )
            {
                // compute derivative with indirect dependency through properties
                adQIdu = tPropLatHeat->val()(0) * tdfdT * tFITemp->gradt(1) * tPropDensity->dPropdDOF( aDofType );
            }
        }

        //------------------------------------------------------------------------------
    }/* end_namespace_fem */
}/* end_namespace_moris */



