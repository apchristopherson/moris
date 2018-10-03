/*
 * cl_HMR.cpp
 *
 *  Created on: May 5, 2018
 *      Author: messe
 */

#include "typedefs.hpp"
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "op_times.hpp" //LINALG/src
#include "fn_trans.hpp" //LINALG/src
#include "fn_eye.hpp" //LINALG/src

#include <GEN/src/cl_GEN_Geometry_Engine.hpp>

#include "cl_MDL_Model.hpp"
#include "cl_FEM_IWG_L2.hpp"

#include "HMR_HDF5_Tools.hpp"
#include "cl_HMR.hpp" //HMR/src
#include "cl_HMR_Mesh.hpp" //HMR/src
#include "cl_HMR_STK.hpp" //HMR/src
#include "cl_HMR_File.hpp" //HMR/src

#include "cl_HMR_Field.hpp"          //HMR/src
#include "cl_HMR_Background_Element_Base.hpp"

namespace moris
{
    namespace hmr
    {
// -----------------------------------------------------------------------------

        HMR::HMR ( Parameters * aParameters ) :
                mParameters( aParameters )
        {
            mDatabase   = std::make_shared< Database >( aParameters );
            mInputMesh  = this->create_mesh( mParameters->get_input_pattern() );
            mOutputMesh = this->create_mesh( mParameters->get_output_pattern() );

          //  this->finalize();
        }

// -----------------------------------------------------------------------------

        // alternative constuctor that converts ref to a pointer
        HMR::HMR ( Parameters & aParameters ) :
                                HMR( & aParameters )
        {

        }

// -----------------------------------------------------------------------------

        // alternative constuctor that uses parameter list
        HMR::HMR ( ParameterList & aParameterList )
            : HMR( new Parameters( aParameterList ) )
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

            mInputMesh = this->create_mesh( mParameters->get_input_pattern() );
            mOutputMesh = this->create_mesh( mParameters->get_output_pattern() );

           // this->finalize();
        }


// -----------------------------------------------------------------------------
        void
        HMR::save_to_exodus( const std::string & aPath, const double aTimeStep )
        {
            if( ! mPerformRefinementCalled )
            {
                this->save_to_exodus(
                        mParameters->get_input_pattern(),
                        aPath,
                        aTimeStep );
            }
            else
            {
                this->save_to_exodus(
                        mParameters->get_output_pattern(),
                        aPath,
                        aTimeStep );
            }

        }

// -----------------------------------------------------------------------------

        void
        HMR::save_last_step_to_exodus(
                            const std::string & aPath,
                            const double aTimeStep )
        {
            MORIS_ERROR( ! mUpdateRefinementCalled,
                    "HMR does not feel comfortable with you calling save_last_step_to_exodus() after you have overwritten the input pattern using update_refinement_pattern()");

            this->save_to_exodus( mParameters->get_input_pattern(),
                                   aPath,
                                   aTimeStep );
        }

// -----------------------------------------------------------------------------

