/*
 * cl_MSI_Solver_Interface.hpp
 *
 *  Created on: Jul 14, 2018
 *      Author: schmidt
 */
#ifndef SRC_FEM_CL_MSI_SOLVER_INTERFACE_HPP_
#define SRC_FEM_CL_MSI_SOLVER_INTERFACE_HPP_

#include "cl_MSI_Model_Solver_Interface.hpp"
#include "cl_DLA_Solver_Interface.hpp"
#include "cl_FEM_Enums.hpp"

extern moris::Comm_Manager gMorisComm;

namespace moris
{
    namespace sol
    {
        class Dist_Vector;
    }

    namespace mtk
    {
        class Mesh;
    }
    namespace mdl
    {
        class Model;
    }

    namespace MSI
    {
        class MSI_Solver_Interface : public moris::Solver_Interface
        {
            private:
                moris::MSI::Model_Solver_Interface * mMSI    = nullptr;
                moris::MSI::Dof_Manager            * mDofMgn = nullptr;

                sol::Dist_Vector                   * mSolutionVector            = nullptr;
                sol::Dist_Vector                   * mPrevSolutionVector        = nullptr;
                sol::Dist_Vector                   * mAdjointSolutionVector     = nullptr;
                sol::Dist_Vector                   * mPreviousAdjointSolutionVector = nullptr;
                sol::Dist_Vector                   * mExactSolFromFile          = nullptr;
                Matrix< DDRMat>  mTime;
                Matrix< DDRMat>  mPrevTime;

                moris::Cell< enum MSI::Dof_Type > mListOfDofTypes;
                moris::Cell< enum MSI::Dof_Type > mListOfSecondaryDofTypes;

                mdl::Model * mModel = nullptr;

            public:
                MSI_Solver_Interface( )
            {
                    mTime     = { { 0.0 }, { 1.0 } };
                    mPrevTime = { { 0.0 }, { 0.0 } };
            };

                //------------------------------------------------------------------------------
                MSI_Solver_Interface( moris::MSI::Model_Solver_Interface * aMSI ) : mMSI( aMSI ),
                        mDofMgn( mMSI->get_dof_manager() )
                {
                    mTime     = { { 0.0 }, { 1.0 } };
                    mPrevTime = { { 0.0 }, { 0.0 } };

                    mMSI->set_solver_interface( this );
                };

                //------------------------------------------------------------------------------
                ~MSI_Solver_Interface(){};

                //------------------------------------------------------------------------------

                void set_solution_vector( sol::Dist_Vector * aSolutionVector );

                //------------------------------------------------------------------------------

                void set_solution_vector_prev_time_step( sol::Dist_Vector * aSolutionVector );

                //------------------------------------------------------------------------------

                void set_adjoint_solution_vector( sol::Dist_Vector * aSolutionVector );

                //------------------------------------------------------------------------------

                void set_previous_adjoint_solution_vector( sol::Dist_Vector * aSolutionVector );

                //------------------------------------------------------------------------------

                void set_model( mdl::Model * aModel )
                {
                    mModel = aModel;
                }

                void postmultiply_implicit_dQds();

                void compute_IQI();

                //------------------------------------------------------------------------------

                void get_exact_solution_from_hdf5_and_calculate_error( const char* aFilename );

                //------------------------------------------------------------------------------

                void get_residual_vector_for_output( const char* aFilename );

                //------------------------------------------------------------------------------

                void write_solution_to_hdf5_file( const char* aFilename );

                //------------------------------------------------------------------------------

                void set_time( const Matrix< DDRMat> & aTime );

                //------------------------------------------------------------------------------

                void set_residual_norm( const real & aResNorm );

                //------------------------------------------------------------------------------

                void set_first_residual_norm( const real & aFirstResNorm );

                //------------------------------------------------------------------------------

                void set_previous_time( const Matrix< DDRMat> & aTime );

