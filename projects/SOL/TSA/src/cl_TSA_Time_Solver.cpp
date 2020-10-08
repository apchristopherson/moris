/*
 * cl_TSA_Time_Solver.cpp
 *
 *  Created on: Okt 10, 2018
 *      Author: schmidt
 */
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_SOL_Matrix_Vector_Factory.hpp"

#include "cl_SOL_Dist_Map.hpp"
#include "cl_SOL_Dist_Vector.hpp"
#include "cl_SOL_Warehouse.hpp"

#include "cl_TSA_Time_Solver.hpp"
#include "cl_TSA_Time_Solver_Factory.hpp"
#include "cl_TSA_Time_Solver_Algorithm.hpp"

#include "fn_Parsing_Tools.hpp"
#include "fn_PRM_SOL_Parameters.hpp"

#include "cl_Communication_Tools.hpp"
#include "cl_Logger.hpp"

// Detailed Logging package
//#include "cl_Tracer.hpp"

using namespace moris;
using namespace tsa;

//-------------------------------------------------------------------------------

Time_Solver::Time_Solver( const enum TimeSolverType aTimeSolverType )
: mTimeSolverType( aTimeSolverType )
{
    // create solver factory
    Time_Solver_Factory  tTimeSolFactory;

    // create solver object
    std::shared_ptr< Time_Solver_Algorithm > tTimeSolver = tTimeSolFactory.create_time_solver( aTimeSolverType );

    mTimeSolverAlgorithmList.clear();

    mTimeSolverAlgorithmList.resize( 1, nullptr );

    mTimeSolverAlgorithmList( 0 ) = tTimeSolver;

    mDofTypeList.resize( 0 );

    this->set_time_solver_parameters();
}

//--------------------------------------------------------------------------------------------------

Time_Solver::Time_Solver(
        const ParameterList         aParameterlist,
        sol::SOL_Warehouse        * aSolverWarehouse,
        const enum tsa::TimeSolverType   aTimeSolverType )
: mParameterListTimeSolver( aParameterlist ),
  mSolverWarehouse( aSolverWarehouse ),
  mSolverInterface( mSolverWarehouse->get_solver_interface() ),
  mTimeSolverType( aTimeSolverType )
{
    mDofTypeList.resize( 0 );

    this->initialize_time_levels();
}

//--------------------------------------------------------------------------------------------------

Time_Solver::Time_Solver(
        moris::Cell< std::shared_ptr< Time_Solver_Algorithm > > & aTimeSolverList,
        const enum TimeSolverType                                 aTimeSolverType )
: mTimeSolverType( aTimeSolverType )
{
    mTimeSolverAlgorithmList = aTimeSolverList;

    this->set_time_solver_parameters();
}

//--------------------------------------------------------------------------------------------------

Time_Solver::~Time_Solver()
{
    this->delete_pointers();
}

//--------------------------------------------------------------------------------------------------

void Time_Solver::delete_pointers()
{
    if( mIsMasterTimeSolver )
    {
        for( auto tFullSolVec : mFullVector)
        {
            delete tFullSolVec;
        }

        mFullVector.clear();

        for( auto tFullSolVec : mFullVectorSensitivity)
        {
            delete tFullSolVec;
        }

        mFullVectorSensitivity.clear();

        mTimeFrames.clear();
    }
}

//--------------------------------------------------------------------------------------------------

void Time_Solver::set_dof_type_list(
        const moris::Cell< enum MSI::Dof_Type > aDofTypeList,
        const moris::sint                       aLevel )
{
    mDofTypeList.push_back( aDofTypeList );
}

//--------------------------------------------------------------------------------------------------

void Time_Solver::set_time_solver_algorithm( std::shared_ptr< Time_Solver_Algorithm > aTimeSolver )
{
    if( mCallCounter == 0 )
    {
        // removes all elements from the Cell and destroy them
        mTimeSolverAlgorithmList.clear();

        // Resize the Cell to size = 1
        mTimeSolverAlgorithmList.resize( 1, nullptr );

        // Set nonlinear solver on first entry
        mTimeSolverAlgorithmList( 0 ) =  aTimeSolver;
    }
    else
    {
        // set nonlinear solver on next entry
        mTimeSolverAlgorithmList.push_back( aTimeSolver );
    }

    mCallCounter = mCallCounter + 1;
}

//-------------------------------------------------------------------------------------------------------

