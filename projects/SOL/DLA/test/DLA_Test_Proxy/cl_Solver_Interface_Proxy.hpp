/*
 * cl_Solver_Interface_Proxy.hpp
 *
 *  Created on: Jun 18, 2018
 *      Author: schmidt
 */
#ifndef SRC_DISTLINALG_CL_SOLVER_INPUT_TEST_HPP_
#define SRC_DISTLINALG_CL_SOLVER_INPUT_TEST_HPP_

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_Communication_Manager.hpp" // COM/src
#include "cl_Communication_Tools.hpp" // COM/src

extern moris::Comm_Manager gMorisComm;

namespace moris
{
class Solver_Interface_Proxy : public Solver_Interface
{
private:
    moris::uint mNumMyDofs;                           // local dimension of the problem
    moris::Matrix< DDSMat > mMyGlobalElements;             // local-to-global map
    moris::uint mNumElements;                         // number local elements
    moris::Matrix< DDSMat > mEleDofConectivity;             // element - dof conectivities
    moris::Matrix< DDRMat > mElementMatrixValues;   // dense element matrix entries
    moris::uint mNumDofsPerElement;                   // dofs per element
    moris::Matrix< DDUMat > mMyConstraintDofs;     // constraint dofs
    moris::Matrix< DDRMat > mMyRHSValues;          // Vector with RHS values

    bool mUseMatrixMarketFiles;                       // determines is matrix and RHS comes from MatrixMarket files

public :
    Solver_Interface_Proxy();

    // ----------------------------------------------------------------------------------------------
    ~Solver_Interface_Proxy(){};

    // ----------------------------------------------------------------------------------------------
    // local dimension of the problem
    uint get_num_my_dofs(){ return mNumMyDofs; };

    uint get_max_num_global_dofs()
    {
        moris::uint tNumMyDofs     = mNumMyDofs;
        moris::uint tMaxNumGlobalDofs = mNumMyDofs;

        // sum up all distributed dofs
        sum_all( tNumMyDofs, tMaxNumGlobalDofs );

        return tMaxNumGlobalDofs;
    };

    void free_block_memory( const uint aBlockInd ){};

    // ----------------------------------------------------------------------------------------------
    // local-to-global map
    Matrix< DDSMat > get_my_local_global_map(){ return mMyGlobalElements; };

    // ----------------------------------------------------------------------------------------------
    // number of elements on proc
    uint get_num_my_blocks(){ return 1; };

    // number of elements on proc
    uint get_num_my_elements(){ return mNumElements; };

    // number of elements on proc
    uint get_num_my_elements_on_block( uint aBlockInd ){ return mNumElements; };

    // ----------------------------------------------------------------------------------------------
    void get_element_matrix(const uint             & aMyElementInd,
                                  Matrix< DDRMat > & aElementMatrix)
    { aElementMatrix = mElementMatrixValues; };

    void get_element_matrix(const uint             & aMyBlockInd,
                            const uint             & aMyElementInd,
                                  Matrix< DDRMat > & aElementMatrix)
    { aElementMatrix = mElementMatrixValues; };

    // ----------------------------------------------------------------------------------------------
    void  get_element_topology(const uint             & aMyElementInd,
                                     Matrix< DDSMat > & aElementTopology)
    { aElementTopology = mEleDofConectivity; };

    void  get_element_topology(const uint             & aMyBlockInd,
                               const uint             & aMyElementInd,
                                     Matrix< DDSMat > & aElementTopology)
    { aElementTopology = mEleDofConectivity; };

    // ----------------------------------------------------------------------------------------------
    Matrix< DDUMat > get_constr_dof(){ return mMyConstraintDofs; };

    // ----------------------------------------------------------------------------------------------
    void get_element_rhs(const uint            & aMyElementInd,
                         Matrix< DDRMat >      & aElementRHS )
    { aElementRHS = mMyRHSValues; };

    void get_element_rhs(const uint            & aMyBlockInd,
                         const uint            & aMyElementInd,
                         Matrix< DDRMat >      & aElementRHS )
    { aElementRHS = mMyRHSValues; };

    // ----------------------------------------------------------------------------------------------

    void use_matrix_market_files( )
    {
        mUseMatrixMarketFiles = true;
    };

    // ----------------------------------------------------------------------------------------------

    const char* get_matrix_market_path( )
    {
        if ( mUseMatrixMarketFiles == true )
        {
            const char* tFilePath ="/home/schmidt/codes/MORIS/test/src/distlinalg/";
            return tFilePath;
        }
        else
        {
            return NULL;
        }
    };
};
}
#endif /* SRC_DISTLINALG_CL_SOLVER_INPUT_TEST_HPP_ */
