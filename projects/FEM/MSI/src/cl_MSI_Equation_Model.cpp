/*
 * cl_Equation_Model.cpp
 *
 *  Created on: May 08, 2020
 *      Author: schmidt
 */

#include "cl_MSI_Design_Variable_Interface.hpp"
#include "cl_MSI_Solver_Interface.hpp"

#include "cl_MSI_Equation_Object.hpp"
#include "cl_MSI_Equation_Set.hpp"
#include "cl_MSI_Equation_Model.hpp"

#include "cl_SOL_Dist_Vector.hpp"
#include "cl_Matrix_Vector_Factory.hpp"
#include "cl_SOL_Enums.hpp"

#include "cl_Logger.hpp"

namespace moris
{
    namespace MSI
    {
        //------------------------------------------------------------------------------

        moris::sint Equation_Model::get_num_rhs( )
        {
            if( !mIsForwardAnalysis )
            {
                mNumSensitivityAnalysisRHS = this->get_requested_IQI_names().size();

                MORIS_ASSERT( mNumSensitivityAnalysisRHS > 0,
                        "MSI::Equation_Model::get_num_rhs(), num rhs not set for sensitivity analysis");

                return mNumSensitivityAnalysisRHS;
            }
            else
            {
                return 1;
            }
        }

        //------------------------------------------------------------------------------

        void Equation_Model::initialize_IQIs()
        {
            moris::uint tNumIQIs = this->get_requested_IQI_names().size();

            mGlobalIQIVal.resize( tNumIQIs );

            for( auto & tQI : mGlobalIQIVal )
            {
                // set size for the QI value
                // FIXME assumed scalar
                tQI.set_size( 1, 1, 0.0 );
            }
        }

        //------------------------------------------------------------------------------

        void Equation_Model::compute_IQIs()
        {
            // get number of IQI on the model
            moris::uint tNumIQIsOnModel =
                    this->get_requested_IQI_names().size();

            // get local number of equation sets
            moris::uint tNumSets = mFemSets.size();

            // loop over local equation sets
            for ( moris::uint tSetIndex = 0; tSetIndex < tNumSets; tSetIndex++ )
            {
                // get number of IQIs on treated equation set
                moris::uint tNumIQIsOnSet = mFemSets( tSetIndex )->get_number_of_requested_IQIs();

                // if some IQI are requested on treated equation set
                if( tNumIQIsOnSet > 0 )
                {
                    // get number of equation objects on treated equation set
                    moris::uint tNumEquationObjectOnSet = mFemSets( tSetIndex )->get_num_equation_objects();

                    // initialize treated equation set // FIXME????
                    mFemSets( tSetIndex )->initialize_set( true );

                    // loop over equation objects on treated equation set
                    for( moris::uint tEquationObjectIndex = 0; tEquationObjectIndex < tNumEquationObjectOnSet; tEquationObjectIndex++ )
                    {
                        // compute QI
                        // FIXME this is elemental right now??
                        mFemSets( tSetIndex )->get_equation_object_list()( tEquationObjectIndex )->compute_QI();

                        // loop over IQIs on model
                        for( moris::uint tIQIIndex = 0; tIQIIndex < tNumIQIsOnModel; tIQIIndex++ )
                        {
                            // assemble QI values into global vector
                            mGlobalIQIVal( tIQIIndex )( 0 ) += mFemSets( tSetIndex )->get_QI()( tIQIIndex )( 0 );
                        }
                    }
                    // free memory on treated equation set
                    mFemSets( tSetIndex )->free_matrix_memory();
                }
            }

            // Normalization
            if (gLogger.mIteration == 0)
            {
                this->normalize_IQIs();
            }
        }

        //------------------------------------------------------------------------------