void Time_Solver::set_time_solver_algorithm(
        std::shared_ptr< Time_Solver_Algorithm > aTimeSolver,
        const moris::uint                        aListEntry )
{
    // Check if list is smaller than given entry
    if( aListEntry >= mTimeSolverAlgorithmList.size() )
    {
        // Resize to new entry value and set nullptr on new entries
        mTimeSolverAlgorithmList.resize( aListEntry + 1, nullptr );
    }

    // Set nonlinear solver on entry
    mTimeSolverAlgorithmList( aListEntry ) = aTimeSolver;
}

//-------------------------------------------------------------------------------------------------------

void Time_Solver::set_sub_time_solver( Time_Solver * aTimeSolver )
{
    if( mCallCounterTimeSolver == 0 )
    {
        // removes all elements from the Cell and destroy them
        mTimeSubSolverList.clear();

        // Resize the Cell to size = 1
        mTimeSubSolverList.resize( 1, nullptr );

        // Set nonlinear solver on first entry
        mTimeSubSolverList( 0 ) =  aTimeSolver;
    }
    else
    {
        // set nonlinear solver on next entry
        mTimeSubSolverList.push_back( aTimeSolver );
    }

    mCallCounterTimeSolver = mCallCounterTimeSolver + 1;
}

//-------------------------------------------------------------------------------------------------------

void Time_Solver::set_sub_time_solver(
        Time_Solver       * aTimeSolver,
        const moris::uint   aListEntry )
{
    // Check if list is smaller than given entry
    if( aListEntry >= mTimeSubSolverList.size() )
    {
        // Resize to new entry value and set nullptr on new entries
        mTimeSubSolverList.resize( aListEntry + 1, nullptr );
    }

    // Set nonlinear solver on entry
    mTimeSubSolverList( aListEntry ) = aTimeSolver;
}

//-------------------------------------------------------------------------------------------------------

moris::Cell< enum MSI::Dof_Type > Time_Solver::get_dof_type_union()
{
    moris::sint tCounter = 0;

    // Loop over all dof type lists to determine the total number of dof types
    for ( moris::uint Ik = 0; Ik < mDofTypeList.size(); ++Ik )
    {
        tCounter = tCounter + mDofTypeList( Ik ).size();
    }

    // Create list of dof types with earlier determines size
    moris::Cell< enum MSI::Dof_Type > tUnionEnumList( tCounter );
    tCounter = 0;

    // Loop over all dof types. Add them to union list
    for ( moris::uint Ik = 0; Ik < mDofTypeList.size(); ++Ik )
    {
        for ( moris::uint Ii = 0; Ii < mDofTypeList( Ik ).size(); ++Ii )
        {
            tUnionEnumList( tCounter++ ) = mDofTypeList( Ik )( Ii );
        }
    }

    return tUnionEnumList;
}

//--------------------------------------------------------------------------------------------------

void Time_Solver::set_solver_warehouse( sol::SOL_Warehouse * aSolverWarehouse )
{
    mSolverWarehouse = aSolverWarehouse;

    mSolverInterface = mSolverWarehouse->get_solver_interface() ;
}

//-------------------------------------------------------------------------------------------------------

void Time_Solver::set_output(
        const uint      aOutputIndex,
        Output_Criteria aOutputCriteria)
{
    mOutputIndices.push_back( aOutputIndex );
    mOutputCriteriaPointer.push_back( aOutputCriteria );
}

//-------------------------------------------------------------------------------------------------------

void Time_Solver::check_for_outputs(
        const moris::real & aTime,
        const bool          aEndOfTimeIteration )
{
    if( mIsForwardSolve )
    {
        uint tCounter = 0;

        // loop over all outputs and check if it is triggered
        for( Output_Criteria tOutputCriterias : mOutputCriteriaPointer )
        {
            bool tIsOutput = false;

            if( tOutputCriterias == nullptr )
            {
                tIsOutput = true;
            }
            else
            {
                tIsOutput = tOutputCriterias( this );
            }

            if( tIsOutput )
            {
                MORIS_LOG_INFO(" Initiate output for output index %-5i", mOutputIndices( tCounter ) );

                mSolverInterface->initiate_output( mOutputIndices( tCounter ), aTime, aEndOfTimeIteration );
            }

            tCounter++;
        }
    }
}

//-------------------------------------------------------------------------------------------------------

void Time_Solver::solve( moris::Cell< sol::Dist_Vector * > & aFullVector )
{
    moris::Cell< enum MSI::Dof_Type > tDofTypeUnion = this->get_dof_type_union();

    mSolverInterface->set_requested_dof_types( tDofTypeUnion );

    mFullVector = aFullVector;

    mTimeSolverAlgorithmList( 0 )->set_time_solver( this );

    mTimeSolverAlgorithmList( 0 )->solve( aFullVector );

    //this->check_for_outputs();
}

