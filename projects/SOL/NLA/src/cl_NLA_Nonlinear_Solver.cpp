/*
 * cl_NLA_Nonlinear_Solver.cpp
 *
 *  Created on: Okt 10, 2018
 *      Author: schmidt
 */
#include "cl_DLA_Solver_Interface.hpp"

#include "cl_NLA_Nonlinear_Solver.hpp"
#include "cl_NLA_Nonlinear_Solver_Factory.hpp"

#include "cl_SOL_Warehouse.hpp"

#include "cl_SOL_Dist_Vector.hpp"

#include "cl_Communication_Tools.hpp"
#include "cl_Logger.hpp"

// Detailed Logging package
//#include "cl_Tracer.hpp"

using namespace moris;
using namespace NLA;

    Nonlinear_Solver::Nonlinear_Solver( const enum NonlinearSolverType aNonLinSolverType ) : mSecundaryDofTypeList( Cell<Cell<enum MSI::Dof_Type>>(0)),
                                                                                             mNonLinSolverType( aNonLinSolverType )
    {
        // create solver factory
        Nonlinear_Solver_Factory  tSolFactory;

        // create solver object
        std::shared_ptr< Nonlinear_Algorithm > tNonLinSolver = tSolFactory.create_nonlinear_solver( aNonLinSolverType );

        mNonlinearSolverAlgorithmList.clear();

        mNonlinearSolverAlgorithmList.resize( 1, nullptr );

        mNonlinearSolverAlgorithmList( 0 ) = tNonLinSolver;

        mStaggeredDofTypeList.resize( 0 );

        this->set_nonlinear_solver_manager_parameters();
    }


    Nonlinear_Solver::Nonlinear_Solver( const enum NonlinearSolverType aNonLinSolverType,
                                        const ParameterList            aParameterlist ) : mSecundaryDofTypeList( Cell<Cell<enum MSI::Dof_Type>>(0)),
                                                                                          mParameterListNonLinearSolver( aParameterlist ),
                                                                                          mNonLinSolverType( aNonLinSolverType )
    {
        mStaggeredDofTypeList.resize( 0 );
    }

    //--------------------------------------------------------------------------------------------------
    Nonlinear_Solver::Nonlinear_Solver(       moris::Cell< std::shared_ptr<Nonlinear_Algorithm > > & aNonlinerSolverList,
                                        const enum NonlinearSolverType                            aNonLinSolverType ) : mSecundaryDofTypeList( Cell<Cell<enum MSI::Dof_Type>>(0)),
                                                                                                                        mNonLinSolverType( aNonLinSolverType )
    {
        mNonlinearSolverAlgorithmList = aNonlinerSolverList;

        this->set_nonlinear_solver_manager_parameters();
    }

    //--------------------------------------------------------------------------------------------------
    Nonlinear_Solver::~Nonlinear_Solver()
    {
        this->free_memory();
    }

    //--------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::free_memory()
    {
        if( mNonlinearProblem != nullptr )
        {
            delete( mNonlinearProblem );

            mNonlinearProblem = nullptr;
        }


    }

    //--------------------------------------------------------------------------------------------------
    void Nonlinear_Solver::set_dof_type_list( const moris::Cell< enum MSI::Dof_Type > aStaggeredDofTypeList,
                                              const moris::sint                       aLevel )
    {
        mStaggeredDofTypeList.push_back( aStaggeredDofTypeList );
    }

    //--------------------------------------------------------------------------------------------------
    void Nonlinear_Solver::set_secondiry_dof_type_list( const moris::Cell< enum MSI::Dof_Type > aStaggeredDofTypeList)
    {
        if ( mSecundaryDofTypeList.size() == 0 )
        {
            mSecundaryDofTypeList.clear();
        }

        mSecundaryDofTypeList.push_back( aStaggeredDofTypeList );
    }

    //--------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::set_nonlinear_algorithm( std::shared_ptr< Nonlinear_Algorithm > aNonLinSolver )
    {
        if( mCallCounter == 0 )
        {
            // removes all elements from the Cell and destroy them
            mNonlinearSolverAlgorithmList.clear();

            // Resize the Cell to size = 1
            mNonlinearSolverAlgorithmList.resize( 1, nullptr );

            // Set nonlinear solver on first entry
            mNonlinearSolverAlgorithmList( 0 ) =  aNonLinSolver;
        }
        else
        {
            // set nonlinear solver on next entry
            mNonlinearSolverAlgorithmList.push_back( aNonLinSolver );
        }

        mCallCounter = mCallCounter + 1;
    }

    //-------------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::set_nonlinear_algorithm(       std::shared_ptr< Nonlinear_Algorithm > aNonLinSolver,
                                                         const moris::uint                         aListEntry )
    {
        // Check if list is smaller than given entry
        if( aListEntry >= mNonlinearSolverAlgorithmList.size() )
        {
            // Resize to new entry value and set nullptr on new entries
            mNonlinearSolverAlgorithmList.resize( aListEntry + 1, nullptr );
        }
        // Set nonlinear solver on entry
        mNonlinearSolverAlgorithmList( aListEntry ) = aNonLinSolver;
    }

    //-------------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::set_sub_nonlinear_solver( Nonlinear_Solver * aNonLinSolver )
    {
        if( mCallCounterNonlinearSolver == 0 )
        {
            // removes all elements from the Cell and destroy them
            mNonLinearSubSolverList.clear();

            // Resize the Cell to size = 1
            mNonLinearSubSolverList.resize( 1, nullptr );

            // Set nonlinear solver on first entry
            mNonLinearSubSolverList( 0 ) =  aNonLinSolver;
        }
        else
        {
            // set nonlinear solver on next entry
            mNonLinearSubSolverList.push_back( aNonLinSolver );
        }

        mCallCounterNonlinearSolver = mCallCounterNonlinearSolver + 1;
    }

    //-------------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::set_sub_nonlinear_solver(       Nonlinear_Solver * aNonLinSolver,
                                                     const moris::uint        aListEntry )
    {
        // Check if list is smaller than given entry
        if( aListEntry >= mNonLinearSubSolverList.size() )
        {
            // Resize to new entry value and set nullptr on new entries
            mNonLinearSubSolverList.resize( aListEntry + 1, nullptr );
        }
        // Set nonlinear solver on entry
        mNonLinearSubSolverList( aListEntry ) = aNonLinSolver;
    }

    //-------------------------------------------------------------------------------------------------------

    moris::Cell< enum MSI::Dof_Type > Nonlinear_Solver::get_dof_type_union()
    {
        moris::sint tCounter = 0;

        // Loop over all dof type lists to determine the total number of dof types
        for ( moris::uint Ik = 0; Ik < mStaggeredDofTypeList.size(); ++Ik )
        {
            tCounter = tCounter + mStaggeredDofTypeList( Ik ).size();
        }

        // Create list of dof types with earlier determines size
        moris::Cell< enum MSI::Dof_Type > tUnionEnumList( tCounter );
        tCounter = 0;

        // Loop over all dof types. Add them to union list
        for ( moris::uint Ik = 0; Ik < mStaggeredDofTypeList.size(); ++Ik )
        {
            for ( moris::uint Ii = 0; Ii < mStaggeredDofTypeList( Ik ).size(); ++Ii )
            {
                tUnionEnumList( tCounter++ ) = mStaggeredDofTypeList( Ik )( Ii );
            }
        }

        return tUnionEnumList;
    }

    //--------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::set_sonlinear_solver_manager_index( const moris::sint aNonlinearSolverManagerIndex )
    {
        mNonlinearSolverManagerIndex = aNonlinearSolverManagerIndex;
    }

    //--------------------------------------------------------------------------------------------------

    moris::sint Nonlinear_Solver::get_sonlinear_solver_manager_index()
    {
        MORIS_ERROR( mNonlinearSolverManagerIndex != -1,
                "Nonlinear_Solver_Manager::get_sonlinear_solver_manager_index(): mNonlinearSolverManagerIndex = -1. Solver manager index not set." );
        return mNonlinearSolverManagerIndex;
    }

    //--------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::set_solver_warehouse( sol::SOL_Warehouse * aSolverWarehouse )
    {
        mSolverWarehouse = aSolverWarehouse;

        mSolverInput = mSolverWarehouse->get_solver_interface() ;

    }

    //-------------------------------------------------------------------------------------------------------

