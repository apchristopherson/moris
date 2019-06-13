/*
 * cl_NLA_Arc_Length.cpp
 *
 *  Created on: Mar 26, 2019
 *      Author: sonne
 */
// moris includes
#include "cl_Communication_Tools.hpp"
//------------------------------------------------------------------------------
// nla includes
#include "cl_NLA_Arc_Length.hpp"
#include "cl_NLA_Convergence.hpp"
#include "cl_NLA_Nonlinear_Solver.hpp"
//------------------------------------------------------------------------------
// dla includes
#include "cl_Matrix_Vector_Factory.hpp"
#include "cl_DLA_Linear_Solver_Algorithm.hpp"
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_DLA_Enums.hpp"
#include "cl_Sparse_Matrix.hpp"
#include "cl_Vector.hpp"
//------------------------------------------------------------------------------
// linalg includes
#include "cl_Matrix.hpp"
//------------------------------------------------------------------------------
// std includes
#include <ctime>
#include <cmath>
//------------------------------------------------------------------------------

using namespace moris;
using namespace NLA;
using namespace dla;

//--------------------------------------------------------------------------------------------------------------------------

Arc_Length_Solver::Arc_Length_Solver()
{
    mLinSolverManager = new dla::Linear_Solver();

    // Set default parameters in parameter list for nonlinear solver
    this->set_nonlinear_solver_parameters();
}

//--------------------------------------------------------------------------------------------------------------------------

Arc_Length_Solver::Arc_Length_Solver( dla::Linear_Solver * aLinSolver )
{
    mLinSolverManager = aLinSolver;

    // Set default parameters in parameter list for nonlinear solver
    this->set_nonlinear_solver_parameters();
}

//--------------------------------------------------------------------------------------------------------------------------

Arc_Length_Solver::~Arc_Length_Solver()
{
}