//--------------------------------------------------------------------------------------------------------------------------

void Time_Solver::solve()
{
    // flags if thats the master time solver and if this is a forward solve
    mIsMasterTimeSolver = true;
    mIsForwardSolve = true;

    // delete pointers
    this->delete_pointers();

    // get solver interface
    mSolverInterface = mSolverWarehouse->get_solver_interface();

    mSolverInterface->set_is_forward_analysis();

    // create map object
    sol::Matrix_Vector_Factory tMatFactory( mSolverWarehouse->get_tpl_type() );

    // build full overlapping map
    mFullMap = tMatFactory.create_map( mSolverInterface->get_my_local_global_overlapping_map() );

    // get number of RHS
    uint tNumRHMS = mSolverInterface->get_num_rhs();

    // set size for full solution vector on timestep 0 and previous solution vector on timestep -1.
    mFullVector.resize( 2, nullptr);

    // full vector and prev full vector
    mFullVector( 0 ) = tMatFactory.create_vector( mSolverInterface, mFullMap, tNumRHMS );
    mFullVector( 1 ) = tMatFactory.create_vector( mSolverInterface, mFullMap, tNumRHMS );

    // set time level 0 sol vec to interface
    mSolverInterface->set_solution_vector( mFullVector( 1 ));
    mSolverInterface->set_solution_vector_prev_time_step( mFullVector( 0 ) );

    // initialize solution vector and prev solution vector
    this->initialize_sol_vec();
    this->initialize_prev_sol_vec();

    moris::Cell< enum MSI::Dof_Type > tDofTypeUnion = this->get_dof_type_union();

    mSolverInterface->set_requested_dof_types( tDofTypeUnion );

    mTimeSolverAlgorithmList( 0 )->set_time_solver( this );

    mTimeSolverAlgorithmList( 0 )->solve( mFullVector );
}

//--------------------------------------------------------------------------------------------------------------------------

void Time_Solver::solve_sensitivity()
{
    mIsForwardSolve = false;

    // create map object
    sol::Matrix_Vector_Factory tMatFactory( mSolverWarehouse->get_tpl_type() );

    uint tNumRHMS = mSolverInterface->get_num_rhs();

    mSolverInterface->set_is_sensitivity_analysis();

    // full vector and previous full vector
    mFullVectorSensitivity.resize( 2, nullptr );
    mFullVectorSensitivity( 0 ) = tMatFactory.create_vector( mSolverInterface, mFullMap, tNumRHMS );
    mFullVectorSensitivity( 1 ) = tMatFactory.create_vector( mSolverInterface, mFullMap, tNumRHMS );

    mFullVectorSensitivity( 0 )->vec_put_scalar( 0.0 );
    mFullVectorSensitivity( 1 )->vec_put_scalar( 0.0 );

    mSolverInterface->set_adjoint_solution_vector( mFullVectorSensitivity( 0 ) );
    mSolverInterface->set_previous_adjoint_solution_vector( mFullVectorSensitivity( 1 ) );

    moris::Cell< enum MSI::Dof_Type > tDofTypeUnion = this->get_dof_type_union();

    mSolverInterface->set_requested_dof_types( tDofTypeUnion );

    mTimeSolverAlgorithmList( 0 )->set_time_solver( this );

    mTimeSolverAlgorithmList( 0 )->solve( mFullVectorSensitivity );
}

//--------------------------------------------------------------------------------------------------------------------------

void Time_Solver::initialize_sol_vec()
{
    // extract initialization string from parameter list
    moris::Cell< moris::Cell< std::string > > tDofTypeAndValuePair;

    string_to_cell_of_cell( mParameterListTimeSolver.get< std::string >( "TSA_Initialize_Sol_Vec" ),
            tDofTypeAndValuePair );

    // get string to dof type map
    map< std::string, enum MSI::Dof_Type > tDofTypeMap = MSI::get_msi_dof_type_map();

    // initialize solution vector with zero
    mFullVector( 1 )->vec_put_scalar( 0.0 );

    if( tDofTypeAndValuePair.size() > 0 )
    {
        // create map object
        sol::Matrix_Vector_Factory tMatFactory( mSolverWarehouse->get_tpl_type() );

        std::shared_ptr<sol::Dist_Map>  tFeeMap = tMatFactory.create_map( mSolverInterface->get_my_local_global_map() );

        uint tNumRHMS = mSolverInterface->get_num_rhs();

        // full vector and previous full vector
        sol::Dist_Vector * tFreeVector = tMatFactory.create_vector( mSolverInterface, tFeeMap, tNumRHMS );

        // initialize solution vector
        tFreeVector->vec_put_scalar( 0.0 );

        // loop over input dof types
        for( uint Ik = 0; Ik < tDofTypeAndValuePair.size(); Ik++ )
        {
            // First string is dof type
            moris::Cell< enum MSI::Dof_Type > tDofType = { tDofTypeMap.find( tDofTypeAndValuePair( Ik )( 0 ) ) };

            // get local global ids for this dof type
            moris::Matrix< IdMat > tAdofIds = mSolverInterface->get_my_local_global_map( tDofType );

            // get value from input
            moris::real tValue = std::stod( tDofTypeAndValuePair( Ik )( 1 ) );

            // add value into Matrix
            moris::Matrix< DDRMat > tValues( tAdofIds.numel(), 1, tValue );

            if( tDofTypeAndValuePair( Ik ).size() == 2 )
            {
                // replace initial values in solution vector
                tFreeVector->replace_global_values(
                        tAdofIds,
                        tValues );
            }
            else if( tDofTypeAndValuePair( Ik ).size() == 3 )
            {
                // get solution vector index
                moris::sint tVectorIndex = std::stoi( tDofTypeAndValuePair( Ik )( 2 ) );

                // replace initial values in solution vector
                tFreeVector->replace_global_values(
                        tAdofIds,
                        tValues,
                        ( uint ) tVectorIndex );
            }
            else
            {
                MORIS_ERROR( false, " Time_Solver::initialize_sol_vec(), TSA_Initialize_Sol_Vec input not correct" );
            }
        }

        mFullVector( 1 )->import_local_to_global( *tFreeVector );

        delete( tFreeVector );
    }
}

//--------------------------------------------------------------------------------------------------------------------------

void Time_Solver::initialize_prev_sol_vec()
{
    // initialize prev solution vector with zero( time level -1 )
    mFullVector( 0 )->vec_put_scalar( 0.0 );
}

//--------------------------------------------------------------------------------------------------------------------------

void Time_Solver::prepare_sol_vec_for_next_time_step()
{
    if( mIsMasterTimeSolver )
    {
        // get num RHS
        uint tNumRHMS = mSolverInterface->get_num_rhs();

        // create map object
        sol::Matrix_Vector_Factory tMatFactory( mSolverWarehouse->get_tpl_type() );

        // full vector and prev full vector
        sol::Dist_Vector * tFullVector = tMatFactory.create_vector( mSolverInterface, mFullMap, tNumRHMS );

        mFullVector.push_back( tFullVector );

        uint tNumSolVec = mFullVector.size();

        mFullVector( tNumSolVec-1 )->vec_plus_vec( 1.0, *(mFullVector( tNumSolVec-2 )), 0.0 );
        //        mFullVector( tNumSolVec-1 )->vec_put_scalar( 0.0 );
    }

}

//--------------------------------------------------------------------------------------------------------------------------

void Time_Solver::initialize_time_levels()
{
    // extract initialization string from parameter list
    moris::Cell< moris::Cell< std::string > > tDofTypeAndTimeLevelPair;

    string_to_cell_of_cell( mParameterListTimeSolver.get< std::string >( "TSA_time_level_per_type" ),
            tDofTypeAndTimeLevelPair );

    // get string to dof type map
    map< std::string, enum MSI::Dof_Type > tDofTypeMap = MSI::get_msi_dof_type_map();

    // loop over input dof types
    for( uint Ik = 0; Ik < tDofTypeAndTimeLevelPair.size(); Ik++ )
    {
        // First string is dof type
        enum MSI::Dof_Type tDofType = tDofTypeMap.find( tDofTypeAndTimeLevelPair( Ik )( 0 ) );

        // get value from input
        moris::uint tValue = (moris::uint)std::stoi( tDofTypeAndTimeLevelPair( Ik )( 1 ) );

        mSolverInterface->set_time_levels_for_type( tDofType, tValue );
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void Time_Solver::set_time_solver_parameters()
{
    // Maximal number of linear solver restarts on fail
    mParameterListTimeSolver = prm::create_time_solver_parameter_list();
}

//--------------------------------------------------------------------------------------------------------------------------

void Time_Solver::get_full_solution( moris::Matrix< DDRMat > & LHSValues )
{
    // get index of last solution vector
    uint tNumSolVec = mFullVector.size() -2;

    // extract solution values
    mFullVector( tNumSolVec )->extract_copy( LHSValues );
}
