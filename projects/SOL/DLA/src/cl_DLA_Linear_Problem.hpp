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
#include "cl_SOL_Enums.hpp"

namespace moris
{
    namespace sol
    {
        class Dist_Matrix;
        class Dist_Vector;
        class Dist_Map;
    }
class Solver_Interface;
namespace dla
{
    class Linear_Problem
    {
    private:

    protected:
        sol::Dist_Matrix   * mMat           = nullptr;
        sol::Dist_Vector   * mVectorRHS     = nullptr;
        sol::Dist_Vector   * mFreeVectorLHS = nullptr;
        sol::Dist_Vector   * mFullVectorLHS = nullptr;
        std::shared_ptr<sol::Dist_Map>  mMap           = nullptr;
        std::shared_ptr<sol::Dist_Map>  mMapFree       = nullptr;

        Solver_Interface * mSolverInterface = nullptr;

        moris::real mCondEstimate;

        enum sol::MapType mTplType = sol::MapType::Epetra;

    public:
        Linear_Problem( Solver_Interface * aInput ) : mMat(NULL),
                                                      mVectorRHS(NULL),
                                                      mFreeVectorLHS(nullptr),
                                                      mMap(NULL),
                                                      mSolverInterface( aInput )
        {};

        virtual ~Linear_Problem(){};

        void assemble_residual_and_jacobian(  );
        void assemble_residual_and_jacobian( sol::Dist_Vector * aFullSolutionVector );
        void assemble_residual();
        void assemble_jacobian();

        void assemble_staggered_residual_contribution();

        void compute_residual_for_adjoint_solve();

        virtual moris::sint solve_linear_system() = 0;

        sol::Dist_Vector * get_free_solver_LHS() { return mFreeVectorLHS; };

        void set_free_solver_LHS( sol::Dist_Vector * aFullSolVector);

        sol::Dist_Vector * get_full_solver_LHS();

        sol::Dist_Vector * get_solver_RHS() { return mVectorRHS; };

        sol::Dist_Matrix * get_matrix() { return mMat; };

        Solver_Interface * get_solver_input() const { return mSolverInterface; };

        virtual void get_solution( moris::Matrix< DDRMat > & LHSValues ) =0;
    };
}
}
#endif /* MORIS_DISTLINALG_CL_DLA_LINEAR_SYSTEM_HPP_ */

