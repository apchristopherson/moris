/*
 * cl_NLA_Nonlinear_Problem.cpp
 *
 *  Created on: Sep 21, 2018
 *      Author: schmidt
 */
#include "cl_NLA_Nonlinear_Problem.hpp"
#include "cl_SOL_Warehouse.hpp"

#include <ctime>

#include "cl_Matrix_Vector_Factory.hpp"
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_DLA_Solver_Factory.hpp"
#include "cl_SOL_Enums.hpp"
#include "cl_SOL_Dist_Vector.hpp"

#include "cl_Communication_Tools.hpp"

using namespace moris;
using namespace NLA;
using namespace dla;

Nonlinear_Problem::Nonlinear_Problem(       sol::SOL_Warehouse      * aNonlinDatabase,
                                            Solver_Interface   * aSolverInterface,
                                            Dist_Vector        * aFullVector,
                                      const moris::sint          aNonlinearSolverManagerIndex,
                                      const bool                 aBuildLinerSystemFlag,
                                      const enum sol::MapType         aMapType) :     mFullVector( aFullVector ),
                                                                                 mBuildLinerSystemFlag( aBuildLinerSystemFlag ),
                                                                                 mMapType( aMapType ),
                                                                                 mNonlinearSolverManagerIndex( aNonlinearSolverManagerIndex )
{
    mSolverInterface = aSolverInterface;

    if( mMapType == sol::MapType::Petsc )
    {
        // Initialize petsc solvers
        PetscInitializeNoArguments();
    }

    moris::Cell< enum MSI::Dof_Type > tRequesedDofTypes = mSolverInterface->get_requested_dof_types();

    // delete pointers if they already exist
    this->delete_pointers();

    // create solver factory
    Solver_Factory  tSolFactory;

    // Build Matrix vector factory
    Matrix_Vector_Factory tMatFactory( mMapType );

    // create map object FIXME ask linear problem for map
    mMap = tMatFactory.create_map( aSolverInterface->get_my_local_global_map( tRequesedDofTypes ) );

    // create map object FIXME ask linear problem for map
    mMapFull = tMatFactory.create_map( aSolverInterface->get_my_local_global_overlapping_map() );


    // create solver object
    if ( mBuildLinerSystemFlag )
    {
        mLinearProblem = tSolFactory.create_linear_system( aSolverInterface,
                                                           mMap,
                                                           mMapFull,
                                                           mMapType );
    }

    //---------------------------arc-length vectors---------------------------------
    //FIXME wrong map used. please fix
    mFext          = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mJacVals       = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mJacVals0      = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDTildeVec     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDTilde0Vec    = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDK            = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDSolve        = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDSolveNMinus1 = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDSolveNMinus2 = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mGlobalRHS     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDFArcDDeltaD  = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDelLamNum     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDelLamDen     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDeltaD        = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mdeltaD        = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    //---------------------------arc-length matrices--------------------------------
    mJacobian  = tMatFactory.create_matrix( aSolverInterface, mMap );

    //------------------------------------------------------------------------------
}

Nonlinear_Problem::Nonlinear_Problem(       Solver_Interface * aSolverInterface,
                                      const moris::sint        aNonlinearSolverManagerIndex,
                                      const bool               aBuildLinerSystemFlag,
                                      const enum sol::MapType       aMapType ) :     mBuildLinerSystemFlag( aBuildLinerSystemFlag ),
                                                                                mMapType( aMapType ),
                                                                                mNonlinearSolverManagerIndex( aNonlinearSolverManagerIndex )
{
    mSolverInterface = aSolverInterface;

    if( mMapType == sol::MapType::Petsc )
    {
        // Initialize petsc solvers
        PetscInitializeNoArguments();
    }

    // Build Matrix vector factory
    Matrix_Vector_Factory tMatFactory( mMapType );

    // create map object FIXME ask liner problem for map
//    mMap = tMatFactory.create_map( aSolverInterface->get_max_num_global_dofs(),
//                                   aSolverInterface->get_my_local_global_map(),
//                                   aSolverInterface->get_constrained_Ids(),
//                                   aSolverInterface->get_my_local_global_overlapping_map());
//    mMap = tMatFactory.create_map( aSolverInterface->get_my_local_global_map(),
//                                   aSolverInterface->get_constrained_Ids() );
    mMap = tMatFactory.create_map( aSolverInterface->get_my_local_global_overlapping_map());

    // full vector
    mFullVector = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDummyFullVector = tMatFactory.create_vector( aSolverInterface, mMap, 1 );       // FIXME delete
    mDummyFullVector->vec_put_scalar( 0.0 );
    aSolverInterface->set_solution_vector_prev_time_step(mDummyFullVector);

    //---------------------------arc-length vectors---------------------------------
    //FIXME wrong map used. please fix
    mFext          = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mJacVals       = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mJacVals0      = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDTildeVec     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDTilde0Vec    = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDK            = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDSolve        = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDSolveNMinus1 = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDSolveNMinus2 = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mGlobalRHS     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDFArcDDeltaD  = tMatFactory.create_vector( aSolverInterface, mMap, 1 );

    mDelLamNum     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDelLamDen     = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mDeltaD        = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    mdeltaD        = tMatFactory.create_vector( aSolverInterface, mMap, 1 );
    //---------------------------arc-length matrices--------------------------------
    mJacobian  = tMatFactory.create_matrix( aSolverInterface, mMap );

    //------------------------------------------------------------------------------

    mFullVector->vec_put_scalar( 0.0 );

    // delete pointers if they already exist
    this->delete_pointers();

    // create solver factory
    Solver_Factory  tSolFactory;

    // create solver object
    if ( mBuildLinerSystemFlag )
    {
        MORIS_LOG_INFO( "Build linear problem with index %-5i \n", mNonlinearSolverManagerIndex );

        mLinearProblem = tSolFactory.create_linear_system( aSolverInterface, mMapType );
    }

    // set flag that interface has been set
    mIsMasterSystem = true;
}

