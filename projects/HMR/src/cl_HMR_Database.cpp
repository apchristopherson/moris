#include "cl_HMR_Mesh.hpp"
#include "cl_HMR_Database.hpp"
#include "cl_HMR_File.hpp"
#include "cl_HMR_Field.hpp"

#include "op_times.hpp"

namespace moris
{
    namespace hmr
    {
// -----------------------------------------------------------------------------

        Database::Database( Parameters * aParameters )
            : mParameters( aParameters )
        {
            // locl parameters ( number of elements per direction etc )
            aParameters->lock();

            // create factory object
            Factory tFactory;

            // create background mesh object
            mBackgroundMesh = tFactory.create_background_mesh( mParameters );

            // update element table
            mBackgroundMesh->collect_active_elements();

            // reset other patterns
            for( uint k=0; k<gNumberOfPatterns; ++ k )
            {
                mBackgroundMesh->reset_pattern( k );
            }

            // initialize mesh objects
            this->create_meshes();

            // initialize T-Matrix objects
            this->init_t_matrices();


        }

// -----------------------------------------------------------------------------

        Database::Database( const std::string & aPath ) :
                mParameters( create_hmr_parameters_from_hdf5_file( aPath ) )
        {


            // create factory
            Factory tFactory;

            // create background mesh object
            mBackgroundMesh = tFactory.create_background_mesh( mParameters );

            // reset all patterns
            for( uint k=0; k<gNumberOfPatterns; ++k )
            {
                mBackgroundMesh->reset_pattern( k );
            }

            this->load_pattern_from_hdf5_file(
                    mParameters->get_input_pattern(), aPath );

            // initialize mesh objects
            this->create_meshes();

            // initialize T-Matrix objects
            this->init_t_matrices();

            // activate input pattern
            this->set_activation_pattern( mParameters->get_input_pattern() );
        }

// -----------------------------------------------------------------------------

        Database::~Database()
        {
            // delete T-Matrix objects
            this->delete_t_matrices();

            // delete B-Spline and Lagrange meshes
            this->delete_meshes();

            // delete Background Mesh
            delete mBackgroundMesh;

            // delete parameters
            if ( mDeleteParametersOnDestruction )
            {
                delete mParameters;
            }
        }

// -----------------------------------------------------------------------------

        void
        Database::load_pattern_from_hdf5_file(
                const uint        & aPattern,
                const std::string & aPath )
        {
            // create file object
            File tHDF5;

            // open file on disk
            tHDF5.open( aPath );

            // load input pattern into file
            tHDF5.load_refinement_pattern( mBackgroundMesh, aPattern  );

            // close hdf5 file
            tHDF5.close();
        }

// -----------------------------------------------------------------------------

        /**
         * sets the flag that the parameter object must be deleted
         * by the destructor
         */
        void
        Database::set_parameter_owning_flag()
        {
            mDeleteParametersOnDestruction = true;
        }

// -----------------------------------------------------------------------------

        void
        Database::create_meshes()
        {
            // delete existing meshes
            this->delete_meshes();

            // create factory object
            Factory tFactory;

            // create BSpline meshes
            uint tNumberOfBSplineMeshes
            = mParameters->get_number_of_bspline_meshes();

            // assign memory for B-Spline meshes
            mBSplineMeshes.resize ( tNumberOfBSplineMeshes, nullptr );

            for( uint k=0; k<tNumberOfBSplineMeshes; ++k )
            {
                mBSplineMeshes( k ) = tFactory.create_bspline_mesh(
                        mParameters,
                        mBackgroundMesh,
                        mParameters->get_bspline_pattern( k ),
                        mParameters->get_bspline_order( k ) );

                mBSplineMeshes( k )->set_index( k );
            }

            // create Lagrange meshes
            uint tNumberOfLagrangeMeshes
                = mParameters->get_number_of_lagrange_meshes();

            // assign memory for Lagrange meshes
            mLagrangeMeshes.resize ( tNumberOfLagrangeMeshes, nullptr );

            for( uint k=0; k<tNumberOfLagrangeMeshes; ++k )
            {
                mLagrangeMeshes( k ) = tFactory.create_lagrange_mesh(
                        mParameters,
                        mBackgroundMesh,
                        mBSplineMeshes( mParameters->get_lagrange_to_bspline( k ) ),
                        mParameters->get_lagrange_pattern( k ),
                        mParameters->get_lagrange_order( k ) );

                mLagrangeMeshes( k )->set_index( k );
            }
        }

// -----------------------------------------------------------------------------