//--------------------------------------------------------------------------------------------------------------------------
void Arc_Length_Solver::solver_nonlinear_system( Nonlinear_Problem *  aNonlinearProblem )
{
    //------------------------------------------------------------------------------
    // temporary vectors for printing solutions
    Matrix<DDRMat> tDis(136,1);
    tDis(0,0)        = 0.0;
    Matrix<DDRMat> tFor(136,1);
    tFor(0,0)        = 0.0;
    //------------------------------------------------------------------------------
    mNonlinearProblem = aNonlinearProblem;

    moris::real tTolR = 0.00000001;
    moris::real tTolF = 0.00000001;
    moris::real tR0   = 1.0;

    moris::real tB      = 0.5;
    moris::real tDeltaA = 0.1;

    moris::real tArcNumer = 0;
    moris::real tArcDenom = 0;
    moris::real tF_tilde  = 0;
    moris::real tFArc     = 0;

    moris::real tDeltaLambda = 0.0;
    moris::real tLambdaK     = 0.0;

    moris::real tLambdaSolve        = 0;
    moris::real tLambdaSolveNMinus1 = 0;
    moris::real tLambdaSolveNMinus2 = 0;

    moris::real tDFArcDDeltaLambda = 0;

    moris::real tDelLambdaNum = 0;
    moris::real tDelLambdaDen = 0;
    moris::real tdeltaLambda  = 0;

//    Dist_Vector* tFext = mNonlinearProblem->get_f_ext();
//    moris::uint tNumDof = 1;
//    moris::Matrix< DDSMat > tEleConn(1,1);
//    tEleConn(0,0) = 0;
//    moris::Matrix< DDRMat > tFextVal(1,1);
//    tFextVal(0,0) = 8.0;
//    tFext->sum_into_global_values( tNumDof,tEleConn,tFextVal );

    Dist_Vector* tFext = mNonlinearProblem->get_f_ext();
    moris::uint tNumDof = 2;
    moris::Matrix< DDSMat > tEleConn(2,1);
    tEleConn(0,0) = 0;
    tEleConn(1,0) = 1;
    moris::Matrix< DDRMat > tFextVal(2,1, 0.0);
    tFextVal(0,0) = 1.5;
    tFextVal(1,0) = 300.75;
    tFext->sum_into_global_values( tNumDof,tEleConn,tFextVal );
    //------------------------------------------------------------------------------

    moris::sint tMaxIts     = mParameterListNonlinearSolver.get< moris::sint >( "NLA_max_iter" );
    moris::real tRelaxation = mParameterListNonlinearSolver.get< moris::real >( "NLA_relaxation_parameter" );

    bool tIsConverged            = false;
    bool tRebuildJacobian        = true;
    bool tHardBreak              = false;
    moris::real tMaxNewTime      = 0.0;
    moris::real tMaxAssemblyTime = 0.0;

    //------------------------------------------------------------------------------
    //--------------------get all vectors and matrices------------------------------
    //------------------------------------------------------------------------------
    Sparse_Matrix* tJac   = mNonlinearProblem->get_full_for_jacobian();   // full consistent tangent matrix (jacobian)
    Dist_Vector* tJacVal  = mNonlinearProblem->get_jacobian_diag();       // vector of diagonal values
    Dist_Vector* tD_tilde = mNonlinearProblem->get_d_tilde();             // displacement vector

    Dist_Vector* tJacVal0  = mNonlinearProblem->get_jacobian_diag_0();
    Dist_Vector* tD_tilde0 = mNonlinearProblem->get_d_tilde0();
    Dist_Vector* tDK       = mNonlinearProblem->get_d_k();

    Dist_Vector* tDSolve        = mNonlinearProblem->get_d_solve();
    tDSolve->vec_put_scalar( 0 );
    Dist_Vector* tDSolveNMinus1 = mNonlinearProblem->get_d_solve_n_minus_1();
    tDSolveNMinus1->vec_put_scalar( 0 );
    Dist_Vector* tDSolveNMinus2 = mNonlinearProblem->get_d_solve_n_minus_2();
    tDSolveNMinus2->vec_put_scalar( 0 );

    Dist_Vector* tGlobalRHS = mNonlinearProblem->get_global_rhs();

    Dist_Vector* tDFArcDDeltaD = mNonlinearProblem->get_df_dDeltaD();
    tDFArcDDeltaD->vec_put_scalar( 0 );

    Dist_Vector* tDelLamNum = mNonlinearProblem->get_del_lam_num();
    Dist_Vector* tDelLamDen = mNonlinearProblem->get_del_lam_den();
    Dist_Vector* tDeltaD    = mNonlinearProblem->get_del_d_upper();
    Dist_Vector* tdeltaD    = mNonlinearProblem->get_del_d();
    //------------------------------------------------------------------------------

    sint tDummy = 1;

    mNonlinearProblem->build_linearized_problem( tRebuildJacobian, tDummy );      // build the linearized problem
    this->solve_linear_system( tDummy, tHardBreak );                              // solve linearized problem

    tGlobalRHS = mNonlinearProblem->get_linearized_problem()->get_solver_RHS();   // set pointer to RHS
    tJac       = mNonlinearProblem->get_linearized_problem()->get_matrix();       // set pointer to jacobian matrix
    //------------------------------------------------------------------------------
    for ( sint timeStep = 1; timeStep < 85; timeStep++ )
    {   // temporary time step loop
        //------------------------------------------------------------------------------
        /*
         * 1) build jacobian
         * 2) use jacobian to solve for d_tilde
         * 3) store jacobian_0 vals, d_tilde0 vals, and arc function denominator
         * 4) run initialization procedures
         */
        //------------------------------------------------------------------------------

        mNonlinearProblem->get_linearized_problem()->assemble_jacobian( tDSolve );  // reassemble jacobian
        mNonlinearProblem->get_linearized_problem()->assemble_residual( tDSolve );  // reassemble residual

        tJac->get_diagonal( *tJacVal );                                             // fill vector tJacVal with diagonal values of the Jacobian matrix

        if ( timeStep < 3 )
        { // since tD_tilde is only used in the first initialization step, only compute it while timeStep < 3
            tGlobalRHS->vec_plus_vec(1,*tFext,0);
            this->solve_linear_system( timeStep, tHardBreak );                      // inv(Ktilde)*Fext

            tD_tilde->vec_plus_vec(1,*mNonlinearProblem->get_linearized_problem()->get_full_solver_LHS(),0);
            mNonlinearProblem->get_linearized_problem()->assemble_residual( tDSolve );  // rebuild residual since it was changed above
        }

        sint tSize = tDeltaD->vec_local_length();
        if ( timeStep==1 )
        {   // store K_tilde0 diagonal values, d_tilde0 vector, and f_arc denominator ( all constant throughout )
            tJacVal0->vec_plus_vec(1,*tJacVal,0);
            tD_tilde0->vec_plus_vec(1,*tD_tilde,0);
            for ( sint i=0; i<tSize; i++ )
            {
                tArcDenom += tD_tilde0->get_values_pointer()[i] * tJacVal0->get_values_pointer()[i] * tD_tilde0->get_values_pointer()[i];
            }
        }

        if ( timeStep < 3 )
        {
            //------------------------------------------------------------------------------
            // procedure 1
            //------------------------------------------------------------------------------
            tArcNumer = 0.0;
            for ( sint i=0; i<tSize; i++ )
            {
                tArcNumer += tD_tilde->get_values_pointer()[i] * tJacVal0->get_values_pointer()[i] * tD_tilde->get_values_pointer()[i];
            }
            tF_tilde = std::sqrt((1-tB)*(tArcNumer/tArcDenom)+tB);

            tDeltaLambda = tDeltaA/tF_tilde;
            tDeltaD->vec_plus_vec(1,*tD_tilde,0);
            tDeltaD->scale_vector( tDeltaLambda );      // DeltaD = DeltaLambda*Dtilde

            tLambdaK = tLambdaSolve+tDeltaLambda;

            tDK->vec_plus_vec(1,*tDSolve,0);
            tDK->vec_plus_vec(1,*tDeltaD,1);            // D_k = D_n + DeltaD
        }
        else
        {
            //------------------------------------------------------------------------------
            // procedure 2
            //------------------------------------------------------------------------------
            tLambdaK = tLambdaSolveNMinus2 - 3*tLambdaSolveNMinus1 + 3*tLambdaSolve;

            tDK->vec_plus_vec(1,*tDSolveNMinus2,0);
            tDK->vec_plus_vec(-3,*tDSolveNMinus1,1);
            tDK->vec_plus_vec(3,*tDSolve,1);

            tDeltaLambda = tLambdaK - tLambdaSolve;

            tDeltaD->vec_plus_vec(1,*tDK,0);
            tDeltaD->vec_plus_vec(-1,*tDSolve,1);       // DeltaD=D_d-D_n
        }
        // update arc length function, residual, and save initial residual for convergence check
        tArcNumer = 0;
        for ( sint i=0; i<tSize; i++ )
        {
            tArcNumer += tDeltaD->get_values_pointer()[i] * tJacVal0->get_values_pointer()[i] * tDeltaD->get_values_pointer()[i];
        }
        tFArc = std::sqrt((1-tB)*(tArcNumer/tArcDenom)+tB*std::pow(tDeltaLambda,2));

        tR0 = tGlobalRHS->vec_norm2();
        mNonlinearProblem->get_linearized_problem()->get_full_solver_LHS()->vec_put_scalar(0.0);
        //------------------------------------------------------------------------------
        // Arc Length loop
        //------------------------------------------------------------------------------
        sint tIter = 1;
        while ( ((std::abs(tGlobalRHS->vec_norm2()/tR0) > tTolR) || ((tFArc-tDeltaA)/tDeltaA > tTolF))  &&  (tIter<=tMaxIts) )
        {
            clock_t tArcLengthLoopStart = clock();
            clock_t tStartAssemblyTime  = clock();

            // assemble RHS and Jacobian
            mNonlinearProblem->set_lambda_value( tLambdaK );
            mNonlinearProblem->get_linearized_problem()->assemble_residual( tDK );
            mNonlinearProblem->get_linearized_problem()->assemble_jacobian( tDK );

            tMaxAssemblyTime = this->calculate_time_needed( tStartAssemblyTime );

            tHardBreak = false;
            //------------------------------------------------------------------------------
            /* Iteration Steps:
             * 1) determine partial derivatives
             * 2) run static condensation method
             * 3) update values and check convergence
             */

            /*
             * (1) Partials
             * --------------------------------------------
             */
            tDFArcDDeltaD->vec_put_scalar( 0 );             // reset the partials
            tDFArcDDeltaLambda = 0;

            for ( sint i=0; i<tSize; i++ )
            {
                tDFArcDDeltaD->get_values_pointer()[i] += tDK->get_values_pointer()[i] * tJacVal0->get_values_pointer()[i];
            }
            moris::real tScaleVal = (1-tB)/(tFArc*tArcDenom);
            tDFArcDDeltaD->scale_vector( tScaleVal );

            tDFArcDDeltaLambda = (tB*tDeltaLambda)/tFArc;

            //------------------------------------------------------------------------------
            /*
             * (2) Static Condensation Method
             * --------------------------------------------
             * 2.1) build denominator
             * 2.2) build numerator
             * 2.3) calculate delta_lambda
             * 2.4) use delta_lambda to calculate delta_d
             * --------------------------------------------
             */
            // reset vectors and scalars
            tDelLamNum->vec_put_scalar( 0 );
            tDelLambdaNum = 0;
            tDelLamDen->vec_put_scalar( 0 );
            tDelLambdaDen = 0;
            //-------------------------------------------------
            // solve linear system for denominator
            tGlobalRHS = mNonlinearProblem->get_linearized_problem()->get_solver_RHS();
            tGlobalRHS->vec_plus_vec(1,*tFext,0);
            this->solve_linear_system( tIter, tHardBreak );

            tDelLamDen->vec_plus_vec(1,*mNonlinearProblem->get_linearized_problem()->get_full_solver_LHS(),0);
            //-------------------------------------------------

            // solve linear system for numerator
            mNonlinearProblem->get_linearized_problem()->assemble_residual( tDK );
            this->solve_linear_system( tIter, tHardBreak );

            tDelLamNum->vec_plus_vec(1,*mNonlinearProblem->get_linearized_problem()->get_full_solver_LHS(),0);

            //-------------------------------------------------
            for( sint i=0; i<tSize; i++ )
            {
                tDelLambdaNum += tDFArcDDeltaD->get_values_pointer()[i] * tDelLamNum->get_values_pointer()[i];
                tDelLambdaDen += tDFArcDDeltaD->get_values_pointer()[i] * tDelLamDen->get_values_pointer()[i];
            }

            tdeltaLambda = (tDeltaA-tFArc-tDelLambdaNum)/(tDelLambdaDen+tDFArcDDeltaLambda);

            tdeltaD->vec_plus_vec(1,*tFext,0);
            tdeltaD->scale_vector( tdeltaLambda );

            tGlobalRHS->vec_plus_vec(1,*tdeltaD,1);
            this->solve_linear_system( tIter, tHardBreak );
            tdeltaD->vec_plus_vec(1,*mNonlinearProblem->get_linearized_problem()->get_full_solver_LHS(),0);

            mNonlinearProblem->get_linearized_problem()->assemble_residual( tDK );
            //------------------------------------------------------------------------------
            /*
             * (3) Updates
             * --------------------------------------------
             * 3.1) update tD_k          = tD_k + tdelta_d
             * 3.2) update tLambda_k     = tLambda_k + tdelta_lambda
             * 3.3) update tDelta_d      = tDelta_d + tdelta_d
             * 3.4) update tDelta_lambda = tDelta_lambda + tdelta_lambda
             *
             * 3.5) update Fint --- N/A
             * 3.6) update f_arc
             * 3.7) update residual
             * 3.8) check convergence
             */
            //------------------------------------------------------------------------------
            tDK->vec_plus_vec(1,*tdeltaD,1);

            tLambdaK = tLambdaK + tdeltaLambda;

            tDeltaD->vec_plus_vec(1,*tdeltaD,1);

            tDeltaLambda = tDeltaLambda + tdeltaLambda;

            tArcNumer = 0.0;    // reset arc numerator
            for ( sint i=0; i<tSize; i++ )
            {
                tArcNumer += tDeltaD->get_values_pointer()[i] * tJacVal0->get_values_pointer()[i] * tDeltaD->get_values_pointer()[i];
            }
            tFArc = std::sqrt((1-tB)*(tArcNumer/tArcDenom)+tB*std::pow(tDeltaLambda,2));

            mNonlinearProblem->set_lambda_value( tLambdaK );
            mNonlinearProblem->get_linearized_problem()->assemble_residual( tDK );

            //------------------------------------------------------------------------------

            Convergence tConvergence;

            tIsConverged = tConvergence.check_for_convergence( this,
                    tIter,
                    mMyNonLinSolverManager->get_ref_norm(),
                    mMyNonLinSolverManager->get_residual_norm(),
                    tMaxAssemblyTime,
                    tMaxNewTime,
                    tHardBreak ) ;

            if ( tIsConverged )
            {
                if ( tHardBreak )
                {
                    continue;
                }
                break;
            }

            // Solve linear system
            this->solve_linear_system( tIter, tHardBreak );

            //PreconTime
            //SolveTime
            ( mNonlinearProblem->get_full_vector())->vec_plus_vec( -tRelaxation, *mNonlinearProblem->get_linearized_problem()
                                                   ->get_full_solver_LHS(), 1.0 );

            tMaxNewTime = this->calculate_time_needed( tArcLengthLoopStart );

//            mNonlinearProblem->print_sol_vec( tIter );

            tIter++;
        }// end iteration loop
        //------------------------------------------------------------------------------
        // store previously converged values
        if ( timeStep > 1 )
        {
            tDSolveNMinus2->vec_plus_vec(1,*tDSolveNMinus1,0);
            tLambdaSolveNMinus2 = tLambdaSolveNMinus1;

            tDSolveNMinus1->vec_plus_vec(1,*tDSolve,0);
            tLambdaSolveNMinus1 = tLambdaSolve;
        }
        // update converged values
        tDSolve->vec_plus_vec(1,*tDK,0);
        tLambdaSolve = tLambdaK;

        // clear iteration loop variables
        tDK->vec_put_scalar( 0 );
        tdeltaD->vec_put_scalar( 0 );
        tLambdaK     = 0.0;
        tDeltaLambda = 0.0;

        //------------------------------------------------------------------------------
        // save solutions into vector to print to screen
        tDis(timeStep,0) = tDSolve->get_values_pointer()[1];

        tFor(timeStep,0) = tFext->get_values_pointer()[1]*tLambdaSolve;
        //------------------------------------------------------------------------------
    }// end timeStep loop

    //------------------------------------------------------------------------------
    // printing solutions
    std::cout<<"displacement:   "<<std::endl;
    for(uint i=0; i<90; i++)
    {
        std::cout<<tDis(i,0)<<std::endl;
    }
    std::cout<<"------------------------------------------------------"<<std::endl;
    std::cout<<"external force:  "<<std::endl;
    for(uint i=0; i<90; i++)
    {
        std::cout<<tFor(i,0)<<std::endl;
    }
    //------------------------------------------------------------------------------
}

