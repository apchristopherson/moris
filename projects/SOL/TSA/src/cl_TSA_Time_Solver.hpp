/*
 * cl_TSA_Time_Solver.hpp
 *
 *  Created on: Okt 6, 2018
 *      Author: schmidt
 */
#ifndef MORIS_DISTLINALG_CL_TSA_TIME_SOLVER_HPP_
#define MORIS_DISTLINALG_CL_TSA_TIME_SOLVER_HPP_

// MORIS header files.
#include "typedefs.hpp" // CON/src
#include "cl_Cell.hpp"
#include <memory>
#include "cl_Param_List.hpp"
#include "cl_MSI_Dof_Type_Enums.hpp"

#include "cl_TSA_Time_Solver_Enums.hpp"

#include <functional>

namespace moris
{
namespace sol
{
    class Dist_Vector;
    class Dist_Map;
    class SOL_Warehouse;
}

    class Solver_Interface;

namespace tsa
{
    class Time_Solver;

    typedef std::function< bool ( moris::tsa::Time_Solver * aTimeSolver ) > Output_Criteria;

//    typedef std::function< bool ( moris::tsa::Time_Solver * aTimeSolver ) > Output_Criteria;
//    typedef std::function< void ( moris::tsa::Time_Solver * aTimeSolver,
//                                  bool &                    aIsActive ) > Output_Criteria_1;

    class Time_Solver_Algorithm;
    class Time_Solver
    {
    private:
        //! List of list of dof types
        moris::Cell< moris::Cell< enum MSI::Dof_Type > >  mDofTypeList;

        //! List with time solvers
        moris::Cell< std::shared_ptr< Time_Solver_Algorithm > > mTimeSolverAlgorithmList;

        //! List with time sub solvers
        moris::Cell< Time_Solver * > mTimeSubSolverList;

        sol::Dist_Vector * mFullVector = nullptr;
        sol::Dist_Vector * mFullVectorSensitivity = nullptr;

        sol::Dist_Map * mFullMap = nullptr;

        moris::Cell< moris::uint >     mOutputIndices;
        moris::Cell< Output_Criteria > mOutputCriteriaPointer;
//        moris::Cell< Output_Criteria_1 > mOutputCriteriaPointer_1;


//        //! Reference norm
//        moris::real mRefNorm = -1.0;
//
//        //! First residual norm of this solve
//        moris::real mFirstResidualNorm = -1.0;
//
//        //! Actual residual norm
//        moris::real mResidualNorm = -1.0;

        moris::ParameterList mParameterListTimeSolver;

        //! Pointer to solver database
        sol::SOL_Warehouse * mSolverWarehouse = nullptr;

        //! Pointer to solver interface
        Solver_Interface * mSolverInterface = nullptr;

        enum TimeSolverType mTimeSolverType = TimeSolverType::END_ENUM;

        moris::uint mCallCounter = 0;
        moris::uint mCallCounterTimeSolver = 0;

        moris::sint mLevel = 0;

        bool mIsMasterTimeSolver = false;

        //--------------------------------------------------------------------------------------------------

    protected:

    public:
        /**
         * @brief Constructor. Creates a default time solver
         *
         * @param[in] aTimeSolverType    Time solver type. Default is MONOLITHIC
         */
        Time_Solver( const enum TimeSolverType aTimeSolverType = TimeSolverType::MONOLITHIC );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Constructor using a given parameterlist
         *
         * @param[in] aParameterlist     User defined parameter list
         * @param[in] aTimeSolverType    Time solver type. Default is Newton
         */
        Time_Solver( const ParameterList         aParameterlist,
                           sol::SOL_Warehouse  * aSolverWarehouse,
                     const enum TimeSolverType   aTimeSolverType = TimeSolverType::MONOLITHIC );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Constructor. Sets given List of time solvers algorithm to this time solver
         *
         * @param[in] aNonlinerSolverList List of nonlinear solvers.
         * @param[in] aNonLinSolverType Nonlinear solver type. Default is Newton
         */
        Time_Solver(       moris::Cell< std::shared_ptr< Time_Solver_Algorithm > > & aTimeSolverList,
                     const enum TimeSolverType                                      aTimeSolverType = TimeSolverType::MONOLITHIC );

        //--------------------------------------------------------------------------------------------------