        void
        Database::delete_meshes()
        {

            // delete all pointers
            for( auto tMesh : mBSplineMeshes )
            {
                // delete this mesh
                delete tMesh;
            }

            // delete all pointers
            for( auto tMesh : mLagrangeMeshes )
            {
                // delete this mesh
                delete tMesh;
            }

        }

// -----------------------------------------------------------------------------

        void
        Database::init_t_matrices()
        {
            // get number of meshes
            uint tNumberOfMeshes
            = mParameters->get_number_of_lagrange_meshes();

            // allocate T-Matrix cell
            mTMatrix.resize( tNumberOfMeshes, nullptr );

            for( uint k=0; k<tNumberOfMeshes; ++k )
            {
                uint tBSplineMeshIndex
                = mParameters->get_lagrange_to_bspline( k );

                // test if both meshes exist
                if ( mBSplineMeshes( tBSplineMeshIndex ) != NULL
                        && mLagrangeMeshes( k ) != NULL )
                {
                    // initialize T-Matrix object
                    mTMatrix( k ) = new T_Matrix( mParameters,
                            mBSplineMeshes( tBSplineMeshIndex ),
                            mLagrangeMeshes( k ) );
                }
            }
        }

// -----------------------------------------------------------------------------

        void
        Database::delete_t_matrices()
        {
            // delete pointers of calculation objects
            for( auto tTMatrix :  mTMatrix )
            {
                if ( tTMatrix != NULL )
                {
                    delete tTMatrix;
                }
            }
        }

// -----------------------------------------------------------------------------

        T_Matrix *
        Database::get_t_matrix( const uint & aLagrangeMeshIndex )
        {
            return mTMatrix( aLagrangeMeshIndex );
        }


// -----------------------------------------------------------------------------

        void
        Database::update_meshes()
        {

            // remember active pattern
            auto tActivePattern = mBackgroundMesh->get_activation_pattern();

            // update all B-Spline meshes
            for( auto tMesh : mBSplineMeshes )
            {
                // synchronize mesh with background mesh
                tMesh->update_mesh();
            }

            // update all Lagrange meshes and link elements to their
            // B-Spline twins
            for( auto tMesh : mLagrangeMeshes )
            {
                // synchronize mesh with background mesh
                tMesh->update_mesh();
            }

            // reset pattern
            mBackgroundMesh->set_activation_pattern( tActivePattern );

            // update T-Matrices
            //this->finalize();
        }

// -----------------------------------------------------------------------------

        void
        Database::finalize()
        {

            // remember active pattern
            auto tActivePattern = mBackgroundMesh->get_activation_pattern();


            // get number of Lagrange meshes
            uint tNumberOfLagrangeMeshes = mLagrangeMeshes.size();

            // loop over all meshes
            for( uint l=0; l<tNumberOfLagrangeMeshes; ++l )
            {
                mTMatrix( l )->evaluate();
            }

            // create communication table
            this->create_communication_table();

            for( auto tMesh : mBSplineMeshes )
            {
                tMesh->calculate_basis_indices( mCommunicationTable );
            }

            if( mParameters->get_number_of_dimensions() == 3 )
            {
                mBackgroundMesh->create_faces_and_edges();
            }
            else
            {
                mBackgroundMesh->create_facets();
            }

            for( auto tMesh: mLagrangeMeshes )
            {
                tMesh->calculate_node_indices();


                // only needed for output mesh
                if( mParameters->get_output_pattern() == tMesh->get_activation_pattern() )
                {
                    // create facets
                    tMesh->create_facets();

                    // create edges
                    if( mParameters->get_number_of_dimensions() == 3 )
                    {
                        tMesh->create_edges();
                    }
                }

            }

            // reset active pattern
            if ( mBackgroundMesh->get_activation_pattern() != tActivePattern )
            {
                mBackgroundMesh->set_activation_pattern( tActivePattern );
            }

            this->check_entity_ids();
        }

// -----------------------------------------------------------------------------

