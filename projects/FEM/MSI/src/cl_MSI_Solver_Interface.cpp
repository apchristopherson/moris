/*
 * cl_MSI_Solver_Interface.cpp
 *
 *  Created on: Sep 23, 2018
 *      Author: schmidt
 */
#include "cl_MSI_Solver_Interface.hpp"
#include "cl_MDL_Model.hpp"
#include "cl_MSI_Equation_Model.hpp"

#include "cl_SOL_Dist_Vector.hpp"

namespace moris
{
    namespace MSI
    {

//-------------------------------------------------------------------------------------------------------

    void MSI_Solver_Interface::get_exact_solution_from_hdf5_and_calculate_error( const char* aFilename )
    {
        mPrevSolutionVector->read_vector_from_HDF5( aFilename );

        mSolutionVector->vec_plus_vec( 1.0, *mPrevSolutionVector, -1.0 );
    }

//-------------------------------------------------------------------------------------------------------

    void MSI_Solver_Interface::get_residual_vector_for_output( const char* aFilename )
    {
//        mPrevSolutionVector->read_vector_from_HDF5( aFilename );
        mPrevSolutionVector->vec_put_scalar( 5.0 );

        mSolutionVector->vec_plus_vec( 1.0, *mPrevSolutionVector, 0.0 );
    }

//-------------------------------------------------------------------------------------------------------

    void MSI_Solver_Interface::write_solution_to_hdf5_file( const char* aFilename )
    {
        mSolutionVector->save_vector_to_HDF5( aFilename );
    }

//-------------------------------------------------------------------------------------------------------

    void MSI_Solver_Interface::initiate_output( const uint aOutputIndex,
                                                const real aTime,
                                                const bool aEndOfTimeIteration )
    {
        // end of time iteration that the exodus file should be closed
        mModel->output_solution( aOutputIndex, aTime, aEndOfTimeIteration );
    }

//------------------------------------------------------------------------------

    void MSI_Solver_Interface::set_solution_vector( sol::Dist_Vector * aSolutionVector )
    {
        mSolutionVector = aSolutionVector;
        mMSI->mEquationModel->set_solution_vector( mSolutionVector );
    }

    void MSI_Solver_Interface::postmultiply_implicit_dQds()
    {
        mMSI->mEquationModel->compute_explicit_and_implicit_dQIdp();
        //mMSI->mEquationModel->compute_explicit_dQIdp();
        //mMSI->mEquationModel->compute_implicit_dQIdp();
    }

    void MSI_Solver_Interface::compute_IQI()
    {
        mMSI->mEquationModel->compute_IQIs();
    }

//------------------------------------------------------------------------------

    void MSI_Solver_Interface::set_solution_vector_prev_time_step( sol::Dist_Vector * aSolutionVector )
    {
        mPrevSolutionVector = aSolutionVector;
        mMSI->mEquationModel->set_previous_solution_vector( mPrevSolutionVector );
    }

//------------------------------------------------------------------------------

     void MSI_Solver_Interface::set_adjoint_solution_vector( sol::Dist_Vector * aSolutionVector )
     {
         mAdjointSolutionVector = aSolutionVector;
         mMSI->mEquationModel->set_adjoint_solution_vector( mAdjointSolutionVector );
     }

//------------------------------------------------------------------------------

     void MSI_Solver_Interface::set_previous_adjoint_solution_vector( sol::Dist_Vector * aSolutionVector )
     {
         mPreviousAdjointSolutionVector = aSolutionVector;
         mMSI->mEquationModel->set_previous_adjoint_solution_vector( mPreviousAdjointSolutionVector );
     }

//------------------------------------------------------------------------------

    void MSI_Solver_Interface::set_time( const Matrix< DDRMat> & aTime )
    {
        mTime = aTime;
        mMSI->mEquationModel->set_time( mTime );
    }

    //------------------------------------------------------------------------------

    void MSI_Solver_Interface::set_residual_norm( const real & aResNorm )
    {
        mMSI->mEquationModel->set_residual_norm( aResNorm );
    }

    //------------------------------------------------------------------------------

    void MSI_Solver_Interface::set_first_residual_norm( const real & aFirstResNorm )
    {
        mMSI->mEquationModel->set_first_residual_norm( aFirstResNorm );
    }

//------------------------------------------------------------------------------

    void MSI_Solver_Interface::set_previous_time( const Matrix< DDRMat> & aTime )
    {
        mPrevTime = aTime;
        mMSI->mEquationModel->set_previous_time( mPrevTime );
    }

//------------------------------------------------------------------------------

    const moris::Cell < moris::Matrix< DDRMat> > & MSI_Solver_Interface::get_criteria( const moris::uint & aMySetInd )
    {
        return mMSI->get_equation_set( aMySetInd )->get_QI();
    }

//------------------------------------------------------------------------------

    void MSI_Solver_Interface::set_requested_IQI_names( const moris::Cell< std::string > & aIQINames )
    {
        mMSI->get_equation_model()->set_requested_IQI_names( aIQINames );
    }

//-------------------------------------------------------------------------------------------------------

    moris::uint MSI_Solver_Interface::get_num_rhs()
    {
        return mMSI->get_equation_model()->get_num_rhs();
    }
    //-------------------------------------------------------------------------------------------------------

    void MSI_Solver_Interface::initialize_set(
            const uint aMyEquSetInd,
            const bool aIsResidual,
            const bool aIsAdjointOffDiagonalTimeContribution )
    {
        mMSI->get_equation_model()->
                set_is_adjoint_off_diagonal_time_contribution( aIsAdjointOffDiagonalTimeContribution );

        mMSI->get_equation_set( aMyEquSetInd )->initialize_set( aIsResidual );
    }

    //-------------------------------------------------------------------------------------------------------

    void MSI_Solver_Interface::report_beginning_of_assembly()
    {
        mMSI->get_equation_model()->reset();
    }

    //------------------------------------------------------------------------------

    void MSI_Solver_Interface::report_end_of_assebly()
    {
        mMSI->get_equation_model()->report_on_assembly();
    }

    //-------------------------------------------------------------------------------------------------------

    void MSI_Solver_Interface::free_block_memory( const uint aMyEquSetInd )
    {
        mMSI->get_equation_model()->
                set_is_adjoint_off_diagonal_time_contribution( false );

        mMSI->get_equation_set( aMyEquSetInd )->free_matrix_memory();
    }

//-------------------------------------------------------------------------------------------------------

    }
}