        ~Time_Solver();

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Sets one of the lists this time solver is operating on. Should be called multiple times for black solvers
         *
         * @param[in] aDofTypeList     List of dof types.
         * @param[in] aLevel           Solver level in the block structure. Default is 0
         */
        void set_dof_type_list( const moris::Cell< enum MSI::Dof_Type > aDofTypeList,
                                const moris::sint                       aLevel =  0 );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Sets sub-time-linear solver this time solver is operating on
         *
         * @param[in] aTimeSolver Pointer to nonlinear solver
         */
        void set_sub_time_solver( Time_Solver * aTimeSolver );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Sets sub-time-linear solver this time solver is operating on
         *
         * @param[in] aTimeSolver    Pointer to time solver
         * @param[in] aListEntry     List entry
         */
        void set_sub_time_solver(       Time_Solver * aTimeSolver,
                                  const moris::uint   aListEntry);

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Sets solver interface
         *
         * @param[in] aSolverInterface Pointer to solver interface
         */
        void set_solver_interface( Solver_Interface * aSolverInterface ){ mSolverInterface = aSolverInterface; };

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Gets solver interface
         *
         */
        Solver_Interface * get_solver_interface(){ return mSolverInterface; };

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Return the sub-time-linear solver this time solver is operating on
         *
         * @param[in] aListEntry        List entry
         * @param[out] Time_Solver      Time_Solver
         */
        Time_Solver * get_sub_time_solver( const moris::uint aListEntry ){ return mTimeSubSolverList( aListEntry ); };

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set time solver algorithm. Uses push back to add the given time solver algorithm to the list.
         *
         * @param[in] aTimeSolver Pointer to time solver algorithm.
         */
        void set_time_solver_algorithm( std::shared_ptr< Time_Solver_Algorithm > aTimeSolver );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set time solver algorithm on position in list
         *
         * @param[in] aTimeSolver   Pointer to time solver algorithm.
         * @param[in] aListEntry    List entry to which the pointer will be set.
         */
        void set_time_solver_algorithm(       std::shared_ptr< Time_Solver_Algorithm > aTimeSolver,
                                         const moris::uint                              aListEntry );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Get function for list of list of dof types
         *
         * @param[out] rListOfListsOfDofTypes Returns the nonlinear solver managers list of list of dof types
         */
        moris::Cell< moris::Cell< enum MSI::Dof_Type > > & get_dof_type_list()    { return mDofTypeList; };

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Get the union of this time solver dof types
         *
         * @param[out] rUnionListOfDofTypes    Returns the union list of this time solvers dof types
         */
        moris::Cell< enum MSI::Dof_Type > get_dof_type_union();

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Retruns pointer to the solver database
         *
         * @param[out] rSolverDatabase Returns the pointer to the solver database
         */
        sol::SOL_Warehouse * get_solver_warehouse(  )    { return mSolverWarehouse;};

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set pointer to the solver database
         *
         * @param[in] rSolverDatabase Poiner to the solver database
         */
        void set_solver_warehouse( sol::SOL_Warehouse * aSolverWarehouse );

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Set output index and criteria
         *
         * @param[in] aOutputIndex       Index connected to this output criteria
         * @param[in] aOutputCriteria    Pointer to the output criteria
         */
        void set_output( const uint            aOutputIndex,
                               Output_Criteria aOutputCriteria );

        //--------------------------------------------------------------------------------------------------

        void solve();

//--------------------------------------------------------------------------------------------------

        void solve_sensitivity();

        //--------------------------------------------------------------------------------------------------

        void solve( sol::Dist_Vector * aFullVector);

        //--------------------------------------------------------------------------------------------------

        void get_full_solution( moris::Matrix< DDRMat > & LHSValues );

        //--------------------------------------------------------------------------------------------------
        /**
         * @brief initialize initial guess with parameter list input
         */
        void initialize_sol_vec();

        //--------------------------------------------------------------------------------------------------
        /**
         * @brief initialize time levels with parameter list input
         */
        void initialize_time_levels();

        //--------------------------------------------------------------------------------------------------

        void check_for_outputs( const moris::real & aTime,
                                const bool          aEndOfTimeIteration);

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Returns a reference to the reference norm
         *
         * @param[out] rRefNorm Returns the nonlinear solver manager reference norm
         */
        //moris::real & get_ref_norm(){ return mRefNorm; }

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Returns a reference to the actual residual norm
         *
         * @param[out] rResidualNorm Returns the nonlinear solver manager residual norm
         */
        //moris::real & get_residual_norm(){ return mResidualNorm; }

        //--------------------------------------------------------------------------------------------------

        /**
         * @brief Returns a reference to the actual first residual norm of this solve
         *
         * @param[out] rNonlinerSolverManagerIndex rResidualNorm Returns the first nonlinear solver manager residual norm
         */
        //moris::real & get_first_residual_norm(){ return mFirstResidualNorm; }

        //--------------------------------------------------------------------------------------------------

        void set_time_solver_parameters();

        //--------------------------------------------------------------------------------------------------

        ParameterListTypes& set_param( char const* aKey )
        {
            return mParameterListTimeSolver( aKey );
        }
    };
}
}
#endif /* MORIS_DISTLINALG_CL_TSA_TIME_SOLVER_HPP_ */

