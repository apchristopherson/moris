/*
 * LinearSolverTrilinos.cpp
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

#include "cl_Linear_Solver_Trilinos.hpp"
#include "cl_Solver_Input.hpp"
#include "cl_DistLinAlg_Enums.hpp"

using namespace moris;

Linear_Solver_Trilinos::Linear_Solver_Trilinos( Solver_Input*   aInput ) : moris::Linear_Solver(),
                                                                           mEpetraProblem()
{
    if ( aInput->get_matrix_market_path() == NULL )
    {
        // Get number local dofs
        moris::uint aNumMyDofs = aInput->get_num_my_dofs();

        Matrix_Vector_Factory    tMatFactory;

        // create map object
        mMap = tMatFactory.create_map( aNumMyDofs,
                                       aInput->get_my_local_global_map(),
                                       aInput->get_constr_dof(),
                                       aInput->get_my_local_global_overlapping_map());      //FIXME

        // Build matrix
        mMat = tMatFactory.create_matrix( aInput, mMap );

        // Build RHS/LHS vector
        mVectorRHS = tMatFactory.create_vector( aInput, mMap, VectorType::FREE );
        mVectorLHS = tMatFactory.create_vector( aInput, mMap, VectorType::FREE );

        mVectorLHSOverlapping = tMatFactory.create_vector( aInput, mMap, VectorType::FULL_OVERLAPPING );

        Model_Solver_Interface tLinProblem( this, aInput, mMat, mVectorRHS );
    }

    else
    {
        // Initialize communicator object
        Communicator_Epetra      tEpetraComm;

        // Build strings fot path to matrix, RHS and map
        char tMapString[500];      strcpy ( tMapString   , aInput->get_matrix_market_path() );   strcat( tMapString   , "map.mtx" );
        char tMatrixString[500];   strcpy ( tMatrixString, aInput->get_matrix_market_path() );   strcat( tMatrixString, "matrix.mtx" );
        char tVectorString[500];   strcpy ( tVectorString, aInput->get_matrix_market_path() );   strcat( tVectorString, "vector.mtx" );

        // Create map from matrix market file
        Epetra_Map * tMap;
        EpetraExt::MatrixMarketFileToMap( tMapString, *tEpetraComm.get_epetra_comm(), tMap );

        // Create matrix from matrix market file
        Epetra_CrsMatrix*   mMatFromMatrixMatket;
        //EpetraExt::MatrixMarketFileToCrsMatrix("/home/schmidt/matrix1.mtx", *tEpetraComm.get_epetra_comm(), mMatFromMatrixMatket);
        EpetraExt::MatrixMarketFileToCrsMatrix( tMatrixString, *tMap, mMatFromMatrixMatket );

        // Create RHS from matrix market file
        Epetra_MultiVector* mVecRHSFromMatrixMatket;
        EpetraExt::MatrixMarketFileToMultiVector( tVectorString , *tMap, mVecRHSFromMatrixMatket );

        Epetra_MultiVector* mVecLHSFromMatrixMatket;
        EpetraExt::MatrixMarketFileToMultiVector( tVectorString, *tMap, mVecLHSFromMatrixMatket );
        mVecLHSFromMatrixMatket->PutScalar(0.0);

        // Set matrix. solution vector and RHS
        mEpetraProblem.SetOperator( mMatFromMatrixMatket );
        mEpetraProblem.SetRHS( mVecRHSFromMatrixMatket );
        mEpetraProblem.SetLHS( mVecLHSFromMatrixMatket );
    }
}

 //----------------------------------------------------------------------------------------

Linear_Solver_Trilinos::~Linear_Solver_Trilinos()
{
    delete( mMat );
    delete( mVectorRHS );
    delete( mVectorLHS );
    delete( mVectorLHSOverlapping );
    delete( mMap );
}

//void Linear_Solver_Trilinos::build_linear_system( Epetra_FECrsMatrix*  aEpetraMat,
//                                                  Epetra_FEVector*     aEpetraVector_x,
//                                                  Epetra_FEVector*     aEpetraVector_b)
//{
//    // Set matrix. solution vector and RHS
//    mEpetraProblem.SetOperator( aEpetraMat );
//    mEpetraProblem.SetRHS( aEpetraVector_b );
//    mEpetraProblem.SetLHS( aEpetraVector_x );
//}

void Linear_Solver_Trilinos::build_linear_system()
 {
     // Set matrix. solution vector and RHS
     mEpetraProblem.SetOperator( mMat->get_matrix() );
     mEpetraProblem.SetRHS( mVectorRHS->get_vector() );
     mEpetraProblem.SetLHS( mVectorLHS->get_vector() );

     //mMat->print_matrix_to_screen();
     //std::cout<<*mVectorRHS->get_vector()<<std::endl;
 }

//------------------------------------------------------------------------------------------
void Linear_Solver_Trilinos::solve_linear_system()
{
    // Get the linear system for the solver
    AztecOO Solver( mEpetraProblem );

    //Set solver options
    Solver.SetAztecOption( AZ_solver, AZ_gmres);
    Solver.SetAztecOption( AZ_precond, AZ_dom_decomp);

    //Solve
    Solver.Iterate(200,1E-8);

    std::cout << "Solver performed " << Solver.NumIters()  << "iterations.\n";
    std::cout << "Norm of the true residual = " << Solver.TrueResidual() << std::endl;
}

//------------------------------------------------------------------------------------------
void Linear_Solver_Trilinos::get_solution( moris::Mat< moris::real > & LHSValues )
{
    mVectorLHS->extract_copy( LHSValues );
}

void Linear_Solver_Trilinos::extract_my_values( const moris::uint                & aNumIndices,
                                                 const moris::Mat< moris::sint > & aGlobalBlockRows,
                                                 const moris::uint               & aBlockRowOffsets,
                                                       moris::Mat< moris::real > & LHSValues )
{
    //mVectorLHS->extract_my_values( aNumIndices, aGlobalBlockRows, aBlockRowOffsets, LHSValues );
    mVectorLHSOverlapping->extract_my_values( aNumIndices, aGlobalBlockRows, aBlockRowOffsets, LHSValues );
}

void Linear_Solver_Trilinos::import( )
{
    mVectorLHSOverlapping->import_local_to_global( *mVectorLHS );
}

//-------------------------------------------------------------------------------------------
void Linear_Solver_Trilinos::solve_eigenvalues()
{
}