        void
        Database::create_communication_table()
        {
            moris_id tParSize = par_size();
            moris_id tMyRank  = par_rank();

            if( tParSize > 1 )
            {
                // in a first step, we identify all processers this proc wants
                // to talk to

                // this is a Bool-like matrix
                Matrix< IdMat > tColumn( tParSize, 1, 0 );

                // test owners of B-Splines
                for( auto tMesh: mBSplineMeshes )
                {
                    // get number of active B-Splines
                    auto tNumberOfBSplines = tMesh->get_number_of_active_basis_on_proc();

                    // loop over all active basis on this mesh
                    for( uint k=0; k<tNumberOfBSplines; ++k )
                    {
                        // get pointer to basis
                        auto tBasis = tMesh->get_active_basis( k );

                        // test if flag of basis is set
                        if ( tBasis->is_flagged() )
                        {
                            // set flag for this proc
                            tColumn( tBasis->get_owner() ) = 1;
                        }
                    }
                }

                // remove self from row
                tColumn( tMyRank ) = 0;

                // communication table
                Matrix< IdMat > tCommTable;

                // matrices to send
                Cell< Matrix< IdMat > > tSend;

                // matrices to receive
                Cell< Matrix< IdMat > > tRecv;

                if( tMyRank != 0 )
                {
                    // create communication table with one entry
                    tCommTable.set_size( 1, 1, 0 );
                    tSend.resize( 1, tColumn );
                }
                else
                {
                    // create comm matrix
                    tCommTable.set_size( tParSize, 1, 0 );

                    // communicate with all other procs
                    for( moris_id k=1; k<tParSize; ++k )
                    {
                        tCommTable( k ) = k;
                    }

                    // nothing to send
                    Matrix< IdMat > tEmpty;
                    tSend.resize( tParSize, tEmpty );
                }

                // exchange matrices
                communicate_mats( tCommTable, tSend, tRecv );

                // process information on master proc
                if ( tMyRank == 0 )
                {
                    // create communication matrix
                    Matrix< IdMat > tCommMatrix( tParSize, tParSize, 0 );

                    // process first row
                    tRecv( 0 ) = tColumn;

                    // loop over all procs and create comm matrix
                    for( moris_id j=0; j<tParSize; ++j )
                    {
                        for( moris_id i=0; i<tParSize; ++i )
                        {
                            if ( tRecv( j )( i, 0 ) != 0 )
                            {
                                tCommMatrix( i, j ) = 1;
                                tCommMatrix( j, i ) = 1;
                            }
                        }
                    }

                    // remove diagonal
                    for( moris_id i=0; i<tParSize; ++i )
                    {
                        tCommMatrix( i, i ) = 0;
                    }

                    // create sending list
                    Matrix< IdMat > tEmpty;
                    tSend.resize( tParSize, tEmpty );

                    for( moris_id j=0; j<tParSize; ++j )
                    {
                        // count nonzero entries
                        uint tCount = 0;
                        for( moris_id i=0; i<tParSize; ++i )
                        {
                            if ( tCommMatrix( i, j ) != 0 )
                            {
                                ++tCount;
                            }
                        }

                        // assign memory
                        tSend( j ).set_size( tCount, 1, 0 );

                        // reset counter
                        tCount = 0;

                        // write values into matrix
                        for( moris_id i=0; i<tParSize; ++i )
                        {
                            if ( tCommMatrix( i, j ) != 0 )
                            {
                                tSend( j )( tCount++ ) = i;
                            }
                        }
                    }
                }

                // exchange matrices with other procs
                communicate_mats( tCommTable, tSend, tRecv );

                // write data into communication table
                if ( tMyRank == 0 )
                {
                    mCommunicationTable = tSend( 0 );
                }
                else
                {
                    mCommunicationTable = tRecv( 0 );
                }
            }
            else // if run in serial
            {
                // communication table is empty
                mCommunicationTable.set_size( 0, 1 );
            }
        }

// -----------------------------------------------------------------------------

        Background_Mesh_Base *
        Database::get_background_mesh()
        {
            return mBackgroundMesh;
        }

// -----------------------------------------------------------------------------
        /**
         * creates a union of two patterns
         */
        void
        Database::unite_patterns(
                const uint & aSourceA,
                const uint & aSourceB,
                const uint & aTarget )
        {
            tic tTimer;

            mBackgroundMesh->unite_patterns(
                    aSourceA,
                    aSourceB,
                    aTarget );

            // create output messahe
            if ( mParameters->is_verbose() )
            {
                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                // print output
                std::fprintf( stdout,"%s United patterns %lu and %lu to %lu.\n               Calculation took %5.3f seconds.\n\n",
                        proc_string().c_str(),
                        ( long unsigned int ) aSourceA,
                        ( long unsigned int ) aSourceB,
                        ( long unsigned int ) aTarget,
                        ( double ) tElapsedTime / 1000 );

            }
        }

// -----------------------------------------------------------------------------

