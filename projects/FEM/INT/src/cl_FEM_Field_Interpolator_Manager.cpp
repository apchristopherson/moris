
#include "cl_FEM_Field_Interpolator_Manager.hpp" //FEM/INT/src
#include "cl_FEM_Set.hpp"                   //FEM/INT/src

namespace moris
{
    namespace fem
    {

//------------------------------------------------------------------------------
        Field_Interpolator_Manager::Field_Interpolator_Manager
        ( const moris::Cell< moris::Cell< enum MSI::Dof_Type > > & aDofTypes,
                MSI::Equation_Set                                * aEquationSet,
                mtk::Master_Slave                                  aIsMaster )
        : mDofTypes( aDofTypes ),
          mEquationSet( aEquationSet ),
          mIsMaster( aIsMaster )
        {
            // set the dof type map
            mDofTypeMap = mEquationSet->get_dof_type_map( aIsMaster );

            // maximum number of dof field interpolators
            mMaxNumDofFI = mEquationSet->get_num_unique_dof_types();

            // FIXME default
            mMaxNumDvFI = 0;
        }

        Field_Interpolator_Manager::Field_Interpolator_Manager
        ( const moris::Cell< moris::Cell< enum MSI::Dof_Type > > & aDofTypes,
          const moris::Cell< moris::Cell< enum GEN_DV > >        & aDvTypes,
                MSI::Equation_Set                                * aEquationSet,
                mtk::Master_Slave                                  aIsMaster )
        : mDofTypes( aDofTypes ),
          mEquationSet( aEquationSet ),
          mIsMaster( aIsMaster ),
          mDvTypes( aDvTypes )
        {
            // set the dof type map
            mDofTypeMap = mEquationSet->get_dof_type_map( aIsMaster );

            // maximum number of dof field interpolators
            mMaxNumDofFI =  mEquationSet->get_num_unique_dof_types();

            // FIXME maximum number of dv field interpolators
            mMaxNumDvFI =  3;          //FIXME FIXME FIXME

            mDvTypeMap = mEquationSet->get_dv_type_map( aIsMaster );
        }

