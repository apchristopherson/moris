/*
 * MatrixPETSc.hpp
 *
 *  Created on: Dec 5, 2018
 *      Author: schmidt
 */
#ifndef SRC_DISTLINALG_CL_MATRIXPETSC_HPP_
#define SRC_DISTLINALG_CL_MATRIXPETSC_HPP_

// C++ system files
#include <cstdio>
#include <iostream>

// MORIS header files.
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"

// Project header files
#include "cl_Map_PETSc.hpp"
#include "cl_Vector_PETSc.hpp"
#include "cl_SOL_Dist_Matrix.hpp"

// TPL header files
#include <petsc.h>
#include <petscsys.h>

namespace moris
{
class Matrix_PETSc : public sol::Dist_Matrix
{
private:
    moris::Matrix< DDUMat >   mDirichletBCVec;

    void dirichlet_BC_vector(       moris::Matrix< DDUMat > & aDirichletBCVec,
                              const moris::Matrix< DDUMat > & aMyConstraintDofs);

protected:

public:
    /** Default contructor */
    Matrix_PETSc(       moris::Solver_Interface * aInput,
                  std::shared_ptr<sol::Dist_Map>  aMap );

    Matrix_PETSc( const moris::uint aRows,
                  const moris::uint aCols );

    /** Destructor */
    ~Matrix_PETSc();

    void fill_matrix( const moris::uint             & aNumMyDofs,
                      const moris::Matrix< DDRMat > & aA_val,
                      const moris::Matrix< DDSMat > & aEleDofConectivity );

    void fill_matrix_row( const moris::Matrix< DDRMat > & aA_val,
                          const moris::Matrix< DDSMat > & aRow,
                          const moris::Matrix< DDSMat > & aCols );

    void get_matrix_values( const moris::Matrix< DDSMat > & aRequestedIds,
                                  moris::Matrix< DDRMat > & aValues );

    void matrix_global_assembly();

    void build_graph( const moris::uint             & aNumMyDof,
                      const moris::Matrix< DDSMat > & aElementTopology );

    void get_diagonal( moris::sol::Dist_Vector & aDiagVec ) const{};

    //FIXME mat_put_scalar only implemented for zeros with petsc. has to be changed
    void mat_put_scalar( const moris::real & aValue )
    {
        MatZeroEntries( mPETScMat );
//      MORIS_ERROR(false, "mat_put_scalar only implemented for zeros with petsc. has to be changed.");
    }

    void sparse_mat_left_scale( const moris::sol::Dist_Vector & aScaleVector )
    { MORIS_ERROR(false, "not yet implemented for petsc");};

    void sparse_mat_right_scale( const moris::sol::Dist_Vector & aScaleVector )
    { MORIS_ERROR(false, "not yet implemented for petsc");};

    void replace_diagonal_values( const moris::sol::Dist_Vector & aDiagVec )
    { MORIS_ERROR(false, "not yet implemented for petsc");};

    virtual void mat_vec_product(
            const moris::sol::Dist_Vector & aInputVec,
                  moris::sol::Dist_Vector & aResult,
            const bool                      aUseTranspose )
    { MORIS_ERROR(false, "not yet implemented for petsc");};

    void print() const;

    void save_matrix_to_matlab_file( const char* aFilename );

    void save_matrix_to_matrix_market_file( const char* aFilename ){};

    void save_matrix_map_to_matrix_market_file( const char* aFilename ){};

    //void BuildSparseGraph(int numElements = 5);

    //Mat get_petsc_matrix()       { return mPETScMat; }
};

}

#endif /* SRC_DISTLINALG_CL_MATRIXPETSC_HPP_ */