        void Equation_Model::initialize_explicit_and_implicit_dQIdp()
        {
            // create map for dQIdpMap
            moris::Matrix_Vector_Factory tMatFactory( sol::MapType::Epetra );
            mdQIdpMap = tMatFactory.create_map(
                    mDesignVariableInterface->get_my_local_global_map() );

            // get number of RHS
            uint tNumRHMS = this->get_num_rhs();

            // create vector for dQIdp implicit and explicit contributions
            if ( mImplicitdQidp != nullptr )
            {
                delete mImplicitdQidp;
            }

            if ( mImplicitdQidp != nullptr )
            {
                delete mExplicitdQidp;
            }

            mImplicitdQidp = tMatFactory.create_vector( mdQIdpMap, tNumRHMS );
            mExplicitdQidp = tMatFactory.create_vector( mdQIdpMap, tNumRHMS );

            // fill dQIdp implicit/explicit vectors with zeros
            mExplicitdQidp->vec_put_scalar( 0.0 );
            mImplicitdQidp->vec_put_scalar( 0.0 );
        }

        //------------------------------------------------------------------------------

        void Equation_Model::compute_implicit_dQIdp()
        {
            // get local number of equation sets
            moris::uint tNumSets = mFemSets.size();

            // loop over local equation sets
            for ( moris::uint tSetIndex = 0; tSetIndex < tNumSets; tSetIndex++ )
            {
                // get number of equation object on treated equation set
                moris::uint tNumEquationObjectOnSet =
                        mFemSets( tSetIndex )->get_num_equation_objects();

                // initialize treated equation set //FIXME????
                mFemSets( tSetIndex )->initialize_set( true );

                // loop over equation objects on treated equation set
                for ( moris::uint tEquationObjectIndex = 0; tEquationObjectIndex < tNumEquationObjectOnSet; tEquationObjectIndex++ )
                {
                    // compute dQIdp implicit
                    mFemSets( tSetIndex )->
                            get_equation_object_list()( tEquationObjectIndex )->
                            compute_dQIdp_implicit();
                }
                // free memory on treated equation set
                mFemSets( tSetIndex )->free_matrix_memory();
            }

            // global assembly to switch entries to the right processor
            mImplicitdQidp->vector_global_asembly();
            //mImplicitdQidp->print();
        }

        //------------------------------------------------------------------------------

        void Equation_Model::compute_explicit_dQIdp()
        {
            // get local number of equation sets
            moris::uint tNumSets = mFemSets.size();

            // loop over local equation sets
            for ( moris::uint iSet = 0; iSet < tNumSets; iSet++ )
            {
                // get number of equation objects on treated equation set
                moris::uint tNumEquationObjectOnSet =
                        mFemSets( iSet )->get_num_equation_objects();

                // initialize treated equation set //FIXME????
                mFemSets( iSet )->initialize_set( true );

                // if some IQI are requested on treated equation set
                if( mFemSets( iSet )->get_number_of_requested_IQIs() > 0 )
                {
                    // loop over equation objects on treated equation set
                    for ( moris::uint iEqObj = 0; iEqObj < tNumEquationObjectOnSet; iEqObj++ )
                    {
                        // compute dQIdp explicit
                        mFemSets( iSet )->
                                get_equation_object_list()( iEqObj )->
                                compute_dQIdp_explicit();
                    }
                    // free memory on treated equation set
                    mFemSets( iSet )->free_matrix_memory();
                }
            }
            // global assembly to switch entries to the right processor
            mExplicitdQidp->vector_global_asembly();
            //mExplicitdQidp->print();
        }

        //-------------------------------------------------------------------------------------------------

        sol::Dist_Vector * Equation_Model::get_dQIdp()
        {
            // create map object
            moris::Matrix_Vector_Factory tMatFactory( sol::MapType::Epetra );

            // get number of RHD
            uint tNumRHMS = this->get_num_rhs();

            // create vector for dQIdp
            mdQIdp = tMatFactory.create_vector( mdQIdpMap, tNumRHMS );

            // fill vector with zero
            mdQIdp->vec_put_scalar( 0.0 );

            // add explicit contribution to dQIdp
            mdQIdp->vec_plus_vec( 1.0, *mExplicitdQidp, 1.0 );

            // add implicit contribution to dQIdp
            mdQIdp->vec_plus_vec( 1.0, *mImplicitdQidp, 1.0 );

            // return dQIdp
            return mdQIdp;
        }

        //-------------------------------------------------------------------------------------------------

        void Equation_Model::normalize_IQIs()
        {
        }

        //-------------------------------------------------------------------------------------------------

    }/* end_namespace_msi */
}/* end_namespace_moris */
