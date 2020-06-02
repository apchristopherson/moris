/*
 * cl_HMR.cpp
 *
 *  Created on: May 5, 2018
 *      Author: messe
 */

// see http://pubs.opengroup.org/onlinepubs/7908799/xsh/dlfcn.h.html
#include "cl_HMR.hpp" //HMR/src

#include "dlfcn.h"

#include "typedefs.hpp"
#include "cl_Map.hpp"
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "op_times.hpp" //LINALG/src
#include "fn_trans.hpp" //LINALG/src
#include "fn_eye.hpp" //LINALG/src
#include "fn_unique.hpp" //LINALG/src

#include "cl_HMR_Database.hpp"     //HMR/src
#include "cl_HMR_Background_Element_Base.hpp"
#include "cl_HMR_Mesh.hpp"
#include "cl_HMR_Mesh_Interpolation.hpp"
#include "cl_HMR_Mesh_Integration.hpp"
#include "cl_HMR_Field.hpp"          //HMR/src
#include "cl_HMR_File.hpp" //HMR/src
#include "cl_HMR_Mesh.hpp" //HMR/src
#include "cl_HMR_STK.hpp" //HMR/src

#include "MTK_Tools.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh.hpp"
#include "cl_MTK_Mapper.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_Mesh_Factory.hpp"

#include "HDF5_Tools.hpp"
#include "cl_HMR_Field.hpp"          //HMR/src

namespace moris
{
    namespace hmr
    {
// -----------------------------------------------------------------------------

        HMR::HMR ( Parameters * aParameters ) : mParameters( aParameters )
        {
            mDatabase = std::make_shared< Database >( aParameters );

            this->create_input_and_output_meshes();

            mDatabase->calculate_t_matrices_for_input();
        }

// -----------------------------------------------------------------------------

        // alternative constuctor that converts ref to a pointer
        HMR::HMR ( Parameters & aParameters ) : HMR( & aParameters )
        {

        }

// -----------------------------------------------------------------------------

        // alternative constuctor that uses parameter list
        HMR::HMR ( ParameterList & aParameterList ) : HMR( new Parameters( aParameterList ) )
        {
            mDatabase->set_parameter_owning_flag();
        }

// -----------------------------------------------------------------------------

        HMR::HMR( const std::string & aPath )
        {
            mDatabase = std::make_shared< Database >( aPath );
            // set shared pointer of database to itself

            mDatabase->set_parameter_owning_flag();

            // set parameters of HMR object
            mParameters = mDatabase->get_parameters();

            this->create_input_and_output_meshes();

            mDatabase->calculate_t_matrices_for_input();
        }

// -----------------------------------------------------------------------------

        HMR::HMR( const std::string & aInPath,
                  const std::string & aOutPath )
        {
            MORIS_ERROR( false,"HMR(); constructor not updated yet");
            mDatabase = std::make_shared< Database >( aInPath, aOutPath );

            // set shared pointer of database to itself
            mDatabase->set_parameter_owning_flag();

            // set parameters of HMR object
            mParameters = mDatabase->get_parameters();

            //mDatabase->calculate_t_matrices_for_input();

            // create union of input and output
//            mDatabase->create_union_pattern();

            // update database
            mDatabase->update_bspline_meshes();
            mDatabase->update_lagrange_meshes();

            // finalize database
            this->finalize();

            this->create_input_and_output_meshes();

            mDatabase->set_activation_pattern( mParameters->get_lagrange_output_pattern() );
        }

// -----------------------------------------------------------------------------

        void HMR::finalize()
        {
            // finish database
            mDatabase->finalize();
        }

// -----------------------------------------------------------------------------

        void HMR::perform()
        {
            this->finalize();

            const Matrix< DDUMat > & OutputMeshIndex = mParameters->get_output_mesh();

            MORIS_ERROR( OutputMeshIndex.numel() == 1, " HMR::perform(), Only one output mesh allowed right! To allow more implement multiple side sets!");

            uint tLagrangeMeshIndex = OutputMeshIndex( 0, 0 );

//            this->calculate_bspline_coordinates( tLagrangeMeshIndex, 0 );

//            this->save_to_exodus( 0, "./hmr_exo/benchmark01.e" );

            moris::hmr::Interpolation_Mesh_HMR * tInterpolationMesh
                         = this->create_interpolation_mesh( tLagrangeMeshIndex );
            moris::hmr::Integration_Mesh_HMR *   tIntegrationMesh
                         = this->create_integration_mesh( tLagrangeMeshIndex, tInterpolationMesh );

            MORIS_ERROR( mMTKPerformer != nullptr, "HMR::perform(), MTK performer not set!" );

            mMTKPerformer->register_mesh_pair( tInterpolationMesh, tIntegrationMesh, true );
        }

// -----------------------------------------------------------------------------

        void HMR::set_performer( std::shared_ptr< mtk::Mesh_Manager > aMTKPerformer )
        {
            mMTKPerformer = aMTKPerformer;
        }

// -----------------------------------------------------------------------------

//        void HMR::renumber_and_save_to_exodus( const std::string & aPath, const double aTimeStep,  const uint aOutputOrder )
//        {
//            MORIS_ERROR(false,"renumber_and_save_to_exodus() not changed yet" );
//            uint tOutputOrder = MORIS_UINT_MAX;
//            uint tIndex = MORIS_UINT_MAX;
//
//            if( aOutputOrder == 0 )
//            {
//                tOutputOrder = mParameters->get_lagrange_orders().max();
//            }
//            else
//            {
//                tOutputOrder = aOutputOrder;
//            }
//
//
//            MORIS_ERROR( tIndex != MORIS_UINT_MAX, "Something went wrong while trying to find mesh for exodus file" );
//
//            MORIS_ASSERT( mDatabase->get_lagrange_mesh_by_index( tIndex )->get_order() == tOutputOrder,
//                    "Picked wrong mesh for output");
//
//            mDatabase->get_lagrange_mesh_by_index( tIndex )->nodes_renumbering_hack_for_femdoc();
//
//            this->save_to_exodus( tIndex,
//                                  aPath,
//                                  aTimeStep );
//        }

// -----------------------------------------------------------------------------

        void HMR::save_last_step_to_exodus( const uint          aIndex,
                                            const std::string & aPath,
                                            const double        aTimeStep )
        {
            MORIS_ERROR( ! mUpdateRefinementCalled,
                    "HMR does not feel comfortable with you calling save_last_step_to_exodus() after you have overwritten the input pattern using update_refinement_pattern()");

            MORIS_ERROR( aIndex != MORIS_UINT_MAX, "Something went wrong while trying to find mesh for exodus file" );

            this->save_to_exodus( aIndex,
                                  aPath,
                                  aTimeStep );
        }

// -----------------------------------------------------------------------------

        void HMR::save_to_exodus( const uint        & aMeshIndex,
                                  const std::string & aPath,
                                  const double aTimeStep  )
        {
            STK * tSTK = mDatabase->get_lagrange_mesh_by_index( aMeshIndex )
                                  ->create_stk_object( aTimeStep );

            // save MTK to exodus
            tSTK->save_to_file( aPath );

            // delete file
            delete tSTK;
        }

// -----------------------------------------------------------------------------

        void HMR::save_to_hdf5( const std::string & aPath,
                                const uint          aLagrangeMeshIndex )
        {
            // create file object
            File tHDF5;

            // create file on disk
            tHDF5.create( aPath );

            // store settings object
            tHDF5.save_settings( mParameters );

            Lagrange_Mesh_Base * tLagrangeMesh = mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex );

            // get pointer to background mesh
            Background_Mesh_Base * tBackgroundMesh = mDatabase->get_background_mesh();

            // remember active pattern
            auto tActivePattern = tBackgroundMesh->get_activation_pattern();

            // save output pattern into file
            tHDF5.save_refinement_pattern( tLagrangeMesh );

            if( tActivePattern != tBackgroundMesh->get_activation_pattern() )
            {
                tBackgroundMesh->set_activation_pattern( tActivePattern );
            }

            // close hdf5 file
            tHDF5.close();
        }

// -----------------------------------------------------------------------------

        void HMR::save_coeffs_to_hdf5_file( const std::string & aFilePath,
                                            const uint        & aLagrangeMeshIndex )
        {
            // Get Lagrange mesh
            Lagrange_Mesh_Base * tMesh = mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex );

