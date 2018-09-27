/*
 * clHMRElement.hpp
 *
 *  Created on: May 1, 2018
 *      Author: messe
 */

#ifndef SRC_HMR_CL_HMR_BACKGROUND_ELEMENT_HPP_
#define SRC_HMR_CL_HMR_BACKGROUND_ELEMENT_HPP_

#include "cl_HMR_Background_Facet.hpp"
#include "cl_HMR_Background_Element_Base.hpp"
#include "cl_HMR_Background_Edge.hpp"
#include "typedefs.hpp" //COR/src
#include "cl_Cell.hpp" //CON/src
#include "cl_Bitset.hpp" //CON/src
#include "cl_Matrix.hpp" //LINALG/src
#include "linalg_typedefs.hpp" //LINALG/src

namespace moris
{
    namespace hmr
    {
        /**
         * \brief Element templated against
         *
         * uint N: number of dimensions (1, 2, or 3)
         * uint C: number of children   (2^N)
         * uint B: number of neighbors  (N^N-1)
         * uint F: number of faces      ( 2*N )
         * uine E: number of edges      ( 3D: 12, 1D: 0 )
         *
         */
        template< uint N, uint C, uint B, uint F , uint E >
        class Background_Element : public Background_Element_Base
        {
//--------------------------------------------------------------------------------
        private:
//--------------------------------------------------------------------------------

                  //! Fixed size array containing children.
                  //! If the element has no children, it contains null pointers
                  Background_Element_Base* mChildren[ C ] = { nullptr };

                  //! bitset defining the child index
                  Bitset< N >              mChildBitset;

                  //! Fixed size array containing neighbors on same level
                  //! Active or refined.
                  Background_Element_Base* mNeighbors[ B ] = { nullptr };

                  //! local ijk position on proc
                  luint                    mIJK[ N ];

                  //! faces for this element
                  Background_Facet *       mFacets[ F ] = { nullptr };

                  //! owner bitset
                  Bitset< F >              mFacetOwnFlags;

                  //! edges for this element
                  Background_Edge **       mEdges;

                  //! edges bitset
                  Bitset< E >              mEdgeOwnFlags;

//--------------------------------------------------------------------------------
        public:
//--------------------------------------------------------------------------------

             /**
              * Default element constructor
              *
              * @param[in]  aParent       Pointer to parent of element,
              *                           null pointer if element is on level zero.
              *
              * @param[in]  aIJK          Pointer to IJK position of element.
              * @param[in]  aID           Global ID of element.
              * @param[in]  aSubDomainID  Proc local subdomain ID of element.
              * @param[in]  aLevel        Level of element.
              * @param[in]  aChildIndex   Tells which child number this element is.
              *                           Pass UINT_MAX if element is on level zero
              * @param[in]  aOwner        For zero level elements, pass UINT_MAX,
              *                           owner is determined later. For all higher
              *                           levels, pass owner of parent.
              */
            Background_Element(
                    Background_Element_Base * aParent,
                    const  uint        & aActivePattern,
                    const luint        * aIJK,
                    const luint        & aID,
                    const uint         & aLevel,
                    const uint         & aChildIndex,
                    const uint         & aOwner ) :
                        Background_Element_Base(
                             aParent,
                             aActivePattern,
                             aID,
                             aLevel,
                             aOwner )
            {

                // write child index into bitset
                this->set_child_index( aChildIndex );

                // save ijk position in memory.
                // Needed to calculate ijk of children.
                for( uint k=0; k<N; ++k )
                {
                    mIJK[ k ] = aIJK[ k ];
                }

                // reset owner flags
                mFacetOwnFlags.reset();
                if( E > 0 )
                {
                    // assign memory for edge container and fill with null pointers
                    mEdges = new Background_Edge* [ E ]{};

                    // reset owner flags
                    mEdgeOwnFlags.reset();
                }
            }

//--------------------------------------------------------------------------------

            /**
             * Default element destructor.
             * Deletes children if children flag is set.
             */
            ~Background_Element()
            {
                if ( mChildrenFlag )
                {
                    for ( auto p:  mChildren )
                    {
                        delete p;
                    }
                }

                // delete faces
                for( uint f=0; f<F; ++f )
                {
                    // test if face is owned
                    if ( mFacetOwnFlags.test( f ) )
                    {
                        // delete face pointer
                        delete mFacets[ f ];
                    }
                }

                // delete edges
                if ( E > 0 )
                {
                    // loop over all edges
                    for( uint e=0; e<E; ++e )
                    {
                        // test if edge is owned
                        if( mEdgeOwnFlags.test( e ) )
                        {
                            // delete edge pointer
                            delete mEdges[ e ];
                        }
                    }

                    // delete edge container
                    delete [] mEdges;
                }
            }
//--------------------------------------------------------------------------------

            /**
             * Returns an array of size [N] telling the proc local ijk-position
             * of the element on the current level.
             *
             * @return luint pointer to array containing ijk-position
             *               careful: element must not go out of scope.
             */
            const luint *
            get_ijk( ) const
            {
                return mIJK;
            }

//--------------------------------------------------------------------------------

            /**
             * This function is called by the background mesh during refinement.
             * The child pointer is put into the member array mChildren.
             *
             * @param[in] aChild   pointer of child to be added
             *
             * @return void
             */
            void
            insert_child(  Background_Element_Base* aChild )
            {
                mChildrenFlag = true;
                mChildren[ aChild->get_child_index() ] = aChild;
            }

//--------------------------------------------------------------------------------

            /**
             * This function is needed by the background mesh during refinement.
             * The ijk-position of a child is needed to calculate the local and
             * global ID of the child element.
             *
             * param[ out ]  Matrix< DDUMat > of size <number of dimensions>
             *                                *< number of children >
             * @return void
             */
            void
            get_ijk_of_children( Matrix< DDLUMat > & aIJK ) const;

//--------------------------------------------------------------------------------

            /**
             * Recursive function that counts all active descendants of an element.
             * If the element is not refined, the function returns one, not zero.
             * This function is needed by the background mesh in order
             * to determine the size of mActiveElements.
             *
             * @param[inout] aCount   Counter to be incremented
             *
             * @return void
             */
            void
            get_number_of_active_descendants( const uint & aPattern, luint & aCount ) const;

//--------------------------------------------------------------------------------
            /**
             * Recursive function that counts all descendants of an element plus
             * the element itself.
             *
             * param[inout] aCount   Counter to be incremented
             *
             * @return void
             */
            void
            get_number_of_descendants( luint & aCount ) const;

//--------------------------------------------------------------------------------

            /**
             * To be called after the cell aElementList has been allocated
             * to the size given by  get_number_of_descendants().
             * Returns an array that consists all related elements, including
             * the element itself.
             *
             * @param[inout] aElementList  cell to which the pointers are added
             * @param[inout] aCount   Counter to be incremented
             *
             * @return void
             */
            void
            collect_descendants(
                    Cell< Background_Element_Base* > & aElementList,
                    luint                            & aElementCount );

//--------------------------------------------------------------------------------

            /**
             * To be called after the cell aElementList has been allocated
             * to the size given by  get_number_of_active_descendants().
             * Needed by the background mesh to update mActiveElements.
             *
             * @param[inout] aElementList  cell to which the pointers are added
             * @param[inout] aCount   Counter to be incremented
             *
             * @return void
             *
             */
            void
            collect_active_descendants(
                    const uint                       & aPattern,
                    Cell< Background_Element_Base *> & aElementList,
                    luint                            & aElementCount );
//--------------------------------------------------------------------------------

            /**
             * Returns a pointer to a child of an element. If the element
             * has no children, a null pointer will be returned.
             *
             * @param[in] aIndex      Index of requested child (2D: 0-3, 3D: 0-7)
             * @return Background_Element_Base*  pointer to selected child
             */
            Background_Element_Base*
            get_child( const uint& aIndex )
            {
                return mChildren[ aIndex ];
            }

//--------------------------------------------------------------------------------

            /**
             * This function is called by the background mesh.
             * The pointer of the neighbor is put into the member array mNeighbors.
             *
             * @param[in] aIndex      number of neighbor
             * @param[in] aNeighbor   pointer of neighbor to be added
             *
             * @return void
             */
            void
            insert_neighbor(
                    const uint              & aIndex,
                    Background_Element_Base * aNeighbor )
            {
                mNeighbors[ aIndex ] = aNeighbor;
            }

//--------------------------------------------------------------------------------

            /**
             * Returns a pointer to a neighbor of an element.
             *
             * @param[ in ] aNeighborIndex       index of requested neighbor
             * @return Background_Element_Base*  pointer to requested neighbor
             */
            Background_Element_Base*
            get_neighbor( const uint & aIndex )
            {
                return mNeighbors[ aIndex ];
            }

//--------------------------------------------------------------------------------