        /**
         * copies a source pattern to a target pattern
         */
        void
        Database::copy_pattern(
                const uint & aSource,
                const uint & aTarget )
        {
            tic tTimer;

            mBackgroundMesh->copy_pattern(
                    aSource,
                    aTarget );

            // this->update_meshes();

            // create output messahe
            if ( mParameters->is_verbose() )
            {
                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                // print output

                std::fprintf( stdout,"%s Copied pattern %lu to %lu.\n               Calculation took %5.3f seconds.\n\n",
                        proc_string().c_str(),
                        ( long unsigned int ) aSource,
                        ( long unsigned int ) aTarget,
                        ( double ) tElapsedTime / 1000 );
            }
        }

// -----------------------------------------------------------------------------

        void
        Database::add_extra_refinement_step_for_exodus()
        {
            // get refined pattern
            auto tPattern = mParameters->get_refined_output_pattern();

            // create refined pattern
            mBackgroundMesh->copy_pattern(
                    mParameters->get_output_pattern(),
                    tPattern );

            // activate output pattern
            mBackgroundMesh->set_activation_pattern( tPattern );

            // collect all active elements on background mesh
            mBackgroundMesh->collect_active_elements();

            // get number of elements
            luint tNumberOfElements = mBackgroundMesh->get_number_of_active_elements_on_proc();

            // flag all active elements
            for( luint e=0; e<tNumberOfElements; ++e )
            {
                mBackgroundMesh->get_element( e )->put_on_refinement_queue();
            }

            // perform refinement
            mBackgroundMesh->perform_refinement();
        }

// -----------------------------------------------------------------------------

        void
        Database::perform_refinement( const bool aResetPattern )
        {

            // get pointer to working pattern
            uint tWorkingPattern = mParameters->get_working_pattern();

            // this function resets the output pattern
            if ( aResetPattern )
            {
                mBackgroundMesh->reset_pattern( mParameters->get_output_pattern() );
            }

            // activate input pattern
            mBackgroundMesh->set_activation_pattern( mParameters->get_output_pattern() );

            // get max level on this mesh
            uint tMaxLevel = mBackgroundMesh->get_max_level();

            // loop over all levels
            for( uint l=0; l<= tMaxLevel; ++l )
            {
                // container for elements on this level
                Cell< Background_Element_Base* > tElementList;

                // collect all elements on this level ( without aura )
                mBackgroundMesh
                    ->collect_elements_on_level( l, tElementList );

                // loop over all elements and flag them for refinement
                for( Background_Element_Base* tElement : tElementList )
                {
                    // test if element is marked on working pattern
                    if ( tElement->is_refined( tWorkingPattern ) )
                    {
                        // flag this element
                        tElement->put_on_refinement_queue();
                    }
                }
                mBackgroundMesh->perform_refinement();
            }


            // #MINREF check for minimum refinement criterion
            while ( mBackgroundMesh->collect_refinement_queue() )
            {
                mBackgroundMesh->perform_refinement();
            }

            // update max level
            tMaxLevel = mBackgroundMesh->get_max_level();

            // tidy up element flags
            for( uint l=0; l<= tMaxLevel; ++l )
            {
                // container for elements on this level
                Cell< Background_Element_Base* > tElementList;

                // collect all elements on this level ( with aura )
                mBackgroundMesh
                ->collect_elements_on_level_including_aura( l, tElementList );

            }

            // tidy up working pattern
            mBackgroundMesh->reset_pattern( tWorkingPattern );

            // test if max polynomial is 3
            if ( mParameters->get_max_polynomial() > 2 )
            {
                // activate extra pattern for exodus
                this->add_extra_refinement_step_for_exodus();
            }

            // create union of input and output
            this->create_union_pattern();

            // update meshes according to new refinement patterns
            this->update_meshes();
        }

// -----------------------------------------------------------------------------