                //------------------------------------------------------------------------------

                void free_block_memory( const uint aMyEquSetInd );

                //------------------------------------------------------------------------------

                void initialize_set(
                        const uint aMyEquSetInd,
                        const bool aIsStaggered,
                        const bool aIsAdjointOffDiagonalTimeContribution = false );

                //------------------------------------------------------------------------------

                void report_beginning_of_assembly();

                //------------------------------------------------------------------------------

                void report_end_of_assembly();

                //------------------------------------------------------------------------------

                void initiate_output(
                        const uint aOutputIndex,
                        const real aTime,
                        const bool aEndOfTimeIteration );

                //------------------------------------------------------------------------------

                void set_requested_dof_types( const moris::Cell< enum MSI::Dof_Type > aListOfDofTypes )
                {
                    mListOfDofTypes = aListOfDofTypes;
                };

                //------------------------------------------------------------------------------

                void set_secondary_dof_types( const moris::Cell< enum MSI::Dof_Type > aListOfDofTypes )
                {
                    mListOfSecondaryDofTypes = aListOfDofTypes;
                };

                //------------------------------------------------------------------------------

                const moris::Cell< enum MSI::Dof_Type > & get_requested_dof_types()
                {
                    return mListOfDofTypes;
                };

                //------------------------------------------------------------------------------

                const moris::Cell< enum MSI::Dof_Type > & get_secondary_dof_types()
                {
                    return mListOfSecondaryDofTypes;
                };

                //------------------------------------------------------------------------------

                void set_time_levels_for_type(
                        const enum Dof_Type   aDofType,
                        const moris::uint     aNumTimeLevels )
                {
                    mDofMgn->set_time_levels_for_type( aDofType, aNumTimeLevels );
                };

                //------------------------------------------------------------------------------

                // number of elements blocks on proc
                moris::uint get_num_my_blocks()
                {
                    return mMSI->get_num_eqn_blocks();
                };

                //------------------------------------------------------------------------------

                // number of elements on proc
                moris::uint get_num_my_elements()
                {
                    return mMSI->get_num_eqn_objs();
                };

                //------------------------------------------------------------------------------

                moris::uint get_num_equation_objects_on_set( uint aMyEquSetInd )
                {
                    return mMSI->get_equation_set( aMyEquSetInd )->get_num_equation_objects();
                };

                //------------------------------------------------------------------------------

                enum fem::Element_Type get_set_type( uint aMyEquSetInd )
                {
                    return mMSI->get_equation_set( aMyEquSetInd )->get_element_type();
                };

                //------------------------------------------------------------------------------

                moris::uint get_num_my_dofs()
                {
                    return mDofMgn->get_num_owned_adofs();
                };

                //------------------------------------------------------------------------------

                moris::uint get_num_rhs();

                //------------------------------------------------------------------------------

                moris::uint get_max_num_global_dofs()
                {
                    moris::uint tNumMyDofs = mDofMgn->get_num_owned_adofs();

                    // sum up all distributed dofs
                    moris::uint tMaxNumGlobalDofs = sum_all( tNumMyDofs );

                    return tMaxNumGlobalDofs;
                };

                //------------------------------------------------------------------------------
                // local-to-global map
                moris::Matrix< DDSMat > get_my_local_global_map()
                {
                    return mDofMgn->get_local_adof_ids();
                };

                //------------------------------------------------------------------------------

                moris::Matrix< DDSMat > get_my_local_global_map( const moris::Cell< enum Dof_Type > & aListOfDofTypes )
                {
                    return mDofMgn->get_local_adof_ids( aListOfDofTypes );
                };

                //------------------------------------------------------------------------------
                Matrix< DDSMat > get_my_local_global_overlapping_map( )
                {
                    return mDofMgn->get_local_overlapping_adof_ids();
                };

                //------------------------------------------------------------------------------