            // Renumber Lagrange nodes to be the same than B-Spline basis. Only serial and linear
            if( mParameters->get_renumber_lagrange_nodes() && tMesh->get_order() == 1 )
            {
                tic tTimer;

                tMesh->nodes_renumbering_hack_for_femdoc();

                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

//                MORIS_LOG_INFO( "%s Renumbering of Lagrange mesh.\n                took %5.3f seconds.\n\n",
//                        proc_string().c_str(),
//                        ( double ) tElapsedTime / 1000 );
                MORIS_LOG_INFO( "%s Renumbering of Lagrange mesh.",
                        proc_string().c_str() );
                MORIS_LOG_INFO( "Took %5.3f seconds.",
                        ( double ) tElapsedTime / 1000 );
                MORIS_LOG_INFO( " " );

            }

            // add order to path
            std::string tFilePath =    aFilePath.substr(0,aFilePath.find_last_of(".")) // base path
                                      + "_" + std::to_string( tMesh->get_order() ) // rank of this processor
            +  aFilePath.substr( aFilePath.find_last_of("."), aFilePath.length() );

            // make path parallel
            tFilePath = parallelize_path( tFilePath );

            // Create a new file using default properties
            hid_t tFileID = H5Fcreate( tFilePath.c_str(),
                                       H5F_ACC_TRUNC,
                                       H5P_DEFAULT,
                                       H5P_DEFAULT);

            // error handler
            herr_t tStatus;

            // save mesh order
            save_scalar_to_hdf5_file( tFileID,
                                      "LagrangeOrder",
                                      tMesh->get_order(),
                                      tStatus );

            // get number of nodes of this mesh
            uint tNumberOfNodes = tMesh->get_number_of_nodes_on_proc();

            // allocate matrix with ids
            Matrix< IdMat > tIDs( tNumberOfNodes, 1 );

            // populate matrix
            for( uint k=0; k<tNumberOfNodes; ++k )
            {
                tIDs( k ) = tMesh->get_node_by_index( k )->get_id();
            }

            // save ids to file
            save_matrix_to_hdf5_file( tFileID,
                                      "NodeID",
                                      tIDs,
                                      tStatus );

            // loop over all B-Spline meshes
            uint tNumberOfBSplineMeshes = tMesh->get_number_of_bspline_meshes();

            for ( uint Im = 0; Im < tNumberOfBSplineMeshes; ++Im )
            {
                // get pointer to mesh
                BSpline_Mesh_Base * tBMesh = tMesh->get_bspline_mesh( Im );

                if ( tBMesh != NULL )
                {
                    // generate label
                    std::string tLabel = "NumberOfCoefficients_" + std::to_string( Im );

                    // count number of coefficients per node
                    Matrix< DDUMat > tNumberOfCoeffs( tNumberOfNodes, 1, 0 );

                    // populate matrix
                    for( uint Ik = 0; Ik < tNumberOfNodes; ++Ik )
                    {
                        tNumberOfCoeffs( Ik ) = tMesh->get_node_by_index( Ik )->get_interpolation( Im )
                                                                              ->get_number_of_coefficients();
                    }

                    // save number of coeffs to file
                    save_matrix_to_hdf5_file( tFileID,
                                              tLabel,
                                              tNumberOfCoeffs,
                                              tStatus );

                    // get max number of coeffs
                    uint tMaxNumCoeffs = tNumberOfCoeffs.max();

                    Matrix< IdMat >  tCoeffIDs( tNumberOfNodes, tMaxNumCoeffs, gNoID );
                    Matrix< DDRMat > tWeights ( tNumberOfNodes, tMaxNumCoeffs, 0.0   );

                    // populate matrix
                    for( uint Ik = 0; Ik < tNumberOfNodes; ++Ik )
                    {
                        // get max number of dofs
                        uint tMaxI = tNumberOfCoeffs( Ik );

                        // get pointer to interpolation object
                        mtk::Vertex_Interpolation * tInterp = tMesh->get_node_by_index( Ik )->get_interpolation( Im );

                        tCoeffIDs( { Ik, Ik }, { 0, tMaxI - 1 } ) = trans(tInterp->get_ids().matrix_data());
                        tWeights( { Ik, Ik }, { 0, tMaxI - 1 } ) = trans(tInterp->get_weights()->matrix_data());

                    }

                    // generate label
                    tLabel = "BSplineIDs_" + std::to_string( Im );

                    // save ids to file
                    save_matrix_to_hdf5_file( tFileID,
                                              tLabel,
                                              tCoeffIDs,
                                              tStatus );

                    // generate  label
                    tLabel = "InterpolationWeights_" + std::to_string( Im );

                    // save weights to file
                    save_matrix_to_hdf5_file( tFileID,
                                              tLabel,
                                              tWeights,
                                              tStatus );
                }
            }

            // close file
            tStatus = H5Fclose( tFileID );
        }

// -----------------------------------------------------------------------------

        void HMR::save_mesh_relations_to_hdf5_file( const std::string & aFilePath,
                                                    const uint        & aLagrangeMeshIndex,
                                                    const uint        & aBsplineMeshIndex )
        {
            // Get Lagrange mesh
            Lagrange_Mesh_Base * tLagrangeMesh = mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex );

            BSpline_Mesh_Base * tMesh = tLagrangeMesh->get_bspline_mesh( aBsplineMeshIndex );

            // add order to path
            std::string tFilePath =    aFilePath.substr(0,aFilePath.find_last_of("."))
                                      + "_" + std::to_string( aBsplineMeshIndex )
            +  aFilePath.substr( aFilePath.find_last_of("."), aFilePath.length() );

            // make path parallel
            tFilePath = parallelize_path( tFilePath );

            // Create a new file using default properties
            hid_t tFileID = H5Fcreate( tFilePath.c_str(),
                                       H5F_ACC_TRUNC,
                                       H5P_DEFAULT,
                                       H5P_DEFAULT);

            // error handler
            herr_t tStatus;

            // save mesh order
            save_scalar_to_hdf5_file( tFileID,
                                      "BSplineOrder",
                                      tMesh->get_order(),
                                      tStatus );

            // get number of nodes of this mesh
            uint tNumberOfBasis = tMesh->get_number_of_indexed_basis();

            // allocate matrix with ids
            Matrix< IdMat > tHMRDomainIDs( tNumberOfBasis, 1 );
            Matrix< IdMat > tHMRIDs( tNumberOfBasis, 1 );
            Matrix< IdMat > tHMRInds( tNumberOfBasis, 1 );

             // populate matrix
             for( uint k=0; k<tNumberOfBasis; ++k )
             {
                 tHMRDomainIDs( k ) = tMesh->get_basis_by_index( k )->get_hmr_id();

                 tHMRIDs( k ) = tMesh->get_basis_by_index( k )->get_id();

                 tHMRInds( k ) = tMesh->get_basis_by_index( k )->get_index();
             }

             // save ids to file
             save_matrix_to_hdf5_file( tFileID,
                                       "Basis_HMR_Domain_ID",
                                       tHMRDomainIDs,
                                       tStatus );

             // save ids to file
             save_matrix_to_hdf5_file( tFileID,
                                       "Basis_HMR_ID",
                                       tHMRIDs,
                                       tStatus );

             // save ids to file
             save_matrix_to_hdf5_file( tFileID,
                                       "Basis_HMR_Ind",
                                       tHMRInds,
                                       tStatus );

              Matrix< IdMat > tHMRLevel( tNumberOfBasis, 1 );

              // populate matrix
              for( uint k=0; k<tNumberOfBasis; ++k )
              {
                  tHMRLevel( k ) = tMesh->get_basis_by_index( k )->get_level();
              }

              // save ids to file
              save_matrix_to_hdf5_file( tFileID,
                                        "Basis_HMR_Level",
                                        tHMRLevel,
                                        tStatus );

              // populate matrix
              for( uint k=0; k<tNumberOfBasis; ++k )
              {
                  // Get vector with external fine indices
                   moris::Matrix< DDSMat > tIndices = tMesh->get_children_ind_for_basis( k );
                   // Get weights
                   moris::Matrix< DDRMat > tWeights = tMesh->get_children_weights_for_parent( k );

                   if ( tIndices.n_cols() == 0 )
                   {
                       tIndices.set_size( 1, 1, -1 );
                   }

                   if ( tWeights.n_cols() == 0 )
                   {
                       tWeights.set_size( 1, 1, -1 );
                   }

                   moris_id tID= tMesh->get_basis_by_index( k )->get_id();

                   // save ids to file
                   save_matrix_to_hdf5_file( tFileID,
                                             "Children for Basis_HMR_Ind ID =" + std::to_string( tID ),
                                             tIndices,
                                             tStatus );

                   // save ids to file
                   save_matrix_to_hdf5_file( tFileID,
                                             "Children for Basis_HMR_Weights ID =" + std::to_string( tID ),
                                             tWeights,
                                             tStatus );
              }

              // populate matrix
              for( uint k=0; k<tNumberOfBasis; ++k )
              {
                  // get the number of carse adofs which are interpolating into this fine adof.
                  moris:: uint tNumCoarseDofs = tMesh->get_basis_by_index( k )->get_number_of_parents();

                  moris::Matrix< DDSMat > tIndices(1, tNumCoarseDofs, -1);

                  // Loop over these coarse adofs
                  for ( moris::uint Ia = 0; Ia < tNumCoarseDofs; Ia++ )
                  {
                      // Get external index of coarse adof
                      moris:: uint tCoarseDofIndex = tMesh->get_basis_by_index( k )->get_parent( Ia )->get_index();

                      tIndices( 0, Ia ) = tCoarseDofIndex;
                  }

                   if ( tIndices.n_cols() == 0 )
                   {
                       tIndices.set_size( 1, 1, -1 );
                   }

                   moris_id tID= tMesh->get_basis_by_index( k )->get_id();

                   // save ids to file
                   save_matrix_to_hdf5_file( tFileID,
                                             "Parents for Basis_HMR_Ind ID =" + std::to_string( tID ),
                                             tIndices,
                                             tStatus );
              }

             // close file
             tStatus = H5Fclose( tFileID );
        }