            /**
             * Recursive function that loops up to a specified level and counts
             * active and refined elements on that level.
             *
             * @param[in]     aLevel    level to be considered
             * @param[inout]  aElementCount    counter for elements
             */
            void
            count_elements_on_level( const uint& aLevel, luint& aElementCount );
//--------------------------------------------------------------------------------

            /**
             * Recursive function that loops up to a specified level and collects
             * active and refined elements on that level.
             *
             * @param[in]     aLevel          level to be considered
             * @param[inout]  aElementList    cell to which the pointers are added
             * @param[inout]  aElementCount   counter for elements
             */
            void
            collect_elements_on_level(
                    const uint                       & aLevel,
                    Cell< Background_Element_Base* > & aElementList,
                    luint                            & aElementCount );
//--------------------------------------------------------------------------------

            void
            collect_neighbors( const uint & aPattern );

//--------------------------------------------------------------------------------

            /**
             * Function for debugging that prints all neighbors of the element
             * to the screen.
             *
             * @return void
             *
             */
            void
            print_neighbors( const uint & aPattern )
            {
                // print header
                std::fprintf( stdout, "\n Neighbors of Element %lu ( ID: %lu, mem:  %lu, child: %u ): \n\n",
                              ( long unsigned int ) mDomainIndex,
                              ( long unsigned int ) mDomainID,
                              ( long unsigned int ) mMemoryIndex,
                              ( unsigned int )      this->get_child_index() );
                for( uint k=0; k<B; ++k )
                {
                    // test if neighbor exists
                    if( mNeighbors[ k ] )
                    {
                        // get id of neighbor
                        long unsigned int tID = mNeighbors[ k ]->get_domain_id();

                        // get active flag
                        int tActive = mNeighbors[ k ]->is_active( aPattern );

                        // print index and id of neighbor
                        std::fprintf( stdout, "    %2u   id: %5lu     a: %d\n",
                                (unsigned int) k, (long unsigned int) tID, tActive);
                    }
                    else
                    {
                        std::fprintf( stdout, "    %2u  null \n",  (unsigned int) k );
                    }
                }

                // print blank line
                std::fprintf( stdout, "\n" );
            }


//--------------------------------------------------------------------------------

            /**
             * Tells which child number an element has.
             * This information is for example needed for the T-Matrix.
             *
             * @return uint    index between 0 and 3 (2D) or 0 and 7 (3D)
             */
            uint
            get_child_index() const
            {
                return mChildBitset.to_ulong();
            }

//--------------------------------------------------------------------------------

            /**
             * Test a bit in the child index bitset
             *
             * @param[in]    aBit    index of bit to test
             *
             * @return bool
             */
            bool
            test_child_index_bit( const uint & aBit ) const
            {
                return mChildBitset.test( aBit );
            }

//--------------------------------------------------------------------------------

            /**
             * creates a bitset that describes the pedigree path
             *
             */
            void
            endcode_pedigree_path(
                    luint        & aAncestorID,
                    Matrix< DDUMat >  & aPedigreeList,
                    luint        & aCounter )
            {

                // create bitset
                Bitset< gBitsetSize > tBitset;

                // clear bitset
                tBitset.reset();

                // get pointer to element
                Background_Element_Base* tParent = this;

                // get level of this element
                uint tLevel = this->get_level();

                // copy level into list
                aPedigreeList( aCounter++ ) = tLevel;

                if ( tLevel > 0 )
                {
                    uint tMax = tLevel * N -1;

                    // Position in bitset
                    uint tPivot = tMax;

                    // reset output bitset
                    tBitset.reset();

                    // loop over all levels and calculate index
                    for( uint k=tLevel; k>0; --k )
                    {
                        for( int i=N-1; i>=0; --i ) // do not use uint here
                        {
                            if ( tParent->test_child_index_bit( i ) )
                            {
                                // set this bit
                                tBitset.set( tPivot );
                            }

                            // decrement pivot
                            --tPivot;
                        }
                        // get next element
                        tParent = tParent->get_parent();
                    }

                    // reset pivot
                    tPivot = 0;

                    while ( tPivot <= tMax )
                    {
                        Bitset< 32 > tChar;
                        for( uint k=0; k<32; ++k )
                        {
                            if( tBitset.test( tPivot++ ) )
                            {
                                tChar.set( k );
                            }
                            if ( tPivot > tMax )
                            {
                                break;
                            }
                        }

                        // copy character into output list
                        aPedigreeList( aCounter++ ) = ( uint ) tChar.to_ulong();
                    }
                }

                // copy domain ID from ancestor
                aAncestorID = tParent->get_domain_id();
            }

//--------------------------------------------------------------------------------

            luint
            get_length_of_pedigree_path()
            {
                return ceil( ( ( real ) ( this->get_level()*N ) ) / 32 ) + 1;
            }

//--------------------------------------------------------------------------------

            /**
             * Returns a cell with pointers to elements on the same level,
             * if they exist.
             *
             * @param[ in  ] aOrder       degree of neighborship
             * @param[ out ] aNeighbors   cell containing found neighbors
             */
            void
            get_neighbors_from_same_level(
                    const uint                        & aOrder,
                    Cell< Background_Element_Base * > & aNeighbors );

//--------------------------------------------------------------------------------

            /**
             * create the faces of this element
             */
            void
            create_facets()
            {

                // loop over all faces
                for( uint f=0; f<F; ++f )
                {

                    // test if facet has not been created yet
                    if ( mFacets[ f ] == NULL )
                    {
                        // set flag that this element is responsible for
                        // deleting this face
                        mFacetOwnFlags.set( f );

                        // test if this element is on the edge of the domain
                        if( mNeighbors[ f ] == NULL )
                        {
                            // this facet has now proc owner, I am master
                            // create face
                            mFacets[ f ] = new Background_Facet( this, f, gNoProcID );
                        }
                        else if( mNeighbors[ f ]->get_level() != mLevel )
                        {
                            // this element belongs to the creator
                            mFacets[ f ] = new Background_Facet( this, f, mOwner );
                        }
                        else
                        {
                            // element picks owner with lower domain_index
                            mFacets[ f ] = new Background_Facet( this, mNeighbors[ f ], f  );

                            // insert element into neighbor
                            mNeighbors[ f ]->insert_facet(
                                    mFacets[ f ],
                                    mFacets[ f ]->get_index_on_other( f ) );
                        }
                    }
                } // end loop over all faces
            }

//--------------------------------------------------------------------------------

            /**
             * creates the edges ( 3D only )
             */
            void
            create_edges();

//--------------------------------------------------------------------------------

            /**
             * reset the flags of the faces
             */
            void
            reset_flags_of_facets();

//--------------------------------------------------------------------------------

            /**
             * returns a face of the background element
             */
            Background_Facet *
            get_facet( const uint & aIndex )
            {
                return mFacets[ aIndex ];
            }

//--------------------------------------------------------------------------------

            /**
             * inserts a face into the background element
             */
            void
            insert_facet( Background_Facet * aFace, const uint & aIndex )
            {
                // copy face to slot
                mFacets[ aIndex ] = aFace;
            }

//--------------------------------------------------------------------------------

            /**
             * returns an edge of the background element ( 3D only )
             */
            Background_Edge *
            get_edge( const uint & aIndex );

//--------------------------------------------------------------------------------

            void
            insert_edge(  Background_Edge * aEdge, const uint & aIndex );

//-------------------------------------------------------------------------------

            void
            reset_flags_of_edges();

//--------------------------------------------------------------------------------
        private:
//--------------------------------------------------------------------------------


