/*
 * cl_DLA_Preconditioner_PETSc.hpp
 *
 *  Created on: Dez 11, 2018
 *      Author: schmidt
 */
#ifndef SRC_DISTLINALG_CL_PRECONDITIONER_PETSC_HPP_
#define SRC_DISTLINALG_CL_PRECONDITIONER_PETSC_HPP_

//#include "core.hpp"
//#include "cl_DLA_Linear_Solver_Algorithm.hpp"
//#include "cl_VectorPETSc.hpp"
//#include "cl_MatrixPETSc.hpp"
//
//#include "cl_Matrix_Vector_Factory.hpp"
//#include "cl_DLA_Solver_Interface.hpp"
//
//#include "cl_DLA_Linear_Problem.hpp"

namespace moris
{
class Dist_Vector;
class Dist_Matrix;
namespace dla
{
class Linear_Solver_PETSc;
class Linear_Problem;
class Preconditioner_PETSc
{
    private:

        Linear_Solver_PETSc * mLinearSolverAlgoritm;

    protected:

    public:
        Preconditioner_PETSc( Linear_Solver_PETSc * aLinearSolverAlgoritm );

        ~Preconditioner_PETSc(){};

        void build_multigrid_preconditioner( Linear_Problem * aLinearSystem );

        void build_schwarz_preconditioner();

};
}
}

#endif /* SRC_DISTLINALG_CL_PRECONDITIONER_PETSC_HPP_ */