        void
        Database::interpolate_field(
                const uint                   & aSourcePattern,
                const std::shared_ptr<Field>   aSource,
                const uint                   & aTargetPattern,
                std::shared_ptr<Field>         aTarget )
        {
            // make sure that mesh orders match
            MORIS_ERROR(
                    aSource->get_interpolation_order()
                    == aTarget->get_interpolation_order(),
                    "Source and Target Field must have same interpolation order" );

            // make sure that both fields are scalar or of equal dimension
            MORIS_ERROR( aSource->get_number_of_dimensions() == aTarget->get_number_of_dimensions(),
                    "Source and Target Field must have same dimension" );


            // get interpolation order

            uint tOrder = mtk::interpolation_order_to_uint(  aSource->get_interpolation_order() );

            // pointer to mesh that is linked to input field
            Lagrange_Mesh_Base * tSourceMesh = nullptr;

            // find pointer to input mesh
            for( Lagrange_Mesh_Base *  tMesh : mLagrangeMeshes )
            {
                if (   tMesh->get_order() == tOrder
                        && tMesh->get_activation_pattern() == aSourcePattern )
                {
                    tSourceMesh = tMesh;
                    break;
                }
            }

            // pointer to mesh that is linked to output field
            Lagrange_Mesh_Base * tTargetMesh = nullptr;

            // find pointer to output mesh
            for( Lagrange_Mesh_Base *  tMesh : mLagrangeMeshes )
            {
                if (   tMesh->get_order() == tOrder
                        && tMesh->get_activation_pattern() == aTargetPattern )
                {
                    tTargetMesh = tMesh;
                    break;
                }
            }

            tTargetMesh->select_activation_pattern();

            // unflag nodes on target
            tTargetMesh->unflag_all_basis();

            // number of elements on target mesh
            auto tNumberOfElements = tTargetMesh->get_number_of_elements();

            // number of nodes per element
            auto tNumberOfNodesPerElement = tTargetMesh->get_number_of_basis_per_element();

            // create unity matrix
            Matrix< DDRMat > tEye( tNumberOfNodesPerElement, tNumberOfNodesPerElement, 0.0 );
            for( uint k=0; k<tNumberOfNodesPerElement; ++k )
            {
                tEye( k, k ) = 1.0;
            }

            // get values of source field
            const Matrix< DDRMat > & tSourceData = aSource->get_node_values();

            // get target data
            Matrix< DDRMat > & tTargetData = aTarget->get_node_values();

            // allocate value matrix
            tTargetData.set_size( tTargetMesh->get_number_of_all_basis_on_proc(), aTarget->get_number_of_dimensions() );

            // containers for source and target data
            Matrix< DDRMat > tElementSourceData( tNumberOfNodesPerElement, aSource->get_number_of_dimensions() );

            // target mesh index
            auto tTargetMeshIndex = tTargetMesh->get_index();

            // loop over all elements
            for( luint e=0; e<tNumberOfElements; ++e )
            {
                // get pointer to target element
                auto tTargetElement = tTargetMesh->get_element( e );

                // get backgrund element
                auto tBackgroundElement = tTargetElement->get_background_element();

                // initialize refinement Matrix
                Matrix< DDRMat > tR( tEye );

                while( ! tBackgroundElement->is_active( aSourcePattern ) )
                {
                    // right multiply refinement matrix
                    tR = tR.matrix_data() * mTMatrix( tTargetMeshIndex )->get_refinement_matrix(
                            tBackgroundElement->get_child_index() ).matrix_data();

                    // jump to parent
                    tBackgroundElement = tBackgroundElement->get_parent();
                }

                // get pointer to source element
                auto tSourceElement = tSourceMesh->get_element_by_memory_index(
                        tBackgroundElement->get_memory_index() );

                // fill source data vector
                for( uint k=0; k<tNumberOfNodesPerElement; ++k )
                {
                    // get pointer to source node
                    auto tNode = tSourceElement->get_basis( k );
                    auto tIndex = tNode->get_index();

                    // copy data from source mesh
                    tElementSourceData.set_row( k, tSourceData.get_row( tIndex ) );
                }

                // copy target data to target mesh
                for( uint k=0; k<tNumberOfNodesPerElement; ++k )
                {
                    // get pointer to target node
                    auto tNode = tTargetElement->get_basis( k );

                    // test if data has already been written to target
                    if ( ! tNode->is_flagged() )
                    {
                        // get node indes
                        auto tIndex = tNode->get_index();

                        tTargetData.set_row( tIndex, tR.get_row( k ) * tElementSourceData );

                        // flag this node
                        tNode->flag();
                    }
                }
            }
        }

// -----------------------------------------------------------------------------