//--------------------------------------------------------------------------------------------------------------------------
void Arc_Length_Solver::solve_linear_system( moris::sint & aIter,
                                             bool        & aHardBreak )
{
    // Solve linear system
    mLinSolverManager->solver_linear_system( mNonlinearProblem->get_linearized_problem(), aIter );
}

//--------------------------------------------------------------------------------------------------------------------------
void Arc_Length_Solver::get_full_solution( moris::Matrix< DDRMat > & LHSValues )
{
    mNonlinearProblem->get_full_vector()->extract_copy( LHSValues );
}

//--------------------------------------------------------------------------------------------------------------------------
void Arc_Length_Solver::get_solution( moris::Matrix< DDRMat > & LHSValues )
{
    mNonlinearProblem->get_full_vector()->extract_copy( LHSValues );
}
//--------------------------------------------------------------------------------------------------------------------------
void Arc_Length_Solver::extract_my_values( const moris::uint             & aNumIndices,
                                           const moris::Matrix< DDSMat > & aGlobalBlockRows,
                                           const moris::uint             & aBlockRowOffsets,
                                                 moris::Matrix< DDRMat > & LHSValues )
{
    mNonlinearProblem->get_full_vector()->extract_my_values( aNumIndices, aGlobalBlockRows, aBlockRowOffsets, LHSValues );
}
//--------------------------------------------------------------------------------------------------------------------------
