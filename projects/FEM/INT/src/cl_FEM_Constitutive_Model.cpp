/*
 * cl_FEM_Constitutive_Model.cpp
 *
 *  Created on: Dec 10, 2019
 *      Author: noel
 */

#include "cl_FEM_Constitutive_Model.hpp"
#include "cl_FEM_Set.hpp"
#include "cl_FEM_Field_Interpolator_Manager.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------
        Constitutive_Model::Constitutive_Model()
        {
            // FIXME for now only 1st order allowed
            uint tOrder = 1;

            // set storage for evaluation
            mdFluxdx.resize( tOrder );
            mdStraindx.resize( tOrder );

            // set flag for evaluation
            mdFluxdxEval.assign( tOrder, true );
            mdStraindxEval.assign( tOrder, true );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::reset_eval_flags()
        {
            // reset the value flag
            mFluxEval         = true;
            mDivFluxEval      = true;
            mTractionEval     = true;
            mTestTractionEval.assign( mDofTypes.size(), true );
            mStrainEval       = true;
            mDivStrainEval    = true;
            mTestStrainEval   = true;
            mConstEval        = true;
            mEnergyDotEval         = true;
            mGradEnergyEval        = true;
            mGradEnergyDotEval     = true;
            mGradDivFluxEval  = true;

            // reset the dof derivative flag
            uint tNumDofTypes = mGlobalDofTypes.size();
            mdFluxdDofEval.assign( tNumDofTypes, true );
            mEnergyDotDofEval.assign( tNumDofTypes, true );
            mGradEnergyDofEval.assign( tNumDofTypes, true );
            mGradEnergyDotDofEval.assign( tNumDofTypes, true );
            mGradDivFluxDofEval.assign( tNumDofTypes, true );
            mddivfluxduEval.assign( tNumDofTypes, true );
            mdTractiondDofEval.assign( tNumDofTypes, true );
            for( uint iDirectDof = 0; iDirectDof < mDofTypes.size(); iDirectDof++ )
            {
                mdTestTractiondDofEval( iDirectDof ).assign( tNumDofTypes, true );
            }
            mdStraindDofEval.assign( tNumDofTypes, true );
            mddivstrainduEval.assign( tNumDofTypes, true );
            mdConstdDofEval.assign( tNumDofTypes, true );

            // reset the dv derivative flag
            uint tNumDvTypes = mGlobalDvTypes.size();
            mdFluxdDvEval.assign( tNumDvTypes, true );
            mdTractiondDvEval.assign( tNumDvTypes, true );
            for( uint iDirectDv = 0; iDirectDv < mDvTypes.size(); iDirectDv++ )
            {
                mdTestTractiondDvEval( iDirectDv ).assign( tNumDvTypes, true );
            }
            mdStraindDvEval.assign( tNumDvTypes, true );
            mdConstdDvEval.assign( tNumDvTypes, true );

            // reset underlying properties
            for( std::shared_ptr< Property > tProp : mProperties )
            {
                if ( tProp != nullptr )
                {
                    tProp->reset_eval_flags();
                }
            }
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::set_dof_type_list( moris::Cell< moris::Cell< MSI::Dof_Type > > aDofTypes )
        {
            // set the dof types
            mDofTypes = aDofTypes;

            // build a map for the dof types
            this->build_dof_type_map();
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::build_dof_type_map()
        {
            // get number of dof types
            uint tNumDofTypes = mDofTypes.size();

            // determine the max Dof_Type enum
            sint tMaxEnum = 0;
            for( uint iDOF = 0; iDOF < tNumDofTypes; iDOF++ )
            {
                tMaxEnum = std::max( tMaxEnum, static_cast< int >( mDofTypes( iDOF )( 0 ) ) );
            }
            tMaxEnum++;

            // set map size
            mDofTypeMap.set_size( tMaxEnum, 1, -1 );

            // loop over the dof types
            for( uint iDOF = 0; iDOF < tNumDofTypes; iDOF++ )
            {
                // fill the dof type map
                mDofTypeMap( static_cast< int >( mDofTypes( iDOF )( 0 ) ), 0 ) = iDOF;
            }
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::build_global_dof_type_list()
        {
            // get number of dof types
            uint tNumDofTypes = mDofTypes.size();

            // set the size of the dof type list
            uint tCounterMax = tNumDofTypes;

            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if( tProperty != nullptr )
                {
                    tCounterMax += tProperty->get_dof_type_list().size();
                }
            }
            mGlobalDofTypes.resize( tCounterMax );
            moris::Cell< sint > tCheckList( tCounterMax, -1 );

            // initialize total dof counter
            uint tCounter = 0;

            // get active dof type for constitutive model
            for ( uint iDOF = 0; iDOF < tNumDofTypes; iDOF++ )
            {
                tCheckList( tCounter ) = static_cast< uint >( mDofTypes( iDOF )( 0 ) );
                mGlobalDofTypes( tCounter ) = mDofTypes( iDOF );
                tCounter++;
            }

            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if( tProperty != nullptr )
                {
                    // get active dof types
                    moris::Cell< moris::Cell< MSI::Dof_Type > > tActiveDofType =
                            tProperty->get_dof_type_list();

                    for ( uint iDOF = 0; iDOF < tActiveDofType.size(); iDOF++ )
                    {
                        // check enum is not already in the list
                        bool tCheck = false;
                        for( uint i = 0; i < tCounter; i++ )
                        {
                            tCheck = tCheck || equal_to( tCheckList( i ), static_cast< uint >( tActiveDofType( iDOF )( 0 ) ) );
                        }

                        // if dof enum not in the list
                        if ( !tCheck )
                        {
                            tCheckList( tCounter ) = static_cast< uint >( tActiveDofType( iDOF )( 0 ) );
                            mGlobalDofTypes( tCounter ) = tActiveDofType( iDOF );
                            tCounter++;
                        }
                    }
                }
            }

            // get the number of unique dof type groups, i.e. the number of interpolators
            mGlobalDofTypes.resize( tCounter );

            // number of dof types
            uint tNumGlobalDofTypes = mGlobalDofTypes.size();
            uint tNumDirectDofTypes = mDofTypes.size();

            // set flag for evaluation
            mEnergyDotDofEval.resize( tNumGlobalDofTypes, true );
            mGradEnergyDotDofEval.resize( tNumGlobalDofTypes, true );
            mGradEnergyDofEval.resize( tNumGlobalDofTypes, true );
            mGradDivFluxDofEval.resize( tNumGlobalDofTypes, true );
            mTestTractionEval.resize( tNumDirectDofTypes, true );
            mdFluxdDofEval.resize( tNumGlobalDofTypes, true );
            mddivfluxduEval.resize( tNumGlobalDofTypes, true );
            mdTractiondDofEval.resize( tNumGlobalDofTypes, true );
            mdTestTractiondDofEval.resize( tNumDirectDofTypes );
            for( uint iDirectDof = 0; iDirectDof < tNumDirectDofTypes; iDirectDof++ )
            {
                mdTestTractiondDofEval( iDirectDof ).assign( tNumGlobalDofTypes, true );
            }
            mdStraindDofEval.resize( tNumGlobalDofTypes, true );
            mddivstrainduEval.resize( tNumGlobalDofTypes, true );
            mdConstdDofEval.resize( tNumGlobalDofTypes, true );

            // set storage for evaluation
            mEnergyDotDof.resize( tNumGlobalDofTypes );
            mGradEnergyDotDof.resize( tNumGlobalDofTypes );
            mGradEnergyDof.resize( tNumGlobalDofTypes );
            mGradDivFluxDof.resize( tNumGlobalDofTypes );
            mTestTraction.resize( tNumDirectDofTypes );
            mdFluxdDof.resize( tNumGlobalDofTypes );
            mddivfluxdu.resize( tNumGlobalDofTypes );
            mdTractiondDof.resize( tNumGlobalDofTypes );
            mdTestTractiondDof.resize( tNumDirectDofTypes );
            for( uint iDirectDof = 0; iDirectDof < tNumDirectDofTypes; iDirectDof++ )
            {
                mdTestTractiondDof( iDirectDof ).resize( tNumGlobalDofTypes );
            }
            mdStraindDof.resize( tNumGlobalDofTypes );
            mddivstraindu.resize( tNumGlobalDofTypes );
            mdConstdDof.resize( tNumGlobalDofTypes );
        }

        //------------------------------------------------------------------------------
        const moris::Cell< moris::Cell< MSI::Dof_Type > > & Constitutive_Model::get_global_dof_type_list()
        {
            if( mGlobalDofBuild )
            {
                // build the stabilization parameter global dof type list
                this->build_global_dof_type_list();

                // update build flag
                mGlobalDofBuild = false;
            }

            if( mGlobalDofMapBuild )
            {
                // build the stabilization parameter global dof type map
                this->build_global_dof_type_map();

                // update build flag
                mGlobalDofMapBuild = false;
            }

            return mGlobalDofTypes;
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::build_global_dof_type_map()
        {
            if( mGlobalDofBuild )
            {
                // build the stabilization parameter global dof type list
                this->build_global_dof_type_list();

                // update build flag
                mGlobalDofBuild = false;
            }

            // get number of global dof types
            uint tNumDofTypes = mGlobalDofTypes.size();

            // determine the max Dof_Type enum
            sint tMaxEnum = 0;
            for( uint iDOF = 0; iDOF < tNumDofTypes; iDOF++ )
            {
                tMaxEnum = std::max( tMaxEnum, static_cast< int >( mGlobalDofTypes( iDOF )( 0 ) ) );
            }
            tMaxEnum++;

            // set the Dof_Type map size
            mGlobalDofTypeMap.set_size( tMaxEnum, 1, -1 );

            // fill the Dof_Type map
            for( uint iDOF = 0; iDOF < tNumDofTypes; iDOF++ )
            {
                // fill the property map
                mGlobalDofTypeMap( static_cast< int >( mGlobalDofTypes( iDOF )( 0 ) ), 0 ) = iDOF;
            }
        }

        //------------------------------------------------------------------------------
        const Matrix< DDSMat > & Constitutive_Model::get_global_dof_type_map()
        {
            if( mGlobalDofMapBuild )
            {
                // build the stabilization parameter global dof type map
                this->build_global_dof_type_map();

                // update build flag
                mGlobalDofMapBuild = false;
            }

            return mGlobalDofTypeMap;
        }

        //------------------------------------------------------------------------------
        bool Constitutive_Model::check_dof_dependency( const moris::Cell< MSI::Dof_Type > & aDofType )
        {
            // set bool for dependency
            bool tDofDependency = false;

            // get dof type index
            uint tDofIndex = static_cast< uint >( aDofType( 0 ) );

            // if aDofType is an active dv type for the constitutive model
            if( tDofIndex < this->get_global_dof_type_map().numel() && this->get_global_dof_type_map()( tDofIndex ) != -1 )
            {
                // bool is set to true
                tDofDependency = true;
            }
            // return bool for dependency
            return tDofDependency;
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::set_dv_type_list( moris::Cell< moris::Cell< PDV_Type > > aDvTypes )
        {
            // set the dv types
            mDvTypes = aDvTypes;

            // build a map for the dv types
            this->build_dv_type_map();
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::build_dv_type_map()
        {
            // get number of dv types
            uint tNumDvTypes = mDvTypes.size();

            // determine the max Dv_Type enum
            sint tMaxEnum = 0;
            for( uint iDV = 0; iDV < tNumDvTypes; iDV++ )
            {
                tMaxEnum = std::max( tMaxEnum, static_cast< int >( mDvTypes( iDV )( 0 ) ) );
            }
            tMaxEnum++;

            // set the Dv_Type map size
            mDvTypeMap.set_size( tMaxEnum, 1, -1 );

            // loop over the dv types
            for( uint iDV = 0; iDV < tNumDvTypes; iDV++ )
            {
                // fill the dv type map
                mDvTypeMap( static_cast< int >( mDvTypes( iDV )( 0 ) ), 0 ) = iDV;
            }
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::build_global_dv_type_list()
        {
            // get number of dv types
            uint tNumDvTypes = mDvTypes.size();

            // set the size of the dv type list
            uint tCounterMax = tNumDvTypes;

            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if( tProperty != nullptr )
                {
                    tCounterMax += tProperty->get_dv_type_list().size();
                }
            }
            mGlobalDvTypes.resize( tCounterMax );
            moris::Cell< sint > tCheckList( tCounterMax, -1 );

            // initialize total dv counter
            uint tCounter = 0;

            // get active dv type for constitutive model
            for ( uint iDV = 0; iDV < tNumDvTypes; iDV++ )
            {
                tCheckList( tCounter ) = static_cast< uint >( mDvTypes( iDV )( 0 ) );
                mGlobalDvTypes( tCounter ) = mDvTypes( iDV );
                tCounter++;
            }

            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if( tProperty != nullptr )
                {
                    // get active dv types
                    moris::Cell< moris::Cell< PDV_Type > > tActiveDvType = tProperty->get_dv_type_list();

                    for ( uint iDV = 0; iDV < tActiveDvType.size(); iDV++ )
                    {
                        // check enum is not already in the list
                        bool tCheck = false;
                        for( uint i = 0; i < tCounter; i++ )
                        {
                            tCheck = tCheck || equal_to( tCheckList( i ), static_cast< uint >( tActiveDvType( iDV )( 0 ) ) );
                        }

                        // if dof enum not in the list
                        if ( !tCheck )
                        {
                            tCheckList( tCounter ) = static_cast< uint >( tActiveDvType( iDV )( 0 ) );
                            mGlobalDvTypes( tCounter ) = tActiveDvType( iDV );
                            tCounter++;
                        }
                    }
                }
            }

            // get the number of unique dv type groups, i.e. the number of interpolators
            mGlobalDvTypes.resize( tCounter );

            // build global dv type map
            this->build_global_dv_type_map();

            // number of dof types
            uint tNumGlobalDvTypes = mGlobalDvTypes.size();

            // set flag for evaluation
            mdFluxdDvEval.assign( tNumGlobalDvTypes, true );
            mdTractiondDvEval.assign( tNumGlobalDvTypes, true );
            mdTestTractiondDvEval.resize( mDvTypes.size() );
            for( uint iDirectDv = 0; iDirectDv < mDvTypes.size(); iDirectDv++ )
            {
                mdTestTractiondDvEval( iDirectDv ).assign( tNumGlobalDvTypes, true );
            }
            mdStraindDvEval.assign( tNumGlobalDvTypes, true );
            mdConstdDvEval.assign( tNumGlobalDvTypes, true );

            // set storage for evaluation
            mdFluxdDv.resize( tNumGlobalDvTypes );
            mdTractiondDv.resize( tNumGlobalDvTypes );
            mdTestTractiondDv.resize( mDvTypes.size() );
            for( uint iDirectDv = 0; iDirectDv < mDvTypes.size(); iDirectDv++ )
            {
                mdTestTractiondDv( iDirectDv ).resize( tNumGlobalDvTypes );
            }
            mdStraindDv.resize( tNumGlobalDvTypes );
            mdConstdDv.resize( tNumGlobalDvTypes );
        }

        //------------------------------------------------------------------------------
        const moris::Cell< moris::Cell< PDV_Type > > & Constitutive_Model::get_global_dv_type_list()
        {
            if( mGlobalDvBuild )
            {
                // build the stabilization parameter global dv type list
                this->build_global_dv_type_list();

                // build the stabilization parameter global dv type map
                this->build_global_dv_type_map();

                // update build flag
                mGlobalDvBuild = false;
            }

            return mGlobalDvTypes;
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::build_global_dv_type_map()
        {
            // get number of global dv types
            uint tNumDvTypes = mGlobalDvTypes.size();

            // determine the max Dv_Type enum
            sint tMaxEnum = 0;
            for( uint iDOF = 0; iDOF < tNumDvTypes; iDOF++ )
            {
                tMaxEnum = std::max( tMaxEnum, static_cast< int >( mGlobalDvTypes( iDOF )( 0 ) ) );
            }
            tMaxEnum++;

            // set the Dv_Type map size
            mGlobalDvTypeMap.set_size( tMaxEnum, 1, -1 );

            // loop over the dv types
            for( uint iDV = 0; iDV < tNumDvTypes; iDV++ )
            {
                // fill the dv type map
                mGlobalDvTypeMap( static_cast< int >( mGlobalDvTypes( iDV )( 0 ) ), 0 ) = iDV;
            }
        }

        //------------------------------------------------------------------------------
        bool Constitutive_Model::check_dv_dependency( const moris::Cell< PDV_Type > & aDvType )
        {
            // set bool for dependency
            bool tDvDependency = false;

            // get dv type index
            uint tDvIndex = static_cast< uint >( aDvType( 0 ) );

            // if aDvType is an active dv type for the constitutive model
            if( tDvIndex < mGlobalDvTypeMap.numel() && mGlobalDvTypeMap( tDvIndex ) != -1 )
            {
                // bool is set to true
                tDvDependency = true;
            }
            // return bool for dependency
            return tDvDependency;
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::set_field_interpolator_manager( Field_Interpolator_Manager * aFieldInterpolatorManager )
        {
            // set the field interpolator manager for the constitutive model
            mFIManager = aFieldInterpolatorManager;

            // loop over the underlying properties
            for( std::shared_ptr< Property > tProp : this->get_properties() )
            {
                if (tProp != nullptr )
                {
                    // set the field interpolator manager for the property
                    tProp->set_field_interpolator_manager( mFIManager );
                }
            }
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::get_non_unique_dof_types( moris::Cell< MSI::Dof_Type > & aDofTypes )
        {
            // initialize dof counter
            uint tCounter = 0;

            // loop over direct dof dependencies
            for ( uint iDOF = 0; iDOF < mDofTypes.size(); iDOF++ )
            {
                // update counter
                tCounter += mDofTypes( iDOF ).size();
            }

            // loop over properties
            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if ( tProperty != nullptr )
                {
                    // get property dof type list
                    moris::Cell< moris::Cell< MSI::Dof_Type > > tActiveDofType = tProperty->get_dof_type_list();

                    // loop over property dof types
                    for ( uint iDOF = 0; iDOF < tActiveDofType.size(); iDOF++ )
                    {
                        // update counter
                        tCounter += tActiveDofType( iDOF ).size();
                    }
                }
            }

            // reserve memory for the non unique dof type list
            aDofTypes.reserve( tCounter );

            // loop over direct dof dependencies
            for ( uint iDOF = 0; iDOF < mDofTypes.size(); iDOF++ )
            {
                // populate the dof type list
                aDofTypes.append( mDofTypes( iDOF ) );
            }

            // loop over the properties
            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if ( tProperty != nullptr )
                {
                    // get property dof type list
                    moris::Cell< moris::Cell< MSI::Dof_Type > > tActiveDofType = tProperty->get_dof_type_list();

                    // loop over property dof types
                    for ( uint iDOF = 0; iDOF < tActiveDofType.size(); iDOF++ )
                    {
                        // populate te dof type list
                        aDofTypes.append( tActiveDofType( iDOF ) );
                    }
                }
            }
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::get_non_unique_dof_and_dv_types(
                moris::Cell< MSI::Dof_Type > & aDofTypes,
                moris::Cell< PDV_Type >        & aDvTypes )
        {
            // initialize dof counter
            uint tDofCounter = 0;
            uint tDvCounter  = 0;

            // loop over direct dof dependencies
            for ( uint iDof = 0; iDof < mDofTypes.size(); iDof++ )
            {
                // update counter
                tDofCounter += mDofTypes( iDof ).size();
            }

            // loop over direct dv dependencies
            for ( uint iDv = 0; iDv < mDvTypes.size(); iDv++ )
            {
                // update counter
                tDvCounter += mDvTypes( iDv ).size();
            }

            // loop over properties
            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if ( tProperty != nullptr )
                {
                    // get property dof type list
                    moris::Cell< MSI::Dof_Type > tActiveDofTypes;
                    moris::Cell< PDV_Type >        tActiveDvTypes;
                    tProperty->get_non_unique_dof_and_dv_types( tActiveDofTypes,
                            tActiveDvTypes );

                    // update counter
                    tDofCounter += tActiveDofTypes.size();
                    tDvCounter  += tActiveDvTypes.size();
                }
            }

            // reserve memory for the non unique dof and dv types
            aDofTypes.reserve( tDofCounter );
            aDvTypes.reserve( tDvCounter );

            // loop over direct dof dependencies
            for ( uint iDof = 0; iDof < mDofTypes.size(); iDof++ )
            {
                // populate the dof type list
                aDofTypes.append( mDofTypes( iDof ) );
            }

            // loop over direct dv dependencies
            for ( uint iDv = 0; iDv < mDvTypes.size(); iDv++ )
            {
                // populate the dv type list
                aDvTypes.append( mDvTypes( iDv ) );
            }

            // loop over the properties
            for ( std::shared_ptr< Property > tProperty : mProperties )
            {
                if ( tProperty != nullptr )
                {
                    // get property dof and dv type list
                    moris::Cell< MSI::Dof_Type > tActiveDofTypes;
                    moris::Cell< PDV_Type >        tActiveDvTypes;
                    tProperty->get_non_unique_dof_and_dv_types( tActiveDofTypes,
                            tActiveDvTypes );

                    // populate the dof and dv type lists
                    aDofTypes.append( tActiveDofTypes );
                    aDvTypes.append( tActiveDvTypes );
                }
            }
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dFluxdDOF_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & adFluxdDOF_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRow = this->flux().n_rows();
            mdFluxdDof( tDofIndex ).set_size( tNumRow, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFI->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble the jacobian
                        mdFluxdDof( tDofIndex ).get_column( tDofCounter ) +=
                                        tFDScheme( 1 )( iPoint ) * this->flux() /
                                        ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }
                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );

            // FIXME
            adFluxdDOF_FD = mdFluxdDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dtractiondu_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & adtractiondu_FD,
                real                                 aPerturbation,
                Matrix< DDRMat >                   & aNormal,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFIDerivative =
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFIDerivative->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFIDerivative->get_number_of_space_time_bases();
            uint tDerNumFields = tFIDerivative->get_number_of_fields();

            // set size for derivative
            uint tNumRow = this->traction( aNormal ).n_rows();
            mdTractiondDof( tDofIndex ).set_size( tNumRow, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFIDerivative->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFIDerivative->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble the jacobian
                        mdTractiondDof( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->traction( aNormal ) /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

//                    // perturbation of the coefficient
//                    Matrix< DDRMat > tCoeffPert = tCoeff;
//                    tCoeffPert( iCoeffRow, iCoeffCol ) +=
//                            aPerturbation * tCoeffPert( iCoeffRow, iCoeffCol );
//
//                    // setting the perturbed coefficients
//                    tFIDerivative->set_coeff( tCoeffPert );
//
//                    // reset constitutive model eval flag
//                    this->reset_eval_flags();
//
//                    // evaluate the residual
//                    Matrix< DDRMat > tTractionPlus = this->traction( aNormal );
//
//                    // perturbation of the coefficient
//                    tCoeffPert = tCoeff;
//                    tCoeffPert( iCoeffRow, iCoeffCol ) -=
//                            aPerturbation * tCoeffPert( iCoeffRow, iCoeffCol );
//
//                    // setting the perturbed coefficients
//                    tFIDerivative->set_coeff( tCoeffPert );
//
//                    // reset constitutive model eval flags
//                    this->reset_eval_flags();
//
//                    // evaluate the residual
//                    Matrix< DDRMat > tTractionMinus = this->traction( aNormal );
//
//                    // evaluate Jacobian
//                    adtractiondu_FD.get_column( tDofCounter ) =
//                            ( tTractionPlus - tTractionMinus ) /
//                            ( 2.0 * aPerturbation * tCoeff( iCoeffRow, iCoeffCol ) );

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFIDerivative->set_coeff( tCoeff );

            // FIXME
            adtractiondu_FD = mdTractiondDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dtesttractiondu_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                const moris::Cell< MSI::Dof_Type > & aTestDofTypes,
                Matrix< DDRMat >                   & adtesttractiondu_FD,
                real                                 aPerturbation,
                Matrix< DDRMat >                   & aNormal,
                Matrix< DDRMat >                   & aJump,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the test dof index
            uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // get the derivative dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFIDerivative =
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFIDerivative->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFIDerivative->get_number_of_space_time_bases();
            uint tDerNumFields = tFIDerivative->get_number_of_fields();

            // set size for derivative
            Matrix< DDRMat > tTestTractionForSize = trans( trans( aJump ) * this->testTraction( aNormal, aTestDofTypes ) );
            uint tNumRow = tTestTractionForSize.n_rows();
            mdTestTractiondDof( tTestDofIndex )( tDofIndex ).set_size( tNumRow, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFIDerivative->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFIDerivative->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble the jacobian
                        mdTestTractiondDof( tTestDofIndex )( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * trans( trans( aJump ) * this->testTraction( aNormal, aTestDofTypes ) ) /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFIDerivative->set_coeff( tCoeff );

            // FIXME
            adtesttractiondu_FD = mdTestTractiondDof( tTestDofIndex )( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_ddivfluxdu_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & addivfluxdu_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFIDerivative =
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFIDerivative->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFIDerivative->get_number_of_space_time_bases();
            uint tDerNumFields = tFIDerivative->get_number_of_fields();

            // set size for derivative
            uint tNumRows = this->divflux().n_rows();
            mddivfluxdu( tDofIndex ).set_size( tNumRows, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFIDerivative->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFIDerivative->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble the jacobian
                        mddivfluxdu( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->divflux() /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFIDerivative->set_coeff( tCoeff );

            // FIXME
            addivfluxdu_FD = mddivfluxdu( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_ddivstraindu_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & addivstraindu_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFIDerivative =
                    mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFIDerivative->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFIDerivative->get_number_of_space_time_bases();
            uint tDerNumFields = tFIDerivative->get_number_of_fields();

            // set size for derivative
            uint tNumRows = this->divstrain().n_rows();
            mddivstraindu( tDofIndex ).set_size( tNumRows, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFIDerivative->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFIDerivative->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble the jacobian
                        mddivstraindu( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->divstrain() /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFIDerivative->set_coeff( tCoeff );

            // FIXME
            addivstraindu_FD = mddivstraindu( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dEnergyDotdDOF_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & adEnergyDotdDOF_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRows = this->EnergyDot().n_rows();
            mEnergyDotDof( tDofIndex ).set_size( tNumRows, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFI->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble dEnergyDotdu
                        mEnergyDotDof( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->EnergyDot() /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );

            // FIXME
            adEnergyDotdDOF_FD = mEnergyDotDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dGradEnergydDOF_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & adGradHdDOF_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRows = this->gradEnergy().n_rows();
            mGradEnergyDof( tDofIndex ).set_size( tNumRows, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFI->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble dGradHdu
                        mGradEnergyDof( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->gradEnergy() /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );

            // FIXME
            adGradHdDOF_FD = mGradEnergyDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dGradEnergyDotdDOF_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & adGradEnergyDotdDOF_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRows = this->gradEnergyDot().n_rows();
            mGradEnergyDotDof( tDofIndex ).set_size( tNumRows, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFI->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble dGradHdu
                        mGradEnergyDotDof( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->gradEnergyDot() /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );

            // FIXME
            adGradEnergyDotdDOF_FD = mGradEnergyDotDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dGradDivFluxdDOF_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & adGradDivFluxdDOF_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDof    = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRows = this->graddivflux().n_rows();
            mGradDivFluxDof( tDofIndex ).set_size( tNumRows, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFI->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble dGradHdu
                        mGradDivFluxDof( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->graddivflux() /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );

            // FIXME
            adGradDivFluxdDOF_FD = mGradDivFluxDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dStraindDOF_FD(
                const moris::Cell< MSI::Dof_Type > & aDofTypes,
                Matrix< DDRMat >                   & adStraindDOF_FD,
                real                                 aPerturbation,
                fem::FDScheme_Type                   aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );
            uint tNumPoints = tFDScheme( 0 ).size();

            // get the derivative dof type index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofTypes( 0 ) ) );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDofTypes( 0 ) );

            // get number of master dofs wrt which derivative is computed
            uint tDerNumDof    = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRow = this->strain().n_rows();
            mdStraindDof( tDofIndex ).set_size( tNumRow, tDerNumDof, 0.0 );

            // coefficients for dof type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dof counter
            uint tDofCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // compute the perturbation absolute value
                    real tDeltaH = aPerturbation * tCoeff( iCoeffRow, iCoeffCol );

                    // loop over the points for FD
                    for( uint iPoint = 0; iPoint < tNumPoints; iPoint++ )
                    {
                        // reset the perturbed coefficents
                        Matrix< DDRMat > tCoeffPert = tCoeff;

                        // pertub the coefficent
                        tCoeffPert( iCoeffRow, iCoeffCol ) += tFDScheme( 0 )( iPoint ) * tDeltaH;

                        // set the perturbed coefficients to FI
                        tFI->set_coeff( tCoeffPert );

                        // reset properties
                        this->reset_eval_flags();

                        // assemble dstraindu
                        mdStraindDof( tDofIndex ).get_column( tDofCounter ) +=
                                tFDScheme( 1 )( iPoint ) * this->strain() /
                                ( tFDScheme( 2 )( 0 ) * tDeltaH );
                    }

                    // update dof counter
                    tDofCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );

            // FIXME
            adStraindDOF_FD = mdStraindDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dFluxdDV_FD(
                const moris::Cell< PDV_Type > & aDvTypes,
                Matrix< DDRMat >              & adFluxdDV_FD,
                real                            aPerturbation,
                fem::FDScheme_Type              aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDvTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDv     = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRow = this->flux().n_rows();
            adFluxdDV_FD.set_size( tNumRow, tDerNumDv, 0.0 );

            // coefficients for dv type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dv counter
            uint tDvCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // perturbation of the coefficient
                    Matrix< DDRMat > tCoeffPert = tCoeff;
                    tCoeffPert( iCoeffRow, iCoeffCol ) += aPerturbation * tCoeffPert( iCoeffRow, iCoeffCol );

                    // setting the perturbed coefficients
                    tFI->set_coeff( tCoeffPert );

                    // reset constitutive model
                    this->reset_eval_flags();

                    // evaluate the residual
                    Matrix< DDRMat > tFlux_Plus = this->flux();

                    // perturbation of the coefficient
                    tCoeffPert = tCoeff;
                    tCoeffPert( iCoeffRow, iCoeffCol ) -= aPerturbation * tCoeffPert( iCoeffRow, iCoeffCol );

                    // setting the perturbed coefficients
                    tFI->set_coeff( tCoeffPert );

                    // reset constitutive model
                    this->reset_eval_flags();

                    // evaluate the residual
                    Matrix< DDRMat > tFlux_Minus = this->flux();

                    // evaluate Jacobian
                    adFluxdDV_FD.get_column( tDvCounter ) =
                            ( tFlux_Plus - tFlux_Minus ) / ( 2.0 * aPerturbation * tCoeff( iCoeffRow, iCoeffCol ) );

                    // update dv counter
                    tDvCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );
        }

        //------------------------------------------------------------------------------
        void Constitutive_Model::eval_dStraindDV_FD(
                const moris::Cell< PDV_Type > & aDvTypes,
                Matrix< DDRMat >              & adStraindDV_FD,
                real                            aPerturbation,
                fem::FDScheme_Type              aFDSchemeType )
        {
            // get the FD scheme info
            moris::Cell< moris::Cell< real > > tFDScheme;
            fd_scheme( aFDSchemeType, tFDScheme );

            // get the field interpolator for type
            Field_Interpolator* tFI = mFIManager->get_field_interpolators_for_type( aDvTypes( 0 ) );

            // get number of coefficients, fields and bases for the considered FI
            uint tDerNumDv     = tFI->get_number_of_space_time_coefficients();
            uint tDerNumBases  = tFI->get_number_of_space_time_bases();
            uint tDerNumFields = tFI->get_number_of_fields();

            // set size for derivative
            uint tNumRow = this->strain().n_rows();
            adStraindDV_FD.set_size( tNumRow, tDerNumDv, 0.0 );

            // coefficients for dv type wrt which derivative is computed
            Matrix< DDRMat > tCoeff = tFI->get_coeff();

            // initialize dv counter
            uint tDvCounter = 0;

            // loop over coefficients columns
            for( uint iCoeffCol = 0; iCoeffCol < tDerNumFields; iCoeffCol++ )
            {
                // loop over coefficients rows
                for( uint iCoeffRow = 0; iCoeffRow < tDerNumBases; iCoeffRow++ )
                {
                    // perturbation of the coefficient
                    Matrix< DDRMat > tCoeffPert = tCoeff;
                    tCoeffPert( iCoeffRow, iCoeffCol ) += aPerturbation * tCoeffPert( iCoeffRow, iCoeffCol );

                    // setting the perturbed coefficients
                    tFI->set_coeff( tCoeffPert );

                    // reset constitutive model
                    this->reset_eval_flags();

                    // evaluate the residual
                    Matrix< DDRMat > tStrain_Plus = this->strain();

                    // perturbation of the coefficient
                    tCoeffPert = tCoeff;
                    tCoeffPert( iCoeffRow, iCoeffCol ) -= aPerturbation * tCoeffPert( iCoeffRow, iCoeffCol );

                    // setting the perturbed coefficients
                    tFI->set_coeff( tCoeffPert );

                    // reset constitutive model
                    this->reset_eval_flags();

                    // evaluate the residual
                    Matrix< DDRMat > tStrain_Minus = this->strain();

                    // evaluate Jacobian
                    adStraindDV_FD.get_column( tDvCounter ) =
                            ( tStrain_Plus - tStrain_Minus ) / ( 2.0 * aPerturbation * tCoeff( iCoeffRow, iCoeffCol ) );

                    // update dv counter
                    tDvCounter++;
                }
            }
            // reset the coefficients values
            tFI->set_coeff( tCoeff );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::flux()
        {
            // if the flux was not evaluated
            if( mFluxEval )
            {
                // evaluate the flux
                this->eval_flux();

                // set bool for evaluation
                mFluxEval = false;
            }
            // return the flux value
            return mFlux;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::divflux()
        {
            // if the divergence of the flux was not evaluated
            if( mDivFluxEval )
            {
                // evaluate the divergence of the flux
                this->eval_divflux();

                // set bool for evaluation
                mDivFluxEval = false;
            }
            // return the divergence of the flux value
            return mDivFlux;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::EnergyDot()
        {
            // if the flux was not evaluated
            if( mEnergyDotEval)
            {
                // evaluate the flux
                this->eval_EnergyDot();

                // set bool for evaluation
                mEnergyDotEval = false;
            }
            // return the flux value
            return mEnergyDot;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::gradEnergyDot()
        {
            // if the flux was not evaluated
            if( mGradEnergyDotEval)
            {
                // evaluate the flux
                this->eval_gradEnergyDot();

                // set bool for evaluation
                mGradEnergyDotEval = false;
            }
            // return the flux value
            return mGradEnergyDot;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::gradEnergy()
        {
            // if the flux was not evaluated
            if( mGradEnergyEval)
            {
                // evaluate the flux
                this->eval_gradEnergy();

                // set bool for evaluation
                mGradEnergyEval = false;
            }
            // return the flux value
            return mGradEnergy;
        }


        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::graddivflux()
        {
            // if the flux was not evaluated
            if( mGradDivFluxEval)
            {
                // evaluate the flux
                this->eval_graddivflux();

                // set bool for evaluation
                mGradDivFluxEval = false;
            }
            // return the flux value
            return mGradDivFlux;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::ddivfluxdu( const moris::Cell< MSI::Dof_Type > & aDofType )
        {
            // if aDofType is not an active dof type for the CM
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::ddivfluxdu - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative of the divergence of the flux was not evaluated
            if( mddivfluxduEval( tDofIndex ) )
            {
                // evaluate the derivative of the divergence of the flux
                this->eval_ddivfluxdu( aDofType );

                // set bool for evaluation
                mddivfluxduEval( tDofIndex ) = false;
            }
            // return the divergence of the flux value
            return mddivfluxdu( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::traction( const Matrix< DDRMat > & aNormal )
        {
            // if the traction was not evaluated
            if( mTractionEval )
            {
                // evaluate the traction
                this->eval_traction( aNormal );

                // set bool for evaluation
                mTractionEval = false;
            }
            // return the traction value
            return mTraction;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::testTraction(
                const Matrix< DDRMat >             & aNormal,
                const moris::Cell< MSI::Dof_Type > & aTestDofTypes )
        {
            // get test dof type index
            uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // if the test traction was not evaluated
            if( mTestTractionEval( tTestDofIndex ) )
            {
                // evaluate the test traction
                this->eval_testTraction( aNormal, aTestDofTypes );

                // set bool for evaluation
                mTestTractionEval( tTestDofIndex ) = false;
            }
            // return the test traction value
            return mTestTraction( tTestDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::strain()
        {
            // if the strain was not evaluated
            if( mStrainEval )
            {
                // evaluate the strain
                this->eval_strain();

                // set bool for evaluation
                mStrainEval = false;
            }
            // return the strain value
            return mStrain;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::divstrain()
        {
            // if the divergence of the strain was not evaluated
            if( mDivStrainEval )
            {
                // evaluate the divergence of the strain
                this->eval_divstrain();

                // set bool for evaluation
                mDivStrainEval = false;
            }
            // return the divergence of the strain value
            return mDivStrain;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::ddivstraindu( const moris::Cell< MSI::Dof_Type > & aDofType )
        {
            // if aDofType is not an active dof type for the CM
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::ddivstraindu - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative of the divergence of the strain was not evaluated
            if( mddivstrainduEval( tDofIndex ) )
            {
                // evaluate the derivative of the divergence of the strain
                this->eval_ddivstraindu( aDofType );

                // set bool for evaluation
                mddivstrainduEval( tDofIndex ) = false;
            }
            // return the divergence of the strain value
            return mddivstraindu( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::testStrain()
        {
            // if the test strain was not evaluated
            if( mTestStrainEval )
            {
                // evaluate the test strain
                this->eval_testStrain();

                // set bool for evaluation
                mTestStrainEval = false;
            }
            // return the test strain value
            return mTestStrain;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::constitutive()
        {
            // if the constitutive matrix was not evaluated
            if( mConstEval )
            {
                // evaluate the constitutive matrix
                this->eval_const();

                // set bool for evaluation
                mConstEval = false;
            }
            // return the constitutive matrix value
            return mConst;
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dfluxdx( uint aOrder )
        {
            MORIS_ERROR(
                    aOrder == 1,
                    "Constitutive_Model::dfluxdx - Works only for 1st order derivative for now." );

            // if the derivative has not been evaluated yet
            if( mdFluxdxEval( aOrder - 1 ) )
            {
                // evaluate the derivative
                this->eval_dfluxdx( aOrder );

                // set bool for evaluation
                mdFluxdxEval( aOrder - 1 ) = false;
            }

            // return the derivative
            return mdFluxdx( aOrder - 1 );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofType )
        {
            // if aDofType is not an active dof type for the CM
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dFluxdDOF - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdFluxdDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dFluxdDOF( aDofType );

                // set bool for evaluation
                mdFluxdDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mdFluxdDof( tDofIndex );
        }

        //-----------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dEnergyDotdDOF( const moris::Cell< MSI::Dof_Type > & aDofType)
        {
            // if aDofType is not an active dof type for the CM
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dEnergyDotdDOF - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mEnergyDotDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dEnergyDotdDOF( aDofType );

                // set bool for evaluation
                mEnergyDotDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mEnergyDotDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dGradHdDOF( const moris::Cell< MSI::Dof_Type > & aDofType)
        {
            // if aDofType is not an active dof type for the CM
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dGradHdDOF - no dependency on this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mGradEnergyDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dGradEnergydDOF( aDofType );

                // set bool for evaluation
                mGradEnergyDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mGradEnergyDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dGradEnergyDotdDOF( const moris::Cell< MSI::Dof_Type > & aDofType)
        {
            // if aDofType is not an active dof type for the CM
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dGradHdDOF - no dependency on this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mGradEnergyDotDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dGradEnergyDotdDOF( aDofType );

                // set bool for evaluation
                mGradEnergyDotDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mGradEnergyDotDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dGradDivFluxdDOF( const moris::Cell< MSI::Dof_Type > & aDofType)
        {
            // if aDofType is not an active dof type for the CM
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dGradDivFluxdDOF - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mGradDivFluxDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dGradDivFluxdDOF( aDofType );

                // set bool for evaluation
                mGradDivFluxDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mGradDivFluxDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dTractiondDOF(
                const moris::Cell< MSI::Dof_Type > & aDofType,
                const Matrix< DDRMat >             & aNormal )
        {
            // if aDofType is not an active dof type for the property
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dTractiondDOF - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdTractiondDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dTractiondDOF( aDofType, aNormal );

                // set bool for evaluation
                mdTractiondDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mdTractiondDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dTestTractiondDOF(
                const moris::Cell< MSI::Dof_Type > & aDofType,
                const Matrix< DDRMat >             & aNormal,
                const moris::Cell< MSI::Dof_Type > & aTestDofTypes )
        {
            // if aDofType is not an active dof type for the property
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dTestTractiondDOF - no dependency in this dof type." );

            // get the test dof index
            uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdTestTractiondDofEval( tTestDofIndex )( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dTestTractiondDOF( aDofType, aNormal, aTestDofTypes );

                // set bool for evaluation
                mdTestTractiondDofEval( tTestDofIndex )( tDofIndex ) = false;
            }

            // return the derivative
            return mdTestTractiondDof( tTestDofIndex )( tDofIndex );
        }

        //------------------------------------------------------------------------------

        const Matrix< DDRMat > & Constitutive_Model::dTestTractiondDOF(
                const moris::Cell< MSI::Dof_Type > & aDofType,
                const Matrix< DDRMat >             & aNormal,
                const Matrix< DDRMat >             & aJump,
                const moris::Cell< MSI::Dof_Type > & aTestDofTypes )
        {
            // if aDofType is not an active dof type for the property
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dTestTractiondDOF - no dependency in this dof type." );

            // get the test dof index
            uint tTestDofIndex = mDofTypeMap( static_cast< uint >( aTestDofTypes( 0 ) ) );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdTestTractiondDofEval( tTestDofIndex )( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dTestTractiondDOF( aDofType, aNormal, aJump, aTestDofTypes );

                // set bool for evaluation
                mdTestTractiondDofEval( tTestDofIndex )( tDofIndex ) = false;
            }

            // return the derivative
            return mdTestTractiondDof( tTestDofIndex )( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dstraindx( uint aOrder )
        {
            MORIS_ERROR(
                    aOrder == 1,
                    "Constitutive_Model::dstraindx - Works only for 1st order derivative for now." );

            // if the derivative has not been evaluated yet
            if( mdStraindxEval( aOrder - 1 ) )
            {
                // evaluate the derivative
                this->eval_dstraindx( aOrder );

                // set bool for evaluation
                mdStraindxEval( aOrder - 1 ) = false;
            }

            // return the derivative
            return mdStraindx( aOrder - 1 );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dStraindDOF( const moris::Cell< MSI::Dof_Type > & aDofType )
        {
            // if aDofType is not an active dof type for the property
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dStraindDOF - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdStraindDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dStraindDOF( aDofType );

                // set bool for evaluation
                mdStraindDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mdStraindDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dConstdDOF( const moris::Cell< MSI::Dof_Type > & aDofType )
        {
            // if aDofType is not an active dof type for the property
            MORIS_ERROR(
                    this->check_dof_dependency( aDofType ),
                    "Constitutive_Model::dConstdDOF - no dependency in this dof type." );

            // get the dof index
            uint tDofIndex = mGlobalDofTypeMap( static_cast< uint >( aDofType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdConstdDofEval( tDofIndex ) )
            {
                // evaluate the derivative
                this->eval_dConstdDOF( aDofType );

                // set bool for evaluation
                mdConstdDofEval( tDofIndex ) = false;
            }

            // return the derivative
            return mdConstdDof( tDofIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dFluxdDV( const moris::Cell< PDV_Type > & aDvType )
        {
            // if aDvType is not an active dv type
            MORIS_ERROR(
                    this->check_dv_dependency( aDvType ),
                    "Constitutive_Model::dFluxdDV - no dependency in this dv type." );

            // get the dv index
            uint tDvIndex = mGlobalDvTypeMap( static_cast< uint >( aDvType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdFluxdDvEval( tDvIndex ) )
            {
                // evaluate the derivative
                this->eval_dFluxdDV( aDvType );

                // set bool for evaluation
                mdFluxdDvEval( tDvIndex ) = false;
            }

            // return the derivative
            return mdFluxdDv( tDvIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dStraindDV( const moris::Cell< PDV_Type > & aDvType )
        {
            // if aDvType is not an active dv type for the property
            MORIS_ERROR(
                    this->check_dv_dependency( aDvType ),
                    "Constitutive_Model::dStraindDV - no dependency in this dv type." );

            // get the dv index
            uint tDvIndex = mGlobalDvTypeMap( static_cast< uint >( aDvType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdStraindDvEval( tDvIndex ) )
            {
                // evaluate the derivative
                this->eval_dStraindDV( aDvType );

                // set bool for evaluation
                mdStraindDvEval( tDvIndex ) = false;
            }

            // return the derivative
            return mdStraindDv( tDvIndex );
        }

        //------------------------------------------------------------------------------
        const Matrix< DDRMat > & Constitutive_Model::dConstdDV( const moris::Cell< PDV_Type > & aDvType )
        {
            // if aDvType is not an active dv type for the property
            MORIS_ERROR(
                    this->check_dv_dependency( aDvType ),
                    "Constitutive_Model::dConstdDV - no dependency in this dv type." );

            // get the dv index
            uint tDvIndex = mGlobalDvTypeMap( static_cast< uint >( aDvType( 0 ) ) );

            // if the derivative has not been evaluated yet
            if( mdConstdDvEval( tDvIndex ) )
            {
                // evaluate the derivative
                this->eval_dConstdDV( aDvType );

                // set bool for evaluation
                mdConstdDvEval( tDvIndex ) = false;
            }

            // return the derivative
            return mdConstdDv( tDvIndex );
        }

    }/* end_fem_namespace */
}/* end_moris_namespace */
