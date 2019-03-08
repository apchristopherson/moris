/*
 * cl_DLA_Linear_System.hpp
 *
 *  Created on: May 16, 2018
 *      Author: schmidt
 */
#ifndef MORIS_DISTLINALG_CL_DLA_LINEAR_SYSTEM_HPP_
#define MORIS_DISTLINALG_CL_DLA_LINEAR_SYSTEM_HPP_

// MORIS header files.
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"

#include "cl_Matrix_Vector_Factory.hpp"
#include "cl_DLA_Enums.hpp"

#include "Epetra_LinearProblem.h"

#include <petscksp.h>

namespace moris
{
class Sparse_Matrix;
class Dist_Vector;
class Map_Class;
class Solver_Interface;
namespace dla
{
    class Linear_Problem
    {
    private:

    protected:
        Sparse_Matrix * mMat;
        Dist_Vector   * mVectorRHS;
        Dist_Vector   * mFreeVectorLHS;
        Dist_Vector   * mFullVectorLHS;
        Map_Class     * mMap;

        Solver_Interface * mInput;

        moris::real mCondEstimate;

        Epetra_LinearProblem      mEpetraProblem;

    public:
        Linear_Problem( Solver_Interface * aInput ) : mMat(NULL),
                                                      mVectorRHS(NULL),
                                                      mFreeVectorLHS(NULL),
                                                      mMap(NULL),
                                                      mInput( aInput ),
                                                      mEpetraProblem()
        {};

        virtual ~Linear_Problem(){};

        void assemble_residual_and_jacobian( Dist_Vector * aFullSolutionVector );
        void assemble_residual( Dist_Vector * aFullSolutionVector );
        void assemble_jacobian( Dist_Vector * aFullSolutionVector );

        void assemble_residual_and_jacobian(  );

        virtual void build_linear_system() = 0;

        virtual moris::sint solve_linear_system() = 0;

        Dist_Vector * get_free_solver_LHS() { return mFreeVectorLHS; };

        void set_free_solver_LHS( Dist_Vector * aFullSolVector);

        Dist_Vector * get_full_solver_LHS();

        Dist_Vector * get_solver_RHS() { return mVectorRHS; };

        Sparse_Matrix * get_matrix() { return mMat; };

        Solver_Interface * get_solver_input() const { return mInput; };

        Epetra_LinearProblem * get_linear_system_epetra() { return & mEpetraProblem; };

        //KSP get_linear_system_petsc() { return mPetscProblem; };

        virtual void get_solution( moris::Matrix< DDRMat > & LHSValues ) =0;
    };
}
}
#endif /* MORIS_DISTLINALG_CL_DLA_LINEAR_SYSTEM_HPP_ */