            void
            set_child_index( const uint & aIndex );

//--------------------------------------------------------------------------------
        };
//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::get_neighbors_from_same_level(
                const uint                        & aOrder,
                Cell< Background_Element_Base * > & aNeighbors )
        {
            MORIS_ERROR( false, "Don't know how search neighbors on same level.");
        }

//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::set_child_index( const uint & aIndex )
        {
            MORIS_ERROR( false, "Don't know how to set child index.");
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 2, 4, 8, 4, 0 >::set_child_index( const uint & aIndex )
        {
            switch( aIndex )
            {
                case( 0 ) :
                {
                    mChildBitset.reset( 0 );
                    mChildBitset.reset( 1 );
                    break;
                }
                case( 1 ) :
                {
                    mChildBitset.set( 0 );
                    mChildBitset.reset( 1 );
                    break;
                }

                case( 2 ) :
                {
                    mChildBitset.reset( 0 );
                    mChildBitset.set( 1 );
                    break;
                }

                case( 3 ) :
                {
                    mChildBitset.set( 0 );
                    mChildBitset.set( 1 );
                    break;
                }
                default :
                {
                    MORIS_ERROR( false, "Invalid child index.");
                    break;
                }
            }
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 3, 8, 26, 6, 12 >::set_child_index( const uint & aIndex )
        {
            switch( aIndex )
            {
                case( 0 ) :
                {
                    mChildBitset.reset( 0 );
                    mChildBitset.reset( 1 );
                    mChildBitset.reset( 2 );
                    break;
                }
                case( 1 ) :
                {
                    mChildBitset.set( 0 );
                    mChildBitset.reset( 1 );
                    mChildBitset.reset( 2 );
                    break;
                }
                case( 2 ) :
                {
                    mChildBitset.reset( 0 );
                    mChildBitset.set( 1 );
                    mChildBitset.reset( 2 );
                    break;
                }
                case( 3 ) :
                {
                    mChildBitset.set( 0 );
                    mChildBitset.set( 1 );
                    mChildBitset.reset( 2 );
                    break;
                }
                case( 4 ) :
                {
                    mChildBitset.reset( 0 );
                    mChildBitset.reset( 1 );
                    mChildBitset.set( 2 );
                    break;
                }
                case( 5 ) :
                {
                    mChildBitset.set( 0 );
                    mChildBitset.reset( 1 );
                    mChildBitset.set( 2 );
                    break;
                }
                case( 6 ) :
                {
                    mChildBitset.reset( 0 );
                    mChildBitset.set( 1 );
                    mChildBitset.set( 2 );
                    break;
                }
                case( 7 ) :
                {
                    mChildBitset.set( 0 );
                    mChildBitset.set( 1 );
                    mChildBitset.set( 2 );
                    break;
                }
                default :
                {
                    MORIS_ERROR( false, "Invalid child index.");
                    break;
                }
            }
        }

//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::get_ijk_of_children(
                Matrix< DDLUMat > & aIJK ) const
        {
            MORIS_ERROR( false, "Don't know how to calculate ijk of children.");
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        template <>
        void
        Background_Element< 1, 2, 2, 2, 0 >::get_ijk_of_children(
                Matrix< DDLUMat > & aIJK ) const
        {
            // set size of IJK output
            aIJK.set_size( 1, 2 );

            // child 0
            aIJK( 0, 0 ) = 2*mIJK[0];

            // child 1
            aIJK( 0, 1 ) = aIJK( 0, 0 ) + 1;
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 2, 4, 8, 4, 0 >::get_ijk_of_children(
                Matrix< DDLUMat > & aIJK ) const
        {
            // set size of IJK output
            aIJK.set_size( 2, 4 );

            // child 0
            aIJK( 0, 0 ) = 2*mIJK[0];
            aIJK( 1, 0 ) = 2*mIJK[1];

            // child 1
            aIJK( 0, 1 ) = aIJK( 0, 0 ) + 1;
            aIJK( 1, 1 ) = aIJK( 1, 0 );

            // child 2
            aIJK( 0, 2 ) = aIJK( 0, 0 );
            aIJK( 1, 2 ) = aIJK( 1, 0 ) + 1;

            // child 3
            aIJK( 0, 3 ) = aIJK( 0, 1 );
            aIJK( 1, 3 ) = aIJK( 1, 2 );
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 3, 8, 26, 6, 12 >::get_ijk_of_children(
                Matrix< DDLUMat > & aIJK ) const
        {
            // set size of IJK output
            aIJK.set_size( 3, 8 );

            // child 0
            aIJK( 0, 0 ) = 2*mIJK[0];
            aIJK( 1, 0 ) = 2*mIJK[1];
            aIJK( 2, 0 ) = 2*mIJK[2];

            // child 1
            aIJK( 0, 1 ) = aIJK( 0, 0 ) + 1;
            aIJK( 1, 1 ) = aIJK( 1, 0 );
            aIJK( 2, 1 ) = aIJK( 2, 0 );

            // child 2
            aIJK( 0, 2 ) = aIJK( 0, 0 );
            aIJK( 1, 2 ) = aIJK( 1, 0 ) + 1;
            aIJK( 2, 2 ) = aIJK( 2, 0 );

            // child 3
            aIJK( 0, 3 ) = aIJK( 0, 1 );
            aIJK( 1, 3 ) = aIJK( 1, 2 );
            aIJK( 2, 3 ) = aIJK( 2, 0 );

            // child 4
            aIJK( 0, 4 ) = aIJK( 0, 0 );
            aIJK( 1, 4 ) = aIJK( 1, 0 );
            aIJK( 2, 4 ) = aIJK( 2, 0 ) + 1;

            // child 5
            aIJK( 0, 5 ) = aIJK( 0, 0 ) + 1;
            aIJK( 1, 5 ) = aIJK( 1, 0 );
            aIJK( 2, 5 ) = aIJK( 2, 4 );

            // child 6
            aIJK( 0, 6 ) = aIJK( 0, 0 );
            aIJK( 1, 6 ) = aIJK( 1, 0 ) + 1;
            aIJK( 2, 6 ) = aIJK( 2, 4 );

            // child 7
            aIJK( 0, 7 ) = aIJK( 0, 1 );
            aIJK( 1, 7 ) = aIJK( 1, 2 );
            aIJK( 2, 7 ) = aIJK( 2, 4 );
        }
//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::get_number_of_active_descendants(
                const  uint & aPattern,
                      luint & aCount ) const
        {
            MORIS_ERROR( false, "Don't know how to count active descendants.");
        }

 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 1, 2, 2, 2, 0 >::get_number_of_active_descendants(
                const  uint & aPattern,
                      luint & aCount ) const
        {
            // test if element is active
            if ( mActiveFlags.test( aPattern ) )
            {
                // increment counter
                ++aCount ;
            }
            else if( mChildrenFlag )
            {
                // ask children to increment counter
                mChildren[ 0 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 1 ]->get_number_of_active_descendants( aPattern, aCount );
            }
        }

 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 2, 4, 8, 4, 0 >::get_number_of_active_descendants(
                const  uint & aPattern,
                      luint & aCount ) const
        {
            // test if element is active
            if ( mActiveFlags.test( aPattern ) )
            {
                // increment counter
                ++aCount ;
            }
            else if( mChildrenFlag )
            {
                // ask children to increment counter
                mChildren[ 0 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 1 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 2 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 3 ]->get_number_of_active_descendants( aPattern, aCount );
            }
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 3, 8, 26, 6, 12 >::get_number_of_active_descendants(
                const  uint & aPattern,
                      luint & aCount ) const
        {
            // test if element is active
            if ( mActiveFlags.test( aPattern ) )
            {
                // increment counter
                ++aCount ;
            }
            else if( mChildrenFlag )
            {
                // ask children to increment counter
                mChildren[ 0 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 1 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 2 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 3 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 4 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 5 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 6 ]->get_number_of_active_descendants( aPattern, aCount );
                mChildren[ 7 ]->get_number_of_active_descendants( aPattern, aCount );
            }
        }

//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::get_number_of_descendants(
                luint & aCount ) const
        {
            MORIS_ERROR( false, "Don't know how to count descendants.");
        }

 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 1, 2, 2, 2, 0 >::get_number_of_descendants(
                luint & aCount ) const
        {
            // add self to counter
            ++aCount;

            // test if children exist
            if ( mChildrenFlag )
            {
                // ask children to increment counter
                mChildren[ 0 ]->get_number_of_descendants( aCount );
                mChildren[ 1 ]->get_number_of_descendants( aCount );
            }
        }

 // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 2, 4, 8, 4, 0 >::get_number_of_descendants(
                luint & aCount ) const
        {
            // add self to counter
            ++aCount;

            // test if children exist
            if ( mChildrenFlag )
            {
                // ask children to increment counter
                mChildren[ 0 ]->get_number_of_descendants( aCount );
                mChildren[ 1 ]->get_number_of_descendants( aCount );
                mChildren[ 2 ]->get_number_of_descendants( aCount );
                mChildren[ 3 ]->get_number_of_descendants( aCount );
            }
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 3, 8, 26, 6, 12 >::get_number_of_descendants(
                luint & aCount ) const
        {
            // add self to counter
            ++aCount ;

            // test if children exist
            if ( mChildrenFlag )
            {
                // ask children to increment counter
                mChildren[ 0 ]->get_number_of_descendants( aCount );
                mChildren[ 1 ]->get_number_of_descendants( aCount );
                mChildren[ 2 ]->get_number_of_descendants( aCount );
                mChildren[ 3 ]->get_number_of_descendants( aCount );
                mChildren[ 4 ]->get_number_of_descendants( aCount );
                mChildren[ 5 ]->get_number_of_descendants( aCount );
                mChildren[ 6 ]->get_number_of_descendants( aCount );
                mChildren[ 7 ]->get_number_of_descendants( aCount );
            }
        }

//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::collect_descendants(
                Cell< Background_Element_Base* > & aElementList,
                luint                            & aElementCount )
        {
            MORIS_ERROR( false, "Don't know how to collect descendants.");
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 1, 2, 2, 2, 0 >::collect_descendants(
                Cell< Background_Element_Base* > & aElementList,
                luint                            & aElementCount )
        {
            // add self to list
            aElementList( aElementCount++ ) = this;

            // test if children exist
            if ( mChildrenFlag )
            {
                // add children to list
                mChildren[ 0 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 1 ]->collect_descendants( aElementList, aElementCount );
            }
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 2, 4, 8, 4, 0 >::collect_descendants(
                Cell< Background_Element_Base* > & aElementList,
                luint                            & aElementCount )
        {
            // add self to list
            aElementList( aElementCount++ ) = this;

            // test if children exist
            if ( mChildrenFlag )
            {
                // add children to list
                mChildren[ 0 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 1 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 2 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 3 ]->collect_descendants( aElementList, aElementCount );
            }
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 3, 8, 26, 6, 12 >::collect_descendants(
                Cell< Background_Element_Base* > & aElementList,
                luint                            & aElementCount )
        {
            // add self to list
            aElementList( aElementCount++ ) = this;

            // test if children exist
            if ( mChildrenFlag )
            {
                // add children to list
                mChildren[ 0 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 1 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 2 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 3 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 4 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 5 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 6 ]->collect_descendants( aElementList, aElementCount );
                mChildren[ 7 ]->collect_descendants( aElementList, aElementCount );
            }
        }

//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::collect_active_descendants(
                const uint                       & aPattern,
                Cell< Background_Element_Base* > & aElementList,
                luint                            & aElementCount )
        {
            MORIS_ERROR( false, "Don't know how to collect active descendants.");
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 1, 2, 2, 2, 0 >::collect_active_descendants(
               const uint                       & aPattern,
               Cell< Background_Element_Base* > & aElementList,
               luint                            & aElementCount )
       {
           // test if self is active
           if ( mActiveFlags.test( aPattern ) )
           {
               // add self to list
               aElementList( aElementCount++ ) = this;
           }
           else if ( mChildrenFlag )
           {
               // add active children to list
               mChildren[ 0 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 1 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
           }
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 2, 4, 8, 4, 0 >::collect_active_descendants(
               const uint                       & aPattern,
               Cell< Background_Element_Base* > & aElementList,
               luint                            & aElementCount )
       {
           // test if self is active
           if ( mActiveFlags.test( aPattern ) )
           {
               // add self to list
               aElementList( aElementCount++ ) = this;
           }
           else if ( mChildrenFlag )
           {
               // add active children to list
               mChildren[ 0 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 1 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 2 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 3 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
           }
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 3, 8, 26, 6, 12 >::collect_active_descendants(
               const uint                       & aPattern,
               Cell< Background_Element_Base* > & aElementList,
               luint                            & aElementCount )
       {
           // test if self is active
           if ( mActiveFlags.test( aPattern ) )
           {
               // add self to list
               aElementList( aElementCount++ ) = this;
           }
           else if( mChildrenFlag )
           {
               // add active children to list
               mChildren[ 0 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 1 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 2 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 3 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 4 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 5 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 6 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
               mChildren[ 7 ]->collect_active_descendants( aPattern, aElementList, aElementCount );
           }
       }

//--------------------------------------------------------------------------------

       template < uint N, uint C, uint B, uint F , uint E >
       void
       Background_Element< N, C, B, F, E >::count_elements_on_level(
               const  uint & aLevel,
                     luint & aElementCount )
       {
           MORIS_ERROR( false, "Don't know how to count elements on level");
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 1, 2, 2, 2, 0 >::count_elements_on_level(
               const  uint & aLevel,
                     luint & aElementCount )
       {
           // test if element is on specified level
           if ( mLevel ==  aLevel )
           {
               // increment counter
               ++aElementCount;
           }
           else if ( ( mLevel < aLevel) && mChildrenFlag )
           {
               // count children
               mChildren[ 0 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 1 ]->count_elements_on_level( aLevel, aElementCount );
           }
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 2, 4, 8, 4, 0 >::count_elements_on_level(
               const  uint & aLevel,
                     luint & aElementCount )
       {
           // test if element is on specified level
           if ( mLevel ==  aLevel )
           {
               // increment counter
               ++aElementCount;
           }
           else if ( ( mLevel < aLevel) && mChildrenFlag )
           {
               // count children
               mChildren[ 0 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 1 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 2 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 3 ]->count_elements_on_level( aLevel, aElementCount );
           }
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 3, 8, 26, 6, 12 >::count_elements_on_level(
               const  uint & aLevel,
                     luint & aElementCount )
       {
           // test if element is on specified level
           if ( mLevel ==  aLevel )
           {
               // increment counter
               ++aElementCount;
           }
           else if ( ( mLevel < aLevel) && mChildrenFlag )
           {
               // count children
               mChildren[ 0 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 1 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 2 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 3 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 4 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 5 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 6 ]->count_elements_on_level( aLevel, aElementCount );
               mChildren[ 7 ]->count_elements_on_level( aLevel, aElementCount );
           }
       }

//--------------------------------------------------------------------------------

       template < uint N, uint C, uint B, uint F , uint E >
       void
       Background_Element< N, C, B, F, E >::collect_elements_on_level(
               const uint                       & aLevel,
               Cell< Background_Element_Base* > & aElementList,
               luint                            & aElementCount )
       {
           MORIS_ERROR( false, "Don't know how to collect elements on level");
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 1, 2, 2, 2, 0 >::collect_elements_on_level(
               const uint                       & aLevel,
               Cell< Background_Element_Base* > & aElementList,
               luint                            & aElementCount )
       {
           // test if element is on specified level
           if ( mLevel ==  aLevel )
           {
               // add this element to list and increment counter
               aElementList( aElementCount++ ) = this;
           }
           else if ( ( mLevel < aLevel) && mChildrenFlag )
           {
               // collect children
               mChildren[ 0 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 1 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );
           }
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 2, 4, 8, 4, 0 >::collect_elements_on_level(
               const uint                       & aLevel,
               Cell< Background_Element_Base* > & aElementList,
               luint                            & aElementCount )
      {
           // add this element to list and increment counter
           if ( mLevel ==  aLevel )
           {
               // increment counter
               aElementList( aElementCount++ ) = this;
           }
           else if ( ( mLevel < aLevel) && mChildrenFlag )
           {
               // collect children
               mChildren[ 0 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 1 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 2 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 3 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );
           }
      }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 3, 8, 26, 6, 12 >::collect_elements_on_level(
               const uint                       & aLevel,
               Cell< Background_Element_Base* > & aElementList,
               luint                            & aElementCount )
       {
           // add this element to list and increment counter
           if ( mLevel ==  aLevel )
           {
               // increment counter
               aElementList( aElementCount++ ) = this;
           }
           else if ( ( mLevel < aLevel) && mChildrenFlag )
           {
               // collect children
               mChildren[ 0 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 1 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 2 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 3 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 4 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 5 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 6 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );

               mChildren[ 7 ]->collect_elements_on_level(
                       aLevel,
                       aElementList,
                       aElementCount );
           }
       }

//--------------------------------------------------------------------------------

       // fixme: neighbors do not account refinement pattern number
       template < uint N, uint C, uint B, uint F , uint E >
       void
       Background_Element< N, C, B, F, E >::collect_neighbors( const uint & aPattern )
       {
           MORIS_ERROR( false, "Don't know how to collect neighbors");
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

       template <>
       void
       Background_Element< 2, 4, 8, 4, 0 >::collect_neighbors( const uint & aPattern )
       {
           switch( this->get_child_index() )
           {
               case( 0 ):
               {
                   // neighbor 0
                   Background_Element_Base* tNeighbor = mParent->get_neighbor( 4 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 4 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           mNeighbors[ 4 ] = tNeighbor;
                       }
                   }

                   // neighbors 1 and 2
                   tNeighbor = mParent->get_neighbor( 0 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 0 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 5 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           mNeighbors[ 0 ] = tNeighbor;
                           mNeighbors[ 5 ] = tNeighbor;
                       }
                   }

                   // neighbors 3 to 5
                   tNeighbor = mParent->get_neighbor( 3 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 3 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 1 ] =   mParent->get_child( 1 );
                           mNeighbors[ 7 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           mNeighbors[ 3 ] = tNeighbor;
                           mNeighbors[ 1 ] = mParent->get_child( 1 );
                           mNeighbors[ 7 ] = tNeighbor;
                       }
                   }
                   else
                   {
                       mNeighbors[ 1 ] = mParent->get_child( 1 );
                   }

                   // neighbor 6
                   mNeighbors[ 2 ] = mParent->get_child( 2 );

                   // neighbor 7
                   mNeighbors[ 6 ] = mParent->get_child( 3 );

                   break;
               }
               case( 1 ):
               {
                   // neighbors 0 and 1
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 0 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 4 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 0 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           mNeighbors[ 4 ] = tNeighbor;
                           mNeighbors[ 0 ] = tNeighbor;
                       }
                   }

                   // neighbor 2
                   tNeighbor = mParent->get_neighbor( 5 );
                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 5 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           mNeighbors[ 5 ] = tNeighbor;
                       }
                   }

                   // neighbor 3
                   mNeighbors[ 3 ] = mParent->get_child( 0 );

                   // neighbor 4 to 7
                   tNeighbor = mParent->get_neighbor( 1 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 1 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 7 ] = mParent->get_child( 2 );
                           mNeighbors[ 2 ] = mParent->get_child( 3 );
                           mNeighbors[ 6 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           mNeighbors[ 1 ] = tNeighbor;
                           mNeighbors[ 7 ] = mParent->get_child( 2 );
                           mNeighbors[ 2 ] = mParent->get_child( 3 );
                           mNeighbors[ 6 ] = tNeighbor;
                       }
                   }
                   else
                   {
                       mNeighbors[ 7 ] =   mParent->get_child( 2 );
                       mNeighbors[ 2 ] =   mParent->get_child( 3 );
                   }
                   break;
               }
               case( 2 ):
               {
                   // neighbors 0 to 3
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 3 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 4 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 0 ] = mParent->get_child( 0 );
                           mNeighbors[ 5 ] = mParent->get_child( 1 );
                           mNeighbors[ 3 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           mNeighbors[ 4 ] = tNeighbor;
                           mNeighbors[ 0 ] = mParent->get_child( 0 );
                           mNeighbors[ 5 ] = mParent->get_child( 1 );
                           mNeighbors[ 3 ] = tNeighbor;
                       }
                   }
                   else
                   {
                       mNeighbors[ 0 ] =  mParent->get_child( 0 );
                       mNeighbors[ 5 ] =  mParent->get_child( 1 );
                   }

                   // neighbor 4
                   mNeighbors[ 1 ] =  mParent->get_child( 3 );

                   // neighbor 5
                   tNeighbor = mParent->get_neighbor( 7 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 7 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           mNeighbors[ 7 ] = tNeighbor;
                       }
                   }

                   // neighbors 6 and 7
                   tNeighbor = mParent->get_neighbor( 2 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 2 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 6 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           mNeighbors[ 2 ] = tNeighbor;
                           mNeighbors[ 6 ] = tNeighbor;
                       }
                   }
                   break;
               }
               case( 3 ):
               {
                   // neighbor 0
                   mNeighbors[ 4 ] = mParent->get_child( 0 );

                   // neighbor 1
                   mNeighbors[ 0 ] = mParent->get_child( 1 );

                   // neighbors 2 to 4
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 1 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 5 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 3 ] =   mParent->get_child( 2 );
                           mNeighbors[ 1 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           mNeighbors[ 5 ] = tNeighbor;
                           mNeighbors[ 3 ] = mParent->get_child( 2 );
                           mNeighbors[ 1 ] = tNeighbor;
                       }
                   }
                   else
                   {
                       mNeighbors[ 3 ] =  mParent->get_child( 2 );
                   }

                   // neighbors 5 and 6
                   tNeighbor = mParent->get_neighbor( 2 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 7 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 2 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           mNeighbors[ 7 ] = tNeighbor;
                           mNeighbors[ 2 ] = tNeighbor;
                       }
                   }

                   // neighbor 7
                   tNeighbor = mParent->get_neighbor( 6 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           mNeighbors[ 6 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           mNeighbors[ 6 ] = tNeighbor;
                       }
                   }
                   break;
               }
           }
       }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template <>
        void
        Background_Element< 3, 8, 26, 6, 12 >::collect_neighbors( const uint & aPattern )
        {

           switch( this->get_child_index() )
           {
           case( 0 ) :
               {
                   // link to siblings
                   mNeighbors[  1 ] = mParent->get_child(  1 );
                   mNeighbors[  2 ] = mParent->get_child(  2 );
                   mNeighbors[  5 ] = mParent->get_child(  4 );
                   mNeighbors[ 12 ] = mParent->get_child(  3 );
                   mNeighbors[ 15 ] = mParent->get_child(  5 );
                   mNeighbors[ 16 ] = mParent->get_child(  6 );
                   mNeighbors[ 24 ] = mParent->get_child(  7 );

                   // get neighbor 0 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 0 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 0 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 11 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 14 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 23 ] = tNeighbor->get_child( 7 );
                       }
                       else
                       {
                           // link to neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor;
                           mNeighbors[ 11 ] = tNeighbor;
                           mNeighbors[ 14 ] = tNeighbor;
                           mNeighbors[ 23 ] = tNeighbor;
                       }
                   }

                   // get neighbor 3 of parent
                   tNeighbor = mParent->get_neighbor( 3 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 3 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 13 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 17 ] = tNeighbor->get_child( 5 );
                           mNeighbors[ 25 ] = tNeighbor->get_child( 7 );
                       }
                       else
                       {
                           // link to neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor;
                           mNeighbors[ 13 ] = tNeighbor;
                           mNeighbors[ 17 ] = tNeighbor;
                           mNeighbors[ 25 ] = tNeighbor;
                       }
                   }