//    void Nonlinear_Solver::solve()
//    {
//        moris::Cell< enum MSI::Dof_Type > tDofTypeUnion = this->get_dof_type_union();
//
//        mSolverInput->set_requested_dof_types( tDofTypeUnion );
//
//        if ( mNonLinSolverType == NonlinearSolverType::NLBGS_SOLVER )
//        {
//            mNonlinearProblem = new Nonlinear_Problem( mSolverWarehouse, mSolverInput, mSolverWarehouse->get_full_vector(), mNonlinearSolverManagerIndex,  false );
//        }
//        else
//        {
//            mNonlinearProblem = new Nonlinear_Problem( mSolverWarehouse, mSolverInput, mSolverWarehouse->get_full_vector(), mNonlinearSolverManagerIndex );
//        }
//
//        mNonlinearSolverAlgorithmList( 0 )->set_nonlinear_solver_manager( this );
//
//        mNonlinearSolverAlgorithmList( 0 )->solver_nonlinear_system( mNonlinearProblem );
//    }

    //-------------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::solve( Dist_Vector * aFullVector )
    {
        //Tracer tTracer(EntityBase::NonLinearSolver, EntityType::Unknown, EntityAction::Solve);

        mSolverInput = mSolverWarehouse->get_solver_interface() ;

        moris::Cell< enum MSI::Dof_Type > tDofTypeUnion = this->get_dof_type_union();

        mSolverInput->set_requested_dof_types( tDofTypeUnion );

        mSolverInput->set_secundary_dof_types( mSecundaryDofTypeList );

        if ( mNonLinSolverType == NonlinearSolverType::NLBGS_SOLVER )
        {
            this->free_memory();

            mNonlinearProblem = new Nonlinear_Problem( mSolverWarehouse,
                                                       mSolverInput,
                                                       aFullVector,
                                                       mNonlinearSolverManagerIndex,
                                                       false,
                                                       mSolverWarehouse->get_tpl_type() );
        }
        else
        {
            this->free_memory();

            mNonlinearProblem = new Nonlinear_Problem( mSolverWarehouse,
                                                       mSolverInput,
                                                       aFullVector,
                                                       mNonlinearSolverManagerIndex,
                                                       true,
                                                       mSolverWarehouse->get_tpl_type() );
        }

        mNonlinearSolverAlgorithmList( 0 )->set_nonlinear_solver_manager( this );

        if ( mNonLinSolverType == NonlinearSolverType::ARC_LENGTH_SOLVER )
        {
            if( mNonlinearSolverAlgorithmList( 0 )->get_my_nonlin_solver()->get_time_step_iter() == 0 )
            {
                mNonlinearSolverAlgorithmList( 0 )->initialize_variables( mNonlinearProblem );
            }
//            mNonlinearSolverAlgorithmList( 0 )->solver_nonlinear_system( mNonlinearProblem );
        }
//        else
//        {
//            mNonlinearSolverAlgorithmList( 0 )->solver_nonlinear_system( mNonlinearProblem );
//        }
        mNonlinearSolverAlgorithmList( 0 )->solver_nonlinear_system( mNonlinearProblem );
    }

    //-------------------------------------------------------------------------------------------------------

    void Nonlinear_Solver::solve( Nonlinear_Problem * aNonlinearProblem )
    {
        moris::sint tErrorStatus = 0;
        moris::sint tMaxNumLinRestarts  = mParameterListNonLinearSolver.get< moris::sint >( "NLA_max_non_lin_solver_restarts" );
        moris::sint tTryRestartOnFailIt = 1;

        MORIS_ERROR( mNonLinSolverType != NonlinearSolverType::NLBGS_SOLVER, "Nonlinear_Solver::solve(); Nonliner Solver is NLBGS" );

        mNonlinearSolverAlgorithmList( 0 )->set_nonlinear_solver_manager( this );

        mNonlinearSolverAlgorithmList( 0 )->solver_nonlinear_system( aNonlinearProblem );

        // Restart the nonlinear solver
        while ( tErrorStatus !=0 && tTryRestartOnFailIt <= tMaxNumLinRestarts && ( moris::sint )mNonlinearSolverAlgorithmList.size() <= tMaxNumLinRestarts )
        {
            if ( par_rank() == 0 )
            {
                MORIS_LOG( " ... Previous nonlinear solve failed. Trying restart %i of %i", tTryRestartOnFailIt, tMaxNumLinRestarts);
            }

            // Re-solve scaled linear system with current solution as an initial guess
            //tErrorStatus = mNonlinearSolverAlgorithmList( tTryRestartOnFailIt )->solver_nonlinear_system( aNonlinearProblem );
            mNonlinearSolverAlgorithmList( tTryRestartOnFailIt )->solver_nonlinear_system( aNonlinearProblem );

            // Iterate TryRestartOnFailIt counter
            tTryRestartOnFailIt = tTryRestartOnFailIt + 1;
        }
    }