// -----------------------------------------------------------------------------

        void HMR::flag_elements_on_working_pattern(       Cell< hmr::Element* > & aElements,
                                                    const uint                    aMinRefinementLevel )
        {
            // get  working pattern
            uint tWorkingPattern = mParameters->get_working_pattern();

            // loop over all active elements
            for( hmr::Element* tCell : aElements )
            {
                // get pointer to Background Element
                Background_Element_Base * tElement = tCell->get_background_element();

                // put this element on the list
                tElement->set_refined_flag( tWorkingPattern );

                // set the minumum refinement level, which is inherited to children
                tElement->update_min_refimenent_level( aMinRefinementLevel );

                // also flag all parents
                while( tElement->get_level() > 0 )
                {
                    // get parent of this element
                    tElement = tElement->get_parent();

                    // set flag for parent of element
                    tElement->set_refined_flag( tWorkingPattern );
                }
            }
        }

// -----------------------------------------------------------------------------

        void HMR::put_elements_on_refinment_queue( Cell< hmr::Element* > & aElements )
        {
            // loop over all active elements
            for( hmr::Element* tCell :  aElements )
            {
                tCell->get_background_element()->put_on_refinement_queue();
            }
        }

// -----------------------------------------------------------------------------

        void HMR::perform_refinement_based_on_working_pattern( const uint aPattern,
                                                               const bool aResetPattern )
        {
            if ( aResetPattern )
            {
                mDatabase->get_background_mesh()->reset_pattern( aPattern );
            }

            // refine database and remember flag
            mDatabase->perform_refinement( aPattern, ! mPerformRefinementCalled );

            // remember that refinement has been called
            mPerformRefinementCalled = true;
        }

// -----------------------------------------------------------------------------

        void HMR::perform_refinement( const uint aPattern )
        {
            // refine database and remember flag
            mDatabase->get_background_mesh()->perform_refinement( aPattern );

            // remember that refinement has been called
            mPerformRefinementCalled = true;
        }

// -----------------------------------------------------------------------------

        void HMR::update_refinement_pattern( const uint aPattern )
        {
            mDatabase->update_bspline_meshes( aPattern );
            mDatabase->update_lagrange_meshes( aPattern );

            // set flag that this function has been called
            mUpdateRefinementCalled = true;
        }

// -----------------------------------------------------------------------------

        void HMR::set_activation_pattern( const uint & aActivationPattern )
        {
            this->get_database()->set_activation_pattern( aActivationPattern );
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Mesh > HMR::create_mesh()
        {
            MORIS_ERROR(false,"create_mesh() not changed yet" );
            return std::make_shared< Mesh >( mDatabase,
                                             mParameters->get_lagrange_orders().max(),
                                             mParameters->get_lagrange_output_pattern() );
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Mesh > HMR::create_mesh( const uint & aLagrangeIndex )
        {
            return std::make_shared< Mesh >( mDatabase,
                                             aLagrangeIndex );
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Mesh > HMR::create_mesh( const uint & aLagrangeOrder,
                                                  const uint & aPattern )
        {
            return std::make_shared< Mesh >( mDatabase,
                                             aLagrangeOrder,
                                             aPattern );
        }

        std::shared_ptr< Mesh > HMR::create_mesh( const uint & aLagrangeOrder,
                                                  const uint & aLagrangePattern,
                                                  const uint & aBsplinePattern )
        {
            return std::make_shared< Mesh >( mDatabase,
                                             aLagrangeOrder,
                                             aLagrangePattern,
                                             aBsplinePattern );
        }

        Interpolation_Mesh_HMR * HMR::create_interpolation_mesh( const uint & aLagrangeMeshIndex)
        {
            return new Interpolation_Mesh_HMR( mDatabase, aLagrangeMeshIndex );
        }


        Interpolation_Mesh_HMR * HMR::create_interpolation_mesh( const uint & aLagrangeOrder,
                                                                 const uint & aPattern )
        {
            return new Interpolation_Mesh_HMR( mDatabase,
                                               aLagrangeOrder,
                                               aPattern );
        }

        Interpolation_Mesh_HMR * HMR::create_interpolation_mesh( const uint & aOrder,
                                                                 const uint & aLagrangePattern,
                                                                 const uint & aBsplinePattern)
        {
            return new Interpolation_Mesh_HMR ( mDatabase,
                                                aOrder,
                                                aLagrangePattern,
                                                aBsplinePattern);
        }

// -----------------------------------------------------------------------------

        Integration_Mesh_HMR * HMR::create_integration_mesh( const uint &                   aLagrangeOrder,
                                                             const uint &                   aPattern,
                                                             Interpolation_Mesh_HMR * aInterpolationMesh)
        {
            return new Integration_Mesh_HMR ( mDatabase,
                                              aLagrangeOrder,
                                              aPattern,
                                              aInterpolationMesh);
        }

// -----------------------------------------------------------------------------

        Integration_Mesh_HMR * HMR::create_integration_mesh( const uint &                   aLagrangeMeshIndex,
                                                             Interpolation_Mesh_HMR * aInterpolationMesh)
        {
            return new Integration_Mesh_HMR ( mDatabase,
                                              aLagrangeMeshIndex,
                                              aInterpolationMesh);
        }


// -----------------------------------------------------------------------------

        std::shared_ptr< Field > HMR::create_field( const std::string & aLabel )
        {
            MORIS_ERROR(false,"create_field() not changed yet" );
            return this->create_field( aLabel,
                                       mParameters->get_lagrange_orders().max(),
                                       mParameters->get_bspline_orders().max() );
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Field > HMR::create_field( const std::string & aLabel,
                                                    const uint        & aLagrangeIndex,
                                                    const uint        & aBSplineIndex )
        {
            //return mInputMesh->create_field( aLabel );
            uint tFieldIndex = mFields.size();

            // add a new field to the list
            mFields.push_back( mMeshes( aLagrangeIndex )->create_field( aLabel, aBSplineIndex ) );

            // return the pointer
            return mFields( tFieldIndex );
        }

// -----------------------------------------------------------------------------

        void HMR::flag_element( const moris_index aElementIndex )
        {
            mDatabase->flag_element( aElementIndex );
        }

// -----------------------------------------------------------------------------

//        std::shared_ptr< Field > HMR::create_field( const Field_Param & aParameters )
//        {
//            MORIS_ERROR(false,"create_field() not changed yet" );
//            MORIS_LOG_INFO( "%s Loading field %s from file %s.\n\n",
//                    proc_string().c_str(),
//                    aParameters.mLabel.c_str(),
//                    aParameters.mSource.c_str() );
//
//            // load the field from an exodos or hdf file
//            std::shared_ptr< Field > aField = this->load_field_from_file( aParameters.mLabel,
//                                                                          aParameters.mSource,
//                                                                          aParameters.mInputLagrangeOrder,
//                                                                          aParameters.mInputBSplineOrder );
//
//            // set the output order, if it was passed to the parameter
//            if( aParameters.mOutputBSplineOrder != 0 )
//            {
//                aField->set_bspline_output_order( aParameters.mOutputBSplineOrder );
//            }
//
//            // set the ID ( actually, we don't need the ID, but it makes sense to store it)
//            aField->set_id( aParameters.mID );
//
//            // return the field pointer
//            return aField;
//        }

// -----------------------------------------------------------------------------

        void HMR::save_background_mesh_to_vtk( const std::string & aFilePath )
        {
            mDatabase->get_background_mesh()->save_to_vtk( aFilePath );
        }

// -----------------------------------------------------------------------------

        void HMR::save_bsplines_to_vtk( const std::string & aFilePath,
                                        const uint        & aLagrangeMeshIndex,
                                        const uint        & aBsplineMeshIndex  )
        {
            // dump mesh
            mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex )
                     ->get_bspline_mesh( aBsplineMeshIndex )->save_to_vtk( aFilePath );
        }

// -----------------------------------------------------------------------------

        void HMR::calculate_bspline_coordinates( const uint        & aLagrangeMeshIndex,
                                                 const uint        & aBsplineMeshIndex  )
        {
            mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex )
                     ->get_bspline_mesh( aBsplineMeshIndex )->calculate_basis_coordinates();
        }

// -----------------------------------------------------------------------------

        void HMR::save_faces_to_vtk( const std::string & aFilePath,
                                     const uint        & aLagrangeMeshIndex )
        {
            // dump mesh
            mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex )->save_faces_to_vtk( aFilePath );
        }

// -----------------------------------------------------------------------------

        void HMR::save_edges_to_vtk( const std::string & aFilePath,
                                     const uint        & aLagrangeMeshIndex )
        {
            MORIS_ERROR( mParameters->get_number_of_dimensions() == 3, "HMR::save_edges_to_vtk() can only be called for 3D meshes" );

            // dump mesh
            mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex )->save_edges_to_vtk( aFilePath );
        }