                Matrix< DDSMat > get_my_local_global_overlapping_map( const moris::Cell< enum Dof_Type > & aListOfDofTypes )
                {
                    return mDofMgn->get_local_overlapping_adof_ids( aListOfDofTypes );
                };

                //------------------------------------------------------------------------------
                void  get_element_topology(
                        const moris::uint      & aMyElementInd,
                        Matrix< DDSMat >       & aElementTopology )
                {
                    mMSI->get_eqn_obj( aMyElementInd )->get_equation_obj_dof_ids( aElementTopology );
                };

                //------------------------------------------------------------------------------
                void  get_element_topology(
                        const moris::uint      & aMyEquSetInd,
                        const moris::uint      & aMyElementInd,
                        Matrix< DDSMat >       & aElementTopology )
                {
                    mMSI->get_equation_set( aMyEquSetInd )->
                            get_equation_object_list()( aMyElementInd )->
                            get_equation_obj_dof_ids( aElementTopology );
                };

                //------------------------------------------------------------------------------

                Matrix< DDUMat > get_constrained_Ids()
                {
                    // Matrix< DDUMat > tLocalConstrIds;// = mDofMgn->get_full_to_free_constraints();
                    return Matrix< DDUMat >(0,0);
                };

                //------------------------------------------------------------------------------

                void get_equation_object_operator(
                        const moris::uint  & aMyElementInd,
                        Matrix< DDRMat >   & aElementMatrix )
                {
                    //mMSI->get_eqn_obj( aMyElementInd )->set_time( mTime );
                    mMSI->get_eqn_obj( aMyElementInd )->get_egn_obj_jacobian( aElementMatrix );
                };

                //------------------------------------------------------------------------------

                void get_equation_object_operator(
                        const moris::uint      & aMyEquSetInd,
                        const moris::uint      & aMyElementInd,
                        Matrix< DDRMat >       & aElementMatrix )
                {
                    //mMSI->get_equation_set( aMyEquSetInd )->get_equation_object_list()( aMyElementInd )->set_time( mTime );
                    mMSI->get_equation_set( aMyEquSetInd )->
                            get_equation_object_list()( aMyElementInd )->
                            get_egn_obj_jacobian( aElementMatrix );
                };
                //------------------------------------------------------------------------------

                void get_equation_object_rhs(
                        const moris::uint        & aMyElementInd,
                        Cell< Matrix< DDRMat > > & aElementRHS )
                {
                    mMSI->get_eqn_obj( aMyElementInd )->get_equation_obj_residual( aElementRHS );
                };

                //------------------------------------------------------------------------------

                void get_equation_object_rhs(
                        const moris::uint        & aMyEquSetInd,
                        const moris::uint        & aMyElementInd,
                        Cell< Matrix< DDRMat > > & aElementRHS )
                {
                    mMSI->get_equation_set( aMyEquSetInd )->
                            get_equation_object_list()( aMyElementInd )->
                            get_equation_obj_residual( aElementRHS );
                };

                void get_equation_object_staggered_rhs(
                        const moris::uint        & aMyEquSetInd,
                        const moris::uint        & aMyElementInd,
                        Cell< Matrix< DDRMat > > & aElementRHS )
                {
                    mMSI->get_equation_set( aMyEquSetInd )->
                            get_equation_object_list()( aMyElementInd )->
                            get_staggered_equation_obj_residual( aElementRHS );
                };

                //------------------------------------------------------------------------------

                void get_equation_object_off_diag_rhs(
                        const moris::uint              & aMyEquSetInd,
                        const moris::uint              & aMyElementInd,
                        Cell< Matrix< DDRMat > >       & aElementRHS )
                {
                    mMSI->get_equation_set( aMyEquSetInd )->
                            get_equation_object_list()( aMyElementInd )->
                            get_equation_obj_off_diagonal_residual( aElementRHS );
                };

                //------------------------------------------------------------------------------

