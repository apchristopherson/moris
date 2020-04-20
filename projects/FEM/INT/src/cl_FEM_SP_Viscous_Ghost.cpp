
//FEM/INT/src
#include "cl_FEM_SP_Viscous_Ghost.hpp"
#include "cl_FEM_Cluster.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

namespace moris
{
    namespace fem
    {

//------------------------------------------------------------------------------
        SP_Viscous_Ghost::SP_Viscous_Ghost()
        {
            // set the property pointer cell size
            mMasterProp.resize( static_cast< uint >( Property_Type::MAX_ENUM ), nullptr );

            // populate the map
            mPropertyMap[ "Viscosity" ] = Property_Type::VISCOSITY;
        }

//------------------------------------------------------------------------------
        void SP_Viscous_Ghost::eval_SP()
        {
            // get the viscosity property
            std::shared_ptr< Property > tViscosityProp = mMasterProp( static_cast< uint >( Property_Type::VISCOSITY ) );

            // compute stabilization parameter value
            mPPVal = mParameters( 0 ) * tViscosityProp->val()( 0 ) * std::pow( mElementSize, 2.0 * ( mOrder - 1.0 ) + 1.0 );
        }

//------------------------------------------------------------------------------
        void SP_Viscous_Ghost::eval_dSPdMasterDOF( const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get the dof type index
            uint tDofIndex = mMasterGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the dof type FI
            Field_Interpolator * tFI = mMasterFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // set size for dSPdMasterDof
            mdPPdMasterDof( tDofIndex ).set_size( 1, tFI->get_number_of_space_time_coefficients(), 0.0 );

            // get the viscosity property
            std::shared_ptr< Property > tViscosityProp = mMasterProp( static_cast< uint >( Property_Type::VISCOSITY ) );

            // if viscosity depends on dof type
            if( tViscosityProp->check_dof_dependency( aDofTypes ) )
            {
                // compute contribution from viscosity
                mdPPdMasterDof( tDofIndex ).matrix_data()
                += mParameters( 0 ) * std::pow( mElementSize, 2.0 * ( mOrder - 1.0 ) + 1.0 )
                 * tViscosityProp->dPropdDOF( aDofTypes );
            }
        }

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */


