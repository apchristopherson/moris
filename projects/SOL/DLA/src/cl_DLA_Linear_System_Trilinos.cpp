/*
 * cl_DLA_Linear_System_Trilinos.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: schmidt
 */
#include "EpetraExt_CrsMatrixIn.h"
#include "EpetraExt_BlockMapIn.h"
#include "EpetraExt_VectorIn.h"
#include "EpetraExt_MultiVectorIn.h"
#include "Epetra_FECrsMatrix.h"
#include "Epetra_RowMatrix.h"

#include "cl_DLA_Linear_System_Trilinos.hpp"
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_SOL_Enums.hpp"

using namespace moris;
using namespace dla;

Linear_System_Trilinos::Linear_System_Trilinos( Solver_Interface * aInput ) : moris::dla::Linear_Problem( aInput )
{
    mTplType = sol::MapType::Epetra;
    if ( aInput->get_matrix_market_path() == NULL )
    {
        sol::Matrix_Vector_Factory    tMatFactory( sol::MapType::Epetra );

        // create map object
        mMapFree = tMatFactory.create_map( aInput->get_my_local_global_map(),
                                           aInput->get_constrained_Ids() );      //FIXME

        mMap = tMatFactory.create_map( aInput->get_my_local_global_overlapping_map() );      //FIXME

        // Build matrix
        mMat = tMatFactory.create_matrix( aInput, mMapFree );

        uint tNumRHS = aInput->get_num_rhs();

        // Build RHS/LHS vector
        mVectorRHS = tMatFactory.create_vector( aInput, mMapFree, tNumRHS );
        mFreeVectorLHS = tMatFactory.create_vector( aInput, mMapFree, tNumRHS );

        mFullVectorLHS = tMatFactory.create_vector( aInput, mMap,  tNumRHS );

        mSolverInterface->build_graph( mMat );
    }

    else
    {
//        // Initialize communicator object
//        Communicator_Epetra      tEpetraComm;
//
//        // Build strings fot path to matrix, RHS and map
//        char tMapString[500];      strcpy ( tMapString   , aInput->get_matrix_market_path() );   strcat( tMapString   , "map.mtx" );
//        char tMatrixString[500];   strcpy ( tMatrixString, aInput->get_matrix_market_path() );   strcat( tMatrixString, "matrix.mtx" );
//        char tVectorString[500];   strcpy ( tVectorString, aInput->get_matrix_market_path() );   strcat( tVectorString, "vector.mtx" );
//
//        // Create map from matrix market file
//        Epetra_Map * tMap;
//        EpetraExt::MatrixMarketFileToMap( tMapString, *tEpetraComm.get_epetra_comm(), tMap );
//
//        // Create matrix from matrix market file
//        Epetra_CrsMatrix*   mMatFromMatrixMatket;
//        //EpetraExt::MatrixMarketFileToCrsMatrix("/home/schmidt/matrix1.mtx", *tEpetraComm.get_epetra_comm(), mMatFromMatrixMatket);
//        EpetraExt::MatrixMarketFileToCrsMatrix( tMatrixString, *tMap, mMatFromMatrixMatket );
//
//        // Create RHS from matrix market file
//        Epetra_MultiVector* mVecRHSFromMatrixMatket;
//        EpetraExt::MatrixMarketFileToMultiVector( tVectorString , *tMap, mVecRHSFromMatrixMatket );
//
//        Epetra_MultiVector* mVecLHSFromMatrixMatket;
//        EpetraExt::MatrixMarketFileToMultiVector( tVectorString, *tMap, mVecLHSFromMatrixMatket );
//        mVecLHSFromMatrixMatket->PutScalar(0.0);
//
//        mMat->get_matrix()             = mMatFromMatrixMatket;
//		mVectorRHS->get_vector()       = mVecRHSFromMatrixMatket;
//		mFreeVectorLHS->get_vector()   = mVecLHSFromMatrixMatket;

    }
}

//----------------------------------------------------------------------------------------
Linear_System_Trilinos::Linear_System_Trilinos( Solver_Interface * aInput,
                                                sol::SOL_Warehouse       * aSolverWarehouse,
                                                std::shared_ptr<sol::Dist_Map>  aFreeMap,
                                                std::shared_ptr<sol::Dist_Map>  aFullMap ) : moris::dla::Linear_Problem( aInput )
{
        mTplType = sol::MapType::Epetra;
        mSolverWarehouse = aSolverWarehouse;
        sol::Matrix_Vector_Factory    tMatFactory( mTplType );

        // Build matrix
        mMat = tMatFactory.create_matrix( aInput, aFreeMap );

        uint tNumRHMS = aInput->get_num_rhs();

        // Build RHS/LHS vector
        mVectorRHS     = tMatFactory.create_vector( aInput, aFreeMap, tNumRHMS );
        mFreeVectorLHS = tMatFactory.create_vector( aInput, aFreeMap, tNumRHMS );

        mFullVectorLHS = tMatFactory.create_vector( aInput, aFullMap, tNumRHMS );

        mSolverInterface->build_graph( mMat );
}

//----------------------------------------------------------------------------------------

Linear_System_Trilinos::~Linear_System_Trilinos()
{
    delete mMat;
    mMat=nullptr;

    delete mVectorRHS;
    mVectorRHS=nullptr;

    delete mFreeVectorLHS;
    mFreeVectorLHS=nullptr;

    delete mFullVectorLHS;
    mFullVectorLHS=nullptr;
}

//------------------------------------------------------------------------------------------
moris::sint Linear_System_Trilinos::solve_linear_system()
{
    moris::sint error = 0;
    // Get the linear system for the solver

    Epetra_LinearProblem      tEpetraProblem;
    tEpetraProblem.SetOperator( mMat->get_matrix() );
    tEpetraProblem.SetRHS( static_cast<Vector_Epetra*>(mVectorRHS)->get_epetra_vector() );
    tEpetraProblem.SetLHS( static_cast<Vector_Epetra*>(mFreeVectorLHS)->get_epetra_vector() );

    AztecOO Solver( tEpetraProblem );

    //Set solver options
    Solver.SetAztecOption( AZ_solver, AZ_gmres);
    Solver.SetAztecOption( AZ_precond, AZ_dom_decomp);
    Solver.SetAztecOption( AZ_diagnostics, 0);
    Solver.SetAztecOption( AZ_output, 0);

    //Solve
    error = Solver.Iterate( 200, 1E-8 );

    return error;
}

//------------------------------------------------------------------------------------------
void Linear_System_Trilinos::get_solution( Matrix< DDRMat > & LHSValues )
{
    mFreeVectorLHS->extract_copy( LHSValues );
}