void Nonlinear_Problem::set_interface( Solver_Interface * aSolverInterface )
{
}

Nonlinear_Problem::~Nonlinear_Problem()
{
    this->delete_pointers();

    if( mMap != nullptr )
    {
        delete( mMap );
    }

    if( mMapFull != nullptr )
    {
        delete( mMapFull );
    }

    if( mIsMasterSystem )
    {
        delete( mFullVector );
//        delete( mDummyFullVector );

        delete( mJacVals );
        delete( mJacVals0 );
        delete( mDTildeVec );
        delete( mDTilde0Vec );
        delete( mDK );
        delete( mDSolve );
        delete( mDSolveNMinus1 );
        delete( mDSolveNMinus2 );
        delete( mGlobalRHS );
        delete( mDFArcDDeltaD );
        delete( mDelLamNum );
        delete( mDelLamDen );
        delete( mDeltaD );
        delete( mdeltaD );
        delete( mFext );

        delete( mJacobian );
    }

    if ( mMapType == sol::MapType::Petsc)
    {
        PetscFinalize();
    }
}

void Nonlinear_Problem::delete_pointers()
{
    if( mLinearProblem != nullptr )
    {
        delete( mLinearProblem );
    }
}

void Nonlinear_Problem::build_linearized_problem( const bool & aRebuildJacobian,
                                                  const sint aNonLinearIt )
{
    // Set VectorFreeSol and LHS
    mLinearProblem->set_free_solver_LHS( mFullVector );

//    this->print_sol_vec( aNonLinearIt );

    if( aRebuildJacobian )
    {
        mLinearProblem->assemble_jacobian( mFullVector );
    }

    mLinearProblem->assemble_residual( mFullVector );
}

void Nonlinear_Problem::build_linearized_problem( const bool & aRebuildJacobian,
                                                  const sint aNonLinearIt,
                                                  const sint aRestart )
{
    delete( mFullVector );

    // Build Matrix vector factory
    Matrix_Vector_Factory tMatFactory;
    mFullVector = tMatFactory.create_vector();

    this->restart_from_sol_vec( aRestart );

    // Set VectorFreeSol and LHS
    mLinearProblem->set_free_solver_LHS( mFullVector );

    if( aRebuildJacobian )
    {
        mLinearProblem->assemble_jacobian( mFullVector );
    }

    mLinearProblem->assemble_residual( mFullVector );
}

Dist_Vector * Nonlinear_Problem::get_full_vector()
{
    return mFullVector;
}

void Nonlinear_Problem::extract_my_values( const moris::uint         & aNumIndices,
                                       const moris::Matrix< DDSMat > & aGlobalBlockRows,
                                       const moris::uint             & aBlockRowOffsets,
                                             moris::Matrix< DDRMat > & LHSValues )
{
    mFullVector->extract_my_values( aNumIndices, aGlobalBlockRows, aBlockRowOffsets, LHSValues );
}

void Nonlinear_Problem::print_sol_vec( const sint aNonLinearIt )
{
    char NonLinNum[100];
    std::sprintf( NonLinNum, "NonLIt.%04u", aNonLinearIt );

    char SolVector[100];
    std::strcpy( SolVector, "SolVector." );
    std::strcat( SolVector, NonLinNum );
    std::strcat( SolVector,".h5\0");

    mFullVector->save_vector_to_HDF5( SolVector );
}

void Nonlinear_Problem::restart_from_sol_vec( const sint aRestart )
{
    char NonLinNum[100];
    std::sprintf( NonLinNum, "NonLIt.%04u", aRestart );

    char SolVector[100];
    std::strcpy( SolVector, "SolVector." );
    std::strcat( SolVector, NonLinNum );
    std::strcat( SolVector,".h5\0");

    mFullVector->read_vector_from_HDF5( SolVector );
}

//--------------------------------------------------------------------------------------------------
void Nonlinear_Problem::set_time_value( const moris::real & aLambda,
                                              moris::uint   aPos )
{
    mSolverInterface->set_time_value( aLambda, aPos );
}