//--------------------------------------------------------------------------------------------------------------------------
    void Nonlinear_Solver::set_nonlinear_solver_manager_parameters()
    {
        // Maximal number of linear solver restarts on fail
        mParameterListNonLinearSolver.insert( "NLA_max_non_lin_solver_restarts" , 0 );
    }

    void Nonlinear_Solver::get_full_solution( moris::Matrix< DDRMat > & LHSValues )
    {
        mNonlinearProblem->get_full_vector()->extract_copy( LHSValues );
    }
//--------------------------------------------------------------------------------------------------------------------------
    void Nonlinear_Solver::set_time_step_iter( const sint aTimeIter )
    {
       mTimeIter = aTimeIter;
    }

    moris::sint Nonlinear_Solver::get_time_step_iter(  )
    {
       return mTimeIter;
    }
//--------------------------------------------------------------------------------------------------------------------------
    Nonlinear_Problem * Nonlinear_Solver::get_my_nonlin_problem()
    {
        return mNonlinearProblem;
    }

    void Nonlinear_Solver::set_nonlin_solver_type( enum NonlinearSolverType aNonLinSolverType )
    {
        mNonLinSolverType = aNonLinSolverType;
    }

    enum NonlinearSolverType Nonlinear_Solver::get_nonlin_solver_type()
    {
        return mNonLinSolverType;
    }

    void Nonlinear_Solver::set_time_solver_type( tsa::Time_Solver_Algorithm* aTimeSolverAlgorithm )
    {
        mMyTimeSolverAlgorithm = aTimeSolverAlgorithm;
    }