                void get_equation_object_operator_and_rhs(
                        const moris::uint        & aMyElementInd,
                        Matrix< DDRMat >         & aElementMatrix,
                        Cell< Matrix< DDRMat > > & aElementRHS )
                {
                    //mMSI->get_eqn_obj( aMyElementInd )->set_time( mTime );
                    mMSI->get_eqn_obj( aMyElementInd )->get_egn_obj_jacobian_and_residual( aElementMatrix, aElementRHS );
                };

                //------------------------------------------------------------------------------

                void get_equation_object_operator_and_rhs(
                        const moris::uint        & aMyEquSetInd,
                        const moris::uint        & aMyElementInd,
                        Matrix< DDRMat >         & aElementMatrix,
                        Cell< Matrix< DDRMat > > & aElementRHS )
                {
                    //mMSI->get_equation_set( aMyEquSetInd )->get_equation_object_list()( aMyElementInd )->set_time( mTime );
                    mMSI->get_equation_set( aMyEquSetInd )->
                            get_equation_object_list()( aMyElementInd )->
                            get_egn_obj_jacobian_and_residual( aElementMatrix, aElementRHS );
                };

                //------------------------------------------------------------------------------

                mtk::Mesh * get_mesh_pointer_for_multigrid( )
                {
                    return mMSI->get_mesh_pointer_for_multigrid();
                };

                //------------------------------------------------------------------------------

                void read_multigrid_maps(
                        const moris::uint               aLevel,
                        const moris::Matrix< DDSMat > & aExtFineIndices,
                        const moris::sint               aTypeTimeIdentifier,
                        moris::Matrix< DDSMat >       & aInternalFineIndices)
                {
                    mMSI->get_msi_multigrid_pointer()->read_multigrid_maps( aLevel, aExtFineIndices, aTypeTimeIdentifier, aInternalFineIndices );
                };

                //------------------------------------------------------------------------------

                const moris::Cell< Matrix< DDSMat > > & get_lists_of_multigrid_identifiers()
                {
                    return mMSI->get_msi_multigrid_pointer()->get_lists_of_multigrid_identifiers();
                };

                //------------------------------------------------------------------------------

                const moris::Cell< Matrix< DDUMat > > & get_lists_of_ext_index_multigrid()
                {
                    return mMSI->get_msi_multigrid_pointer()->get_lists_of_ext_index_multigrid();
                };

                //------------------------------------------------------------------------------

                const moris::Cell< moris::Cell< Matrix< DDSMat > > > & get_multigrid_map( )
                {
                    return mMSI->get_msi_multigrid_pointer()->get_multigrid_map();
                };

                //------------------------------------------------------------------------------

                const moris::Matrix< DDUMat > & get_number_remaining_dofs()
                {
                    return mMSI->get_msi_multigrid_pointer()->get_number_remaining_dofs();
                };

                //------------------------------------------------------------------------------

                const Matrix< DDSMat > & get_type_time_identifier_to_type_map()
                {
                    return mDofMgn->get_typetime_identifier_to_type_map();
                };

                //------------------------------------------------------------------------------

                moris::sint get_adof_index_for_type( moris::uint aDofType )
                {
                    return mMSI->get_adof_index_for_type( aDofType );
                };

                //------------------------------------------------------------------------------

                void calculate_criteria(
                        const moris::uint & aMySetInd,
                        const moris::uint & aMyEquationObjectInd )
                {
                    mMSI->get_equation_set( aMySetInd )->get_equation_object_list()( aMyEquationObjectInd )->compute_QI();
                };

                //------------------------------------------------------------------------------

                const moris::Cell < moris::Matrix< DDRMat> > & get_criteria( const moris::uint & aMySetInd );

                //------------------------------------------------------------------------------

                void set_requested_IQI_names( const moris::Cell< std::string > & aIQINames );
        };
    }
}

#endif /* SRC_FEM_CL_MSI_SOLVER_INTERFACE_HPP_ */