        void
        Database::check_entity_ids()
        {

            if( par_size() > 1 )
            {

                tic tTimer;

                uint tCount = 0;

                // loop over all Lagrange meshes
                for( Lagrange_Mesh_Base* tMesh : mLagrangeMeshes )
                {


                    tMesh->select_activation_pattern();

                    // check elements
                    uint tNumberOfEntities = tMesh->get_number_of_elements();

                    moris_id tMaxID = tMesh->get_max_element_id();

                    for( uint k=0; k<tNumberOfEntities; ++k )
                    {
                        moris_id tID = tMesh->get_element( k )->get_id() ;

                        MORIS_ERROR( 0 < tID && tID <=tMaxID,
                                "Invalid Element ID" );
                    }


                    if( tMesh->get_activation_pattern() == mParameters->get_output_pattern() )
                    {


                        // check facets
                        tNumberOfEntities = tMesh->get_number_of_facets();

                        tMaxID = tMesh->get_max_facet_id();

                        for( uint k=0; k<tNumberOfEntities; ++k )
                        {
                            moris_id tID = tMesh->get_facet( k )->get_id();
                            MORIS_ERROR( 0 < tID && tID <= tMaxID, "Invalid Facet ID" );
                        }

                        // check edges
                        if( mParameters->get_number_of_dimensions() == 3 )
                        {
                            std::cout << "#EdgeCheck: Writing VTK debug data ..." << std::endl;
                            tMesh->save_edges_to_vtk("Edges.vtk");
                            tMesh->save_to_vtk("Lagrange.vtk");
                            mBackgroundMesh->save_to_vtk("BackgroundMesh.vtk");

                            tNumberOfEntities = tMesh->get_number_of_edges();
                            tMaxID = tMesh->get_max_edge_id();

                            for( uint k=0; k<tNumberOfEntities; ++k )
                            {
                                moris_id tID = tMesh->get_edge( k )->get_id();

                                MORIS_ERROR( 0 < tID && tID <= tMaxID, "Invalid Edge ID" );
                            }
                        }
                    }
                    tNumberOfEntities = tMesh->get_number_of_nodes_on_proc();

                    tMaxID = tMesh->get_max_node_id();

                    for( uint k=0; k<tNumberOfEntities; ++k )
                    {
                        moris_id tID = tMesh->get_node_by_index( k )->get_id();

                        MORIS_ERROR( 0 < tID && tID <= tMaxID, "Invalid Node ID" );

                    }

                    if( tMesh->get_activation_pattern() == mParameters->get_output_pattern() )
                    {
                        for( uint k=0; k<tNumberOfEntities; ++k )
                        {

                            Matrix< IdMat > tIDs = tMesh->
                                    get_node_by_index( k )->get_interpolation()->get_ids();


                            MORIS_ERROR( tIDs.min() > 0 && tIDs.max() < INT_MAX,
                                    "Invalid B-Spline ID" );

                        }
                    }

                    ++tCount;
               }

                // loop over all B-Spline meshes
                for( BSpline_Mesh_Base* tMesh : mBSplineMeshes )
                {
                    tMesh->select_activation_pattern();

                    // get number of splines
                    uint tNumberOfEntities = tMesh->get_number_of_active_basis_on_proc();

                    for( uint k=0; k<tNumberOfEntities; ++k )
                    {
                        if( tMesh->get_active_basis( k )->is_flagged() )
                        {
                            MORIS_ERROR(
                                tMesh->get_active_basis( k )->get_domain_index() < gNoEntityID,
                                "Invalid B-Spline ID" );
                        }
                    }
                }

                if( mParameters->is_verbose() )
                {

                    // stop timer
                    real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                    // print output
                    std::fprintf( stdout,"%s passed entity ID test, testing took %5.3f seconds.\n\n",
                            proc_string().c_str(),
                            ( double ) tElapsedTime / 1000 );
                }
            }
        }
    } /* namespace hmr */
} /* namespace moris */
