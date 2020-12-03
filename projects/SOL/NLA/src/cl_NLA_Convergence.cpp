/*
 * cl_NLA_Convergence.cpp
 *
 *  Created on: Sep 21, 2018
 *      Author: schmidt
 */
#include <ctime>

#include "cl_NLA_Convergence.hpp"

#include "cl_SOL_Enums.hpp"
#include "cl_SOL_Dist_Vector.hpp"

#include "cl_DLA_Linear_Problem.hpp"

#include "cl_Communication_Tools.hpp"

#include "cl_Logger.hpp"
#include "cl_Tracer.hpp"

namespace moris
{
    namespace NLA
    {
        //--------------------------------------------------------------------------------------------------------------------------
        bool Convergence::check_for_convergence(
                Nonlinear_Algorithm * tNonLinSolver,
                moris::sint         & aIt,
                moris::real         & aRefNorm,
                moris::real         & aResNorm,
                const moris::real   & aAssemblyTime,
                const moris::real   & aSolvTime,
                bool                & aHartBreak )
        {
            bool tIsConverged = false;

//            Cell< moris::real > solNorm = tNonLinSolver->mNonlinearProblem->get_full_vector()->vec_norm2();

            Cell< moris::real > solNorm = tNonLinSolver->mNonlinearProblem->get_linearized_problem()->get_free_solver_LHS()->vec_norm2();

            aResNorm = tNonLinSolver->mNonlinearProblem->get_linearized_problem()->get_solver_RHS()->vec_norm2()( 0 );

            if ( aIt <= 1)
            {
                aRefNorm = aResNorm;
                MORIS_LOG_SPEC( "ReferenceNorm" , aRefNorm );
//                MORIS_LOG( "--------------------------------------------------------------------------------");
//                MORIS_LOG( " Newton ... refNorm for pseudo-time step is %+1.15e", aRefNorm );
//                MORIS_LOG( "--------------------------------------------------------------------------------" );

//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "        NlinIt  |  NlinResNorm            |  NlinResDrop  |  SolVecNorm             ||  LinAsmTime  |  NewItrTime" );
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  |  %-10.15e  ||  %9.4e  |  %9.4e ", 1, aResNorm, 0.0, solNorm(0), aAssemblyTime, 0.0);
//                }
            }

            // log residual and solution norms
            MORIS_LOG_SPEC( "ResidualNorm", aResNorm );
            MORIS_LOG_SPEC( "SolutionNorm", solNorm(0) );
            if ( aIt > 1)
                MORIS_LOG_SPEC( "NlinResDrop" , aResNorm/aRefNorm );

            MORIS_ERROR( !( std::isnan( aResNorm ) || std::isinf( aResNorm )), "Convergence::check_for_convergence(): Residual contains NAN or INF, exiting!");

            MORIS_ERROR( !( aResNorm > 1e20 ), "Convergence::check_for_convergence(): Residual Norm has exceeded 1e20");

            // Check for convergence
            if ( ( aIt > 1 ) && ( aResNorm  < aRefNorm * tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_rel_res_norm_drop" ) ) )
            {
                MORIS_LOG_INFO( "NlinResDrop < %6.1e", tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_rel_res_norm_drop" ) );

//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  |  %-10.15e  ||  %-10.4e  |  (NlinResDrop < %6.1e)",
//                            aIt, aResNorm, ( aResNorm/aRefNorm ), solNorm(0),  aAssemblyTime, tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_rel_res_norm_drop" ) );
//                }

                tIsConverged = true;
            }
            else if ( ( aIt > 1 ) && ( aResNorm < tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_tot_res_norm" ) ) )
            {
                MORIS_LOG_INFO( "NlinResDrop < %6.1e", tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_rel_res_norm_drop" ) );

//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  |  %-10.15e  ||  %-10.4e  |  (NlinResNorm < %6.1e)",
//                            aIt, aResNorm, (aResNorm/aRefNorm), solNorm(0), aAssemblyTime, tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_tot_res_norm" ) );
//                }

                tIsConverged = true;
            }
            else if ( ( aIt > 1 ) && ( aResNorm > aRefNorm * tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_max_rel_res_norm" ) ) )
            {
                MORIS_LOG_INFO( "MaxRelResNorm > %6.1e", tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_max_rel_res_norm" ) );

                // case for residual drop getting too big, not converged, need to retry
//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  |  %-10.15e  ||  %-10.4e  |  (MaxRelResNorm > %6.1e)",
//                            aIt, aResNorm, ( aResNorm/aRefNorm ), solNorm(0), aAssemblyTime, tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_max_rel_res_norm" ) );
//                }

                tIsConverged = false;

                if ( tNonLinSolver->mParameterListNonlinearSolver.get< bool >( "NLA_hard_break" ) )
                {
                    aIt = tNonLinSolver->mParameterListNonlinearSolver.get< moris::sint >( "NLA_max_iter" );
                    aHartBreak = true;
                }
            }
//            else if( ( aIt > 1 ) )
//            {
//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  |  %-10.15e  ||  %9.4e  |  %9.4e ", aIt, aResNorm, (aResNorm/aRefNorm), solNorm(0), aAssemblyTime, aSolvTime  );
//                }
//            }

            return tIsConverged;
        }

        //--------------------------------------------------------------------------------------------------------------------------

        bool Convergence::check_for_convergence(
                Nonlinear_Algorithm * tNonLinSolver,
                moris::sint         & aIt,
                moris::real         & aRefNorm,
                moris::real         & aResNorm,
                const moris::real   & aSolvTime,
                bool                & aHartBreak )
        {
            bool tIsConverged = false;

            if ( aIt <= 1)
            {
//                MORIS_LOG( "--------------------------------------------------------------------------------");
//                MORIS_LOG( " NLBGS ... refNorm for pseudo-time step is %+1.15e", aRefNorm );
//                MORIS_LOG( "--------------------------------------------------------------------------------" );
                MORIS_LOG_SPEC( "ReferenceNorm" , aRefNorm );

//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "        NlinIt  |  NlinResNorm            |  NlinResDrop  ||  NewItrTime" );
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  ||  %9.4e ", 0, aRefNorm, 0.0,  0.0  );
//                }
            }

            // log residual and solution norms
            MORIS_LOG_SPEC( "ResidualNorm", aResNorm );
            MORIS_LOG_SPEC( "IterationTime", aSolvTime );
            if ( aIt > 1)
                MORIS_LOG_SPEC( "NlinResDrop" , aResNorm/aRefNorm );

            MORIS_ERROR( !( std::isnan( aResNorm ) || std::isinf( aResNorm )), "Convergence::check_for_convergence(): Residual contains NAN or INF, exiting!");

            MORIS_ERROR( !( aResNorm > 1e20 ), "Convergence::check_for_convergence(): Residual Norm has exceeded 1e20");

            // Check for convergence
            if ( ( aIt >= 1 ) && ( aResNorm < aRefNorm * tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_rel_res_norm_drop" ) ) )
            {
                MORIS_LOG_INFO( "NlinResDrop < %6.1e", tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_rel_res_norm_drop" ) );

//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  ||  %-10.4e  |  (NlinResDrop < %6.1e)",
//                            aIt, aResNorm, ( aResNorm/aRefNorm ), aSolvTime, tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_rel_res_norm_drop" ) );
//                }

                tIsConverged = true;
            }
            else if ( ( aIt >= 1 ) && ( aResNorm < tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_tot_res_norm" ) ) )
            {
                MORIS_LOG_INFO( "NlinResNorm < %6.1e", tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_tot_res_norm" ) );

//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  ||  %-10.4e  |  (NlinResNorm < %6.1e)",
//                            aIt, aResNorm, (aResNorm/aRefNorm), aSolvTime, tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_tot_res_norm" ) );
//                }

                tIsConverged = true;
            }
            else if ( ( aIt >= 1 ) && ( aResNorm > aRefNorm * tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_max_rel_res_norm" ) ) )
            {
                MORIS_LOG_INFO( "MaxRelResNorm > %6.1e", tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_max_rel_res_norm" ) );

                // case for residual drop getting too big, not converged, need to retry
//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  ||  %-10.4e  |  (MaxRelResNorm > %6.1e)",
//                            aIt, aResNorm, ( aResNorm/aRefNorm ), aSolvTime, tNonLinSolver->mParameterListNonlinearSolver.get< moris::real >( "NLA_max_rel_res_norm" ) );
//                }

                tIsConverged = false;

                if ( tNonLinSolver->mParameterListNonlinearSolver.get< bool >( "NLA_hard_break" ) )
                {
                    aIt = tNonLinSolver->mParameterListNonlinearSolver.get< moris::sint >( "NLA_max_iter" );
                    aHartBreak = true;
                }
            }
//            else if( ( aIt >= 1 ) )
//            {
//                if ( par_rank() == 0 )
//                {
//                    MORIS_LOG( "         %-5i  |  %-15.15e  |  %-11.5e  ||  %9.4e  |  %9.4e ",
//                            aIt, aResNorm, (aResNorm/aRefNorm), aSolvTime, aSolvTime );
//                }
//            }

            return tIsConverged;
        }
    }
}