        Field_Interpolator_Manager::Field_Interpolator_Manager
        ( const moris::Cell< moris::Cell< enum MSI::Dof_Type > > & aDofTypes,
                MSI::Equation_Set                                * aEquationSet,
                MSI::Model_Solver_Interface                      * aModelSolverInterface,
                mtk::Master_Slave                                  aIsMaster )
        : mDofTypes( aDofTypes ),
          mEquationSet( aEquationSet ),
          mIsMaster( aIsMaster )
        {
            // set the dof type map
            mDofTypeMap = mEquationSet->get_dof_type_map( aIsMaster );

            // maximum number of dof field interpolators
            mMaxNumDofFI = mEquationSet->get_num_unique_dof_types();

            // FIXME default
            mMaxNumDvFI = 0;
        }

//------------------------------------------------------------------------------
        Field_Interpolator_Manager::~Field_Interpolator_Manager()
        {
            // delete pointers on the FI manager
            this->delete_pointers();
        }

//------------------------------------------------------------------------------
        void Field_Interpolator_Manager::delete_pointers()
        {
            // delete the dof field interpolator pointers
            for( Field_Interpolator* tFI : mFI )
            {
                delete tFI;
            }
            mFI.clear();

            // delete the dv field interpolator pointers
            for( Field_Interpolator* tFI : mDvFI )
            {
                delete tFI;
            }
            mDvFI.clear();

//            // delete the IP geometry interpolator pointer
//            if( mIPGeometryInterpolator != nullptr )
//            {
//                delete mIPGeometryInterpolator;
//            }
//
//            // delete the IG geometry interpolator pointer
//            if( mIGGeometryInterpolator != nullptr )
//            {
//                delete mIGGeometryInterpolator;
//            }
        }

//------------------------------------------------------------------------------
        void Field_Interpolator_Manager::create_field_interpolators
        ( MSI::Model_Solver_Interface * aModelSolverInterface )
        {
            // dof field interpolators------------------------------------------

            // set the size of the cell of field interpolators
            mFI.resize( mMaxNumDofFI, nullptr );

            // loop over the dof type groups
            for( uint iDof = 0; iDof < mDofTypes.size(); iDof++ )
            {
                // get the number of time level for the dof type group
                uint tNumTimeNodes = aModelSolverInterface->get_time_levels_for_type( mDofTypes( iDof )( 0 ) );
                std::cout<<"Field_Interpolator_Manager::create_field_interpolators - Time Node: "<<tNumTimeNodes<<std::endl;

                // get the set index for the dof type group
                uint tDofIndex = mEquationSet->get_dof_index_for_type_1( mDofTypes( iDof )( 0 ), mIsMaster );

                // create the field interpolation rule for the dof type group
                Interpolation_Rule tFieldInterpolationRule( reinterpret_cast< Set* >( mEquationSet )->mIPGeometryType,
                                                            Interpolation_Type::LAGRANGE,
                                                            reinterpret_cast< Set* >( mEquationSet )->mIPSpaceInterpolationOrder,
                                                            reinterpret_cast< Set* >( mEquationSet )->get_auto_time_interpolation_type( tNumTimeNodes ), // fixme
                                                            // If interpolation type CONSTANT, iInterpolation order is not used
                                                            reinterpret_cast< Set* >( mEquationSet )->get_auto_interpolation_order( tNumTimeNodes, mtk::Geometry_Type::LINE ) ); //fixme

                // check if the fiedl interpolator was created previously
                MORIS_ASSERT( mFI( tDofIndex ) == nullptr, "Field_Interpolator_Manager::create_field_interpolators - Field interpolator was created previously" );

                // create a field interpolator for the dof type group
                mFI( tDofIndex ) = new Field_Interpolator( mDofTypes( iDof ).size(),
                                                           tFieldInterpolationRule,
                                                           mIPGeometryInterpolator,
                                                           mDofTypes( iDof ) );
            }

            // dv field interpolators------------------------------------------

            // set the size of the cell of field interpolators
            mDvFI.resize( mMaxNumDvFI, nullptr );

            // loop over the dv type groups
            for( uint iDv = 0; iDv < mDvTypes.size(); iDv++ )
            {
                // get the number of time level for the dv type group
                // FIXME where do we get this info
                uint tNumTimeNodes = 1;

                // get the set index for the dv type group
                uint tDvIndex = mEquationSet->get_dv_index_for_type_1( mDvTypes( iDv )( 0 ), mIsMaster );

                // create the field interpolation rule for the dv type group
                Interpolation_Rule tFieldInterpolationRule( reinterpret_cast< Set* >( mEquationSet )->mIPGeometryType,
                                                            Interpolation_Type::LAGRANGE,
                                                            reinterpret_cast< Set* >( mEquationSet )->mIPSpaceInterpolationOrder,
                                                            reinterpret_cast< Set* >( mEquationSet )->get_auto_time_interpolation_type( tNumTimeNodes ), // fixme
                                                            // If interpolation type CONSTANT, iInterpolation order is not used
                                                            reinterpret_cast< Set* >( mEquationSet )->get_auto_interpolation_order( tNumTimeNodes, mtk::Geometry_Type::LINE ) ); //fixme

                // check if the field interpolator was created previously
                MORIS_ASSERT( mDvFI( tDvIndex ) == nullptr,
                              "Field_Interpolator_Manager::create_field_interpolators - Field interpolator was created previously." );

                // create a field interpolator for the dof type group
                mDvFI( tDvIndex ) = new Field_Interpolator( mDvTypes( iDv ).size(),
                                                           tFieldInterpolationRule,
                                                           mIPGeometryInterpolator,
                                                           mDvTypes( iDv ) );
            }
        }

//------------------------------------------------------------------------------
        void Field_Interpolator_Manager::create_geometry_interpolators()
        {
            // get element type for set
            fem::Element_Type tElementType = reinterpret_cast< Set* >( mEquationSet )->mElementType;

            // bool true if time sideset
            bool tIsTimeSide = ( tElementType == fem::Element_Type::TIME_SIDESET );

            // bool true if sideset or double sideset
            bool tIsSide = ( tElementType != fem::Element_Type::BULK )
                         &&( tElementType != fem::Element_Type::TIME_SIDESET );

            // create geometry interpolation rule for IP elements
            Interpolation_Rule tIPGeometryInterpolationRule( reinterpret_cast< Set* >( mEquationSet )->mIPGeometryType,
                                                             Interpolation_Type::LAGRANGE,
                                                             reinterpret_cast< Set* >( mEquationSet )->mIPSpaceInterpolationOrder,
                                                             mtk::Geometry_Type::LINE,
                                                             Interpolation_Type::LAGRANGE,
                                                             mtk::Interpolation_Order::LINEAR ); // FIXME not linear?

            // FIXME default should be given by the MSI
            mtk::Geometry_Type       tIGTimeGeometryType = mtk::Geometry_Type::LINE;
            fem::Interpolation_Type  tIGTimeInterpType   = fem::Interpolation_Type::LAGRANGE;
            mtk::Interpolation_Order tIGTimeInterpOrder  = mtk::Interpolation_Order::LINEAR;

            // if time sideset
            if ( tIsTimeSide )
            {
                tIGTimeGeometryType = mtk::Geometry_Type::POINT;
                tIGTimeInterpType   = fem::Interpolation_Type::CONSTANT;
                tIGTimeInterpOrder  = mtk::Interpolation_Order::CONSTANT;
            }

             // create geometry interpolation rule for IG elements
             Interpolation_Rule tIGGeometryInterpolationRule( reinterpret_cast< Set* >( mEquationSet )->mIGGeometryType,
                                                              Interpolation_Type::LAGRANGE,
                                                              reinterpret_cast< Set* >( mEquationSet )->mIGSpaceInterpolationOrder,
                                                              tIGTimeGeometryType,
                                                              tIGTimeInterpType,
                                                              tIGTimeInterpOrder );

             // create a geometry interpolator for IP cells
             mIPGeometryInterpolator = new Geometry_Interpolator( tIPGeometryInterpolationRule, tIsSide, tIsTimeSide );

             // create a geometry interpolator for IG cells
             mIGGeometryInterpolator = new Geometry_Interpolator( tIGGeometryInterpolationRule, tIsSide, tIsTimeSide );
        }

//------------------------------------------------------------------------------
        Field_Interpolator * Field_Interpolator_Manager::get_field_interpolators_for_type( enum MSI::Dof_Type aDofType )
        {
            // check of the equation set pointer was set for the FI manager
            MORIS_ASSERT( mEquationSet != nullptr, "Field_Interpolator_Manager::get_field_interpolators_for_type - Equation Set pointer not set");

            // get the set index for the requested dof type
            sint tDofIndex = mEquationSet->get_dof_index_for_type_1( aDofType, mIsMaster );

            // if the index was set for the equation set
            if( tDofIndex != -1 )
            {
                // check if the FI exists for the FI manager
                MORIS_ASSERT( (sint)mFI.size() > tDofIndex,
                              "Field_Interpolator_Manager::get_field_interpolators_for_type - field interpolator does not exist" );

                // return the FI
                return mFI( tDofIndex );
            }
            else
            {
                return nullptr;
            }
        }

//------------------------------------------------------------------------------
        Field_Interpolator * Field_Interpolator_Manager::get_field_interpolators_for_type( enum GEN_DV aDvType )
        {
            // get the set index for the requested dv type
            sint tDvIndex =  mEquationSet->get_dv_index_for_type_1( aDvType, mIsMaster );

            // if the index was set for the equation set
            if( tDvIndex != -1 )
            {
                // check if the FI exists for the FI manager
                MORIS_ASSERT( (sint)mDvFI.size() > tDvIndex,
                              "Field_Interpolator_Manager::get_field_interpolators_for_type - field interpolator does not exist" );

                // return the FI
                return mDvFI( tDvIndex );
            }
            else
            {
                return nullptr;
            }
        }

//------------------------------------------------------------------------------
        void Field_Interpolator_Manager::set_space_time( Matrix< DDRMat > & aParamPoint )
        {
            // loop over the dof field interpolators
            for ( uint iDofFI = 0; iDofFI < mDofTypes.size(); iDofFI++ )
            {
                // get the set index for the dof type
                sint tDofIndex = mDofTypeMap( static_cast< uint >( mDofTypes( iDofFI )( 0 ) ) );

                // set the evaluation point
                mFI( tDofIndex )->set_space_time( aParamPoint );
            }

            // loop over the dv field interpolators
            for ( uint iDvFI = 0; iDvFI < mDvTypes.size(); iDvFI++ )
            {
                // get the set index for the dv type
                sint tDvIndex = mDvTypeMap( static_cast< uint >( mDvTypes( iDvFI )( 0 ) ) );

                // set the evaluation point
                mDvFI( tDvIndex )->set_space_time( aParamPoint );
            }

            // IP geometry interpolator
            mIPGeometryInterpolator->set_space_time( aParamPoint );
        }

//------------------------------------------------------------------------------
        void Field_Interpolator_Manager::set_space_time_from_local_IG_point( Matrix< DDRMat > & aLocalParamPoint )
        {
            // set evaluation point in the IG param space for IG geometry interpolator
            mIGGeometryInterpolator->set_space_time( aLocalParamPoint );

            // bring evaluation point in the IP param space
            Matrix< DDRMat > tGlobalParamPoint;
            mIGGeometryInterpolator->map_integration_point( tGlobalParamPoint );

            // set evaluation point for interpolators (FIs and IP GI)
            this->set_space_time( tGlobalParamPoint );
        }

//------------------------------------------------------------------------------
        void Field_Interpolator_Manager::set_coeff_for_type( enum MSI::Dof_Type   aDofType,
                                                             Matrix< DDRMat >   & aCoeff )
        {
            // get field interpolator for dof type and set coefficients
            this->get_field_interpolators_for_type( aDofType )->set_coeff( aCoeff );
        }

//------------------------------------------------------------------------------
        void Field_Interpolator_Manager::set_coeff_for_type( enum GEN_DV        aDvType,
                                                             Matrix< DDRMat > & aCoeff )
        {
            // get field interpolator for dof type and set coefficients
            this->get_field_interpolators_for_type( aDvType )->set_coeff( aCoeff );
        }

//------------------------------------------------------------------------------

    } /* namespace fem */
} /* namespace moris */

