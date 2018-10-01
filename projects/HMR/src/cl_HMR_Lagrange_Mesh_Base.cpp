#include <cstdio>
#include <fstream>

#include "cl_Stopwatch.hpp" //CHR/src

#include "typedefs.hpp"
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"

#include "fn_save_matrix_to_binary_file.hpp"

#include "HMR_Tools.hpp"

#include "cl_HMR_Lagrange_Mesh_Base.hpp" //HMR/src
#include "cl_HMR_Background_Facet.hpp"
#include "cl_HMR_Background_Edge.hpp"
#include "cl_HMR_Facet.hpp"

namespace moris
{
    namespace hmr
    {
//------------------------------------------------------------------------------
//   public:
//------------------------------------------------------------------------------

        Lagrange_Mesh_Base::Lagrange_Mesh_Base (
                const Parameters     * aParameters,
                Background_Mesh_Base * aBackgroundMesh,
                BSpline_Mesh_Base    * aBSplineMesh,
                const uint           & aOrder,
                const uint           & aActivationPattern ) :
                        Mesh_Base(
                                aParameters,
                                aBackgroundMesh,
                                aOrder,
                                aActivationPattern ),
                         mBSplineMesh( aBSplineMesh )

        {
            // sanity check
            if ( aBSplineMesh != NULL )
            {
                MORIS_ERROR( aBSplineMesh->get_order() >= aOrder,
                        "Error while creating Lagrange mesh. Linked B-Spline mesh must have same or higher order.");
            }

            this->reset_fields();
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::update_mesh()
        {
            // start timer
            tic tTimer;

            // activate pattern on background mesh
            this->select_activation_pattern();

            // tidy up memory
            this->delete_pointers();
            this->delete_facets();

            // create Lagrange Elements from Background Elements
            this->create_elements();

            // create nodes
            this->create_nodes();

            // update list of used nodes
            this->update_node_list();

            // update element indices
            this->update_element_indices();

            // link elements to B-Spline meshes
            if ( mBSplineMesh != NULL )
            {
                this->link_twins();
            }

            // print a debug statement if verbosity is set
            if ( mParameters->is_verbose() )
            {
                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                // print output
                std::fprintf( stdout,"%s Created Lagrange mesh.\n               Mesh has %lu active and refined elements and %lu nodes.\n               Creation took %5.3f seconds.\n\n",
                        proc_string().c_str(),
                        ( long unsigned int ) this->get_number_of_elements(),
                        ( long unsigned int ) this->get_number_of_nodes_on_proc(),
                        ( double ) tElapsedTime / 1000 );
            }
        }

// ----------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::save_to_file( const std::string & aFilePath )
        {
            // get the file extension
            auto tFileExt = aFilePath.substr(aFilePath.find_last_of(".")+1,
                                             aFilePath.length() );

            // guess routine from extension
            if (tFileExt == "vtk")
            {
                this->save_to_vtk( aFilePath );
            }
            else if(tFileExt == "msh")
            {
                this->save_to_gmsh( aFilePath );
            }
            else
            {
                MORIS_ERROR( false,
                        "Wrong file type passed to Lagrange mesh.\n If you want to save an exodus file, create an MTK object first." );
            }
        }

//------------------------------------------------------------------------------

        uint
        Lagrange_Mesh_Base::create_field_data( const std::string & aLabel )
        {
            MORIS_ERROR( mFieldData.size() == mFieldLabels.size() ,
                    "Sizes of Field labels and Data container does not match " );

            // first field is always element mesh
            mFieldLabels.push_back( aLabel );


            uint aIndex = mFieldData.size();

            // initialize empty matrix. It is populated later
            Matrix< DDRMat > tEmpty;
            mFieldData.push_back( tEmpty );

            return aIndex;
        }

//------------------------------------------------------------------------------
//   protected:
// -----------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::create_nodes_on_higher_levels()
        {
            // get max level of mest
            uint tMaxLevel = mBackgroundMesh->get_max_level();

            // loop over all levels
            for ( uint l=0; l<tMaxLevel; ++l )
            {
                // get all elements from this level
                Cell< Background_Element_Base* > tElements;

                mBackgroundMesh->collect_elements_on_level_including_aura(
                        l, tElements );

                // loop over all elements on this level
                for( auto tElement : tElements )
                {

                    // test if this element has children and is not padding
                    // and is refined
                    if ( tElement->has_children() && ! tElement->is_padding() &&
                            tElement->is_refined( mActivationPattern ) )
                    {
                        // calculate nodes of children
                        mAllElementsOnProc( tElement->get_memory_index() )
                                ->create_basis_for_children(
                                        mAllElementsOnProc,
                                        mNumberOfAllBasis );
                    }
                }
            }
        }

//------------------------------------------------------------------------------
//   private:
//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::create_nodes()
        {
            // nodes on first level are created separately
            this->create_nodes_on_level_zero();

            // it is easier to create all zero level nodes first
            // and remove obsolete ones. Don't worry, there are not that many.
            this->delete_nodes_on_padding_elements();

            // now we do the nodes on higher levels
            this->create_nodes_on_higher_levels();

            // fill cell with all remaining nodes
            this->collect_nodes();

            // calculate system wide IDs (helpful for debugging)
            this->calculate_node_ids();

            // find out node to element connectivity
            this->determine_elements_connected_to_basis();

            // and determine the node ownership to be the smallest proc ID
            // of all elements connected to this node
            this->guess_basis_ownership();

            // Make sure that node ownership is correct. Correct otherwise.
            this->confirm_basis_ownership();

            // calculate node coordinates with respect to user defined offset
            this->calculate_node_coordinates();

            // create node numbers
            this->calculate_node_indices();


        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::delete_nodes_on_padding_elements()
        {
            // get number of elements on coarsest level
            auto tNumberOfElements = mAllCoarsestElementsOnProc.size();

            // counter for nodes
            luint tCount = 0;

            // loop over all elements on coarsest level
            for( luint e=0; e<tNumberOfElements; ++e)
            {
                // get pointer to Lagrange element
                Element* tElement
                    = mAllCoarsestElementsOnProc( e );

                // test if element is not padding
                if ( ! tElement->is_padding() )
                {
                    // loop over all nodes of element
                    for( uint k=0; k<mNumberOfBasisPerElement; ++k )
                    {
                        // get pointer to node
                        Basis* tNode = tElement->get_basis( k );

                        if ( ! tNode->is_flagged() )
                        {
                            // set basis as active
                            tNode->flag();

                            // increment counter
                            ++tCount;
                        }
                    }
                }
            }

            // ask background mesh for number of elements per direction
            Matrix< DDLUMat > tNumberOfElementsPerDirection =
                mBackgroundMesh->get_number_of_elements_per_direction_on_proc();

            // assign Cell for nodes to be deleted
            Cell< Basis* > tNodes( mNumberOfAllBasis-tCount, nullptr );

            // reset counter
            tCount = 0;

            // loop over all elements on coarsest level
            for( luint e=0; e<tNumberOfElements; ++e)
            {
                // get pointer to Lagrange element
                Element* tElement
                    = mAllCoarsestElementsOnProc( e );

                // test if element is padding
                if ( tElement->is_padding() )
                {
                    // loop over all nodes of element
                    for( uint k=0; k<mNumberOfBasisPerElement; ++k )
                    {
                        // get pointer to node
                        Basis* tNode = tElement->get_basis( k );

                        // test if node exists
                        if ( tNode != NULL )
                        {
                            // test if node is not flagged
                            if ( ! tNode->is_flagged() )
                            {
                                // flag node
                                tNode->flag();

                                // copy node to delete list
                                tNodes( tCount++ ) = tNode;
                            }
                        }
                    }
                }

            }

            // delete nodes
            for ( uint k=0; k<tCount; ++k )
            {
                delete tNodes( k );

                // decrement number of nodes
                --mNumberOfAllBasis;
            }

            // tidy up: unflag all remaining nodes
            // loop over all elements on coarsest level
            for( luint e=0; e<tNumberOfElements; ++e)
            {
                // get pointer to Lagrange element
                Element* tElement
                = mAllCoarsestElementsOnProc( e );

                // test if element is not padding
                if ( ! tElement->is_padding() )
                {
                    // loop over all nodes of element
                    for( uint k=0; k<mNumberOfBasisPerElement; ++k )
                    {
                        // get pointer to node
                        Basis* tNode = tElement->get_basis( k );

                        // unset flag
                        tNode->unflag();
                    }
                }
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::collect_nodes()
        {
            // clear node list
            mAllBasisOnProc.clear();

            // reserve size
            mAllBasisOnProc.resize( mNumberOfAllBasis, nullptr );

            // initialize counter
            luint tCount = 0;

            // get number of active elements on proc
            luint tNumberOfElements = mBackgroundMesh
                ->get_number_of_active_elements_on_proc_including_aura();

            // get rank
            moris_id tMyRank = par_rank();

            // reset element counter
            mNumberOfElements = 0;

            // loop over all active elements on proc
            for ( luint e=0; e<tNumberOfElements; ++e )
            {
                // get pointer to background element
                Background_Element_Base* tBackElement = mBackgroundMesh
                        ->get_element_from_proc_domain_including_aura( e );

                // get pointer to Lagrange element
                Element* tElement
                    = mAllElementsOnProc( tBackElement->get_memory_index() );

                if ( ! tBackElement->is_deactive( mActivationPattern )  )
                {
                    // flag nodes that are used by this proc
                    if ( tBackElement->get_owner() == tMyRank )

                    {
                        for ( uint k=0; k<mNumberOfBasisPerElement; ++k )
                        {
                            tElement->get_basis( k )->use();
                        }

                        // increment element counter
                        ++mNumberOfElements;
                    }

                    // loop over all nodes of this element
                    for ( uint k=0; k<mNumberOfBasisPerElement; ++k )
                    {
                        // get pointer to node
                        Basis* tNode = tElement->get_basis( k );

                        // test if node is flagged
                        if ( ! tNode->is_flagged() )
                        {
                            // set index in memory
                            tNode->set_memory_index( tCount );

                            // add node to list
                            mAllBasisOnProc( tCount ++ ) = tNode;

                            // flag node
                            tNode->flag();
                        }
                    }
                }
            }
            // make sure that number of nodes is correct
            MORIS_ERROR( tCount == mNumberOfAllBasis, "Number of Nodes does not match." );
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::calculate_node_ids()
        {

            switch ( mParameters->get_number_of_dimensions() )
            {
                case( 1 ):
                {
                    for( auto tNode  : mAllBasisOnProc )
                    {
                        // get ij position of node
                        const luint * tI = tNode->get_ijk();

                        // calculate ID and write to node
                        tNode->set_domain_id(
                                this->calculate_node_id(
                                        tNode->get_level(),
                                        tI[0] ) );
                    }

                    break;
                }
                case( 2 ):
                {
                    for( auto tNode  : mAllBasisOnProc )
                    {
                        // get ij position of node
                        const luint * tIJ = tNode->get_ijk();

                        // calculate ID and write to node
                        tNode->set_domain_id(
                                this->calculate_node_id(
                                        tNode->get_level(),
                                        tIJ[0],
                                        tIJ[1]) );
                    }

                    break;
                }
                case( 3 ):
                {
                    // 3D case
                    for( auto tNode  : mAllBasisOnProc )
                    {
                        // get ij position of node
                        const luint * tIJK = tNode->get_ijk();

                        // calculate ID and write to node
                        tNode->set_domain_id(
                                this->calculate_node_id(
                                        tNode->get_level(),
                                        tIJK[0],
                                        tIJK[1],
                                        tIJK[2]) );
                    }

                    break;
                }
                default:
                {
                    MORIS_ERROR( false,
                            "Lagrange_Mesh: Invalid number of dimensions");
                    break;
                }
            }
        }

//------------------------------------------------------------------------------
        /**
         * calculates system wide unique node indices for MTK
         */
        void
        Lagrange_Mesh_Base::calculate_node_indices()
        {
            // reset node counters
            mNumberOfUsedAndOwnedNodes = 0;
            mNumberOfUsedNodes = 0;

            // get number of ranks
            moris_id tNumberOfProcs = par_size();

            // initialize local index of node
            // reset counter


            if( tNumberOfProcs == 1 ) // serial mode
            {
                for( auto tNode : mAllBasisOnProc )
                {
                    // test if node is used by current setup
                    if ( tNode->is_used() )
                    {
                        // in serial local index and domain index are identical
                        tNode->set_domain_index( mNumberOfUsedAndOwnedNodes++ );
                        tNode->set_local_index( mNumberOfUsedNodes++ );
                    }
                }
            }
            else // parallel mode
            {

                // get my rank
                moris_id tMyRank = par_rank();

                for( auto tNode : mAllBasisOnProc )
                {
                    // test if node is used by current setup
                    if ( tNode->is_used() )
                    {
                        // test if node is owned
                        if ( tNode->get_owner() == tMyRank )
                        {
                            tNode->set_domain_index( mNumberOfUsedAndOwnedNodes++ );
                        }

                        // set local index of node
                        tNode->set_local_index( mNumberOfUsedNodes++ );
                    }

                    // make sure that this basis is not flagged
                    tNode->unflag();
                }

                // communicate number of owned nodes with other procs
                Matrix< DDLUMat > tNodesOwnedPerProc;
                comm_gather_and_broadcast( mNumberOfUsedAndOwnedNodes, tNodesOwnedPerProc );

                // get proc neighbors from background mesh
                auto tProcNeighbors = mBackgroundMesh->get_proc_neigbors();

                // calculate node offset table
                Matrix< DDLUMat > tNodeOffset( tNumberOfProcs, 1, 0 );
                for( moris_id p=1; p<tNumberOfProcs; ++p )
                {
                    tNodeOffset( p ) =   tNodeOffset( p-1 )
                                       + tNodesOwnedPerProc( p-1 );
                }

                // remember for MTK output
                mMaxNodeDomainIndex = tNodeOffset( tNumberOfProcs-1 )
                                    + tNodesOwnedPerProc( tNumberOfProcs-1 );

                // get my offset

                luint tMyOffset = tNodeOffset( tMyRank );

                // loop over all nodes on proc
                for( auto tNode : mAllBasisOnProc )
                {
                    // test if the is used and node belongs to me
                    if ( tNode->is_used() )
                    {
                        if ( tNode->get_owner() == tMyRank )
                        {
                            // set global node index
                            tNode->set_domain_index(
                                    tNode->get_domain_index()
                                    + tMyOffset );
                        }
                    }
                }

                // now the global node indices of used and owned nodes
                // must be communicated to the other procs

                // get number of proc neighbors
                uint tNumberOfProcNeighbors
                    = mBackgroundMesh->get_number_of_proc_neighbors();

                // create cell of matrices to send
                Matrix< DDLUMat > tEmpty;
                Cell< Matrix< DDLUMat > > tSendIndex( tNumberOfProcNeighbors, tEmpty );

                // loop over all proc neighbors
                for ( uint p = 0; p<tNumberOfProcNeighbors; ++p )
                {
                    if (    tProcNeighbors( p ) < tNumberOfProcs
                         && tProcNeighbors( p ) != tMyRank )
                    {
                        // cell containing node pointers
                        Cell< Basis* > tNodes;

                        // collect nodes within inverse aura
                        this->collect_basis_from_aura( p, 1, tNodes );

                        // initialize node counter
                        luint tCount = 0;

                        // loop over all nodes
                        for( auto tNode : tNodes )
                        {
                            // test if node belongs to me
                            if (  tNode->get_owner() == tMyRank )
                            {
                                // increment counter
                                ++tCount;
                            }
                        }

                        // assign memory for send matrix
                        tSendIndex( p ).set_size( tCount, 1 );

                        // reset counter
                        tCount = 0;

                        // loop over all nodes
                        for( auto tNode : tNodes )
                        {
                            // test if node belongs to me
                            if ( tNode->get_owner() == tMyRank )
                            {
                                // write index of node into array
                                tSendIndex( p )( tCount++ ) = tNode->get_domain_index();
                            }
                        }
                    } // end proc exists and is not me
                } // end loop over all procs

                // matrices to receive
                Cell< Matrix< DDLUMat > > tReceiveIndex;

                // communicate ownership to neighbors
                communicate_mats(
                        tProcNeighbors,
                        tSendIndex,
                        tReceiveIndex );

                // loop over all proc neighbors
                for ( uint p = 0; p<tNumberOfProcNeighbors; ++p )
                {
                    // get rank of neighbor
                    auto tNeighborRank = tProcNeighbors( p );

                    if (    tNeighborRank < tNumberOfProcs
                            && tNeighborRank != tMyRank )
                    {
                        // cell containing node pointers
                        Cell< Basis* > tNodes;

                        // collect nodes within aura
                        this->collect_basis_from_aura( p, 0, tNodes );

                        // initialize node counter
                        luint tCount = 0;

                        // loop over all nodes
                        for( auto tNode : tNodes )
                        {
                            // test if this node belongs to neighbor
                            if ( tNode->get_owner() == tNeighborRank )
                            {
                                // assign index to node
                                tNode->set_domain_index(
                                        tReceiveIndex( p )( tCount++ ) );
                            }
                        }

                    }
                } // end loop over all procs
            } // end parallel
        }

//------------------------------------------------------------------------------

        Element *
        Lagrange_Mesh_Base::get_child( Element * aElement,
                const uint            & aChildIndex )
        {
            // get pointer to background element
            Background_Element_Base* tBackElement = aElement->get_background_element();

            if ( tBackElement->has_children() )
            {
                // get child of background element
                Background_Element_Base* tBackChild = tBackElement
                        ->get_child( aChildIndex );

                // grab child from element list
                return mAllElementsOnProc( tBackChild->get_memory_index() );
            }
            else
            {
                // return nothing
                return nullptr;
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::reset_fields()
        {
            mFieldLabels.clear();
            mFieldData.clear();

            // first field is always element level
            mFieldLabels.push_back("Element_Level");

            // second field is always element owner
            mFieldLabels.push_back("Element_Owner");

            // third field is always vertex IDs
            mFieldLabels.push_back("Vertex_IDs");

            // initialize empty matrix. It is populated later
            Matrix< DDRMat > tEmpty;
            for( uint k=0; k<3; ++k )
            {
                mFieldData.push_back( tEmpty );
            }
        }

//------------------------------------------------------------------------------

        /* void
        Lagrange_Mesh_Base::add_field( const std::string & aLabel,
                                       const Matrix< DDRMat > & aData )
        {
            mFieldLabels.push_back( aLabel );
            mFieldData.push_back( aData );
        } */

//------------------------------------------------------------------------------

        STK *
        Lagrange_Mesh_Base::create_stk_object()
        {
            MORIS_ERROR( mOrder <= 2 , "Tried to create an STK object for third or higher order. \n This is not supported by Exodus II.");

            // create new MTK object
            STK* aSTK = new STK( this );

            // create data
            aSTK->create_mesh_data();

            // return MTK object
            return aSTK;
        }

//------------------------------------------------------------------------------

        bool
        Lagrange_Mesh_Base::test_for_double_nodes()
        {
            // strategy: fill a matrix with node IDs. Make them unique.
            // each node must appear only once

            this->determine_elements_connected_to_basis();

            // get numnber of nodes
            luint tNumberOfNodes = mAllBasisOnProc.size();
            // matrix which will contain node IDs
            Matrix< DDLUMat > tNodeIDs( tNumberOfNodes, 1 );

            // loop over all nodes
            for( luint k=0; k<tNumberOfNodes; ++k )
            {
                // get node
                Basis* tNode = mAllBasisOnProc( k );

                // get level of node
                luint tLevel = tNode->get_level();

                if ( tLevel == 0 )
                {
                    tNodeIDs( k ) = tNode->get_domain_id();
                }
                else
                {
                    // get ijk of node
                    const luint* tNodeIJK = tNode->get_ijk();

                    // copy array into writable array
                    luint tIJK[ 3 ];
                    for( uint i = 0; i<mNumberOfDimensions; ++i )
                    {
                        tIJK[ i ] = tNodeIJK[ i ];
                    }

                    bool tCheck = true;

                    // now see if there is any node above
                    while ( tLevel > 0 && tCheck )
                    {
                        for( uint i = 0; i<mNumberOfDimensions; ++i )
                        {
                            tCheck = tCheck && ( tIJK[ i ] % 2 == 0 );
                        }
                        if ( tCheck )
                        {
                            // go up
                            for( uint i = 0; i<mNumberOfDimensions; ++i )
                            {
                                tIJK[ i ] /= 2;
                            }

                            // decrement level
                            --tLevel;
                        }
                    }

                    // calculate new id of node
                    if ( mNumberOfDimensions == 1 )
                    {
                        tNodeIDs( k ) =  this->calculate_node_id( tLevel, tIJK[ 0 ] );
                    }
                    else if ( mNumberOfDimensions == 2 )
                    {
                        tNodeIDs( k ) =  this->calculate_node_id( tLevel, tIJK[ 0 ], tIJK[ 1 ] );
                    }
                    else if ( mNumberOfDimensions == 3 )
                    {
                        tNodeIDs( k ) =  this->calculate_node_id( tLevel, tIJK[ 0 ], tIJK[ 1 ], tIJK[ 2 ] );
                    }
                }
            }

            /*        // init a boost bitset
                    BoostBitset tBitset( tNodeIDs.max()+1 );

                    // loop over all nodes
                    for( luint k=0; k<tNumberOfNodes; ++k )
                    {
                        // write id of node into array

                        // test if ID was already flagged
                        if ( ! tBitset.test(  tNodeIDs( k ) ) )
                        {
                            tBitset.set( tNodeIDs( k ) );
                        }
                        else
                        {
                            // other coordinate
                            luint j=0;
                            // search for corresponding node
                            for( luint i=0; i<tNumberOfNodes; ++i )
                            {
                                if ( tNodeIDs( i ) == tNodeIDs( k ) )
                                {
                                    j = i;
                                    break;
                                }

                            }

                            std::fprintf( stdout,
                                    "Error: Node %lu has the ID %lu, which is already used bu node %lu.\n\n",
                                    ( long unsigned int ) k,
                                    ( long unsigned int ) tNodeIDs( k ),
                                    ( long unsigned int ) j
                            );

                            // print elements of node 1
                            std::fprintf( stdout, "Elements connected to Node %lu : \n", ( long unsigned int ) k );

                            Matrix< DDLUMat > tElements = mElementsPerNode( mAllBasisOnProc( k )->get_domain_index() );
                             for( luint i=0; i<tElements.length(); ++i )
                            {
                                // get element
                                Element* tElement = mAllElementsOnProc ( tElements( i ) );

                                std::fprintf( stdout, "    Element %4lu    ID %4lu      Parent:  %4lu\n",
                                        ( long unsigned int ) tElement->get_background_element()->get_subdomain_index(),
                                        ( long unsigned int ) tElement->get_background_element()->get_domain_id(),
                                        ( long unsigned int ) tElement->get_background_element()->get_parent()->get_domain_id() );
                            }
                            std::fprintf( stdout, "\n" );

                            // print elements of node2
                            std::fprintf( stdout, "Elements connected to Node %lu : \n", ( long unsigned int ) j );

                            tElements = mElementsPerNode( mAllBasisOnProc( j )->get_domain_index() );

                            for( luint i=0; i<tElements.length(); ++i )
                            {
                                // get element
                                Element* tElement = mAllElementsOnProc ( tElements( i ) );

                                std::fprintf( stdout, "    Element %4lu    ID %4lu     Parent:  %4lu\n",
                                        ( long unsigned int ) tElement->get_background_element()->get_subdomain_index(),
                                        ( long unsigned int ) tElement->get_background_element()->get_domain_id(),
                                        ( long unsigned int ) tElement->get_background_element()->get_parent()->get_domain_id()  );

                            }
                            std::fprintf( stdout, "\n" );

                            exit(-1);
                        }

                    } */

            // make matrix unique
            Matrix< DDLUMat > tNodeUniqueIDs;
            unique( tNodeIDs, tNodeUniqueIDs );

            // make sure that number of nodes is the same
            return tNodeUniqueIDs.length() == tNumberOfNodes;
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::save_to_gmsh( const std::string & aFilePath )
        {
            // start timer
            tic tTimer;

            // get my rank
            moris_id tMyRank = par_rank();

            // modify filename
            std::string tFilePath;
            if ( moris::par_size() > 1 )
            {
                tFilePath = aFilePath + "." +  std::to_string( par_size() ) + "." +  std::to_string( tMyRank );
            }
            else
            {
                tFilePath = aFilePath;
            }

            // create output file
            std::FILE * tFile = std::fopen( tFilePath.c_str(), "w+");

            // write header
            std::fprintf( tFile, "$MeshFormat\n2.2 0 8\n$EndMeshFormat\n");

            // write node coordinates
            std::fprintf( tFile, "$Nodes\n%lu\n", ( long unsigned int ) mNumberOfUsedNodes );

            // get mesh scale factor
            real tScale = mParameters->get_gmsh_scale();

            switch ( mParameters->get_number_of_dimensions() )
            {
                case( 2 ) :
                {
                    // loop over all nodes
                    for( auto tNode : mAllBasisOnProc )
                    {
                        // test if this node is relevant
                        if ( tNode->is_used() )
                        {
                            // get coordinates of node
                            const real* tXY = tNode->get_xyz();

                            // write coordinates to ASCII file
                            std::fprintf( tFile,
                                    "%lu %.17f %.17f 0\n",
                                    ( long unsigned int ) tNode->get_domain_index()+1,
                                    ( double ) tXY[ 0 ]*tScale,
                                    ( double ) tXY[ 1 ]*tScale );
                        }
                    }
                    break;
                }
                case( 3 ) :
                {
                    // loop over all nodes
                    for( auto tNode : mAllBasisOnProc )
                    {
                        // test if this node is relevant
                        if ( tNode->is_used() )
                        {
                            // get coordinates of node
                            const real* tXYZ = tNode->get_xyz();

                            // write coordinates to ASCII file
                            std::fprintf( tFile,
                                    "%lu %.17f %.17f %.17f\n",
                                    ( long unsigned int ) tNode->get_domain_index()+1,
                                    ( double ) tXYZ[ 0 ]*tScale,
                                    ( double ) tXYZ[ 1 ]*tScale,
                                    ( double ) tXYZ[ 2 ]*tScale );
                        }
                    }
                    break;

               }
               default :
               {
                   MORIS_ERROR( false, "wrong number of dimensions\n");
                   break;
               }
            }

            // end node tag
            std::fprintf( tFile, "$EndNodes\n" );

            // write element tag
            std::fprintf( tFile, "$Elements\n%lu\n",
                    ( long unsigned int ) mNumberOfElements );

            // loop over all elements
            for( auto tElement: mAllElementsOnProc )
            {
                // test if this element is relevant
                if ( tElement->get_owner() == tMyRank && tElement->is_active() )
                {
                    // print element line
                    std::fprintf( tFile, "%lu %s\n",
                            ( long unsigned int ) tElement->get_domain_index()+1,
                            tElement->get_gmsh_string().c_str() );
                }
            }

            // finish element tag
            std::fprintf( tFile, "$EndElements\n" );

            // close file
            std::fclose( tFile );

            // print a debug statement if verbosity is set
            if ( mParameters->is_verbose() )
            {
                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                // print output
                std::fprintf( stdout,"%s Created GMSH File: %s\n               Writing took %5.3f seconds.\n\n",
                        proc_string().c_str(),
                        tFilePath.c_str(),
                        ( double ) tElapsedTime / 1000 );
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::link_twins( )
        {
            // get number of elements of interest
            auto tNumberOfElements = this->get_number_of_elements();

            // loop over all elements of interest
            for( uint k=0; k<tNumberOfElements; ++k )
            {
                // get pointer to Lagrange element
                auto tLagrangeElement = this->get_element( k );

                // link elements
                tLagrangeElement->set_twin( mBSplineMesh->get_element( k ) );
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::save_to_vtk( const std::string & aFilePath )
        {
            // start timer
            tic tTimer;

            // modify filename
            std::string tFilePath = parallelize_path( aFilePath );

            // open the file
            std::ofstream tFile(tFilePath, std::ios::binary);

            // containers
            //float tFValue = 0;
            //int   tIValue = 0;
            float tFChar = 0;
            int   tIChar = 0;

            tFile << "# vtk DataFile Version 3.0" << std::endl;
            tFile << "GO BUFFS!" << std::endl;
            tFile << "BINARY" << std::endl;
            //tFile << "ASCII" << std::endl;
            luint tNumberOfNodes = mAllBasisOnProc.size();

            // write node data
            tFile << "DATASET UNSTRUCTURED_GRID" << std::endl;

            tFile << "POINTS " << tNumberOfNodes << " float"  << std::endl;

            // ask settings for numner of dimensions
            auto tNumberOfDimensions = mParameters->get_number_of_dimensions();

            if ( tNumberOfDimensions == 2 )
            {
                // loop over all nodes
                for ( luint k = 0; k < tNumberOfNodes; ++k )
                {
                    // get coordinate from node
                    const real* tXY = mAllBasisOnProc( k )->get_xyz();

                    // write coordinates to mesh
                    tFChar = swap_byte_endian( (float) tXY[ 0 ] );
                    tFile.write( (char*) &tFChar, sizeof(float));
                    tFChar = swap_byte_endian( (float) tXY[ 1 ] );
                    tFile.write( (char*) &tFChar, sizeof(float));
                    tFChar = swap_byte_endian( (float) 0 );
                    tFile.write( (char*) &tFChar, sizeof(float));
                }
            }
            else if ( tNumberOfDimensions == 3 )
            {
                // loop over all nodes
                for ( luint k = 0; k < tNumberOfNodes; ++k )
                {
                    // get coordinate from node
                    const real* tXYZ = mAllBasisOnProc( k )->get_xyz();

                    // write coordinates to mesh
                    tFChar = swap_byte_endian( (float) tXYZ[ 0 ] );
                    tFile.write( (char*) &tFChar, sizeof(float));
                    tFChar = swap_byte_endian( (float) tXYZ[ 1 ] );
                    tFile.write( (char*) &tFChar, sizeof(float));
                    tFChar = swap_byte_endian( (float) tXYZ[ 2 ] );
                    tFile.write( (char*) &tFChar, sizeof(float));
                }
            }

            tFile << std::endl;

            // write element topology
            int tCellType = mAllElementsOnProc( 0 )->get_vtk_type();

            // count number of non padding elements
            luint tNumberOfElements = 0;

            // can only write element data if vtk map exists
            if ( tCellType != 0 )
            {
                int tNumberOfNodesPerElement = swap_byte_endian( (int) mNumberOfBasisPerElement );


                luint tNumberOfAllElementsOnProc = mAllElementsOnProc.size();

                for( luint k=0; k<tNumberOfAllElementsOnProc; ++k )
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        // increment element counter
                        ++tNumberOfElements;
                    }
                }

                // write header for cells
                tFile << "CELLS " << tNumberOfElements << " "
                        << ( mNumberOfBasisPerElement + 1 )*tNumberOfElements  << std::endl;


                // matrix containing node indices
                Matrix< DDLUMat > tNodes( mNumberOfBasisPerElement, 1 );

                // loop over all elements
                for( luint k=0; k<tNumberOfAllElementsOnProc; ++k )
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        tFile.write( (char*) &tNumberOfNodesPerElement, sizeof(int)) ;

                        // ask element for nodes
                        mAllElementsOnProc( k )->get_basis_indices_for_vtk( tNodes );

                        for( uint i=0; i <mNumberOfBasisPerElement; ++i )
                        {
                            tIChar = swap_byte_endian( (int) tNodes( i ) );
                            tFile.write((char *) &tIChar, sizeof(int));
                            //tFile << " " << (int) mAllElementsOnProc( k )->get_basis( i )->get_memory_index();
                        }
                        //tFile << std::endl;
                    }
                }

                tFile << std::endl;

                // write cell types
                tFile << "CELL_TYPES " << tNumberOfElements << std::endl;
                tIChar = swap_byte_endian( tCellType );
                for ( luint k = 0; k < tNumberOfElements; ++k)
                {
                    tFile.write( (char*) &tIChar, sizeof(int));
                    //tFile << tCellType << std::endl;
                }


                // write element data
                tFile << "CELL_DATA " << tNumberOfElements << std::endl;

                // write element ID
                tFile << "SCALARS ELEMENT_ID int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for (moris::uint k = 0; k <  tNumberOfAllElementsOnProc; ++k)
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        //tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_background_element()->get_domain_id() );
                        tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_id() );
                        tFile.write( (char*) &tIChar, sizeof(int));
                    }
                }
                tFile << std::endl;

                // write element ID
                tFile << "SCALARS ELEMENT_INDEX int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for (moris::uint k = 0; k <  tNumberOfAllElementsOnProc; ++k)
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        //tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_background_element()->get_domain_id() );
                        tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_index() );
                        tFile.write( (char*) &tIChar, sizeof(int));
                    }
                }
                tFile << std::endl;


                // write proc owner
                tFile << "SCALARS ELEMENT_OWNER int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for (moris::uint k = 0; k <  tNumberOfAllElementsOnProc; ++k)
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_owner() );
                        tFile.write( (char*) &tIChar, sizeof(float));
                    }
                }
                tFile << std::endl;

                // write level
                tFile << "SCALARS ELEMENT_LEVEL int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for (moris::uint k = 0; k <  tNumberOfAllElementsOnProc; ++k)
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_level() );
                        tFile.write( (char*) &tIChar, sizeof(float));
                    }
                }
                tFile << std::endl;

                // write level
                tFile << "SCALARS ELEMENT_CHILD_INDEX int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for (moris::uint k = 0; k <  tNumberOfAllElementsOnProc; ++k)
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_background_element()->get_child_index() );
                        tFile.write( (char*) &tIChar, sizeof(float));
                    }
                }
                tFile << std::endl;

                // write memory index
                tFile << "SCALARS ELEMENT_MEMORY_INDEX int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for (moris::uint k = 0; k <  tNumberOfAllElementsOnProc; ++k)
                {
                    if ( mAllElementsOnProc( k )->is_active() )
                    {
                        tIChar = swap_byte_endian( (int) mAllElementsOnProc( k )->get_background_element()->get_memory_index() );
                        tFile.write( (char*) &tIChar, sizeof(float));
                    }
                }
                tFile << std::endl;
            }

            // write node data
            tFile << "POINT_DATA " << tNumberOfNodes << std::endl;

            tFile << "SCALARS NODE_ID int" << std::endl;
            tFile << "LOOKUP_TABLE default" << std::endl;
            for (moris::uint k = 0; k <  tNumberOfNodes; ++k)
            {

                tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_id() );
                tFile.write( (char*) &tIChar, sizeof(float));
            }
            tFile << std::endl;

            tFile << "SCALARS NODE_INDEX int" << std::endl;
            tFile << "LOOKUP_TABLE default" << std::endl;
            for (moris::uint k = 0; k <  tNumberOfNodes; ++k)
            {

                tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_index() );
                tFile.write( (char*) &tIChar, sizeof(float));
            }
            tFile << std::endl;

            tFile << "SCALARS NODE_OWNER int" << std::endl;
            tFile << "LOOKUP_TABLE default" << std::endl;
            for (moris::uint k = 0; k <  tNumberOfNodes; ++k)
            {

                tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_owner() );
                tFile.write( (char*) &tIChar, sizeof(float));
            }
            tFile << std::endl;

            tFile << "SCALARS NODE_INDEX int" << std::endl;
            tFile << "LOOKUP_TABLE default" << std::endl;
            for (moris::uint k = 0; k <  tNumberOfNodes; ++k)
            {

                tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_index() );
                tFile.write( (char*) &tIChar, sizeof(float));
            }
            tFile << std::endl;
            // close the output file
            tFile.close();

            if ( mParameters->is_verbose() )
            {
                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                // print output
                std::fprintf( stdout,"%s Created VTK debug file.\n               Mesh has %lu active and refined Elements and %lu Nodes.\n               Creation took %5.3f seconds.\n\n",
                        proc_string().c_str(),
                        ( long unsigned int ) tNumberOfElements,
                        ( long unsigned int ) tNumberOfNodes,
                        ( double ) tElapsedTime / 1000 );
            }

        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::create_nodes_on_level_zero()
        {
            // ask background mesh for number of dimensions
            Matrix< DDLUMat > tNumberOfElements
            = mBackgroundMesh->get_number_of_elements_per_direction_on_proc();

            if( mNumberOfDimensions == 2 )
            {
                // get maximum numbers in i direction
                luint tImax = tNumberOfElements( 0, 0 );

                // get maximum numbers in j direction
                luint tJmax = tNumberOfElements( 1, 0 );

                // initialize element counter
                luint tCount = 0;

                // loop over all elements
                for( luint j=0; j<tJmax; ++j )
                {
                    for ( luint i=0; i<tImax; ++i )
                    {
                        mAllCoarsestElementsOnProc( tCount++ )
                            ->create_basis_on_level_zero(
                                mAllElementsOnProc,
                                mNumberOfAllBasis );
                    }
                }
            }
            else if( mNumberOfDimensions == 3 )
            {
                // get maximum numbers in i direction
                luint tImax = tNumberOfElements( 0, 0 );

                // get maximum numbers in j direction
                luint tJmax = tNumberOfElements( 1, 0 );

                // get maximum numbers in k direction
                luint tKmax = tNumberOfElements( 2, 0 );

                // initialize element counter
                luint tCount = 0;

                // loop over all elements
                for( luint k=0; k<tKmax; ++k )
                {
                    for( luint j=0; j<tJmax; ++j )
                    {
                        for ( luint i=0; i<tImax; ++i )
                        {
                            mAllCoarsestElementsOnProc( tCount++ )
                                ->create_basis_on_level_zero(
                                    mAllElementsOnProc,
                                    mNumberOfAllBasis );
                        }
                    }
                }
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::update_node_list()
        {
            // tidy up memory
            mNodes.clear();

            // assign memory
            mNodes.resize( mNumberOfUsedNodes, nullptr );

            // initialize counter
            luint tCount = 0;

            for( auto tNode : mAllBasisOnProc )
            {
                if ( tNode->is_used() )
                {
                    mNodes( tCount++ ) = tNode;
                }
            }

            MORIS_ERROR( tCount == mNumberOfUsedNodes, "Number Of used Nodes does not match" );
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::create_facets()
        {

            // get my rank
            moris_id tMyRank = par_rank();

            // delete existing lagrange facets
            this->delete_facets();

            // step 1: unflag all facets

            // loop over all elements
            for( Element * tElement :  mAllElementsOnProc )
            {
                if( tElement->get_owner() == tMyRank )
                {
                    // make sure that all background faces are unflagged
                    tElement->get_background_element()->reset_flags_of_facets();
                }
            }

            // step 2 : determine number of facets per element
            uint tNumberOfFacetsPerElement = 0;
            if ( mParameters->get_number_of_dimensions() == 2 )
            {
                tNumberOfFacetsPerElement = 4;
            }
            else if( mParameters->get_number_of_dimensions() == 3 )
            {
                tNumberOfFacetsPerElement = 6;
            }



            // step 2: count number of active or refined facets on mesh

            // initialize counter
            uint tCount = 0;

            // loop over all active elements
            for( Element * tElement : mAllElementsOnProc )
            {
                // only process elements that I own
                if( tElement->get_owner() == tMyRank )
                {
                    // test if element is not deactive
                    if( ! tElement->is_deactive() && ! tElement->is_padding() )
                    {

                        // get pointer to Element
                        Background_Element_Base *
                        tBackElement = tElement->get_background_element();

                        for( uint f=0; f<tNumberOfFacetsPerElement; ++f )
                        {
                            // get pointer to face
                            Background_Facet * tBackFacet = tBackElement->get_facet( f );

                            // test if background facet is not flagged and element
                            if( ! tBackFacet->is_flagged() )
                            {
                                // flag facet
                                tBackFacet->flag();

                                // increment counter
                                ++tCount;
                            }
                        }
                    }
                }
            }


            // step 2: create lagrange facets
            mFacets.resize( tCount, nullptr );

            // reset counter
            tCount = 0;

            // counter for owned facets
            uint tOwnedCount = 0;

            // loop over all active elements
            for( Element * tElement : mAllElementsOnProc )
            {
                // pick pointer to element
                // only process elements that I own
                if( tElement->get_owner() == tMyRank )
                {
                    if( ! tElement->is_deactive() && ! tElement->is_padding() )
                    {
                        Background_Element_Base *
                        tBackElement = tElement->get_background_element();

                        for( uint f=0; f<tNumberOfFacetsPerElement; ++f )
                        {
                            // get pointer to facet
                            Background_Facet * tBackFacet = tBackElement->get_facet( f );

                            // test if facet is flagged
                            if( tBackFacet->is_flagged() )
                            {
                                // create facet
                                Facet * tFacet = this->create_facet( tBackFacet );

                                // test owner of facet
                                if( tFacet->get_owner() == tMyRank )
                                {
                                    tFacet->set_id( tOwnedCount++ );
                                }

                                // set index for this facet
                                tFacet->set_index( tCount );

                                // copy facet into array
                                mFacets( tCount++ ) = tFacet;

                                // unflag facet
                                tBackFacet->unflag();
                            }
                        }
                    }
                }
            }

            // step 5: write facets into cells
            for( Facet * tFacet : mFacets )
            {
                // get master
                Element * tMaster = tFacet->get_hmr_master();

                // get slave
                Element * tSlave = tFacet->get_hmr_slave();

                // master is always active
                tMaster->set_hmr_facet(
                        tFacet,
                        tFacet->get_index_on_master() );

                if( tSlave != NULL )
                {
                    // insert element into slave
                    tSlave->set_hmr_facet(
                            tFacet,
                            tFacet->get_index_on_slave() );
                }
            }


            // step 6: synchronize proc IDs if parallel
            if( par_size() > 1 )
            {
                this->synchronize_facet_ids( tOwnedCount );
            }

            // step 7 : link facets to basis

            // reset facet containers
            for( Basis * tBasis : mAllBasisOnProc )
            {
                tBasis->delete_facet_container();
            }


            // count facets
            for( Facet * tFacet : mFacets )
            {
                // only connect active facets
                if ( tFacet->is_active() )
                {
                    // get number of connected basis
                    uint tNumberOfBasis = tFacet->get_number_of_vertices();

                    for( uint k=0; k<tNumberOfBasis; ++k )
                    {
                        tFacet->get_basis( k )->increment_facet_counter();
                    }
                }
            }

            // insert facet containers
            for( Basis * tBasis : mAllBasisOnProc )
            {
                tBasis->init_facet_container();
            }

            for( Facet * tFacet : mFacets )
            {
                // only connect active facets
                if ( tFacet->is_active() )
                {
                    // get number of connected basis
                    uint tNumberOfBasis = tFacet->get_number_of_vertices();

                    for( uint k=0; k<tNumberOfBasis; ++k )
                    {
                        tFacet->get_basis( k )->insert_facet( tFacet );
                    }
                }
            }


            /*std::cout << par_rank() << " flag 1" << std::endl;
            // step 7 : link facets with children
            if( mParameters->get_number_of_dimensions() == 2 )
            {
                this->link_facet_children_2d();
            }
            else if ( mParameters->get_number_of_dimensions() == 3 )
            {
                this->link_facet_children_3d();
            }
            std::cout << par_rank() << " flag 2" << std::endl; */
        }
//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::create_edges()
        {
            // get my rank
            moris_id tMyRank = par_rank();

            // delete existing edges
            this->delete_edges();

            // step 1: unflag all edges on background mesh
            for( Element * tElement :  mAllElementsOnProc )
            {
                if( tElement->get_owner() == tMyRank )
                {
                    // make sure that all background faces are unflagged
                    tElement->get_background_element()->reset_flags_of_edges();
                }
            }

            // step 2: count number of active or refined edges on mesh

            // initialize counter
            uint tCount = 0;

            // loop over all active elements
            for( Element * tElement : mAllElementsOnProc )
            {
                // only process elements that I own
                if( tElement->get_owner() == tMyRank )
                {
                    // test if element is not deactive
                    if( ! tElement->is_deactive() && ! tElement->is_padding() )
                    {
                        // get pointer to Element
                        Background_Element_Base *
                        tBackElement = tElement->get_background_element();

                        // loop over all edges
                        for( uint e=0; e<12; ++e )
                        {
                            // get pointer to edge
                            Background_Edge * tBackEdge = tBackElement->get_edge( e );

                            // tesi if edge is not flagged
                            if( ! tBackEdge->is_flagged() )
                            {
                                // flag edge
                                tBackEdge->flag();

                                // increment counter
                                ++tCount;
                            }
                        }
                    }
                }
            }

            // step 3: create Lagrange edges

            mEdges.resize( tCount, nullptr );

            // reset counter
            tCount = 0;

            // counter for owned facets
            uint tOwnedCount = 0;

            // loop over all active elements
            for( Element * tElement : mAllElementsOnProc )
            {
                // pick pointer to element
                // only process elements that I own
                if( tElement->get_owner() == tMyRank )
                {
                    if( ! tElement->is_deactive() && ! tElement->is_padding() )
                    {
                        Background_Element_Base *
                        tBackElement = tElement->get_background_element();

                        for( uint e=0; e<12; ++e )
                        {
                            // get pointer to facet
                            Background_Edge* tBackEdge = tBackElement->get_edge( e );

                            // test if facet is flagged
                            if( tBackEdge->is_flagged() )
                            {

                                // create edge
                                Edge * tEdge = this->create_edge( tBackEdge );

                                // test owner of facet
                                if( tEdge->get_owner() == tMyRank )
                                {
                                    tEdge->set_id( tOwnedCount++ );
                                }

                                // set index for this facet
                                tEdge->set_index( tCount );

                                // copy facet into array
                                mEdges( tCount++ ) = tEdge;

                                // unflag edge
                                tBackEdge->unflag();
                            }
                        }
                    }
                }
            }

            // step 5: write edges into cells
            for( Edge * tEdge: mEdges )
            {
                // get number of elements
                uint tNumberOfElements = tEdge->get_number_of_elements();

                // loop over all elements of this edge
                for( uint e = 0; e<tNumberOfElements; ++e )
                {
                    // insert edge into element
                    tEdge->get_element( e )->set_hmr_edge(
                            tEdge,
                            tEdge->get_index_on_element( e ) );
                }
            }

            // step 6: synchronize proc IDs if parallel
            if( par_size() > 1 )
            {
                this->synchronize_edge_ids( tOwnedCount );
            }

            // step 7 : link edges to basis
            // reset edge containers
            for( Basis * tBasis : mAllBasisOnProc )
            {
                tBasis->delete_edge_container();
            }


            // count edges
            for( Edge * tEdge : mEdges )
            {
                // only connect active edges
                if ( tEdge->is_active() )
                {
                    // get number of connected basis
                    uint tNumberOfBasis = tEdge->get_number_of_vertices();

                    for( uint k=0; k<tNumberOfBasis; ++k )
                    {
                        tEdge->get_basis( k )->increment_edge_counter();
                    }
                }
            }

            // insert edge containers
            for( Basis * tBasis : mAllBasisOnProc )
            {
                tBasis->init_edge_container();
            }

            for( Edge * tEdge : mEdges )
            {
                // only connect active edges
                if ( tEdge->is_active() )
                {
                    // get number of connected basis
                    uint tNumberOfBasis = tEdge->get_number_of_vertices();

                    for( uint k=0; k<tNumberOfBasis; ++k )
                    {
                        tEdge->get_basis( k )->insert_edge( tEdge );
                    }
                }
            }
        }

//------------------------------------------------------------------------------

        Facet *
        Lagrange_Mesh_Base::create_facet( Background_Facet * aFacet )
        {
            MORIS_ERROR( false,
                    "create_facet() must not be called from base class" );
            return nullptr;
        }

//------------------------------------------------------------------------------

        Edge *
        Lagrange_Mesh_Base::create_edge( Background_Edge * aEdge )
        {
            MORIS_ERROR( false,
                    "create_edge() must not be called from base class" );
            return nullptr;
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::delete_facets()
        {
            for( auto tFacet : mFacets )
            {
                delete tFacet;
            }

            mFacets.clear();
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::delete_edges()
        {
            for( auto tEdge : mEdges )
            {
                delete tEdge;
            }

            mEdges.clear();
        }


//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::synchronize_facet_ids( const uint & aOwnedCount )
        {

            // get number of procs
            moris_id tNumberOfProcs = par_size();

            // get my rank
            moris_id tMyRank = par_rank();

            // communicate number of owned nodes with other procs
            Matrix< DDUMat > tFacetsOwnedPerProc;
            comm_gather_and_broadcast( aOwnedCount, tFacetsOwnedPerProc );

            // calculate node offset table
            Matrix< DDUMat > tFacetOffset( tNumberOfProcs, 1, 0 );
            for( moris_id p=1; p<tNumberOfProcs; ++p )
            {
                tFacetOffset( p ) =   tFacetOffset( p-1 )
                                            + tFacetsOwnedPerProc( p-1 );
            }

            // remember max index for vtk
            mMaxFacetDomainIndex = tFacetOffset( tNumberOfProcs -1 ) +
                    tFacetsOwnedPerProc( tNumberOfProcs-1 );

            moris_id tMyOffset = tFacetOffset( tMyRank );

            // update owned nodes
            for( Facet * tFacet : mFacets )
            {
                if( tFacet->get_owner() == tMyRank )
                {
                    tFacet->set_id( tFacet->get_id() + tMyOffset );
                }
            }

            // step 4b: synchronize IDs for non owned facets

            // get proc neighbors from background mesh
            auto tProcNeighbors = mBackgroundMesh->get_proc_neigbors();

            // get number of proc neighbors
            uint tNumberOfNeighbors
                = mBackgroundMesh->get_number_of_proc_neighbors();

            // create cell of matrices to send
            Matrix< DDLUMat > tEmptyLuint;
            Cell< Matrix< DDLUMat > > tAncestorListSend;
            tAncestorListSend.resize( tNumberOfNeighbors, { tEmptyLuint } );

            Matrix< DDUMat > tEmptyUint;
            Cell< Matrix< DDUMat > > tPedigreeListSend;
            tPedigreeListSend.resize( tNumberOfNeighbors, { tEmptyUint } );

            Cell< Matrix< DDUMat > > tFacetIndexListSend;
            tFacetIndexListSend.resize( tNumberOfNeighbors, { tEmptyUint } );

            // loop over all proc neighbors
            for ( uint p = 0; p<tNumberOfNeighbors; ++p )
            {
                auto tNeighbor = tProcNeighbors( p );

                if ( tNeighbor < tNumberOfProcs && tNeighbor != tMyRank )
                {

                    // count facets that belong to neighbor
                    luint tElementCounter = 0;

                    // initialize counter for memory needed for pedigree tree
                    luint tMemoryCounter = 0;

                    // loop over all faces on this mesh
                    for( Facet * tFacet : mFacets )
                    {
                        if( tFacet->get_owner() == tNeighbor )
                        {
                            // increment counter
                            ++tElementCounter;

                            // get memory needed for pedigree path

                            tMemoryCounter
                                += tFacet->get_hmr_master()
                                    ->get_background_element()
                                    ->get_length_of_pedigree_path();
                        }
                    }

                    if ( tElementCounter > 0 )
                    {
                        // prepare matrix containing ancestors
                        tAncestorListSend( p ).set_size( tElementCounter, 1 );

                        // prepare matrix containing pedigree list
                        tPedigreeListSend( p ).set_size( tMemoryCounter, 1 );

                        // prepare matrix containing face indices
                        tFacetIndexListSend( p ).set_size( tElementCounter, 1 );

                        // reset counter for elements
                        tElementCounter = 0;

                        // reset pedigree memory counter
                        tMemoryCounter = 0;

                        // loop over all faces on this mesh
                        for( Facet * tFacet : mFacets )
                        {
                            if( tFacet->get_owner() == tNeighbor )
                            {
                                // save index on master
                                tFacetIndexListSend( p )( tElementCounter )
                                                            = tFacet->get_index_on_master();

                                // calculate path of facet
                                tFacet->get_hmr_master()
                                        ->get_background_element()
                                        ->endcode_pedigree_path(
                                        tAncestorListSend( p )( tElementCounter++ ),
                                        tPedigreeListSend( p ),
                                        tMemoryCounter );

                            }
                        }
                    }
                }
            } /* end loop over all procs */

            // initialize matrices for receiving
            Cell< Matrix< DDLUMat > > tAncestorListReceive;
            Cell< Matrix< DDUMat > >  tPedigreeListReceive;
            Cell< Matrix< DDUMat > >  tFacetIndexListReceive;

            // communicate ancestor IDs
            communicate_mats(
                    tProcNeighbors,
                    tAncestorListSend,
                    tAncestorListReceive );

            // clear memory
            tAncestorListSend.clear();

            // communicate pedigree list
            communicate_mats(
                    tProcNeighbors,
                    tPedigreeListSend,
                    tPedigreeListReceive );

            // clear memory
            tPedigreeListSend.clear();

            // communicate indices
            communicate_mats(
                    tProcNeighbors,
                    tFacetIndexListSend,
                    tFacetIndexListReceive );


            // loop over all received lists
            for ( uint p=0; p<tNumberOfNeighbors; ++p )
            {
                // get number of elements on refinement list
                luint tNumberOfElements = tAncestorListReceive( p ).length();

                // reset memory counter
                luint tMemoryCounter = 0;

                // resize  sending list
                tFacetIndexListSend( p ).resize( tNumberOfElements, 1 );

                // loop over all received elements
                for ( uint k=0; k<tNumberOfElements; ++k )
                {
                    // decode path and get pointer to element
                    Background_Element_Base*
                    tBackElement = mBackgroundMesh->decode_pedigree_path(
                            tAncestorListReceive( p )( k ),
                            tPedigreeListReceive( p ),
                            tMemoryCounter );

                    // get pointer to master
                    Element * tMaster =
                            this->get_element_by_memory_index(
                                    tBackElement->get_memory_index() );

                    // get pointer to facet
                    Facet * tFacet = tMaster->get_hmr_facet(
                            tFacetIndexListReceive( p )( k ) );

                    // copy ID into send index
                    tFacetIndexListSend( p )( k ) = tFacet->get_id();
                }
            }  /* end loop over all procs */

            // reset receive list
            tFacetIndexListReceive.clear();

            // communicate ids
            communicate_mats(
                    tProcNeighbors,
                    tFacetIndexListSend,
                    tFacetIndexListReceive );

            // reset send list
            tFacetIndexListSend.clear();

            // loop over all received lists
            for ( uint p=0; p<tNumberOfNeighbors; ++p )
            {


                if( tFacetIndexListReceive( p ).length() > 0 )
                {
                    // get neighbor id
                    auto tNeighbor = tProcNeighbors( p );

                    // reset counter
                    uint tCount = 0;

                    // loop over all faces on this mesh
                    for( Facet * tFacet : mFacets )
                    {
                        if( tFacet->get_owner() == tNeighbor )
                        {
                            // set index of facet
                            tFacet->set_id( tFacetIndexListReceive( p )( tCount++ ) );
                        }
                    }

                }
            } /* end loop over all procs */
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::synchronize_edge_ids( const uint & aOwnedCount )
        {

            // get number of procs
            moris_id tNumberOfProcs = par_size();

            // get my rank
            moris_id tMyRank = par_rank();

            // communicate number of owned nodes with other procs
            Matrix< DDUMat > tEdgesOwnedPerProc;
            comm_gather_and_broadcast( aOwnedCount, tEdgesOwnedPerProc );

            // calculate node offset table
            Matrix< DDUMat > tEdgeOffset( tNumberOfProcs, 1, 0 );
            for( moris_id p=1; p<tNumberOfProcs; ++p )
            {
                tEdgeOffset( p ) =   tEdgeOffset( p-1 ) + tEdgesOwnedPerProc( p-1 );
            }

            mMaxEdgeDomainIndex = tEdgeOffset( tNumberOfProcs-1 )
                                + tEdgesOwnedPerProc( tNumberOfProcs-1 );

            moris_id tMyOffset = tEdgeOffset( tMyRank );

            // update owned nodes
            for( Edge * tEdge : mEdges )
            {
                if( tEdge->get_owner() == tMyRank )
                {
                    tEdge->set_id( tEdge->get_id() + tMyOffset );
                }
            }

            // step 4b: synchronize IDs for non owned facets

            // get proc neighbors from background mesh
            auto tProcNeighbors = mBackgroundMesh->get_proc_neigbors();

            // get number of proc neighbors
            uint tNumberOfNeighbors
            = mBackgroundMesh->get_number_of_proc_neighbors();

            // create cell of matrices to send
            Matrix< DDLUMat > tEmptyLuint;
            Cell< Matrix< DDLUMat > > tAncestorListSend;
            tAncestorListSend.resize( tNumberOfNeighbors, { tEmptyLuint } );

            Matrix< DDUMat > tEmptyUint;
            Cell< Matrix< DDUMat > > tPedigreeListSend;
            tPedigreeListSend.resize( tNumberOfNeighbors, { tEmptyUint } );

            Cell< Matrix< DDUMat > > tEdgeIndexListSend;
            tEdgeIndexListSend.resize( tNumberOfNeighbors, { tEmptyUint } );

            // loop over all proc neighbors
            for ( uint p = 0; p<tNumberOfNeighbors; ++p )
            {
                auto tNeighbor = tProcNeighbors( p );

                if ( tNeighbor < tNumberOfProcs && tNeighbor != tMyRank )
                {

                    // count facets that belong to neighbor
                    luint tElementCounter = 0;

                    // initialize counter for memory needed for pedigree tree
                    luint tMemoryCounter = 0;

                    // loop over all faces on this mesh
                    for( Edge * tEdge : mEdges )
                    {
                        if( tEdge->get_owner() == tNeighbor )
                        {
                            // increment counter
                            ++tElementCounter;

                            // get memory needed for pedigree path

                            tMemoryCounter
                            += tEdge->get_hmr_master()
                            ->get_background_element()
                            ->get_length_of_pedigree_path();
                        }
                    }

                    if ( tElementCounter > 0 )
                    {
                        // prepare matrix containing ancestors
                        tAncestorListSend( p ).set_size( tElementCounter, 1 );

                        // prepare matrix containing pedigree list
                        tPedigreeListSend( p ).set_size( tMemoryCounter, 1 );

                        // prepare matrix containing face indices
                        tEdgeIndexListSend( p ).set_size( tElementCounter, 1 );

                        // reset counter for elements
                        tElementCounter = 0;

                        // reset pedigree memory counter
                        tMemoryCounter = 0;

                        // loop over all faces on this mesh
                        for( Edge * tEdge : mEdges )
                        {
                            if( tEdge->get_owner() == tNeighbor )
                            {
                                // save index on master
                                tEdgeIndexListSend( p )( tElementCounter )
                                                                           = tEdge->get_index_on_master();

                                // calculate path of facet
                                tEdge->get_hmr_master()
                                                       ->get_background_element()
                                                       ->endcode_pedigree_path(
                                                               tAncestorListSend( p )( tElementCounter++ ),
                                                               tPedigreeListSend( p ),
                                                               tMemoryCounter );

                            }
                        }
                    }
                }
            } /* end loop over all procs */

            // initialize matrices for receiving
            Cell< Matrix< DDLUMat > > tAncestorListReceive;
            Cell< Matrix< DDUMat > >  tPedigreeListReceive;
            Cell< Matrix< DDUMat > >  tEdgeIndexListReceive;

            // communicate ancestor IDs
            communicate_mats(
                    tProcNeighbors,
                    tAncestorListSend,
                    tAncestorListReceive );

            // clear memory
            tAncestorListSend.clear();

            // communicate pedigree list
            communicate_mats(
                    tProcNeighbors,
                    tPedigreeListSend,
                    tPedigreeListReceive );

            // clear memory
            tPedigreeListSend.clear();

            // communicate indices
            communicate_mats(
                    tProcNeighbors,
                    tEdgeIndexListSend,
                    tEdgeIndexListReceive );


            // loop over all received lists
            for ( uint p=0; p<tNumberOfNeighbors; ++p )
            {
                // get number of elements on refinement list
                luint tNumberOfElements = tAncestorListReceive( p ).length();

                // reset memory counter
                luint tMemoryCounter = 0;

                // resize  sending list
                tEdgeIndexListSend( p ).resize( tNumberOfElements, 1 );

                // loop over all received elements
                for ( uint k=0; k<tNumberOfElements; ++k )
                {
                    // decode path and get pointer to element
                    Background_Element_Base*
                    tBackElement = mBackgroundMesh->decode_pedigree_path(
                            tAncestorListReceive( p )( k ),
                            tPedigreeListReceive( p ),
                            tMemoryCounter );

                    // get pointer to master
                    Element * tMaster =
                            this->get_element_by_memory_index(
                                    tBackElement->get_memory_index() );

                    // get pointer to facet
                    Edge * tEdge = tMaster->get_hmr_edge(
                            tEdgeIndexListReceive( p )( k ) );

                    // copy ID into send index
                    tEdgeIndexListSend( p )( k ) = tEdge->get_id();
                }
            }  /* end loop over all procs */

            // reset receive list
            tEdgeIndexListReceive.clear();

            // communicate ids
            communicate_mats(
                    tProcNeighbors,
                    tEdgeIndexListSend,
                    tEdgeIndexListReceive );

            // reset send list
            tEdgeIndexListSend.clear();

            // loop over all received lists
            for ( uint p=0; p<tNumberOfNeighbors; ++p )
            {


                if( tEdgeIndexListReceive( p ).length() > 0 )
                {
                    // get neighbor id
                    auto tNeighbor = tProcNeighbors( p );

                    // reset counter
                    uint tCount = 0;

                    // loop over all faces on this mesh
                    for( Edge * tEdge : mEdges )
                    {
                        if( tEdge->get_owner() == tNeighbor )
                        {
                            // set index of facet
                            tEdge->set_id( tEdgeIndexListReceive( p )( tCount++ ) );
                        }
                    }

                }
            } /* end loop over all procs */
        }

//------------------------------------------------------------------------------

 /*       void
        Lagrange_Mesh_Base::link_facet_children_2d()
        {
            for( Facet * tFacet : mFacets )
            {
                // get master
                Element * tMaster = tFacet->get_hmr_master();

                if( tMaster->is_refined() )
                {
                    // reserve memory for children
                    tFacet->allocate_child_container( 2 );
                    if( par_rank() == 0 )
                    {
                        std::cout << "Master " << tMaster->get_domain_id() << std::endl;
                        for( uint k = 0; k<4; ++k )
                        {
                            std::cout << k << " " << ( tMaster->get_child( mAllElementsOnProc, k) != NULL ) << std::endl;
                        }
                    }
                    // get index on master
                    switch( tFacet->get_index_on_master() )
                    {
                        case( 0 ) :
                        {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 0 )->get_hmr_facet( 0 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 1 )->get_hmr_facet( 0 ), 1 );
                            break;
                        }
                        case( 1 ) :
                        {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 1 )->get_hmr_facet( 1 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 3 )->get_hmr_facet( 1 ), 1 );
                            break;
                        }
                        case( 2 ) :
                        {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 3 )->get_hmr_facet( 2 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 2 )->get_hmr_facet( 2 ), 1 );
                            break;
                        }
                        case( 3 ) :
                        {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 2 )->get_hmr_facet( 3 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 0 )->get_hmr_facet( 3 ), 1 );
                            break;
                        }
                        default :
                        {
                            MORIS_ERROR( false, "invalid child index" );
                        }
                    }
                }
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::link_facet_children_3d()
        {
            for( Facet * tFacet : mFacets )
            {
                // get master
                Element * tMaster = tFacet->get_hmr_master();

                if( tMaster->is_refined() )
                {
                    // reserve memory for children
                    tFacet->allocate_child_container( 4 );

                    // get index on master
                    switch( tFacet->get_index_on_master() )
                    {
                        case( 0 ) :
                        {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 0 )->get_hmr_facet( 0 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 1 )->get_hmr_facet( 0 ), 1 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 4 )->get_hmr_facet( 0 ), 2 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 5 )->get_hmr_facet( 0 ), 3 );
                            break;
                        }
                        case( 1 ) :
                        {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 1 )->get_hmr_facet( 1 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 3 )->get_hmr_facet( 1 ), 1 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 5 )->get_hmr_facet( 1 ), 2 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 7 )->get_hmr_facet( 1 ), 3 );
                            break;
                        }
                        case( 2 ) :
                        {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 3 )->get_hmr_facet( 2 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 2 )->get_hmr_facet( 2 ), 1 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 7 )->get_hmr_facet( 2 ), 2 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 6 )->get_hmr_facet( 2 ), 3 );
                            break;
                       }
                       case( 3 ) :
                       {
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 2 )->get_hmr_facet( 3 ), 0 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 0 )->get_hmr_facet( 3 ), 1 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 6 )->get_hmr_facet( 3 ), 2 );
                            tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 4 )->get_hmr_facet( 3 ), 3 );
                            break;
                       }
                       case( 4 ) :
                       {
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 2 )->get_hmr_facet( 4 ), 0 );
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 3 )->get_hmr_facet( 4 ), 1 );
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 0 )->get_hmr_facet( 4 ), 2 );
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 1 )->get_hmr_facet( 4 ), 3 );
                           break;
                       }
                       case( 5 ) :
                       {
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 4 )->get_hmr_facet( 5 ), 0 );
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 5 )->get_hmr_facet( 5 ), 1 );
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 6 )->get_hmr_facet( 5 ), 2 );
                           tFacet->insert_child( tMaster->get_child( mAllElementsOnProc, 7 )->get_hmr_facet( 5 ), 3 );
                           break;
                       }
                       default :
                       {
                           MORIS_ERROR( false, "invalid child index" );
                       }
                    }
                }
            }
        } */

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::save_faces_to_vtk( const std::string & aPath )
        {
            if( mFacets.size() > 0 )
            {
                std::string tFilePath = parallelize_path( aPath );

                // open the file
                std::ofstream tFile(tFilePath, std::ios::binary);

                // containers
                float tFChar = 0;
                int   tIChar = 0;

                tFile << "# vtk DataFile Version 3.0" << std::endl;
                tFile << "GO BUFFS!" << std::endl;
                tFile << "BINARY" << std::endl;
                int tNumberOfNodes = mAllBasisOnProc.size();

                // write node data
                tFile << "DATASET UNSTRUCTURED_GRID" << std::endl;

                tFile << "POINTS " << tNumberOfNodes << " float"  << std::endl;

                // ask settings for numner of dimensions
                auto tNumberOfDimensions = mParameters->get_number_of_dimensions();

                if ( tNumberOfDimensions == 2 )
                {
                    // loop over all nodes
                    for ( int k = 0; k < tNumberOfNodes; ++k )
                    {
                        // get coordinate from node
                        const real* tXY = mAllBasisOnProc( k )->get_xyz();

                        // write coordinates to mesh
                        tFChar = swap_byte_endian( (float) tXY[ 0 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) tXY[ 1 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) 0 );
                        tFile.write( (char*) &tFChar, sizeof(float));
                    }
                }
                else if ( tNumberOfDimensions == 3 )
                {
                    // loop over all nodes
                    for ( int k = 0; k < tNumberOfNodes; ++k )
                    {
                        // get coordinate from node
                        const real* tXYZ = mAllBasisOnProc( k )->get_xyz();

                        // write coordinates to mesh
                        tFChar = swap_byte_endian( (float) tXYZ[ 0 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) tXYZ[ 1 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) tXYZ[ 2 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                    }
                }

                tFile << std::endl;

                // get vtk index for edge
                int tCellType = 0;

                if ( mParameters->get_number_of_dimensions() == 2 )
                {
                    switch( this->get_order() )
                    {
                        case( 1 ) :
                        {
                            tCellType = 3;
                            break;
                        }
                        case( 2 ) :
                        {
                            tCellType = 21;
                            break;
                        }
                        case( 3 ) :
                        {
                            tCellType = 35;

                            break;
                        }
                        default :
                        {
                            tCellType = 68;
                            break;
                        }
                    }
                }
                else if( mParameters->get_number_of_dimensions() == 3 )
                {
                    switch( this->get_order() )
                    {
                        case( 1 ) :
                        {
                            tCellType = 9;
                            break;
                        }
                        case( 2 ) :
                        {
                            tCellType = 28;
                            break;
                        }
                        default :
                        {
                            tCellType = 70;
                            break;
                        }
                    }
                }

                // get number of nodes per cell
                int tNumberOfNodesPerElement = mFacets( 0 )->get_vertex_ids().length();

                // value to write in VTK file
                int tNumberOfNodesVTK = swap_byte_endian( (int) tNumberOfNodesPerElement );

                // get number of faces
                int tNumberOfElements = mFacets.size();

                // write header for cells
                tFile << "CELLS " << tNumberOfElements << " "
                        << ( tNumberOfNodesPerElement + 1 )*tNumberOfElements  << std::endl;

                // loop over all faces
                for( Facet * tFacet : mFacets )
                {
                    tFile.write( (char*) &tNumberOfNodesVTK, sizeof(int) );

                    // loop over all nodes of this element
                    for( int k=0; k<tNumberOfNodesPerElement; ++k )
                    {
                        // write node to mesh file
                        tIChar = swap_byte_endian( ( int ) tFacet->get_basis( k )->get_memory_index() );
                        tFile.write((char *) &tIChar, sizeof(int));
                    }
                }

                // write cell types
                tFile << "CELL_TYPES " << tNumberOfElements << std::endl;
                tIChar = swap_byte_endian( tCellType );
                for ( int k = 0; k < tNumberOfElements; ++k)
                {
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write element data
                tFile << "CELL_DATA " << tNumberOfElements << std::endl;

                // write face ID
                tFile << "SCALARS FACET_ID int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Facet * tFacet : mFacets )
                {
                    tIChar = swap_byte_endian( (int) tFacet->get_id() );
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write face index
                tFile << "SCALARS FACET_INDEX int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Facet * tFacet : mFacets )
                {
                    tIChar = swap_byte_endian( (int) tFacet->get_index() );
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write owner
                tFile << "SCALARS FACET_OWNER int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Facet * tFacet : mFacets )
                {
                    tIChar = swap_byte_endian( (int) tFacet->get_owner() );
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write level
                tFile << "SCALARS FACET_LEVEL int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Facet * tFacet : mFacets )
                {
                    tIChar = swap_byte_endian( (int) tFacet->get_hmr_master()
                            ->get_background_element()->get_level() );

                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write level
                tFile << "SCALARS FACET_STATE int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Facet * tFacet : mFacets )
                {
                    if( tFacet->is_active() )
                    {
                        tIChar = swap_byte_endian( (int) 1 );
                    }
                    else
                    {
                        tIChar = swap_byte_endian( (int) 0 );
                    }
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write node data
                tFile << "POINT_DATA " << tNumberOfNodes << std::endl;

                tFile << "SCALARS NODE_ID int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for ( int k = 0; k <  tNumberOfNodes; ++k)
                {

                    tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_id() );
                    tFile.write( (char*) &tIChar, sizeof(float));
                }
                tFile << std::endl;

                tFile << "SCALARS NODE_INDEX int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for ( int k = 0; k <  tNumberOfNodes; ++k)
                {

                    tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_index() );
                    tFile.write( (char*) &tIChar, sizeof(float));
                }
                tFile << std::endl;

                // close the output file
                tFile.close();
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::save_edges_to_vtk( const std::string & aPath )
        {
            if( mFacets.size() > 0 )
            {
                std::string tFilePath = parallelize_path( aPath );

                // open the file
                std::ofstream tFile(tFilePath, std::ios::binary);

                // containers
                float tFChar = 0;
                int   tIChar = 0;

                tFile << "# vtk DataFile Version 3.0" << std::endl;
                tFile << "GO BUFFS!" << std::endl;
                tFile << "BINARY" << std::endl;
                int tNumberOfNodes = mAllBasisOnProc.size();

                // write node data
                tFile << "DATASET UNSTRUCTURED_GRID" << std::endl;

                tFile << "POINTS " << tNumberOfNodes << " float"  << std::endl;

                // ask settings for numner of dimensions
                auto tNumberOfDimensions = mParameters->get_number_of_dimensions();

                if ( tNumberOfDimensions == 2 )
                {
                    // loop over all nodes
                    for ( int k = 0; k < tNumberOfNodes; ++k )
                    {
                        // get coordinate from node
                        const real* tXY = mAllBasisOnProc( k )->get_xyz();

                        // write coordinates to mesh
                        tFChar = swap_byte_endian( (float) tXY[ 0 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) tXY[ 1 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) 0 );
                        tFile.write( (char*) &tFChar, sizeof(float));
                    }
                }
                else if ( tNumberOfDimensions == 3 )
                {
                    // loop over all nodes
                    for ( int k = 0; k < tNumberOfNodes; ++k )
                    {
                        // get coordinate from node
                        const real* tXYZ = mAllBasisOnProc( k )->get_xyz();

                        // write coordinates to mesh
                        tFChar = swap_byte_endian( (float) tXYZ[ 0 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) tXYZ[ 1 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                        tFChar = swap_byte_endian( (float) tXYZ[ 2 ] );
                        tFile.write( (char*) &tFChar, sizeof(float));
                    }
                }

                tFile << std::endl;

                // get vtk index for edge
                int tCellType = 0;

                switch( this->get_order() )
                {
                case( 1 ) :
                                {
                    tCellType = 3;
                    break;
                                }
                case( 2 ) :
                                {
                    tCellType = 21;
                    break;
                                }
                case( 3 ) :
                                {
                    tCellType = 35;

                    break;
                                }
                default :
                {
                    tCellType = 68;
                    break;
                }
                }

                // get number of nodes per cell
                int tNumberOfNodesPerElement = mEdges( 0 )->get_vertex_ids().length();

                // value to write in VTK file
                int tNumberOfNodesVTK = swap_byte_endian( (int) tNumberOfNodesPerElement );

                // get number of faces
                int tNumberOfElements = mEdges.size();

                // write header for cells
                tFile << "CELLS " << tNumberOfElements << " "
                        << ( tNumberOfNodesPerElement + 1 )*tNumberOfElements  << std::endl;

                // loop over all faces
                for( Edge * tEdge : mEdges )
                {
                    tFile.write( (char*) &tNumberOfNodesVTK, sizeof(int) );

                    // loop over all nodes of this element
                    for( int k=0; k<tNumberOfNodesPerElement; ++k )
                    {
                        // write node to mesh file
                        tIChar = swap_byte_endian( ( int ) tEdge->get_basis( k )->get_memory_index() );
                        tFile.write((char *) &tIChar, sizeof(int));
                    }
                }

                // write cell types
                tFile << "CELL_TYPES " << tNumberOfElements << std::endl;
                tIChar = swap_byte_endian( tCellType );
                for ( int k = 0; k < tNumberOfElements; ++k)
                {
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write element data
                tFile << "CELL_DATA " << tNumberOfElements << std::endl;

                // write element ID
                tFile << "SCALARS EDGE_ID int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Edge * tEdge : mEdges )
                {
                    tIChar = swap_byte_endian( (int) tEdge->get_id() );
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                tFile << "SCALARS EDGE_INDEX int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Edge * tEdge : mEdges )
                {
                    tIChar = swap_byte_endian( (int) tEdge->get_index() );
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write owner
                tFile << "SCALARS EDGE_OWNER int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Edge * tEdge : mEdges )
                {
                    tIChar = swap_byte_endian( (int) tEdge->get_owner() );
                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write level
                tFile << "SCALARS EDGE_LEVEL int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for( Edge * tEdge : mEdges )
                {
                    tIChar = swap_byte_endian( (int) tEdge->get_hmr_master()
                            ->get_background_element()->get_level() );

                    tFile.write( (char*) &tIChar, sizeof(int));
                }
                tFile << std::endl;

                // write level
                /* tFile << "SCALARS FACET_STATE int" << std::endl;
                        tFile << "LOOKUP_TABLE default" << std::endl;
                        for( Facet * tFacet : mFacets )
                        {
                            if( tFacet->is_active() )
                            {
                                tIChar = swap_byte_endian( (int) 1 );
                            }
                            else
                            {
                                tIChar = swap_byte_endian( (int) 0 );
                            }
                            tFile.write( (char*) &tIChar, sizeof(int));
                        }
                        tFile << std::endl; */

                // write node data
                tFile << "POINT_DATA " << tNumberOfNodes << std::endl;

                tFile << "SCALARS NODE_ID int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for ( int k = 0; k <  tNumberOfNodes; ++k)
                {

                    tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_id() );
                    tFile.write( (char*) &tIChar, sizeof(float));
                }
                tFile << std::endl;

                tFile << "SCALARS NODE_INDEX int" << std::endl;
                tFile << "LOOKUP_TABLE default" << std::endl;
                for ( int k = 0; k <  tNumberOfNodes; ++k)
                {

                    tIChar = swap_byte_endian( (int) mAllBasisOnProc( k )->get_index() );
                    tFile.write( (char*) &tIChar, sizeof(float));
                }
                tFile << std::endl;

                // close the output file
                tFile.close();
            }
        }

//------------------------------------------------------------------------------

        void
        Lagrange_Mesh_Base::save_coeffs_to_binary_file( const std::string & aFilePath )
        {
            // start timer
            tic tTimer;

            // make path parallel
            std::string tFilePath = parallelize_path( aFilePath );

            // Step 1: determine size of output matrix
            uint tCount = 0;

            // get number of nodes from mesh
            uint tNumberOfNodes = this->get_number_of_nodes_on_proc();

            // increment counter: first entry is number of nodes
            ++tCount;

            // loop over all nodes
            for( uint k=0; k<tNumberOfNodes; ++k )
            {
                // get pointer to node
                mtk::Vertex * tNode = this->get_node_by_index( k );

                // increment counter for node ID, node index and number of coeffs
                // + 2*number of coefficients
                tCount += 3 + 2*tNode->get_interpolation()->get_number_of_coefficients();

            }

            // Step 2: allocate output matrix and populate it with data

            // allocate output matrix
            Matrix< DDRMat > tOutput( tCount, 1 );

            // reset counter
            tCount = 0;

            // write number of nodes
            tOutput( tCount++ ) = tNumberOfNodes;

            // loop over all nodes
            for( uint k=0; k<tNumberOfNodes; ++k )
            {
                // get pointer to node
                mtk::Vertex * tNode = this->get_node_by_index( k );

                // write node Index to matrix
                tOutput( tCount++ ) = tNode->get_index();

                // write node ID to matrix
                tOutput( tCount++ ) = tNode->get_id();

                // get number of coeffs
                uint tNumberOfCoeffs = tNode
                        ->get_interpolation()->get_number_of_coefficients();

                // write number of coeffs to matrix
                tOutput( tCount++ ) = tNumberOfCoeffs;

                // get IDs
                Matrix< IdMat >  tIDs = tNode ->get_interpolation()->get_ids();

                // get weights
                const Matrix< DDRMat > & tWeights = *tNode->get_interpolation()->get_weights();

                // loop over all coeffs and write dof ids
                for( uint i=0; i<tNumberOfCoeffs; ++i )
                {
                    tOutput( tCount++ ) =  tIDs( i );
                }

                // loop over all coeffs and write weights
                for( uint i=0; i<tNumberOfCoeffs; ++i )
                {
                    tOutput( tCount++ ) =  tWeights( i );
                }
            }

            MORIS_ASSERT( tCount = tOutput.length(), "Something went wrong while writing coeffs to file." );

            // step 3: store output matrix into file
            save_matrix_to_binary_file( tOutput, tFilePath );

            if ( mParameters->is_verbose() )
            {
                // stop timer
                real tElapsedTime = tTimer.toc<moris::chronos::milliseconds>().wall;

                // print output
                std::fprintf( stdout,"%s Saved coefficients to binary file:\n               %s.\n               Saving took %5.3f seconds.\n\n",
                        proc_string().c_str(),
                        tFilePath.c_str(),
                        ( double ) tElapsedTime / 1000 );
            }

        }

//------------------------------------------------------------------------------
    } /* namespace hmr */
} /* namespace moris */