                   // get neighbor 4 of parent
                   tNeighbor = mParent->get_neighbor( 4 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 4 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor->get_child( 4 );
                           mNeighbors[  7 ] = tNeighbor->get_child( 5 );
                           mNeighbors[  8 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 20 ] = tNeighbor->get_child( 7 );
                       }
                       else
                       {
                           // link to neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor;
                           mNeighbors[  7 ] = tNeighbor;
                           mNeighbors[  8 ] = tNeighbor;
                           mNeighbors[ 20 ] = tNeighbor;
                       }
                   }

                   // get neighbor 6 of parent
                   tNeighbor = mParent->get_neighbor( 6 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 6 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 6 of parent
                           mNeighbors[  6 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 19 ] = tNeighbor->get_child( 7 );
                       }
                       else
                       {
                           // link to neighbor 6 of parent
                           mNeighbors[  6 ] = tNeighbor;
                           mNeighbors[ 19 ] = tNeighbor;
                       }
                   }

                   // get neighbor 9 of parent
                   tNeighbor = mParent->get_neighbor( 9 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 9 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 9 of parent
                           mNeighbors[  9 ] = tNeighbor->get_child( 5 );
                           mNeighbors[ 21 ] = tNeighbor->get_child( 7 );
                       }
                       else
                       {
                           // link to neighbor 9 of parent
                           mNeighbors[  9 ] = tNeighbor;
                           mNeighbors[ 21 ] = tNeighbor;
                       }
                   }

                   // get neighbor 10 of parent
                   tNeighbor = mParent->get_neighbor( 10 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 10 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 10 of parent
                           mNeighbors[ 10 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 22 ] = tNeighbor->get_child( 7 );
                       }
                       else
                       {
                           // link to neighbor 10 of parent
                           mNeighbors[ 10 ] = tNeighbor;
                           mNeighbors[ 22 ] = tNeighbor;
                       }
                   }

                   // get neighbor 18 of parent
                   tNeighbor = mParent->get_neighbor( 18 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 18 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 18 of parent
                           mNeighbors[ 18 ] = tNeighbor->get_child( 7 );
                       }
                       else
                       {
                           // link to neighbor 18 of parent
                           mNeighbors[ 18 ] = tNeighbor;
                       }
                   }

                   break;
               }
               case( 1 ) :
               {
                   // link to siblings
                   mNeighbors[  2 ] = mParent->get_child(  3 );
                   mNeighbors[  3 ] = mParent->get_child(  0 );
                   mNeighbors[  5 ] = mParent->get_child(  5 );
                   mNeighbors[ 13 ] = mParent->get_child(  2 );
                   mNeighbors[ 16 ] = mParent->get_child(  7 );
                   mNeighbors[ 17 ] = mParent->get_child(  4 );
                   mNeighbors[ 25 ] = mParent->get_child(  6 );

                   // get neighbor 0 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 0 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 0 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 10 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 14 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 22 ] = tNeighbor->get_child( 6 );
                       }
                       else
                       {
                           // link to neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor;
                           mNeighbors[ 10 ] = tNeighbor;
                           mNeighbors[ 14 ] = tNeighbor;
                           mNeighbors[ 22 ] = tNeighbor;
                       }
                   }

                   // get neighbor 1 of parent
                   tNeighbor = mParent->get_neighbor( 1 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 1 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 12 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 15 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 24 ] = tNeighbor->get_child( 6 );
                       }
                       else
                       {
                           // link to neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor;
                           mNeighbors[ 12 ] = tNeighbor;
                           mNeighbors[ 15 ] = tNeighbor;
                           mNeighbors[ 24 ] = tNeighbor;
                       }
                   }

                   // get neighbor 4 of parent
                   tNeighbor = mParent->get_neighbor( 4 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 4 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor->get_child( 5 );
                           mNeighbors[  8 ] = tNeighbor->get_child( 7 );
                           mNeighbors[  9 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 21 ] = tNeighbor->get_child( 6 );
                       }
                       else
                       {
                           // link to neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor;
                           mNeighbors[  8 ] = tNeighbor;
                           mNeighbors[  9 ] = tNeighbor;
                           mNeighbors[ 21 ] = tNeighbor;
                       }
                   }

                   // get neighbor 6 of parent
                   tNeighbor = mParent->get_neighbor( 6 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 6 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 6 of parent
                           mNeighbors[  6 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 18 ] = tNeighbor->get_child( 6 );
                       }
                       else
                       {
                           // link to neighbor 6 of parent
                           mNeighbors[  6 ] = tNeighbor;
                           mNeighbors[ 18 ] = tNeighbor;
                       }
                   }

                   // get neighbor 7 of parent
                   tNeighbor = mParent->get_neighbor( 7 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 7 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 7 of parent
                           mNeighbors[  7 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 20 ] = tNeighbor->get_child( 6 );
                       }
                       else
                       {
                           // link to neighbor 7 of parent
                           mNeighbors[  7 ] = tNeighbor;
                           mNeighbors[ 20 ] = tNeighbor;
                       }
                   }

                   // get neighbor 11 of parent
                   tNeighbor = mParent->get_neighbor( 11 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 11 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 11 of parent
                           mNeighbors[ 11 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 23 ] = tNeighbor->get_child( 6 );
                       }
                       else
                       {
                           // link to neighbor 11 of parent
                           mNeighbors[ 11 ] = tNeighbor;
                           mNeighbors[ 23 ] = tNeighbor;
                       }
                   }

                   // get neighbor 19 of parent
                   tNeighbor = mParent->get_neighbor( 19 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 19 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 19 of parent
                           mNeighbors[ 19 ] = tNeighbor->get_child( 6 );
                       }
                       else
                       {
                           // link to neighbor 19 of parent
                           mNeighbors[ 19 ] = tNeighbor;
                       }
                   }

                   break;
               }
               case( 2 ) :
               {

                   // link to siblings
                   mNeighbors[  0 ] = mParent->get_child(  0 );
                   mNeighbors[  1 ] = mParent->get_child(  3 );
                   mNeighbors[  5 ] = mParent->get_child(  6 );
                   mNeighbors[ 11 ] = mParent->get_child(  1 );
                   mNeighbors[ 14 ] = mParent->get_child(  4 );
                   mNeighbors[ 15 ] = mParent->get_child(  7 );
                   mNeighbors[ 23 ] = mParent->get_child(  5 );

                   // get neighbor 2 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 2 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 2 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 12 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 16 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 24 ] = tNeighbor->get_child( 5 );
                       }
                       else
                       {
                           // link to neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor;
                           mNeighbors[ 12 ] = tNeighbor;
                           mNeighbors[ 16 ] = tNeighbor;
                           mNeighbors[ 24 ] = tNeighbor;
                       }
                   }

                   // get neighbor 3 of parent
                   tNeighbor = mParent->get_neighbor( 3 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 3 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 10 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 17 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 22 ] = tNeighbor->get_child( 5 );
                       }
                       else
                       {
                           // link to neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor;
                           mNeighbors[ 10 ] = tNeighbor;
                           mNeighbors[ 17 ] = tNeighbor;
                           mNeighbors[ 22 ] = tNeighbor;
                       }
                   }

                   // get neighbor 4 of parent
                   tNeighbor = mParent->get_neighbor( 4 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 4 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor->get_child( 6 );
                           mNeighbors[  6 ] = tNeighbor->get_child( 4 );
                           mNeighbors[  7 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 19 ] = tNeighbor->get_child( 5 );
                       }
                       else
                       {
                           // link to neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor;
                           mNeighbors[  6 ] = tNeighbor;
                           mNeighbors[  7 ] = tNeighbor;
                           mNeighbors[ 19 ] = tNeighbor;
                       }
                   }

                   // get neighbor 8 of parent
                   tNeighbor = mParent->get_neighbor( 8 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 8 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 8 of parent
                           mNeighbors[  8 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 20 ] = tNeighbor->get_child( 5 );
                       }
                       else
                       {
                           // link to neighbor 8 of parent
                           mNeighbors[  8 ] = tNeighbor;
                           mNeighbors[ 20 ] = tNeighbor;
                       }
                   }

                   // get neighbor 9 of parent
                   tNeighbor = mParent->get_neighbor( 9 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 9 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 9 of parent
                           mNeighbors[  9 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 18 ] = tNeighbor->get_child( 5 );
                       }
                       else
                       {
                           // link to neighbor 9 of parent
                           mNeighbors[  9 ] = tNeighbor;
                           mNeighbors[ 18 ] = tNeighbor;
                       }
                   }

                   // get neighbor 13 of parent
                   tNeighbor = mParent->get_neighbor( 13 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 13 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 13 of parent
                           mNeighbors[ 13 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 25 ] = tNeighbor->get_child( 5 );
                       }
                       else
                       {
                           // link to neighbor 13 of parent
                           mNeighbors[ 13 ] = tNeighbor;
                           mNeighbors[ 25 ] = tNeighbor;
                       }
                   }

                   // get neighbor 21 of parent
                   tNeighbor = mParent->get_neighbor( 21 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 21 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 21 of parent
                           mNeighbors[ 21 ] = tNeighbor->get_child( 5 );
                       }
                       else
                       {
                           // link to neighbor 21 of parent
                           mNeighbors[ 21 ] = tNeighbor;
                       }
                   }
                   break;
               }
               case( 3 ) :
               {
                   // link to siblings
                   mNeighbors[  0 ] = mParent->get_child(  1 );
                   mNeighbors[  3 ] = mParent->get_child(  2 );
                   mNeighbors[  5 ] = mParent->get_child(  7 );
                   mNeighbors[ 10 ] = mParent->get_child(  0 );
                   mNeighbors[ 14 ] = mParent->get_child(  5 );
                   mNeighbors[ 17 ] = mParent->get_child(  6 );
                   mNeighbors[ 22 ] = mParent->get_child(  4 );

                   // get neighbor 1 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 1 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 1 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 11 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 15 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 23 ] = tNeighbor->get_child( 4 );
                       }
                       else
                       {
                           // link to neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor;
                           mNeighbors[ 11 ] = tNeighbor;
                           mNeighbors[ 15 ] = tNeighbor;
                           mNeighbors[ 23 ] = tNeighbor;
                       }
                   }

                   // get neighbor 2 of parent
                   tNeighbor = mParent->get_neighbor( 2 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 2 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 13 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 16 ] = tNeighbor->get_child( 5 );
                           mNeighbors[ 25 ] = tNeighbor->get_child( 4 );
                       }
                       else
                       {
                           // link to neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor;
                           mNeighbors[ 13 ] = tNeighbor;
                           mNeighbors[ 16 ] = tNeighbor;
                           mNeighbors[ 25 ] = tNeighbor;
                       }
                   }

                   // get neighbor 4 of parent
                   tNeighbor = mParent->get_neighbor( 4 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 4 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor->get_child( 7 );
                           mNeighbors[  6 ] = tNeighbor->get_child( 5 );
                           mNeighbors[  9 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 18 ] = tNeighbor->get_child( 4 );
                       }
                       else
                       {
                           // link to neighbor 4 of parent
                           mNeighbors[  4 ] = tNeighbor;
                           mNeighbors[  6 ] = tNeighbor;
                           mNeighbors[  9 ] = tNeighbor;
                           mNeighbors[ 18 ] = tNeighbor;
                       }
                   }

                   // get neighbor 7 of parent
                   tNeighbor = mParent->get_neighbor( 7 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 7 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 7 of parent
                           mNeighbors[  7 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 19 ] = tNeighbor->get_child( 4 );
                       }
                       else
                       {
                           // link to neighbor 7 of parent
                           mNeighbors[  7 ] = tNeighbor;
                           mNeighbors[ 19 ] = tNeighbor;
                       }
                   }

                   // get neighbor 8 of parent
                   tNeighbor = mParent->get_neighbor( 8 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 8 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 8 of parent
                           mNeighbors[  8 ] = tNeighbor->get_child( 5 );
                           mNeighbors[ 21 ] = tNeighbor->get_child( 4 );
                       }
                       else
                       {
                           // link to neighbor 8 of parent
                           mNeighbors[  8 ] = tNeighbor;
                           mNeighbors[ 21 ] = tNeighbor;
                       }
                   }

                   // get neighbor 12 of parent
                   tNeighbor = mParent->get_neighbor( 12 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 12 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 12 of parent
                           mNeighbors[ 12 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 24 ] = tNeighbor->get_child( 4 );
                       }
                       else
                       {
                           // link to neighbor 12 of parent
                           mNeighbors[ 12 ] = tNeighbor;
                           mNeighbors[ 24 ] = tNeighbor;
                       }
                   }

                   // get neighbor 20 of parent
                   tNeighbor = mParent->get_neighbor( 20 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 20 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 20 of parent
                           mNeighbors[ 20 ] = tNeighbor->get_child( 4 );
                       }
                       else
                       {
                           // link to neighbor 20 of parent
                           mNeighbors[ 20 ] = tNeighbor;
                       }
                   }

                   break;
               }
               case( 4 ) :
               {
                   // link to siblings
                   mNeighbors[  1 ] = mParent->get_child(  5 );
                   mNeighbors[  2 ] = mParent->get_child(  6 );
                   mNeighbors[  4 ] = mParent->get_child(  0 );
                   mNeighbors[  7 ] = mParent->get_child(  1 );
                   mNeighbors[  8 ] = mParent->get_child(  2 );
                   mNeighbors[ 12 ] = mParent->get_child(  7 );
                   mNeighbors[ 20 ] = mParent->get_child(  3 );

                   // get neighbor 0 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 0 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 0 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor->get_child( 6 );
                           mNeighbors[  6 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 11 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 19 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           // link to neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor;
                           mNeighbors[  6 ] = tNeighbor;
                           mNeighbors[ 11 ] = tNeighbor;
                           mNeighbors[ 19 ] = tNeighbor;
                       }
                   }

                   // get neighbor 3 of parent
                   tNeighbor = mParent->get_neighbor( 3 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 3 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor->get_child( 5 );
                           mNeighbors[  9 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 13 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 21 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           // link to neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor;
                           mNeighbors[  9 ] = tNeighbor;
                           mNeighbors[ 13 ] = tNeighbor;
                           mNeighbors[ 21 ] = tNeighbor;
                       }
                   }

                   // get neighbor 5 of parent
                   tNeighbor = mParent->get_neighbor( 5 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 5 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 15 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 16 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 24 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           // link to neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor;
                           mNeighbors[ 15 ] = tNeighbor;
                           mNeighbors[ 16 ] = tNeighbor;
                           mNeighbors[ 24 ] = tNeighbor;
                       }
                   }

                   // get neighbor 10 of parent
                   tNeighbor = mParent->get_neighbor( 10 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 10 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 10 of parent
                           mNeighbors[ 10 ] = tNeighbor->get_child( 7 );
                           mNeighbors[ 18 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           // link to neighbor 10 of parent
                           mNeighbors[ 10 ] = tNeighbor;
                           mNeighbors[ 18 ] = tNeighbor;
                       }
                   }

                   // get neighbor 14 of parent
                   tNeighbor = mParent->get_neighbor( 14 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 14 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 14 of parent
                           mNeighbors[ 14 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 23 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           // link to neighbor 14 of parent
                           mNeighbors[ 14 ] = tNeighbor;
                           mNeighbors[ 23 ] = tNeighbor;
                       }
                   }

                   // get neighbor 17 of parent
                   tNeighbor = mParent->get_neighbor( 17 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 17 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 17 of parent
                           mNeighbors[ 17 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 25 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           // link to neighbor 17 of parent
                           mNeighbors[ 17 ] = tNeighbor;
                           mNeighbors[ 25 ] = tNeighbor;
                       }
                   }

                   // get neighbor 22 of parent
                   tNeighbor = mParent->get_neighbor( 22 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 22 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 22 of parent
                           mNeighbors[ 22 ] = tNeighbor->get_child( 3 );
                       }
                       else
                       {
                           // link to neighbor 22 of parent
                           mNeighbors[ 22 ] = tNeighbor;
                       }
                   }

                   break;
               }
               case( 5 ) :
               {

                   // link to siblings
                   mNeighbors[  2 ] = mParent->get_child(  7 );
                   mNeighbors[  3 ] = mParent->get_child(  4 );
                   mNeighbors[  4 ] = mParent->get_child(  1 );
                   mNeighbors[  8 ] = mParent->get_child(  3 );
                   mNeighbors[  9 ] = mParent->get_child(  0 );
                   mNeighbors[ 13 ] = mParent->get_child(  6 );
                   mNeighbors[ 21 ] = mParent->get_child(  2 );

                   // get neighbor 0 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 0 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 0 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor->get_child( 7 );
                           mNeighbors[  6 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 10 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 18 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           // link to neighbor 0 of parent
                           mNeighbors[  0 ] = tNeighbor;
                           mNeighbors[  6 ] = tNeighbor;
                           mNeighbors[ 10 ] = tNeighbor;
                           mNeighbors[ 18 ] = tNeighbor;
                       }
                   }

                   // get neighbor 1 of parent
                   tNeighbor = mParent->get_neighbor( 1 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 1 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor->get_child( 4 );
                           mNeighbors[  7 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 12 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 20 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           // link to neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor;
                           mNeighbors[  7 ] = tNeighbor;
                           mNeighbors[ 12 ] = tNeighbor;
                           mNeighbors[ 20 ] = tNeighbor;
                       }
                   }

                   // get neighbor 5 of parent
                   tNeighbor = mParent->get_neighbor( 5 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 5 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 16 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 17 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 25 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           // link to neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor;
                           mNeighbors[ 16 ] = tNeighbor;
                           mNeighbors[ 17 ] = tNeighbor;
                           mNeighbors[ 25 ] = tNeighbor;
                       }
                   }

                   // get neighbor 11 of parent
                   tNeighbor = mParent->get_neighbor( 11 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 11 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 11 of parent
                           mNeighbors[ 11 ] = tNeighbor->get_child( 6 );
                           mNeighbors[ 19 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           // link to neighbor 11 of parent
                           mNeighbors[ 11 ] = tNeighbor;
                           mNeighbors[ 19 ] = tNeighbor;
                       }
                   }

                   // get neighbor 14 of parent
                   tNeighbor = mParent->get_neighbor( 14 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 14 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 14 of parent
                           mNeighbors[ 14 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 22 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           // link to neighbor 14 of parent
                           mNeighbors[ 14 ] = tNeighbor;
                           mNeighbors[ 22 ] = tNeighbor;
                       }
                   }

                   // get neighbor 15 of parent
                   tNeighbor = mParent->get_neighbor( 15 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 15 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 15 of parent
                           mNeighbors[ 15 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 24 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           // link to neighbor 15 of parent
                           mNeighbors[ 15 ] = tNeighbor;
                           mNeighbors[ 24 ] = tNeighbor;
                       }
                   }

                   // get neighbor 23 of parent
                   tNeighbor = mParent->get_neighbor( 23 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 23 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 23 of parent
                           mNeighbors[ 23 ] = tNeighbor->get_child( 2 );
                       }
                       else
                       {
                           // link to neighbor 23 of parent
                           mNeighbors[ 23 ] = tNeighbor;
                       }
                   }
                   break;
               }
               case( 6 ) :
               {
                   // link to siblings
                   mNeighbors[  0 ] = mParent->get_child(  4 );
                   mNeighbors[  1 ] = mParent->get_child(  7 );
                   mNeighbors[  4 ] = mParent->get_child(  2 );
                   mNeighbors[  6 ] = mParent->get_child(  0 );
                   mNeighbors[  7 ] = mParent->get_child(  3 );
                   mNeighbors[ 11 ] = mParent->get_child(  5 );
                   mNeighbors[ 19 ] = mParent->get_child(  1 );

                   // get neighbor 2 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 2 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 2 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor->get_child( 4 );
                           mNeighbors[  8 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 12 ] = tNeighbor->get_child( 5 );
                           mNeighbors[ 20 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           // link to neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor;
                           mNeighbors[  8 ] = tNeighbor;
                           mNeighbors[ 12 ] = tNeighbor;
                           mNeighbors[ 20 ] = tNeighbor;
                       }
                   }

                   // get neighbor 3 of parent
                   tNeighbor = mParent->get_neighbor( 3 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 3 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor->get_child( 7 );
                           mNeighbors[  9 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 10 ] = tNeighbor->get_child( 5 );
                           mNeighbors[ 18 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           // link to neighbor 3 of parent
                           mNeighbors[  3 ] = tNeighbor;
                           mNeighbors[  9 ] = tNeighbor;
                           mNeighbors[ 10 ] = tNeighbor;
                           mNeighbors[ 18 ] = tNeighbor;
                       }
                   }

                   // get neighbor 5 of parent
                   tNeighbor = mParent->get_neighbor( 5 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 5 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 14 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 15 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 23 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           // link to neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor;
                           mNeighbors[ 14 ] = tNeighbor;
                           mNeighbors[ 15 ] = tNeighbor;
                           mNeighbors[ 23 ] = tNeighbor;
                       }
                   }

                   // get neighbor 13 of parent
                   tNeighbor = mParent->get_neighbor( 13 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 13 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 13 of parent
                           mNeighbors[ 13 ] = tNeighbor->get_child( 5 );
                           mNeighbors[ 21 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           // link to neighbor 13 of parent
                           mNeighbors[ 13 ] = tNeighbor;
                           mNeighbors[ 21 ] = tNeighbor;
                       }
                   }

                   // get neighbor 16 of parent
                   tNeighbor = mParent->get_neighbor( 16 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 16 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 16 of parent
                           mNeighbors[ 16 ] = tNeighbor->get_child( 0 );
                           mNeighbors[ 24 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           // link to neighbor 16 of parent
                           mNeighbors[ 16 ] = tNeighbor;
                           mNeighbors[ 24 ] = tNeighbor;
                       }
                   }

                   // get neighbor 17 of parent
                   tNeighbor = mParent->get_neighbor( 17 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 17 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 17 of parent
                           mNeighbors[ 17 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 22 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           // link to neighbor 17 of parent
                           mNeighbors[ 17 ] = tNeighbor;
                           mNeighbors[ 22 ] = tNeighbor;
                       }
                   }

                   // get neighbor 25 of parent
                   tNeighbor = mParent->get_neighbor( 25 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 25 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 25 of parent
                           mNeighbors[ 25 ] = tNeighbor->get_child( 1 );
                       }
                       else
                       {
                           // link to neighbor 25 of parent
                           mNeighbors[ 25 ] = tNeighbor;
                       }
                   }
                   break;
               }
               case( 7 ) :
               {
                   // link to siblings
                   mNeighbors[  0 ] = mParent->get_child(  5 );
                   mNeighbors[  3 ] = mParent->get_child(  6 );
                   mNeighbors[  4 ] = mParent->get_child(  3 );
                   mNeighbors[  6 ] = mParent->get_child(  1 );
                   mNeighbors[  9 ] = mParent->get_child(  2 );
                   mNeighbors[ 10 ] = mParent->get_child(  4 );
                   mNeighbors[ 18 ] = mParent->get_child(  0 );

                   // get neighbor 1 of parent
                   Background_Element_Base* tNeighbor
                       = mParent->get_neighbor( 1 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 1 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor->get_child( 6 );
                           mNeighbors[  7 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 11 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 19 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           // link to neighbor 1 of parent
                           mNeighbors[  1 ] = tNeighbor;
                           mNeighbors[  7 ] = tNeighbor;
                           mNeighbors[ 11 ] = tNeighbor;
                           mNeighbors[ 19 ] = tNeighbor;
                       }
                   }

                   // get neighbor 2 of parent
                   tNeighbor = mParent->get_neighbor( 2 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 2 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor->get_child( 5 );
                           mNeighbors[  8 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 13 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 21 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           // link to neighbor 2 of parent
                           mNeighbors[  2 ] = tNeighbor;
                           mNeighbors[  8 ] = tNeighbor;
                           mNeighbors[ 13 ] = tNeighbor;
                           mNeighbors[ 21 ] = tNeighbor;
                       }
                   }

                   // get neighbor 5 of parent
                   tNeighbor = mParent->get_neighbor( 5 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 5 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor->get_child( 3 );
                           mNeighbors[ 14 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 17 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 22 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           // link to neighbor 5 of parent
                           mNeighbors[  5 ] = tNeighbor;
                           mNeighbors[ 14 ] = tNeighbor;
                           mNeighbors[ 17 ] = tNeighbor;
                           mNeighbors[ 22 ] = tNeighbor;
                       }
                   }

                   // get neighbor 12 of parent
                   tNeighbor = mParent->get_neighbor( 12 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 12 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 12 of parent
                           mNeighbors[ 12 ] = tNeighbor->get_child( 4 );
                           mNeighbors[ 20 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           // link to neighbor 12 of parent
                           mNeighbors[ 12 ] = tNeighbor;
                           mNeighbors[ 20 ] = tNeighbor;
                       }
                   }

                   // get neighbor 15 of parent
                   tNeighbor = mParent->get_neighbor( 15 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 15 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 15 of parent
                           mNeighbors[ 15 ] = tNeighbor->get_child( 2 );
                           mNeighbors[ 23 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           // link to neighbor 15 of parent
                           mNeighbors[ 15 ] = tNeighbor;
                           mNeighbors[ 23 ] = tNeighbor;
                       }
                   }

                   // get neighbor 16 of parent
                   tNeighbor = mParent->get_neighbor( 16 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 16 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 16 of parent
                           mNeighbors[ 16 ] = tNeighbor->get_child( 1 );
                           mNeighbors[ 25 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           // link to neighbor 16 of parent
                           mNeighbors[ 16 ] = tNeighbor;
                           mNeighbors[ 25 ] = tNeighbor;
                       }
                   }

                   // get neighbor 24 of parent
                   tNeighbor = mParent->get_neighbor( 24 );

                   // test if neighbor exists
                   if ( tNeighbor != NULL )
                   {
                       // test if neighbor 24 has children
                       if ( tNeighbor->has_children( aPattern ) )
                       {
                           // link to children of neighbor 24 of parent
                           mNeighbors[ 24 ] = tNeighbor->get_child( 0 );
                       }
                       else
                       {
                           // link to neighbor 24 of parent
                           mNeighbors[ 24 ] = tNeighbor;
                       }
                   }
                   break;
               }
           }
        }

