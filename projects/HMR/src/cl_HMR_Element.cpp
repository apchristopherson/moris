#include "cl_HMR_Element.hpp" //HMR/src

#include "cl_HMR_Edge.hpp"
#include "cl_HMR_Facet.hpp"

namespace moris
{
    namespace hmr
    {
//------------------------------------------------------------------------------

        Element::Element( Background_Element_Base  * aElement,
                          const uint               & aActivationPattern ) : mElement( aElement ),
                                                                            mActivationPattern( aActivationPattern )
        {
        }

//------------------------------------------------------------------------------

        Element * Element::get_neighbor(       moris::Cell< Element * > & aAllElementsOnProc,
                                         const luint                    & aNeighborNumber )
        {
            // get neighbor of background element
            Background_Element_Base* tElement = mElement->get_neighbor( aNeighborNumber );

            // test if neighbor exists
            if ( tElement != NULL )
            {
                // test if neighbor is on the same level
                if ( tElement->get_level() == mElement->get_level() )
                {
                    return aAllElementsOnProc( tElement->get_memory_index() );
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }

//-------------------------------------------------------------------------------

        Element * Element::get_child(       moris::Cell< Element * > & aAllElementsOnProc,
                                      const uint                     & aChildIndex )
        {
            if( mElement->has_children() )
            {
                return aAllElementsOnProc( mElement->get_child( aChildIndex )->get_memory_index() );
            }
            else
            {
                return nullptr;
            }
        }

//-------------------------------------------------------------------------------

        // special funciton for HMR
        Facet * Element::get_hmr_facet( const uint & aIndex )
        {
            MORIS_ERROR( false, "get_lagrange_facet() cannot be called from this element type" );
            return nullptr;
        }

//-------------------------------------------------------------------------------

        void Element::set_hmr_facet( Facet * aFacet, const uint & aIndex )
        {
            MORIS_ERROR( false, "set_hmr_facet() cannot be called from this element type" );
        }

//-------------------------------------------------------------------------------

        Edge * Element::get_hmr_edge( const uint & aIndex )
        {
            MORIS_ERROR( false, "get_hmr_edge() cannot be called from this element type" );
            return nullptr;
        }

//-------------------------------------------------------------------------------

        const Edge * Element::get_hmr_edge( const uint & aIndex ) const
        {
            MORIS_ERROR( false, "get_hmr_edge() cannot be called from this element type" );
            return nullptr;
        }

//-------------------------------------------------------------------------------

        void Element::set_hmr_edge( Edge * aEdge, const uint & aIndex )
        {
            MORIS_ERROR( false, "set_hmr_edge() cannot be called from this element type" );
        }

//-------------------------------------------------------------------------------
     } /* namespace hmr */
} /* namespace moris */

