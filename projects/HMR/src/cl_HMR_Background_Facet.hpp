/*
 * cl_HMR_Background_Facet.hpp
 *
 *  Created on: Sep 23, 2018
 *      Author: messe
 */

#ifndef PROJECTS_HMR_SRC_CL_HMR_BACKGROUND_FACET_HPP_
#define PROJECTS_HMR_SRC_CL_HMR_BACKGROUND_FACET_HPP_

#include "typedefs.hpp"
#include "cl_Bitset.hpp" //CON/src
#include "HMR_Globals.hpp" //HMR/src

namespace moris
{
    namespace hmr
    {
//-----------------------------------------------------------------------------

        class Background_Element_Base;

//-----------------------------------------------------------------------------
        class Background_Facet
        {
            //! Pointer to parent. Points to null for faces
            //! on coarsest level.
            // Background_Facet *      mParent;

            //! Contains the ID of the proc that owns the element.
            // moris_id                    mOwner;

            //! Tells if a face is active
            // Bitset< gNumberOfPatterns > mActiveFlags;

            //! Tells if a face is refined
            // Bitset< gNumberOfPatterns > mRefinedFlags;

            //! Tells if the face has children.
            // bool                        mChildrenFlag = false;

            //! index in memory
            // uint                        mMemoryIndex;


            //! reference element ( the element with the lower id )
            Background_Element_Base *   mMasterElement;
            Background_Element_Base *   mSlaveElement;

            //! face index for master element
            uint                        mIndexOnMaster;


            //! owner of this facet
            moris_id                    mOwner;

            //! multi purpose flag
            bool                        mFlag = false;
//------------------------------------------------------------------------------
        public:
//------------------------------------------------------------------------------

            /**
             * Default constructor
             */
            Background_Facet(
                    Background_Element_Base * aElementA,
                    Background_Element_Base * aElementB,
                    const  uint             & aIndexOnaElementA  );

//------------------------------------------------------------------------------

            Background_Facet(
                    Background_Element_Base * aElement,
                    const  uint             & aIndexOnElement,
                    const  moris_id         & aProcID );

//------------------------------------------------------------------------------

            /**
             * trivial destructor
             */
            ~Background_Facet(){};

//------------------------------------------------------------------------------

            /**
             * sets the flag
             */
            void
            flag();

//------------------------------------------------------------------------------

            /**
             * resets the flag
             */
            void
            unflag();

//--------------------------------------------------------------------------------

            /**
             * test if flag is set
             */
            bool
            is_flagged() const;

//--------------------------------------------------------------------------------

            /**
             * get pointer to master element
             */
            Background_Element_Base *
            get_master();
//--------------------------------------------------------------------------------

            /**
             * get pointer to slave element
             */
            Background_Element_Base *
            get_slave();

//--------------------------------------------------------------------------------

            /**
             * sets the slave element
             */
            void
            set_slave( Background_Element_Base * aElement );

//--------------------------------------------------------------------------------

            /**
             * returns the index in relation to the master element
             */
            uint
            get_index_on_master() const;

//--------------------------------------------------------------------------------

            /**
             * returns the index in relation to the master element
             */
            uint
            get_index_on_slave() const;

//--------------------------------------------------------------------------------

            /**
             * returns the index in relation to the master element
             */
            uint
            get_index_on_other( const uint & aIndex ) const;

//--------------------------------------------------------------------------------

            moris_id
            get_owner() const;

//--------------------------------------------------------------------------------
        };

//-------------------------------------------------------------------------------
    } /* namespace hmr */
} /* namespace moris */

#endif /* PROJECTS_HMR_SRC_CL_HMR_BACKGROUND_FACET_HPP_ */