// ----------------------------------------------------------------------------

        void HMR::save_mesh_to_vtk( const std::string & aFilePath,
                                    const uint        & aLagrangeMeshIndex )
        {
            // dump mesh
            mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex )->save_to_vtk( aFilePath );
        }

// ----------------------------------------------------------------------------

        std::shared_ptr< Field > HMR::load_field_from_hdf5_file( const std::string & aLabel,
                                                                 const std::string & aFilePath,
                                                                 const uint          aLagrangeIndex,
                                                                 const uint          aBSpineIndex )
        {
            // opens an existing file with read and write access
            hid_t tFileID = open_hdf5_file( aFilePath );

            // error handler
            herr_t tStatus = 0;

            std::shared_ptr< moris::hmr::Mesh > tMesh = this->create_mesh( aLagrangeIndex );

            uint tFieldIndex = mFields.size();

            // add a new field to the list ( zero will be overwritten )
            mFields.push_back( tMesh->create_field( aLabel, aBSpineIndex ) );

            // get a pointer to this field
            std::shared_ptr< Field > aField = mFields( tFieldIndex );

            load_matrix_from_hdf5_file( tFileID, aLabel, aField->get_coefficients(), tStatus );

            // close hdf5 file
            close_hdf5_file( tFileID );

            uint tNumberOfCoeffs = aField->get_coefficients().length();

            uint tNumberOfCoeffs_BSpline= tMesh->get_lagrange_mesh()->get_bspline_mesh( aBSpineIndex )->get_number_of_active_basis_on_proc();

            MORIS_ERROR( tNumberOfCoeffs == tNumberOfCoeffs_BSpline,
                    "load_field_from_hdf5_file(), file and BSpline number of coefficients does not match. Check BSpline Mesh Index");

            // get pointer to B-Spline mesh
            uint tBSplineOrder= tMesh->get_lagrange_mesh()->get_bspline_mesh( aBSpineIndex )->get_order();

            // set order of B-Splines
            aField->set_bspline_order( tBSplineOrder );

            // get number of nodes from input mesh
            uint tNumberOfNodes = tMesh->get_num_nodes();

            // allocate field of nodes
            aField->get_node_values().set_size( tNumberOfNodes, 1 );

            // evaluate node values
            aField->evaluate_node_values();

            // return the pointer
            return aField;
        }

// ----------------------------------------------------------------------------

        std::shared_ptr< Field > HMR::load_field_from_exo_file( const std::string & aLabel,
                                                                const std::string & aFilePath,
                                                                const uint          aLagrangeIndex,
                                                                const uint          aBSpineIndex )
        {
            // create mesh object
            mtk::Mesh * tMesh = mtk::create_mesh( MeshType::STK, aFilePath, nullptr, false );

            std::shared_ptr< moris::hmr::Mesh > tHmrMesh = this->create_mesh( aLagrangeIndex );

            uint tFieldIndex = mFields.size();

            // add a new field to the list ( zero will be overwritten )
            mFields.push_back( tHmrMesh->create_field( aLabel, aBSpineIndex ) );

            // get a pointer to this field
            std::shared_ptr< Field > aField = mFields( tFieldIndex );

            // load nodes from mesh
            uint tNumberOfExodusNodes = tMesh->get_num_nodes();
            uint tNumberOfNodes = tHmrMesh->get_num_nodes();

            // make sure that mesh is correct
            MORIS_ERROR( tNumberOfExodusNodes == tNumberOfNodes,
                            "Number of Nodes does not match. Did you specify the correct mesh?" );

            // create array of indices for MTK interface
            Matrix< IndexMat > tIndices( 1, tNumberOfExodusNodes  );
            for( uint k = 0; k < tNumberOfExodusNodes; ++k )
            {
                tIndices( k ) = k;
            }

            // get ref to values
            Matrix< DDRMat > & tValues = aField->get_node_values();

            // allocate nodal field
            tValues.set_size( tNumberOfExodusNodes, 1 );

            tValues = tMesh->get_entity_field_value_real_scalar( tIndices,
                                                                 aLabel,
                                                                 EntityRank::NODE );

            // read reverse map in case of renumbering
            Matrix< DDSMat > tReverseMap;
            if(  mParameters->get_renumber_lagrange_nodes() )
            {
                // load values into field
                herr_t tStatus = 0;
                hid_t tHDF5File = open_hdf5_file( "Reverse_Map_1.hdf5" );
                load_matrix_from_hdf5_file( tHDF5File, "Id", tReverseMap, tStatus );
                close_hdf5_file( tHDF5File );
            }

            // having the values, we must now rearrange them in the order of the HMR mesh.
            // Therefore, we create a map
            map< moris_id, real > tValueMap;
            for( uint k = 0; k < tNumberOfExodusNodes; ++k )
            {
                // get ID of this node in exodus mesh
                uint tExodusNodeId = tMesh->get_glb_entity_id_from_entity_loc_index( k,EntityRank::NODE );

                MORIS_ERROR( tExodusNodeId > 0,"Exodus node ID for index %-5i is negative.",k);

                // use map between current exodus IDs and original output mesh id
                if(  mParameters->get_renumber_lagrange_nodes() )
                {
                    MORIS_ERROR( tExodusNodeId - 1 < (uint)tReverseMap.size(0),
                            "Node ID in Exodus mesh exceeds size of Reverse_Map_1.hdf5 field mesh: Node ID %-5i versus %-5i.", tExodusNodeId, tReverseMap.size(0));

                    MORIS_ERROR( tReverseMap( tExodusNodeId - 1 ) >= 0,
                            "Reverse map (%i) points to negative index.", tExodusNodeId);

                    tExodusNodeId = tReverseMap( tExodusNodeId - 1 ) + 1;
                }

                // construct map < exodus node Id, field value>
                tValueMap[ tExodusNodeId ] = tValues( k );
            }

            // make sure that field is a row matrix
            tValues.set_size( tNumberOfNodes, 1 );
            tValues.fill(MORIS_REAL_MAX);

            // now, we rearrange the values according to the ID of the Lagrange Mesh
            for( uint k=0; k<tNumberOfNodes; ++k )
            {
                tValues( k ) = tValueMap.find( tHmrMesh->get_mtk_vertex( k ).get_id() );

                MORIS_ERROR( tValues( k ) < MORIS_REAL_MAX,
                        "Map did not cover component %i of vecto tValues: %e", k, tValues( k ));
            }

            // finally, we set the order of the B-Spline coefficients
            aField->set_bspline_order( tHmrMesh->get_lagrange_mesh()->get_bspline_mesh( aBSpineIndex )->get_order() );

            // delete mesh pointer
            delete tMesh;

            // return the pointer
            return aField;
        }

