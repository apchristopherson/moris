/*
 * cl_HMR_Lagrange_Facet_Line3.hpp
 *
 *  Created on: September 25, 2018
 *  using MATLAB
 */
 
#ifndef SRC_HMR_CL_HMR_LAGRANGE_FACET_LINE3_HPP_
#define SRC_HMR_CL_HMR_LAGRANGE_FACET_LINE3_HPP_

#include "cl_HMR_Lagrange_Facet.hpp"

namespace moris
{
    namespace hmr
    {

// ----------------------------------------------------------------------------

        template<>
        mtk::Geometry_Type
        Lagrange_Facet< 2, 3 >::get_geometry_type() const
        {
            return mtk::Geometry_Type::LINE;
        }

// ----------------------------------------------------------------------------

        template<>
        mtk::Interpolation_Order
        Lagrange_Facet< 2, 3 >::get_interpolation_order() const
        {
            return mtk::Interpolation_Order::QUADRATIC;
        }

// ----------------------------------------------------------------------------

        template<>
        void
        Lagrange_Facet< 2, 3 >::copy_vertex_pointers( const uint & aIndex )
        {
            // pick side of parent element
            switch( aIndex )
            {
                case( 0 ) :
                {
                    mVertices[ 0 ] = mMaster->get_basis( 0 );
                    mVertices[ 1 ] = mMaster->get_basis( 1 );
                    mVertices[ 2 ] = mMaster->get_basis( 4 );
                    break;
                }
                case( 1 ) :
                {
                    mVertices[ 0 ] = mMaster->get_basis( 1 );
                    mVertices[ 1 ] = mMaster->get_basis( 2 );
                    mVertices[ 2 ] = mMaster->get_basis( 5 );
                    break;
                }
                case( 2 ) :
                {
                    mVertices[ 0 ] = mMaster->get_basis( 2 );
                    mVertices[ 1 ] = mMaster->get_basis( 3 );
                    mVertices[ 2 ] = mMaster->get_basis( 6 );
                    break;
                }
                case( 3 ) :
                {
                    mVertices[ 0 ] = mMaster->get_basis( 3 );
                    mVertices[ 1 ] = mMaster->get_basis( 0 );
                    mVertices[ 2 ] = mMaster->get_basis( 7 );
                    break;
                }
            }
        }

// ----------------------------------------------------------------------------
    } /* namespace hmr */
} /* namespace moris */

#endif /* SRC_HMR_CL_HMR_LAGRANGE_FACET_LINE3_HPP_ */