        void
        HMR::save_to_exodus(
                const uint        & aMeshIndex,
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

        void
        HMR::save_to_hdf5( const std::string & aPath )
        {
            // create file object
            File tHDF5;

            // create file on disk
            tHDF5.create( aPath );

            // store settings object
            tHDF5.save_settings( mParameters );

            // get pointer to background mesh
            Background_Mesh_Base * tBackgroundMesh = mDatabase->get_background_mesh();

            // remember active pattern
            auto tActivePattern = tBackgroundMesh->get_activation_pattern();

            // save output pattern into file
            tHDF5.save_refinement_pattern(
                    tBackgroundMesh,
                    mParameters->get_output_pattern() );

            if( tActivePattern != tBackgroundMesh->get_activation_pattern() )
            {
                tBackgroundMesh->set_activation_pattern( tActivePattern );
            }

            // close hdf5 file
            tHDF5.close();
        }

// -----------------------------------------------------------------------------

        void
        HMR::save_coeffs_to_binary_files(
                const std::string & aFilePath )
        {

            // get number of meshes
            uint tNumberOfLagrangeMeshes = mDatabase->get_number_of_lagrange_meshes();

            // loop over all meshes
            for( uint m=0; m<tNumberOfLagrangeMeshes; ++m )
            {
                // get pointer to mesh
                Lagrange_Mesh_Base * tMesh = mDatabase->get_lagrange_mesh_by_index( m );

                // test if mesh links to output pattern
                if( tMesh->get_activation_pattern() == mParameters->get_output_pattern() )
                {
                    // calculate file path
                    std::string tFilePath =
                            aFilePath.substr(0,aFilePath.find_last_of(".")) // base path

                            // get order of lagrange mesh
                            + "_" + std::to_string( tMesh->get_order() )

                    // get order of bspline mesh
                    + "_" + std::to_string( tMesh->get_bspline_order() )

                    // finish path
                    +  aFilePath.substr( aFilePath.find_last_of("."), aFilePath.length() );

                    tMesh->save_coeffs_to_binary_file( tFilePath );
                }
            }
        }
// -----------------------------------------------------------------------------

        void
        HMR::save_coeffs_to_hdf5_file( const std::string & aFilePath )
        {
            // fixme: at the moment, this function only stores the coeffs
            // of one order. This function will be modified soon so that
            // multiple refinement orders are supported

            // make path parallel
            std::string tFilePath = parallelize_path( aFilePath );



            // get pointer to output mesh
            Lagrange_Mesh_Base * tMesh = nullptr;

            for( uint k=0; k<mDatabase->get_number_of_lagrange_meshes(); ++k )
            {
                tMesh = mDatabase->get_lagrange_mesh_by_index( k );
                if( tMesh->get_activation_pattern() == mParameters->get_output_pattern() )
                {
                    // cancel the loop. We only save one mesh and one order
                    break;
                }
            }

            // Create a new file using default properties
           herr_t tFileID = H5Fcreate(
                    tFilePath.c_str(),
                    H5F_ACC_TRUNC,
                    H5P_DEFAULT,
                    H5P_DEFAULT);

            // error handler
            herr_t tStatus;

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
            save_matrix_to_hdf5_file(
                    tFileID,
                    "NodeID",
                    tIDs,
                    tStatus );


            // count number of coefficients per node
            Matrix< DDUMat > tNumberOfCoeffs( tNumberOfNodes, 1 );

            // populate matrix
            for( uint k=0; k<tNumberOfNodes; ++k )
            {
                tNumberOfCoeffs( k ) = tMesh->get_node_by_index( k )
                        ->get_interpolation()->get_number_of_coefficients();

            }

            // save number of coeffs to file
            save_matrix_to_hdf5_file(
                    tFileID,
                    "NumberOfCoefficients",
                    tNumberOfCoeffs,
                    tStatus );

            // get max number of coeffs
            uint tMaxNumCoeffs = tNumberOfCoeffs.max();

            // count number of coefficients per node
            Matrix< IdMat > tCoeffIDs( tNumberOfNodes, tMaxNumCoeffs, gNoID );
            Matrix< DDRMat >  tWeights( tNumberOfNodes, tMaxNumCoeffs, 0.0 );


            // populate matrix
            for( uint k=0; k<tNumberOfNodes; ++k )
            {
                // get max number of dofs
                uint tMaxI = tNumberOfCoeffs( k );

                // get pointer to interpolation object
                mtk::Vertex_Interpolation * tInterp = tMesh
                        ->get_node_by_index( k )
                        ->get_interpolation();

                Matrix< IdMat >    tLocalIDs = tInterp->get_ids();
                const Matrix< DDRMat > & tLocalWeights = *tInterp->get_weights();

                // copy data into global matrix
                for( uint i=0; i<tMaxI; ++i )
                {
                    tCoeffIDs( k, i ) = tLocalIDs( i );
                    tWeights( k, i ) = tLocalWeights( i );
                }
            }

            // save ids to file
            save_matrix_to_hdf5_file(
                    tFileID,
                    "BSplineIDs",
                    tCoeffIDs,
                    tStatus );

            // save weights to file
            save_matrix_to_hdf5_file(
                    tFileID,
                    "InterpolationWeights",
                    tWeights,
                    tStatus );

            // close file
            tStatus = H5Fclose( tFileID );
        }

// -----------------------------------------------------------------------------

        void
        HMR::flag_elements(
                      Cell< mtk::Cell* > & aElements,
                const uint                 aPattern )
        {

            // get  working pattern
            uint tWorkingPattern = mParameters->get_working_pattern();

            // get pointer to background mesh
            Background_Mesh_Base * tBackgroundMesh = mDatabase->get_background_mesh();

            // check aPattern was set
            if( aPattern == MORIS_UINT_MAX )
            {
                // if default value is set, use output pattern
                tBackgroundMesh->set_activation_pattern( mParameters->get_input_pattern() );
            }
            else
            {
                // activate specified pattern on Background mesh
                tBackgroundMesh->set_activation_pattern( aPattern );
            }

            // loop over all active elements
            for( mtk::Cell* tCell :  aElements )
            {
                // get pointer to Background Element
                Background_Element_Base * tElement
                    = tBackgroundMesh->get_element( tCell->get_index() );

                // put this element on the list
                tElement->set_refined_flag( tWorkingPattern );

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

        void
        HMR::perform_refinement()
        {
            // refine database
            mDatabase->perform_refinement( ! mPerformRefinementCalled );

            // set flag for refinement
            mPerformRefinementCalled = true;
        }

// -----------------------------------------------------------------------------

        void
        HMR::update_refinement_pattern()
        {
            mDatabase->copy_pattern(
                    mParameters->get_output_pattern(),
                    mParameters->get_input_pattern() );

            // get number of bspline meshes
            uint tNumberOfBsplineMeshes = mDatabase->get_number_of_bspline_meshes();

            // update bspline meshes
            for( uint k=0; k<tNumberOfBsplineMeshes; ++k )
            {
                // get pointer to bspline mesh
                BSpline_Mesh_Base * tMesh = mDatabase->get_bspline_mesh_by_index( k );

                if( tMesh->get_activation_pattern() == mParameters->get_input_pattern() )
                {
                    tMesh->update_mesh();
                }
            }

            // get number of bspline meshes
            uint tNumberOfLagrangeMeshes = mDatabase->get_number_of_lagrange_meshes();

            // update lagrange meshes
            for( uint k=0; k<tNumberOfLagrangeMeshes; ++k )
            {
                // get pointer to bspline mesh
                Lagrange_Mesh_Base * tMesh = mDatabase->get_lagrange_mesh_by_index( k );

                if( tMesh->get_activation_pattern() == mParameters->get_input_pattern() )
                {
                    tMesh->update_mesh();
                }
            }

            // set flag that this function has been called
            mUpdateRefinementCalled = true;
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Mesh >
        HMR::create_mesh()
        {
            return std::make_shared< Mesh >( mDatabase,
                                        mParameters->get_max_polynomial(),
                                        mParameters->get_output_pattern() );
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Mesh >
        HMR::create_mesh( const uint & aPattern )
        {
            return std::make_shared< Mesh >( mDatabase,
                                        mParameters->get_max_polynomial(),
                                        aPattern );
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Field >
        HMR::create_field( const std::string & aLabel )
        {
            //return mInputMesh->create_field( aLabel );
            uint tFieldIndex = mFields.size();

            // add a new field to the list
            mFields.push_back( mInputMesh->create_field( aLabel ) );

            // return the pointer
            return mFields( tFieldIndex );
        }

// -----------------------------------------------------------------------------

        std::shared_ptr< Field>
        HMR::map_field_on_mesh(
                std::shared_ptr< Field > aField,
                std::shared_ptr< Mesh >  aMesh )
        {
            tic tTimer;

            // create pointer to output field
            auto aOutField = aMesh->create_field( aField->get_label() );

            // create a temporary union mesh
            Mesh * tUnionMesh = new Mesh( mDatabase,
                    mtk::interpolation_order_to_uint( aField->get_interpolation_order() ),
                    mParameters->get_union_pattern() );

            // create a temporary union field
            auto tUnionField = tUnionMesh->create_field( aField->get_label() );

            // interpolate input field to union
            mDatabase->interpolate_field(
                    mParameters->get_input_pattern(),
                    aField,
                    mParameters->get_union_pattern(),
                    tUnionField );

            // perform L2 projection :

            // create IWG object
            moris::fem::IWG_L2 tIWG;

            // create model
            mdl::Model tModel(
                    tUnionMesh,
                    tIWG,
                    tUnionField->get_node_values(),
                    aOutField->get_coefficients() );

            // delete the pointer to the union mesh
            delete tUnionMesh;

            // finally, the node values on the output mesh are evaluated
            aOutField->evaluate_node_values();

            // print output if verbose level is set
            if ( mParameters->is_verbose() )
            {
                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                std::fprintf( stdout,"%s Performed L2 projection.\n               Operation took %5.3f seconds.\n\n",
                        proc_string().c_str(),
                        ( double ) tElapsedTime / 1000 );
            }

            // return output mesh
            return aOutField;
        }

// -----------------------------------------------------------------------------

        uint
        HMR::flag_volume_and_surface_elements( const std::shared_ptr<Field> aScalarField )
        {
            // the funciton returns the number of flagged elements
            uint aElementCounter = 0;

            // create geometry engine
            gen::Geometry_Engine tRefMan;

            // candidates for refinement
            Cell< mtk::Cell* > tCandidates;

            // elements to be flagged for refinement
            Cell< mtk::Cell* > tRefinementList;

            // get candidates from volume
            this->get_candidates_for_volume_refinement( tCandidates );

            // call refinement manager and get volume cells
            tRefMan.find_cells_within_levelset(
                    tRefinementList,
                    tCandidates,
                    aScalarField );

            // add length of list to counter
            aElementCounter += tRefinementList.size();

            // flag elements in database
            this->flag_elements( tRefinementList );

            // get candidates for surface
            this->get_candidates_for_surface_refinement( tCandidates );


            // call refinement manager and get intersected cells
            tRefMan.find_cells_intersected_by_levelset(
                    tRefinementList,
                    tCandidates,
                    aScalarField );

            // add length of list to counter
            aElementCounter += tRefinementList.size();

            // flag elements in hmr
            this->flag_elements( tRefinementList );


            // return number of flagged elements
            return aElementCounter;
        }

// -----------------------------------------------------------------------------

        uint
        HMR::flag_surface_elements( const std::shared_ptr<Field> aScalarField )
        {
            // the funciton returns the number of flagged elements
            uint aElementCounter = 0;

            // create geometry engine
            gen::Geometry_Engine tRefMan;

            // candidates for refinement
            Cell< mtk::Cell* > tCandidates;

            // elements to be flagged for refinement
            Cell< mtk::Cell* > tRefinementList;

            // get candidates for surface
            this->get_candidates_for_surface_refinement( tCandidates );


            // call refinement manager and get intersected cells
            tRefMan.find_cells_intersected_by_levelset(
                    tRefinementList,
                    tCandidates,
                    aScalarField );

            // add length of list to counter
            aElementCounter += tRefinementList.size();

            // flag elements in hmr
            this->flag_elements( tRefinementList );


            // return number of flagged elements
            return aElementCounter;
        }

// -----------------------------------------------------------------------------

        void
        HMR::get_candidates_for_refinement(
                Cell< mtk::Cell* >   & aCandidates,
                const             uint aMaxLevel )
        {
            // reset candidate list
            aCandidates.clear();

            // make sure that input pattern is active
            mDatabase->set_activation_pattern( mParameters->get_input_pattern() );

            // get working pattern
            uint tWorkingPattern = mParameters->get_working_pattern();


            // get pointer to background mesh
            Background_Mesh_Base * tBackgroundMesh = mDatabase->get_background_mesh();

            // number of active elements
            uint tNumberOfElements
                = tBackgroundMesh->get_number_of_active_elements_on_proc();

            // allocate output list
            aCandidates.resize(
                    tNumberOfElements,
                    nullptr );

            // initialize counter
            uint tCount = 0;

            // pick first lagrange mesh on input pattern
            // fixme: add option to pick another one
            Lagrange_Mesh_Base * tMesh = mDatabase->get_lagrange_mesh_by_index( 0 );

            // make sure that this mesh uses correct pattern
            MORIS_ASSERT( tMesh->get_activation_pattern() ==  mParameters->get_input_pattern(),
                    "wrong pattern picked for get_candidates_for_refinement()");


            // loop over all elements
            for( uint e=0; e<tNumberOfElements; ++e )
            {
                // get pointer to background element
                Background_Element_Base * tElement
                    = tBackgroundMesh->get_element( e );

                // test if element is not flagged and below level
                if ( tElement->get_level() < aMaxLevel
                        && ! tElement->is_refined( tWorkingPattern ) )
                {
                    // add element to queue
                    aCandidates( tCount++ )
                                          = tMesh->get_element_by_memory_index( tElement->get_memory_index() );
                }
            }

            // shrink output cell to fit
            aCandidates.resize( tCount );
        }
// -----------------------------------------------------------------------------

        void
        HMR::get_candidates_for_volume_refinement( Cell< mtk::Cell* > & aCandidates )
        {
            this->get_candidates_for_refinement(
                    aCandidates, mParameters->get_max_volume_level() );
        }

// -----------------------------------------------------------------------------

        void
        HMR::get_candidates_for_surface_refinement( Cell< mtk::Cell* > & aCandidates )
        {
            this->get_candidates_for_refinement(
                    aCandidates, mParameters->get_max_surface_level() );
        }
// -----------------------------------------------------------------------------

        void
        HMR::save_background_mesh_to_vtk( const std::string & aFilePath )
        {
            mDatabase->get_background_mesh()->save_to_vtk( aFilePath );
        }

// -----------------------------------------------------------------------------

        void
        HMR::save_bsplines_to_vtk( const std::string & aFilePath )
        {
            for( uint k=0; k<mDatabase->get_number_of_lagrange_meshes(); ++k  )
            {
                // pick mesh
                if( mDatabase->get_lagrange_mesh_by_index( k )->get_activation_pattern()
                        == mParameters->get_output_pattern() )
                {
                    // dump mesh
                    mDatabase->get_lagrange_mesh_by_index( k )
                            ->get_bspline_mesh()->save_to_vtk( aFilePath );
                    break;
                }
            }
        }

// -----------------------------------------------------------------------------

        void
        HMR::save_faces_to_vtk( const std::string & aFilePath )
        {
            for( uint k=0; k<mDatabase->get_number_of_lagrange_meshes(); ++k  )
            {
                // pick mesh
                if( mDatabase->get_lagrange_mesh_by_index( k )->get_activation_pattern()
                        == mParameters->get_output_pattern() )
                {
                    // dump mesh
                    mDatabase->get_lagrange_mesh_by_index( k )->save_faces_to_vtk( aFilePath );

                    break;
                }
            }
        }

// -----------------------------------------------------------------------------

        void
        HMR::save_edges_to_vtk( const std::string & aFilePath )
        {
            if( mParameters->get_number_of_dimensions() == 3 )
            {
                for( uint k=0; k<mDatabase->get_number_of_lagrange_meshes(); ++k  )
                {
                    // pick mesh
                    if( mDatabase->get_lagrange_mesh_by_index( k )->get_activation_pattern()
                            == mParameters->get_output_pattern() )
                    {
                        // dump mesh
                        mDatabase->get_lagrange_mesh_by_index( k )->save_edges_to_vtk( aFilePath );

                        break;
                    }
                }
            }
            else
            {
                MORIS_ERROR( false, "save_edges_to_vtk() can only be called for 3D meshes");
            }
        }

// ----------------------------------------------------------------------------

        void
        HMR::save_mesh_to_vtk( const std::string & aFilePath )
        {
            for( uint k=0; k<mDatabase->get_number_of_lagrange_meshes(); ++k  )
            {
                // pick mesh
                if( mDatabase->get_lagrange_mesh_by_index( k )->get_activation_pattern()
                        == mParameters->get_output_pattern() )
                {
                    // dump mesh
                    mDatabase->get_lagrange_mesh_by_index( k )->save_to_vtk( aFilePath );

                    break;
                }
            }
        }

// ----------------------------------------------------------------------------

        void
        HMR::perform_refinement_and_map_fields()
        {
            // perform refinement
            this->perform_refinement();

            // calculate T-Matrices, edges and faces
            this->finalize();

            // get number of fields
            uint tNumberOfFields = mFields.size();

            // loop over all fields
            for( uint k=0; k<tNumberOfFields; ++k )
            {
                // perform projection
                auto tOutField = this->map_field_on_mesh(
                        mFields( k ), mOutputMesh );

                // point mesh in database to new data
                mFields( k )->change_mesh(
                        mOutputMesh->get_lagrange_mesh(),
                        tOutField->get_field_index() );
            }
        }

// ----------------------------------------------------------------------------

        std::shared_ptr< Field >
        HMR::load_field_from_hdf5_file( const std::string & aFilePath )
        {
            // make filename parallel
            std::string tFilePath = parallelize_path( aFilePath );


            // opens an existing file with read and write access
            hid_t tFileID = H5Fopen( tFilePath.c_str(),
                            H5F_ACC_RDWR,
                            H5P_DEFAULT );

            // error handler
            herr_t tStatus;

            // get label

            std::string tLabel;
            load_string_from_hdf5_file( tFileID, "Label", tLabel, tStatus );

            // get Lagrange order
            /*uint tLagrangeOrder;
            load_scalar_from_hdf5_file( tFileID, "LagrangeOrder", tLagrangeOrder, tStatus );

            // get BSpline order
            uint tBSplineOrder;
            load_scalar_from_hdf5_file( tFileID, "BSplineOrder", tBSplineOrder, tStatus );

            // get number of meshes
            uint tNumberOfMeshes = mDatabase->get_number_of_lagrange_meshes();

            // find correct mesh
            Lagrange_Mesh_Base * tMesh = nullptr;
            for( uint k=0; k<tNumberOfMeshes; ++k )
            {
                // get pointer to mesh
                tMesh = mDatabase->get_lagrange_mesh_by_index( k );

                if(        tMesh->get_activation_pattern() == mParameters->get_input_pattern()
                        && tMesh->get_order() == tLagrangeOrder
                        && tMesh->get_bspline_order() == tBSplineOrder )
                {
                    break;
                }
            } */

            //return mInputMesh->create_field( aLabel );
            uint tFieldIndex = mFields.size();

            // add a new field to the list
            mFields.push_back( mInputMesh->create_field( tLabel ) );

            // get a pointer to this field
            std::shared_ptr< Field > aField = mFields( tFieldIndex );

            // load node values
            load_matrix_from_hdf5_file(
                    tFileID,
                    "NodeValues",
                    aField->get_node_values(),
                    tStatus );

            // load bspline coefficients
            load_matrix_from_hdf5_file(
                    tFileID,
                    "BSplineCoefficients",
                    aField->get_coefficients(),
                    tStatus );

            // close file
            tStatus = H5Fclose( tFileID );

            // return the pointer
            return aField;

        }

// ----------------------------------------------------------------------------


    } /* namespace hmr */
} /* namespace moris */
