
//FEM/INT/src
#include "cl_FEM_SP_Pressure_Ghost.hpp"
#include "cl_FEM_Cluster.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        SP_Pressure_Ghost::SP_Pressure_Ghost()
        {
            // set the property pointer cell size
            mMasterProp.resize( static_cast< uint >( Property_Type::MAX_ENUM ), nullptr );

            // populate the map
            mPropertyMap[ "Viscosity" ] = static_cast< uint >( Property_Type::VISCOSITY );
            mPropertyMap[ "Density" ]   = static_cast< uint >( Property_Type::DENSITY );
        }

        //------------------------------------------------------------------------------

        void SP_Pressure_Ghost::set_dof_type_list(
                moris::Cell< moris::Cell< MSI::Dof_Type > > & aDofTypes,
                moris::Cell< std::string >                  & aDofStrings,
                mtk::Master_Slave                             aIsMaster )
        {
            // switch on master slave
            switch ( aIsMaster )
            {
                case mtk::Master_Slave::MASTER :
                {
                    // set dof type list
                    mMasterDofTypes = aDofTypes;

                    // loop on dof type
                    for( uint iDof = 0; iDof < aDofTypes.size(); iDof++ )
                    {
                        // get dof string
                        std::string tDofString = aDofStrings( iDof );

                        // get dof type
                        MSI::Dof_Type tDofType = aDofTypes( iDof )( 0 );

                        // if velocity
                        if( tDofString == "Velocity" )
                        {
                            mMasterDofVelocity = tDofType;
                        }
                        else
                        {
                            // error unknown dof string
                            MORIS_ERROR( false ,
                                    "SP_Pressure_Ghost::set_dof_type_list - Unknown aDofString : %s \n",
                                    tDofString.c_str() );
                        }
                    }
                    break;
                }

                case mtk::Master_Slave::SLAVE :
                {
                    // set dof type list
                    mSlaveDofTypes = aDofTypes;
                    break;
                }

                default:
                    MORIS_ERROR( false, "SP_Pressure_Ghost::set_dof_type_list - unknown master slave type." );
            }
        }

        //------------------------------------------------------------------------------

        moris::Cell< std::tuple<
        fem::Measure_Type,
        mtk::Primary_Void,
        mtk::Master_Slave > > SP_Pressure_Ghost::get_cluster_measure_tuple_list()
        {
            return { mElementSizeTuple };
        }

        //------------------------------------------------------------------------------

        void SP_Pressure_Ghost::eval_SP()
        {
            // get element size cluster measure value
            real tElementSize = mCluster->get_cluster_measure(
                    std::get<0>( mElementSizeTuple ),
                    std::get<1>( mElementSizeTuple ),
                    std::get<2>( mElementSizeTuple ) )->val()( 0 );

            // get the viscosity and density property
            std::shared_ptr< Property > & tViscosityProp =
                    mMasterProp( static_cast< uint >( Property_Type::VISCOSITY ) );
            std::shared_ptr< Property > & tDensityProp   =
                    mMasterProp( static_cast< uint >( Property_Type::DENSITY ) );

            // get the velocity FI
            Field_Interpolator * tVelocityFI =
                    mMasterFIManager->get_field_interpolators_for_type( mMasterDofVelocity );

            // compute infinity norm of u
            real tInfinityNorm = std::abs( tVelocityFI->val()( 0 ) );
            for( uint iDim = 0; iDim < tVelocityFI->val().numel(); iDim++ )
            {
                real tAbsVelocity = std::abs( tVelocityFI->val()( iDim ) );
                if ( tInfinityNorm < tAbsVelocity )
                {
                    tInfinityNorm = tAbsVelocity;
                }
            }

            // compute deltaT
            real tDeltaT = mMasterFIManager->get_IP_geometry_interpolator()->get_time_step();

            // compute deltaP
            real tDeltaP =
                    tViscosityProp->val()( 0 ) / tElementSize +
                    tDensityProp->val()( 0 ) * tInfinityNorm / 6.0 +
                    tDensityProp->val()( 0 ) * tElementSize / ( 12.0 * mParameters( 1 )( 0 ) * tDeltaT );

            // compute stabilization parameter value
            mPPVal = mParameters( 0 ) * std::pow( tElementSize, 2 * mOrder ) / tDeltaP;
        }

        //------------------------------------------------------------------------------

        void SP_Pressure_Ghost::eval_dSPdMasterDOF(
                const moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // get element size cluster measure value
            real tElementSize = mCluster->get_cluster_measure(
                    std::get<0>( mElementSizeTuple ),
                    std::get<1>( mElementSizeTuple ),
                    std::get<2>( mElementSizeTuple ) )->val()( 0 );

            // get the dof type index
            uint tDofIndex = mMasterGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the dof type FI
            Field_Interpolator * tFI = mMasterFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // set size for dSPdMasterDof
            mdPPdMasterDof( tDofIndex ).set_size( 1, tFI->get_number_of_space_time_coefficients(), 0.0 );

            // get velocity field interpolator
            Field_Interpolator * tVelocityFI =
                    mMasterFIManager->get_field_interpolators_for_type( mMasterDofVelocity );

            // get the viscosity property
            std::shared_ptr< Property > & tViscosityProp =
                    mMasterProp( static_cast< uint >( Property_Type::VISCOSITY ) );

            // get the density property
            std::shared_ptr< Property > & tDensityProp =
                    mMasterProp( static_cast< uint >( Property_Type::DENSITY ) );

            // compute infinity norm
            uint tInfinityNormIndex = 0;
            real tInfinityNorm = std::abs( tVelocityFI->val()( 0 ) );
            for( uint iDim = 0; iDim < tVelocityFI->val().numel(); iDim++ )
            {
                real tAbsVelocity = std::abs( tVelocityFI->val()( iDim ) );
                if ( tInfinityNorm < tAbsVelocity )
                {
                    tInfinityNormIndex = iDim;
                    tInfinityNorm = tAbsVelocity;
                }
            }

            // compute deltaT
            real tDeltaT = mMasterFIManager->get_IP_geometry_interpolator()->get_time_step();

            // compute deltaP
            real tDeltaP = 
                    tViscosityProp->val()( 0 ) / tElementSize +
                    tDensityProp->val()( 0 ) * tInfinityNorm / 6.0 +
                    tDensityProp->val()( 0 ) * tElementSize / ( 12.0 * mParameters( 1 )( 0 ) * tDeltaT );

            // if dof type == velocity
            if( aDofTypes( 0 ) == mMasterDofVelocity )
            {
                // compute derivative of the infinity norm
                Matrix< DDRMat > tdInfinityNormdu = tVelocityFI->N().get_row( tInfinityNormIndex );
                if( tVelocityFI->val()( tInfinityNormIndex ) < 0.0 )
                {
                    tdInfinityNormdu = -1.0 * tdInfinityNormdu;
                }

                // compute contribution from velocity
                mdPPdMasterDof( tDofIndex ) -=
                        mParameters( 0 )( 0 ) * std::pow( tElementSize, 2 * mOrder ) *
                        tDensityProp->val()( 0 ) * tdInfinityNormdu /
                        ( 6.0 * std::pow( tDeltaP, 2.0 ) );
            }

            // if viscosity depends on dof type
            if( tViscosityProp->check_dof_dependency( aDofTypes ) )
            {
                // compute contribution from viscosity
                mdPPdMasterDof( tDofIndex ) -=
                        mParameters( 0 )( 0 ) * std::pow( tElementSize, 2 * mOrder ) *
                        tViscosityProp->dPropdDOF( aDofTypes ) /
                        ( tElementSize * std::pow( tDeltaP, 2.0 ) );
            }

            // if density depends on dof type
            if( tDensityProp->check_dof_dependency( aDofTypes ) )
            {
                // compute contribution from density
                mdPPdMasterDof( tDofIndex ) -=
                        mParameters( 0 )( 0 ) * std::pow( tElementSize, 2 * mOrder ) *
                        tDensityProp->dPropdDOF( aDofTypes ) *
                        ( tInfinityNorm / 6.0 + tElementSize / ( 12.0 * mParameters( 1 )( 0 ) * tDeltaT ) ) /
                        std::pow( tDeltaP, 2.0 );
            }
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */


