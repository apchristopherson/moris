
#include "cl_DLA_Linear_Solver.hpp"

#include "cl_DLA_Solver_Factory.hpp"
#include "cl_DLA_Linear_Solver_Aztec.hpp"
#include "cl_SOL_Enums.hpp"
#include "cl_SOL_Dist_Vector.hpp"

#include "cl_Communication_Tools.hpp"

#include "cl_Logger.hpp"

#include "cl_Stopwatch.hpp" //CHR/src

using namespace moris;
using namespace dla;

Linear_Solver::Linear_Solver()
{
    // create solver factory
    Solver_Factory  tSolFactory;

    // create solver object
    std::shared_ptr< Linear_Solver_Algorithm > tLinSolver = tSolFactory.create_solver( sol::SolverType::AMESOS_IMPL );

//    tLinSolver->set_param("AZ_diagnostics") = AZ_none;
//    tLinSolver->set_param("AZ_output") = AZ_none;

    mLinearSolverList.clear();

    mLinearSolverList.resize( 1,nullptr );

    mLinearSolverList( 0 ) = tLinSolver;

    this->set_linear_solver_manager_parameters();
}

//Linear_Solver::~Linear_Solver()
//{}

//--------------------------------------------------------------------------------------------------
void Linear_Solver::set_linear_algorithm( std::shared_ptr< Linear_Solver_Algorithm > aLinSolverAlgorithm )
{
    if( mCallCounter == 0 )
    {
        // removes all elements from the Cell and destroy them
        mLinearSolverList.clear();

        // Resize the Cell to size = 1
        mLinearSolverList.resize( 1 );

        // Set linear solver on first entry
        mLinearSolverList( 0 ) = aLinSolverAlgorithm ;
    }
    else
    {
        // set nonlinear solver on next entry
        mLinearSolverList.push_back( aLinSolverAlgorithm );
    }

    mCallCounter = mCallCounter + 1;
}

//-------------------------------------------------------------------------------------------------------
void Linear_Solver::set_linear_algorithm( const moris::uint                                aListEntry,
                                                std::shared_ptr< Linear_Solver_Algorithm > aLinSolverAlgorithm )
{
    // Check if list is smaller than given entry
    if( mLinearSolverList.size() >= aListEntry )
    {
        // Resize to new entry value and set nullptr on new entries
        mLinearSolverList.resize( aListEntry + 1, nullptr);
    }
    // Set linear solver on entry
    mLinearSolverList( aListEntry ) = aLinSolverAlgorithm;
}

//-------------------------------------------------------------------------------------------------------
void Linear_Solver::solver_linear_system(       dla::Linear_Problem * aLinearProblem,
                                          const moris::sint           aIter )
{
    tic tTimer;

    moris::sint tErrorStatus = 0;
    moris::sint tMaxNumLinRestarts  = mParameterListLinearSolver.get< moris::sint >( "DLA_max_lin_solver_restarts" );
    moris::sint tTryRestartOnFailIt = 1;

    tErrorStatus = mLinearSolverList( 0 )->solve_linear_system( aLinearProblem, aIter );

    // Restart the linear solver using the current solution as an initial guess if the previous linear solve failed
    while ( tErrorStatus !=0 && tTryRestartOnFailIt <= tMaxNumLinRestarts && ( moris::sint )mLinearSolverList.size() <= tMaxNumLinRestarts )
    {
        if ( par_rank() == 0 )
        {
            // Compute current solution vector norm
            moris::real tSolVecNorm = aLinearProblem->get_free_solver_LHS()->vec_norm2();

            MORIS_LOG( " ... Previous linear solve failed. Trying restart %i of %i, using current solution with SolVecNorm = %5.15e as an initial guess. \n",
                                   tTryRestartOnFailIt, tMaxNumLinRestarts, tSolVecNorm);
        }

        // Re-solve scaled linear system with current solution as an initial guess
        tErrorStatus = mLinearSolverList( tTryRestartOnFailIt )->solve_linear_system( aLinearProblem, aIter );

        // Iterate TryRestartOnFailIt counter
        tTryRestartOnFailIt = tTryRestartOnFailIt + 1;
    }

//    if ( ( tErrorStatus != 0 && mParameterListLinearSolver.get< bool >( "DLA_hard_break" ) ) && !mParameterListNonlinearSolver.get< bool >( "NLA_rebuild_on_fail" ) )
//    {
//        if( par_rank() == 0 )
//        {
//            MORIS_LOG( "\n Linear Solver status absolute value = %i\n", tErrorStatus );
//            MORIS_ERROR( false, "Linear Solver did not exit with status 0!\n" );
//        }
//    }

    if( ( tErrorStatus != 0 ) )
    {
        if( par_rank() == 0)
        {
            MORIS_LOG( "\n Linear Solver status absolute value = %i\n", tErrorStatus );
            MORIS_LOG( "Linear Solver did not exit with status 0!\n" );
        }
    }

    real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

    if( par_rank() == 0)
    {
        MORIS_LOG_INFO( " Solve of linear system took %5.3f seconds.\n", ( double ) tElapsedTime / 1000);
    }

//    if ( tErrorStatus != 0 && mParameterListLinearSolver.get< bool >( "DLA_hard_break" ) )
//    {
//        aIter = mParameterListNonlinearSolver.get< moris::sint >( "NLA_max_iter" );
//        aHardBreak = true;
//    }
}

//--------------------------------------------------------------------------------------------------------------------------
    void Linear_Solver::set_linear_solver_manager_parameters()
    {
        // Maximal number of linear solver restarts on fail
        mParameterListLinearSolver.insert( "DLA_max_lin_solver_restarts" , 0 );

        // Maximal number of linear solver restarts on fail
        mParameterListLinearSolver.insert( "DLA_hard_break" , true );

        // Determines if lin solve should restart on fail
        mParameterListLinearSolver.insert( "DLA_rebuild_lin_solver_on_fail" , false );
    }

