/*
 * cl_DLA_Linear_System_Manager.hpp
 *
 *  Created on: Okt 6, 2018
 *      Author: schmidt
 */
#ifndef MORIS_DISTLINALG_CL_DLA_Linear_Solver_MANAGER_HPP_
#define MORIS_DISTLINALG_CL_DLA_Linear_Solver_MANAGER_HPP_

// MORIS header files.
#ifdef MORIS_HAVE_PARALLEL
 #include <mpi.h>
#endif

#include "typedefs.hpp" // CON/src
#include "cl_Cell.hpp"
#include <memory>
#include "cl_Param_List.hpp"


namespace moris
{
namespace dla
{
    class Linear_Solver;
    class Linear_Problem;
    class Linear_Solver_Manager
    {
    private:
        //! Linear solver list
        moris::Cell< std::shared_ptr< Linear_Solver > > mLinearSolverList;

        moris::uint mCallCounter = 0;

        Param_List< boost::variant< bool, sint, real > > mParameterListLinearSolver;

    protected:

    public:
        //--------------------------------------------------------------------------------------------------
        /**
         * @brief Constructor. Creates a default linear solver.
         */
        Linear_Solver_Manager();

        //--------------------------------------------------------------------------------------------------

        ~Linear_Solver_Manager();

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set linear solver. Uses push back to add the given linear solver to the list.
         *
         * @param[in] aNonLinSolver Pointer to nonlinear solver.
         */
        void set_linear_solver( std::shared_ptr< Linear_Solver > aLinSolver );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set linear solver on position in list
         *
         * @param[in] aLinSolver Pointer to nonlinear solver.
         * @param[in] aListEntry Pointer to nonlinear solver.
         */
        void set_linear_solver( const moris::uint                      aListEntry,
                                      std::shared_ptr< Linear_Solver > aLinSolver );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Solve linear system
         *
         * @param[in] aLinearProblem Pointer to linear problem.
         * @param[in] aLinearProblem Iteration number.
         */
        void solver_linear_system(       dla::Linear_Problem * aLinearProblem,
                                   const moris::sint           aIter );

        //--------------------------------------------------------------------------------------------------

        void set_linear_solver_manager_parameters();

        //--------------------------------------------------------------------------------------------------

        boost::variant< bool, sint, real > &  set_param( char const* aKey )
        {
            return mParameterListLinearSolver( aKey );
        }

    };
}
}
#endif /* MORIS_DISTLINALG_CL_DLA_Linear_Solver_MANAGER_HPP_ */