//--------------------------------------------------------------------------------

        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::reset_flags_of_facets()
        {
            MORIS_ERROR( false, "Don't know how to reset face flags");
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<>
        void
        Background_Element< 2, 4, 8, 4, 0 >::reset_flags_of_facets()
        {
            mFacets[ 0 ]->unflag();
            mFacets[ 1 ]->unflag();
            mFacets[ 2 ]->unflag();
            mFacets[ 3 ]->unflag();
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<>
        void
        Background_Element< 3, 8, 26, 6, 12 >::reset_flags_of_facets()
        {
            mFacets[ 0 ]->unflag();
            mFacets[ 1 ]->unflag();
            mFacets[ 2 ]->unflag();
            mFacets[ 3 ]->unflag();
            mFacets[ 4 ]->unflag();
            mFacets[ 5 ]->unflag();
        }

//--------------------------------------------------------------------------------

        /**
         * returns an edge of the background element ( 3D only )
         */
        template < uint N, uint C, uint B, uint F , uint E >
        Background_Edge *
        Background_Element< N, C, B, F, E >::get_edge( const uint & aIndex )
        {
            MORIS_ERROR( false, "get_edge() is only available for the 3D element" );
            return nullptr;
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<>
        Background_Edge *
        Background_Element< 3, 8, 26, 6, 12 >::get_edge( const uint & aIndex )
        {
            return mEdges[ aIndex ];
        }

//--------------------------------------------------------------------------------

        /**
         * inserts an edge into the element ( 3D only )
         */
        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::insert_edge( Background_Edge * aEdge, const uint & aIndex )
        {
            MORIS_ERROR( false, "insert_edge() is only available for the 3D element" );
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<>
        void
        Background_Element< 3, 8, 26, 6, 12 >::insert_edge( Background_Edge * aEdge, const uint & aIndex )
        {
            mEdges[ aIndex ] = aEdge;

            aEdge->insert_element( this, aIndex );
        }

//--------------------------------------------------------------------------------

        /**
         * inserts an edge into the element ( 3D only )
         */
        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::create_edges()
        {
            MORIS_ERROR( false, "create_edges() is only available for the 3D element" );
        }

//--------------------------------------------------------------------------------

        /**
         * resets the edge flags
         */
        template < uint N, uint C, uint B, uint F , uint E >
        void
        Background_Element< N, C, B, F, E >::reset_flags_of_edges()
        {
            MORIS_ERROR( false, "reset_flags_of_edges() is only available for the 3D element" );
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<>
        void
        Background_Element< 3, 8, 26, 6, 12 >::reset_flags_of_edges()
        {
            for( uint k=0; k<12; ++k )
            {
                mEdges [ k ]->unflag();
            }
        }

//--------------------------------------------------------------------------------

    } /* namespace hmr */
} /* namespace moris */

#endif /* SRC_HMR_CL_HMR_BACKGROUND_ELEMENT_HPP_ */
