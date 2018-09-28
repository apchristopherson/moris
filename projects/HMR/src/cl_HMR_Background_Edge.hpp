/*
 * cl_HMR_Background_Edge.hpp
 *
 *  Created on: Sep 24, 2018
 *      Author: messe
 */

#ifndef PROJECTS_HMR_SRC_CL_HMR_BACKGROUND_EDGE_HPP_
#define PROJECTS_HMR_SRC_CL_HMR_BACKGROUND_EDGE_HPP_

#include "typedefs.hpp"

namespace moris
{
    namespace hmr
    {
//-------------------------------------------------------------------------------

        class Background_Element_Base;

//-------------------------------------------------------------------------------
        class Background_Edge
        {
            //! flag of this edge
            bool mFlag = false;

            //! tells how many elements are connected
            uint mElementCounter = 0;

            //! pointer with connected elements
            Background_Element_Base * mElements[ 4 ] = { nullptr };

            //! edge indices
            uint                      mIndexInElement[ 4 ];

//-------------------------------------------------------------------------------
        public:
//-------------------------------------------------------------------------------

            Background_Edge( Background_Element_Base * aElement, const uint & aIndex );

//-------------------------------------------------------------------------------

            ~Background_Edge(){};

//-------------------------------------------------------------------------------

            void
            flag();

//-------------------------------------------------------------------------------

            void
            unflag();

//-------------------------------------------------------------------------------

            bool
            is_flagged() const;

//-------------------------------------------------------------------------------

            void
            insert_element( Background_Element_Base * aElement, const uint & aIndex );

//-------------------------------------------------------------------------------

            uint
            get_number_of_elements() const;

//-------------------------------------------------------------------------------

            Background_Element_Base *
            get_element( const uint & aIndex );

//-------------------------------------------------------------------------------

            uint
            get_index_on_element( const uint & aIndex ) const;

//-------------------------------------------------------------------------------
        };
    }
}

#endif /* PROJECTS_HMR_SRC_CL_HMR_BACKGROUND_EDGE_HPP_ */
