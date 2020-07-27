/*
 * cl_NLA_Nonlinear_Algorithm.cpp
 *
 *  Created on: Sep 21, 2018
 *      Author: schmidt
 */
#include <ctime>

#include "cl_NLA_Nonlinear_Algorithm.hpp"
#include "cl_NLA_Nonlinear_Problem.hpp"

#include "cl_DLA_Solver_Interface.hpp"
#include "cl_DLA_Linear_Solver.hpp"

#include "cl_SOL_Dist_Vector.hpp"

#include "cl_Communication_Tools.hpp"

#include "cl_PRM_SOL_Parameters.hpp"

extern moris::Comm_Manager gMorisComm;

using namespace moris;
using namespace NLA;
using namespace dla;

// ----------------------------------------------------------------------------

void Nonlinear_Algorithm::set_linear_solver( dla::Linear_Solver * aLinSolver  )
{
    // Delete liner solver manager
    delete mLinSolverManager;

    // Set liner solver manager
    mLinSolverManager = aLinSolver;
}

//--------------------------------------------------------------------------------------------------------------------------

moris::real Nonlinear_Algorithm::calculate_time_needed( const clock_t aTime )
{
    moris::real tDeltaTime = (moris::real) ( clock() - aTime ) / CLOCKS_PER_SEC;

    moris::real tDeltaTimeMax = max_all( tDeltaTime );

    return tDeltaTimeMax;
}

//--------------------------------------------------------------------------------------------------------------------------

void Nonlinear_Algorithm::set_nonlinear_solver_manager( Nonlinear_Solver* aNonlinSolverManager )
{
    mLinSolverOwned = false;

    mMyNonLinSolverManager = aNonlinSolverManager;
}

//--------------------------------------------------------------------------------------------------------------------------

void Nonlinear_Algorithm::set_nonlinear_solver_parameters()
{
    // Allowable Newton solver iterations
    mParameterListNonlinearSolver = prm::create_nonlinear_algorithm_parameter_list();
}

//--------------------------------------------------------------------------------------------------------------------------

Nonlinear_Solver* Nonlinear_Algorithm::get_my_nonlin_solver()
{
    MORIS_ASSERT(mMyNonLinSolverManager != nullptr, "Nonlinear_Algorithm::get_my_nonlin_solver(): nonlinear solver manager not set");
    return mMyNonLinSolverManager;
}