// ----------------------------------------------------------------------------

        std::shared_ptr< Field > HMR::load_field_from_file( const std::string & aLabel,
                                                            const std::string & aFilePath,
                                                            const uint          aLagrangeIndex,
                                                            const uint          aBSpineIndex )
        {
            // detect file type
            std::string tType = aFilePath.substr( aFilePath.find_last_of(".")+1, aFilePath.length() );

            if( tType == "hdf5" || tType == "h5" )
            {
                // assume this is a hdf file
                return this->load_field_from_hdf5_file( aLabel, aFilePath, aLagrangeIndex, aBSpineIndex );
            }
            else
            {
                // assume this is an exodus file
                return this->load_field_from_exo_file( aLabel, aFilePath, aLagrangeIndex, aBSpineIndex );
            }
        }

// ----------------------------------------------------------------------------

        void HMR::create_input_and_output_meshes()
        {
            // clear memory
            mMeshes.clear();

            // get number of Lagrange meshes
            uint tNumberOfMeshes =  mParameters->get_number_of_lagrange_meshes();

            // create meshes
            for( uint Ik=0; Ik<tNumberOfMeshes; ++Ik )
            {
                mMeshes.push_back(  this->create_mesh( Ik ) );
            }
        }

// ----------------------------------------------------------------------------

        void HMR::perform_initial_refinement( const uint aPattern )
        {
            // get minimum refinement from parameters object
            uint tInitialRefinement = mParameters->get_initial_refinement();

            for( uint Ik=0; Ik<tInitialRefinement; ++Ik )
            {
                // get pointer to background mesh
                Background_Mesh_Base * tBackMesh =  mDatabase->get_background_mesh();

                // get number of active elements on mesh
                uint tNumberOfElements = tBackMesh->get_number_of_active_elements_on_proc();

                // flag all elements
                for( uint e=0; e<tNumberOfElements; ++e )
                {
                    // get pointer to background element
                    Background_Element_Base * tElement = tBackMesh->get_element( e );

//                    // set minumum level for this element
//                    tElement->set_min_refimenent_level( tInitialRefinement );         //FIXME

                    // flag this element
                    tElement->put_on_refinement_queue();
                }

                // run the refiner
                this->perform_refinement_based_on_working_pattern( aPattern );

                mDatabase->update_bspline_meshes( aPattern );
                mDatabase->update_lagrange_meshes( aPattern );
            }

//            if( mParameters->get_additional_lagrange_refinement()  == 0 )
//            {
//                // union pattern is needed, otherwise error is thrown
////                this->get_database()->copy_pattern( mParameters->get_lagrange_output_pattern(),
////                                                    mParameters->get_union_pattern() );
//
//                // update database
////                mDatabase->update_bspline_meshes();
////                mDatabase->update_lagrange_meshes();
//            }
//            else
//            {
//                MORIS_ERROR(false, "not implemented yet");
//                // select B-Spline flags on output for second flagging
//                this->get_database()->set_activation_pattern( mParameters->get_bspline_output_pattern() );
//
//                // add delta for Lagrange
//                tInitialRefinement += mParameters->get_additional_lagrange_refinement();
//
//                // get number of active elements on mesh
//                tNumberOfElements = tBackMesh->get_number_of_active_elements_on_proc();
//
//                // flag all elements
//                for( uint e=0; e<tNumberOfElements; ++e )
//                {
//                    // get pointer to background element
//                    Background_Element_Base * tElement = tBackMesh->get_element( e );
//
//                    // set minumum level for this element
//                    tElement->set_min_refimenent_level( tInitialRefinement );
//
//                    // flag this element
//                    tElement->put_on_refinement_queue();
//                }
//
//                this->perform_refinement( RefinementMode::LAGRANGE_INIT );
//            }
        }

// ----------------------------------------------------------------------------

        void HMR::user_defined_flagging( int ( *aFunction )(       Element                    * aElement,
                                                             const Cell< Matrix< DDRMat > >   & aElementLocalValues,
                                                                   ParameterList              & aParameters ),
                                         Cell< std::shared_ptr< Field > > & aFields,
                                         ParameterList                    & aParameters,
                                         const uint                       & aPattern )
        {
            // remember current active scheme
            uint tActivePattern = mDatabase->get_activation_pattern();

            // set activation pattern to input
            if( tActivePattern != aPattern )
            {
                // set active pattern to input mesh
                mDatabase->set_activation_pattern( aPattern );
            }

            // get number of fields
            uint tNumberOfFields = aFields.size();

            MORIS_ERROR( tNumberOfFields > 0, "No fields defined for refinement" );

            // create empty cell of fields
            Matrix< DDRMat> tEmpty;
            Cell< Matrix< DDRMat> > tFields( tNumberOfFields, tEmpty );

            // get number of elements from input mesh
            uint tNumberOfElements  = mDatabase->get_background_mesh()->get_number_of_active_elements_on_proc();

            // loop over all fields
            for( auto tField: aFields )
            {
                // test if field has node values
                if( tField->get_node_values().length() == 0 )
                {
                    tField->evaluate_node_values();
                }
            }

            // grab max level from settings
            uint tMaxLevel = mParameters->get_max_refinement_level();

            uint tLagrangeInputMeshIndex = mParameters->get_lagrange_input_mesh()(0);             //FIXME this works only for one input mesh

            // loop over all elements
            for( uint e=0; e<tNumberOfElements; ++e )
            {
                // get pointer to element
                Element * tElement =  mMeshes( tLagrangeInputMeshIndex )->get_lagrange_mesh()->get_element( e );

                // only consider element if level is below max specified level

                // loop over all fields
                for( uint f = 0; f<tNumberOfFields; ++f )
                {
                    // grab nodal values
                    aFields( f )->get_element_local_node_values( e, tFields( f ) );
                }

                // check flag from user defined function
                int tFlag = aFunction( tElement,
                                       tFields,
                                       aParameters );

                // chop flag if element is at max defined level
                if( tElement->get_level() > tMaxLevel )
                {
                    // an element above the max level can only be coarsened
                    tFlag = -1;
                }
                else if( tElement->get_level() == tMaxLevel)
                {
                    // an element on the max level can only be kept or coarsened
                    // but nor refined
                    tFlag = std::min( tFlag, 0 );
                }

                // perform flagging test
                if( tFlag == 1 )
                {
                    // flag this element and parents of neighbors
                    mDatabase->flag_element( e );
                }
                else if ( tFlag == 0 )
                {
                    // flag the parent of this element
                    mDatabase->flag_parent( e );
                }
            }

//            // get max level on this mesh
//            uint tMaxLevelOnMesh = mDatabase->get_background_mesh()->get_max_level();
//
//            if( mParameters->get_refinement_buffer() > 0 )
//            {
//                // get number of levels
//                for( uint tLevel=0; tLevel<=tMaxLevelOnMesh; ++tLevel )
//                {
//                    // create extra buffer
//                    mDatabase->create_extra_refinement_buffer_for_level( tLevel );
//                }
//            }

            // reset activation pattern of database
            if( tActivePattern != aPattern )
            {
                mDatabase->set_activation_pattern( tActivePattern );
            }
        }

// ----------------------------------------------------------------------------

        void HMR::user_defined_flagging( int ( *aFunction )(       Element                            * aElement,
                                                                     const Cell< Matrix< DDRMat > >   & aElementLocalValues,
                                                                           ParameterList              & aParameters ),
                                                 Cell< Matrix< DDRMat > >         & aFields,
                                                 ParameterList                    & aParameters,
                                                 const uint                       & aPattern )
                {
                    // remember current active scheme
                    uint tActivePattern = mDatabase->get_activation_pattern();

                    // set activation pattern to input
                    if( tActivePattern != aPattern )
                    {
                        // set active pattern to input mesh
                        mDatabase->set_activation_pattern( aPattern );
                    }

                    // get number of fields
                    uint tNumberOfFields = aFields.size();

                    MORIS_ERROR( tNumberOfFields > 0, "No fields defined for refinement" );

                    // create empty cell of fields
                    Matrix< DDRMat> tEmpty;
                    Cell< Matrix< DDRMat> > tFields( tNumberOfFields, tEmpty );

                    // get number of elements from input mesh
                    uint tNumberOfElements  = mDatabase->get_background_mesh()->get_number_of_active_elements_on_proc();


                    // grab max level from settings
                    uint tMaxLevel = mParameters->get_max_refinement_level();

                    uint tLagrangeInputMeshIndex = mParameters->get_lagrange_input_mesh()(0);             //FIXME this works only for one input mesh

                    // loop over all elements
                    for( uint e=0; e<tNumberOfElements; ++e )
                    {
                        // get pointer to element
                        Element * tElement =  mMeshes( tLagrangeInputMeshIndex )->get_lagrange_mesh()->get_element( e );

                        // only consider element if level is below max specified level

                        Matrix< IndexMat > tElementsInds = tElement->get_vertex_inds();

                        tFields( 0 ).set_size(tElementsInds.numel(), 1 );

                        // loop over all fields
                        for( uint f = 0; f<tElementsInds.numel(); ++f )
                        {
                            // grab nodal values
                            MORIS_ASSERT(aFields.size() == 1,"only implemented for one field right now." );

                            tFields( 0 )( f ) = aFields( 0 )( tElementsInds( f ) );
                        }

                        // check flag from user defined function
                        int tFlag = aFunction( tElement,
                                               tFields,
                                               aParameters );

                        // chop flag if element is at max defined level
                        if( tElement->get_level() > tMaxLevel )
                        {
                            // an element above the max level can only be coarsened
                            tFlag = -1;
                        }
                        else if( tElement->get_level() == tMaxLevel)
                        {
                            // an element on the max level can only be kept or coarsened
                            // but nor refined
                            tFlag = std::min( tFlag, 0 );
                        }

                        // perform flagging test
                        if( tFlag == 1 )
                        {
                            // flag this element and parents of neighbors
                            mDatabase->flag_element( e );
                        }
                        else if ( tFlag == 0 )
                        {
                            // flag the parent of this element
                            mDatabase->flag_parent( e );
                        }
                    }


                    // reset activation pattern of database
                    if( tActivePattern != aPattern )
                    {
                        mDatabase->set_activation_pattern( tActivePattern );
                    }
                }

        // ----------------------------------------------------------------------------

        void HMR::get_candidates_for_refinement(       Cell< hmr::Element* > & aCandidates,
                                                 const uint                    aLagrangeMeshIndex)
        {
            // reset candidate list
            aCandidates.clear();

            // Get Lagrange mesh pattern
            uint tPattern = mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex )->get_activation_pattern();

            // make sure that input pattern is active
            mDatabase->set_activation_pattern( tPattern );

            // get pointer to background mesh
            Background_Mesh_Base * tBackgroundMesh = mDatabase->get_background_mesh();

            uint tMaxLevel = tBackgroundMesh->get_max_level();

            // pick first Lagrange mesh on input pattern
            // fixme: add option to pick another one
            Lagrange_Mesh_Base * tMesh = mDatabase->get_lagrange_mesh_by_index( aLagrangeMeshIndex );

            // counter for elements
            uint tCount = 0;

            // loop over all levels and determine size of Cell
            for( uint l = 0; l <= tMaxLevel; ++l )
            {
                Cell< Background_Element_Base * > tBackgroundElements;

                tBackgroundMesh->collect_elements_on_level_within_proc_domain( l, tBackgroundElements );

                // element must be active or refined
                for( Background_Element_Base * tElement : tBackgroundElements )
                {
                    // if element  is active or refined but not padding
                    if( ( tElement->is_active( tPattern ) || tElement->is_refined( tPattern ) ) && ! tElement->is_padding() )
                    {
                        // increment counter
                        ++tCount;
                    }
                }
            }

            // allocate memory for output
            aCandidates.resize( tCount, nullptr );

            // reset counter
            tCount = 0;
            // loop over all levels
            for( uint l=0; l<=tMaxLevel; ++l )
            {
                Cell< Background_Element_Base * > tBackgroundElements;
                tBackgroundMesh->collect_elements_on_level_within_proc_domain( l, tBackgroundElements );

                // element must be active or refined
                for(  Background_Element_Base * tElement : tBackgroundElements )
                {
                    if( ( tElement->is_active( tPattern ) ||  tElement->is_refined( tPattern ) ) && ! tElement->is_padding() )
                    {
                        aCandidates( tCount++ ) = tMesh->get_element_by_memory_index( tElement->get_memory_index() );
                    }
                }
            }
        }
// -----------------------------------------------------------------------------

        uint HMR::flag_volume_and_surface_elements_on_working_pattern( const std::shared_ptr<Field> aScalarField )
        {
            // the funciton returns the number of flagged elements
            uint aElementCounter = 0;

            // candidates for refinement
            Cell< hmr::Element*  > tCandidates;

            // elements to be flagged for refinement
            Cell< hmr::Element* > tRefinementList;

            uint tLagrangeMeshIndex = aScalarField->get_lagrange_mesh_index();

            // get candidates for surface
            this->get_candidates_for_refinement( tCandidates,
                                                 tLagrangeMeshIndex );

            // call refinement manager and get intersected cells
            this->find_cells_intersected_by_levelset( tRefinementList,
                                                      tCandidates,
                                                      aScalarField->get_node_values() );

            // add length of list to counter
            aElementCounter += tRefinementList.size();

            // flag elements in HMR
            this->flag_elements_on_working_pattern( tRefinementList, aScalarField->get_min_surface_level() );

            // get candidates from volume
            this->get_candidates_for_refinement( tCandidates,
                                                 tLagrangeMeshIndex);

            // call refinement manager and get volume cells
            this->find_cells_within_levelset( tRefinementList,
                                              tCandidates,
                                              aScalarField->get_node_values() );

            // add length of list to counter
            aElementCounter += tRefinementList.size();

            // flag elements in database
            this->flag_elements_on_working_pattern( tRefinementList, aScalarField->get_min_volume_level()  );

            // return number of flagged elements
            return aElementCounter;
        }

// -----------------------------------------------------------------------------

        uint HMR::flag_surface_elements_on_working_pattern( const std::shared_ptr<Field> aScalarField )
        {
            // the funciton returns the number of flagged elements
            uint aElementCounter = 0;

            // candidates for refinement
            Cell< hmr::Element* > tCandidates;

            // elements to be flagged for refinement
            Cell< hmr::Element* > tRefinementList;

            uint tLagrangeMeshIndex = aScalarField->get_lagrange_mesh_index();

            // get candidates for surface
            this->get_candidates_for_refinement( tCandidates,
                                                 tLagrangeMeshIndex );

            // call refinement manager and get intersected cells
            this->find_cells_intersected_by_levelset( tRefinementList,
                                                      tCandidates,
                                                      aScalarField->get_node_values() );

            // add length of list to counter
            aElementCounter += tRefinementList.size();

            // flag elements in HMR
            this->flag_elements_on_working_pattern( tRefinementList, aScalarField->get_min_surface_level() );

            // return number of flagged elements
            return aElementCounter;
        }

// -----------------------------------------------------------------------------

        uint HMR::based_on_field_put_elements_on_queue( const Matrix< DDRMat > & aFieldValues,
                                                        const uint             & aLagrangeMeshIndex )
        {
            uint aElementCounter = 0;

            // candidates for refinement
            Cell< hmr::Element* > tCandidates;

            // elements to be flagged for refinement
            Cell< hmr::Element* > tRefinementList;

            // get candidates for surface
            this->get_candidates_for_refinement( tCandidates,
                                                 aLagrangeMeshIndex);

            // call refinement manager and get intersected cells
            this->find_cells_intersected_by_levelset( tRefinementList,
                                                      tCandidates,
                                                      aFieldValues );

            // add length of list to counter
            aElementCounter += tRefinementList.size();

            // flag elements in HMR
            this->put_elements_on_refinment_queue( tRefinementList );

            // return number of flagged elements
            return aElementCounter;
        }

// ----------------------------------------------------------------------------

        /**
         * needed for tutorials
         */
//        void HMR::perform_refinement_and_map_fields( const uint aPattern )
//        {
//            MORIS_ERROR(false,"perform_refinement_and_map_fields() not changed yet" );
//            // - - - - - - - - - - - - - - - - - - - - - -
//            // step 0: perform simple refinement
//            // - - - - - - - - - - - - - - - - - - - - - -
//
//            // in the tutorial, lagrange and B-Spline are the same refinement
//            this->perform_refinement( aPattern );
//
//            // create union of input and output
//            mDatabase->create_union_pattern();     //FIXME
//
//            // finalize database
//            this->finalize();
//
//            // - - - - - - - - - - - - - - - - - - - - - -
//            // step 1: find out which orders are needed
//            // - - - - - - - - - - - - - - - - - - - - - -
//
//            // number of input fields
//            uint tNumberOfFields = mFields.size();
//
//            // counter
//            uint tCount = 0;
//
//            // container for orders of fields
//            Matrix< DDUMat > tInputFieldOrders( 2*tNumberOfFields, 1 );
//
//            // loop over all fields
//            for( uint f=0; f<tNumberOfFields; ++f )
//            {
//                MORIS_ASSERT(false,"potentialy problematic");
//                tInputFieldOrders( tCount++ ) = mFields( f )->get_bspline_order();
//                tInputFieldOrders( tCount++ ) = mFields( f )->get_lagrange_order();
//            }
//
//            // chop container
//            tInputFieldOrders.resize( tCount, 1 );
//
//            // make orders unique
//            Matrix< DDUMat > tMeshOrders;
//            unique( tInputFieldOrders, tMeshOrders );
//
//            uint tNumberOfMappers = tMeshOrders.length();
//
//            // create map for mappers
//            Matrix< DDUMat > tMapperIndex( gMaxBSplineOrder+1, 1, MORIS_UINT_MAX );
//
//            for( uint k = 0; k<tNumberOfMappers; ++k )
//            {
//                tMapperIndex( tMeshOrders( k ) ) = k;
//            }
//
//            // - - - - - - - - - - - - - - - - - - - - - -
//            // step 2: create union meshes and mappers
//            // - - - - - - - - - - - - - - - - - - - - - -
//            mtk::Mesh_Manager tMeshManager;
//            Cell< std::shared_ptr< Interpolation_Mesh_HMR > > tUnionInterpMeshes;
//            Cell< std::shared_ptr< Integration_Mesh_HMR > >   tUnionIntegMeshes;
//            Cell< std::shared_ptr< Interpolation_Mesh_HMR > > tInputInterpMeshes;
//            Cell< std::shared_ptr< Integration_Mesh_HMR > >   tInputIntegMeshes;
//            Cell< mapper::Mapper * > tMappers( tNumberOfMappers, nullptr );
//
//            for( uint m=0; m<tNumberOfMappers; ++m )
//            {
//                //FIXME: CHANGE INTEGRATION MESHES TO DIRECTLY USE INTERPOLATION MESHES. (ELIMINATE DUPLICATE MESH CREATION)
//                // create interpolation mesh input
//                std::shared_ptr< Interpolation_Mesh_HMR > tInputInterpMesh = this->create_interpolation_mesh(tMeshOrders( m ), mParameters->get_lagrange_input_pattern() );
//
//                // add to vector of input interpolation meshes
//                tInputInterpMeshes.push_back( tInputInterpMesh );
//
//                // create integration mesh input
//                std::shared_ptr< Integration_Mesh_HMR > tInputIntegMesh = this->create_integration_mesh(tMeshOrders( m ), mParameters->get_lagrange_input_pattern(), *tInputInterpMesh );
//
//                // add to vector of input integration meshes
//                tInputIntegMeshes.push_back( tInputIntegMesh );
//
//                // create interpolation mesh union
//                std::shared_ptr< Interpolation_Mesh_HMR > tUnionInterpMesh = this->create_interpolation_mesh(tMeshOrders( m ), mParameters->get_union_pattern() ) ;
//
//                // add to vector of union interpolation meshes
//                tUnionInterpMeshes.push_back( tUnionInterpMesh );
//
//                // create integration mesh union
//                std::shared_ptr< Integration_Mesh_HMR > tUnionIntegMesh = this->create_integration_mesh(tMeshOrders( m ), mParameters->get_union_pattern(), *tUnionInterpMesh );
//
//                // add to vector of union interpolation meshes
//                tUnionIntegMeshes.push_back(tUnionIntegMesh);
//
//                // add pairs to mesh manager
//                moris::uint tMeshPairIndex = tMeshManager.register_mesh_pair(tUnionInterpMeshes(m).get(),tUnionIntegMeshes(m).get());
//
//                // create mapper
//                tMappers( m ) = new mapper::Mapper( &tMeshManager,tMeshPairIndex );
//            }
//
//            // - - - - - - - - - - - - - - - - - - - - - -
//            // step 3: map and project fields
//            // - - - - - - - - - - - - - - - - - - - - - -
//
//            for( uint f=0; f<tNumberOfFields; ++f )
//            {
//
//                // get pointer to input field
//                std::shared_ptr< Field > tInputField = mFields( f );
//
//                // get order
//                MORIS_ASSERT(false,"potentialy prblematic");
//                uint tBSplineOrder = tInputField->get_bspline_order();
//
//                // get index of mapper
//                uint m = tMapperIndex( tBSplineOrder );
//
//                // get pointer to field on union mesh
//                std::shared_ptr< Field > tUnionField =  tUnionInterpMeshes( m )->create_field(
//                        tInputField->get_label(),
//                        tBSplineOrder );
//
//
//                if( tInputField->get_lagrange_order() >= tBSplineOrder )
//                {
//                    // interpolate field onto union mesh
//                    mDatabase->interpolate_field( mParameters->get_lagrange_input_pattern(),
//                                                  tInputField,
//                                                  mParameters->get_union_pattern(),
//                                                  tUnionField );
//                }
//                else
//                {
//                    // first, project field on mesh with correct order
//                    std::shared_ptr< Field > tTemporaryField = tInputInterpMeshes( m )->create_field( tInputField->get_label(),
//                                                                                                      tBSplineOrder );
//
//                    mDatabase->change_field_order( tInputField, tTemporaryField );
//
//                    // now, interpolate this field onto the inion
//                    mDatabase->interpolate_field( mParameters->get_lagrange_input_pattern(),
//                                                  tTemporaryField,
//                                                  mParameters->get_union_pattern(),
//                                                  tUnionField );
//                }
//
//                // perform mapping
//                tMappers( m )->perform_mapping( tInputField->get_label(),
//                                                EntityRank::NODE,
//                                                tInputField->get_label(),
//                                                tUnionField->get_bspline_rank() );
//
//                // a small sanity test
//                MORIS_ASSERT(  tUnionField->get_coefficients().length() == tUnionInterpMeshes( m )->get_num_entities(
//                                mtk::order_to_entity_rank( tBSplineOrder ) ),
//                                "Number of B-Splines does not match" );
//
//                // get pointer to output mesh
//                std::shared_ptr< Mesh >  tOutputMesh = this->create_mesh( tInputField->get_lagrange_order(),
//                                                                          mParameters->get_lagrange_output_pattern() );
//
//                // create output field
//                std::shared_ptr< Field >  tOutputField = tOutputMesh->create_field( tInputField->get_label(),
//                                                                                    tBSplineOrder );
//
//                // move coefficients to output field
//                // fixme: to be tested with Eigen also
//                tOutputField->get_coefficients() = std::move( tUnionField->get_coefficients() );
//
//                // allocate nodes for output
//                tOutputField->get_node_values().set_size( tOutputMesh->get_num_nodes(), 1 );
//
//                // evaluate nodes
//                tOutputField->evaluate_node_values();
//
//                // make this field point to the output mesh
//                tInputField->change_mesh( tOutputField->get_mesh(),
//                                          tOutputField->get_field_index() );
//            }
//
//            // delete mappers
//            for( mapper::Mapper * tMapper : tMappers )
//            {
//                delete tMapper;
//            }
//        }
// ----------------------------------------------------------------------------

        void HMR::map_field_to_output(       std::shared_ptr< Field > aField,
                                       const uint                     aMesh_Index,
                                       const uint                     aBsplineMeshIndex)
        {
            // grab orders of meshes
            uint tSourceLagrangeOrder = aField->get_lagrange_order();
            uint tTargetLagrangeOrder = mDatabase->get_lagrange_mesh_by_index( aMesh_Index )->get_order();

            uint tTargetPattern = mDatabase->get_lagrange_mesh_by_index( aMesh_Index )->get_activation_pattern();

            // get order of Union Mesh
            uint tOrder = std::max( tSourceLagrangeOrder, tTargetLagrangeOrder );

            // create union pattern
            mDatabase->create_union_pattern( aField->get_lagrange_pattern(),
                                             tTargetPattern,
                                             mParameters->get_union_pattern() );

            // create union mesh
            Interpolation_Mesh_HMR * tUnionInterpolationMesh = this->create_interpolation_mesh( tOrder,
                                                                                                mParameters->get_union_pattern(),
                                                                                                tTargetPattern );   // order, lagrange pattern, bspline pattern

            // create union field
            std::shared_ptr< Field > tUnionField = tUnionInterpolationMesh->create_field( aField->get_label(),
                                                                                          aBsplineMeshIndex );        //index to 0 so that we only need one mesh

            // map source lagrange field to target lagrange field
            if( tSourceLagrangeOrder >= tTargetLagrangeOrder )
            {
                // interpolate field onto union mesh
                mDatabase->interpolate_field( aField->get_lagrange_pattern(),
                                              aField,
                                              mParameters->get_union_pattern(),
                                              tUnionField );
            }
            else
            {
                // mesh the input field is based on                                             //FIXME
                std::shared_ptr< Mesh > tInputMesh = this->create_mesh( tOrder,
                                                                        aField->get_lagrange_pattern(),
                                                                        aField->get_lagrange_pattern() );

                // first, project field on mesh with correct order
                std::shared_ptr< Field > tTemporaryField = tInputMesh->create_field( aField->get_label(),
                                                                                     0 );

                mDatabase->change_field_order( aField, tTemporaryField );

                // now, interpolate this field onto the union
                mDatabase->interpolate_field( aField->get_lagrange_pattern(),
                                              tTemporaryField,
                                              mParameters->get_union_pattern(),
                                              tUnionField );
            }

            // construct union integration mesh (note: this is not ever used but is needed for mesh manager)
            Integration_Mesh_HMR* tIntegrationUnionMesh = this->create_integration_mesh( tOrder, mParameters->get_union_pattern(), tUnionInterpolationMesh );


            // Add union mesh to mesh manager
            mtk::Mesh_Manager tMeshManager;
            moris::uint tMeshPairIndex = tMeshManager.register_mesh_pair( tUnionInterpolationMesh, tIntegrationUnionMesh );

            // create mapper
            mapper::Mapper tMapper( &tMeshManager,
                                    tMeshPairIndex,
                                    aBsplineMeshIndex );

            // project field to union
            tMapper.perform_mapping( aField->get_label(),
                                     EntityRank::NODE,
                                     aField->get_label(),
                                     tUnionField->get_bspline_rank() );

            // a small sanity test
//            MORIS_ASSERT( tUnionField->get_coefficients().length() == tUnionInterpolationMesh->get_num_coeffs( aBsplineMeshIndex ),
//                            "Number of B-Splines does not match" );

            // get pointer to output mesh
            std::shared_ptr< Mesh > tOutputMesh = this->create_mesh( tTargetLagrangeOrder,
                                                                     tTargetPattern );

            // create output field
            std::shared_ptr< Field > tOutputField = tOutputMesh->create_field( aField->get_label(),
                                                                               aBsplineMeshIndex );     // BSplineIndex

            // move coefficients to output field
            tOutputField->get_coefficients() = std::move( tUnionField->get_coefficients() );

            // allocate nodes for output
            tOutputField->get_node_values().set_size( tOutputMesh->get_num_nodes(), 1 );

            // evaluate nodes
            tOutputField->evaluate_node_values();

            // make this field point to the output mesh
            aField->change_mesh( tOutputField->get_mesh(),
                                 tOutputField->get_field_index() );
        }

// ----------------------------------------------------------------------------

        void HMR::map_field_to_output_union(       std::shared_ptr< Field > aField,
                                             const uint                     aUnionOrder )
        {
            // grab orders of meshes
            uint tLagrangeOrder = aField->get_lagrange_order();

           // get order of Union Mesh
            //uint tOrder = std::max( tLagrangeOrder, aUnionOrder );

            // create union mesh
            Interpolation_Mesh_HMR * tUnionInterpolationMesh = this->create_interpolation_mesh( aUnionOrder,
                                                                                                mParameters->get_union_pattern(),
                                                                                                mParameters->get_union_pattern() );

            // create union field
            std::shared_ptr< Field > tUnionField = tUnionInterpolationMesh->create_field( aField->get_label(),
                                                                                          0 );

            // map source lagrange field to target lagrange field
            if( tLagrangeOrder == aUnionOrder )
            {
                // interpolate field onto union mesh
                mDatabase->interpolate_field( aField->get_lagrange_pattern(),
                                              aField,
                                              mParameters->get_union_pattern(),
                                              tUnionField );
            }
            else
            {
                // mesh the input field is based on
                std::shared_ptr< Mesh > tInputMesh = this->create_mesh( aUnionOrder,
                                                                        aField->get_lagrange_pattern(),
                                                                        aField->get_lagrange_pattern() );

                // first, project field on mesh with correct order
                std::shared_ptr< Field > tTemporaryField = tInputMesh->create_field( aField->get_label(),
                                                                                     0 );

                mDatabase->change_field_order( aField, tTemporaryField );

                // now, interpolate this field onto the union
                mDatabase->interpolate_field( aField->get_lagrange_pattern(),
                                              tTemporaryField,
                                              mParameters->get_union_pattern(),
                                              tUnionField );
            }

            // make this field point to the output mesh
            aField->change_mesh( tUnionField->get_mesh(),
                                 tUnionField->get_field_index() );
        }

// ----------------------------------------------------------------------------

        void HMR::find_cells_intersected_by_levelset(        Cell< hmr::Element * > & aCells,
                                                             Cell< hmr::Element * > & aCandidates,
                                                      const  Matrix< DDRMat >    & aVertexValues,
                                                      const  real                  aLowerBound,
                                                      const  real                  aUpperBound )
        {
                // make sure that input makes sense
                MORIS_ASSERT( aLowerBound <= aUpperBound,
                        "find_cells_intersected_by_levelset() : aLowerBound bound must be less or equal aUpperBound" );

                // make sure that the field is a scalar field
                MORIS_ASSERT( aVertexValues.n_cols() == 1,
                        "find_cells_within_levelset() can only be performed on scalar fields" );

                // initialize output cell
                aCells.resize( aCandidates.size(), nullptr );

                // initialize counter
                uint tCount = 0;

                // loop over all candidates
                for( hmr::Element * tCell : aCandidates )
                {
                    // get cell of vertex pointers
                    Cell< mtk::Vertex * > tVertices = tCell->get_vertex_pointers();

                    // get number of vertices on this element
                    uint tNumberOfVertices = tVertices.size();

                    // assign matrix with vertex values
                    Matrix< DDRMat > tCellValues( tNumberOfVertices, 1 );

                    // loop over all vertices and extract scalar field
                    for( uint k=0; k<tNumberOfVertices; ++k )
                    {
                        // copy value from field into element local matrix
                        tCellValues( k ) = aVertexValues( tVertices( k )->get_index() );
                    }

                    // test if cell is inside
                    if ( tCellValues.min() <= aUpperBound && tCellValues.max() >= aLowerBound )
                    {
                        // copy pointer to output
                        aCells( tCount++ ) = tCell;
                    }
                }

                // shrink output to fit
                aCells.resize( tCount );
        }

        void HMR::find_cells_within_levelset(        Cell< hmr::Element * > & aCells,
                                                     Cell< hmr::Element * > & aCandidates,
                                              const  Matrix< DDRMat >       & aVertexValues,
                                              const  uint                     aUpperBound )
        {
            // make sure that the field is a scalar field
            MORIS_ASSERT( aVertexValues.n_cols() == 1, "find_cells_within_levelset() can only be performed on scalar fields" );

            // initialize output cell
            aCells.resize( aCandidates.size(), nullptr );

            // initialize counter
            uint tCount = 0;

            // loop over all candidates
            for( hmr::Element * tCell : aCandidates )
            {
                // get cell of vertex pointers
                Cell< mtk::Vertex * > tVertices = tCell->get_vertex_pointers();

                // get number of vertices on this element
                uint tNumberOfVertices = tVertices.size();

                // assign matrix with vertex values
                Matrix< DDRMat > tCellValues( tNumberOfVertices, 1 );

                // loop over all vertices and extract scalar field
                for( uint k=0; k<tNumberOfVertices; ++k )
                {
                    // copy value from field into element local matrix
                    tCellValues( k ) = aVertexValues( tVertices( k )->get_index() );
                }

                // test if cell is inside
                if(  tCellValues.max() <= aUpperBound )
                {
                    // copy pointer to output
                    aCells( tCount++ ) = tCell;
                }
            }

            // shrink output to fit
            aCells.resize( tCount );
        }

// ----------------------------------------------------------------------------
    } /* namespace hmr */
} /* namespace moris */
