/*
 * cl_NLA_Nonlinear_Algorithm.hpp
 *
 *  Created on: Nov 18, 2018
 *      Author: schmidt
 */
#ifndef MORIS_DISTLINALG_CL_NLA_NONLINEAR_ALGORITHM_HPP_
#define MORIS_DISTLINALG_CL_NLA_NONLINEAR_ALGORITHM_HPP_

// MORIS header files.
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"

#include "cl_NLA_Nonlinear_Solver_Enums.hpp"
#include "cl_NLA_Nonlinear_Problem.hpp"
#include "cl_DLA_Linear_Solver.hpp"

#include "cl_Param_List.hpp"

namespace moris
{
class Dist_Map;
class Dist_Vector;
class Solver_Interface;
namespace tsa
{
    class Time_Solver_Algorithm;
}
namespace dla
{
    class Linear_Solver_Algorithm;
}
namespace NLA
{
    class Nonlinear_Solver;
    class Nonlinear_Algorithm
    {
    private:

    protected:
        //! Pointer to my nonlinear solver manager
        Nonlinear_Solver * mMyNonLinSolverManager = nullptr;

        //! Pointer to the linear solver manager
        dla::Linear_Solver * mLinSolverManager = nullptr;

        //! pointer to the nonliner problem
        Nonlinear_Problem * mNonlinearProblem = nullptr;

        //! Parameterlist for this nonlinear solver
        Param_List< boost::variant< bool, sint, real, const char* > > mParameterListNonlinearSolver;

        bool mLinSolverOwned = true;

        friend class Convergence;

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set the parameters in the nonlinear solver parameter list
         */
        void set_nonlinear_solver_parameters();

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Member function which keeps track of used time for a particular purpose.
         */
        moris::real calculate_time_needed( const clock_t aTime );

    public:
        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Constructor
         */
        Nonlinear_Algorithm()
        {
            // Set default parameters in parameter list for nonlinear solver
            this->set_nonlinear_solver_parameters();
        };

        //--------------------------------------------------------------------------------------------------

        virtual ~Nonlinear_Algorithm(){};

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set the linear solver
         *
         * @param[in] aLinSolverManager Linear solver manager
         */
        void set_linear_solver( dla::Linear_Solver * aLinSolver );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Call to solve the nonlinear system
         *
         * @param[in] aNonlinearProblem Nonlinear problem
         */
        virtual void solver_nonlinear_system( Nonlinear_Problem * aNonlinearProblem ) = 0;

        //--------------------------------------------------------------------------------------------------

        virtual void get_full_solution( moris::Matrix< DDRMat > & LHSValues ) = 0;

        //--------------------------------------------------------------------------------------------------

        virtual void extract_my_values( const moris::uint             & aNumIndices,
                                        const moris::Matrix< DDSMat > & aGlobalBlockRows,
                                        const moris::uint             & aBlockRowOffsets,
                                              moris::Matrix< DDRMat > & LHSValues ) = 0;

        //--------------------------------------------------------------------------------------------------

        void set_nonlinear_solver_manager( Nonlinear_Solver* aNonlinSolverManager );

        //--------------------------------------------------------------------------------------------------

        virtual boost::variant< bool, sint, real, const char* > & set_param( char const* aKey ) = 0;

        //--------------------------------------------------------------------------------------------------

        Nonlinear_Solver* get_my_nonlin_solver();

        virtual void set_my_time_solver_algorithm( std::shared_ptr< tsa::Time_Solver_Algorithm > aMyTimeSolverAlgorithm )
        {
            MORIS_ASSERT(false, "set_my_time_solver_algorithm(): function not implemented");
        }

        virtual void initialize_variables( Nonlinear_Problem *  aNonlinearProblem )
        {
            MORIS_ASSERT( false, "initialize_variables(): this function is to be used for the arc length algorithm" );
        }

    };
}
}
#endif /* MORIS_DISTLINALG_CL_NLA_NONLINEAR_ALGORITHM_HPP_ */
