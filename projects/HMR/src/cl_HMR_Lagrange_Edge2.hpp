/*
 * cl_HMR_Lagrange_Edge2.hpp
 *
 *  Created on: September 27, 2018
 *  using MATLAB
 */
 
#ifndef SRC_HMR_CL_HMR_LAGRANGE_EDGE2_HPP_
#define SRC_HMR_CL_HMR_LAGRANGE_EDGE2_HPP_

#include "typedefs.hpp"
#include "cl_HMR_Lagrange_Edge.hpp"

namespace moris
{
    namespace hmr
    {
// ----------------------------------------------------------------------------

        template <>
        mtk::Interpolation_Order
        Lagrange_Edge< 2 >::get_interpolation_order() const
        {
            return mtk::Interpolation_Order::LINEAR;
        }

// ----------------------------------------------------------------------------

        template <>
        void
        Lagrange_Edge< 2 >::copy_vertex_pointers()
        {
            // get pointer to master
            Element * tMaster = mElements( mIndexOfMaster );

            // pick edges from corresponding edge
            switch ( this->get_index_on_master() )
            {
                case( 0 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 1 );
                    mVertices[ 1 ] = tMaster->get_basis( 0 );
                    break;
                }
                case( 1 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 2 );
                    mVertices[ 1 ] = tMaster->get_basis( 1 );
                    break;
                }
                case( 2 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 3 );
                    mVertices[ 1 ] = tMaster->get_basis( 2 );
                    break;
                }
                case( 3 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 3 );
                    mVertices[ 1 ] = tMaster->get_basis( 0 );
                    break;
                }
                case( 4 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 0 );
                    mVertices[ 1 ] = tMaster->get_basis( 4 );
                    break;
                }
                case( 5 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 1 );
                    mVertices[ 1 ] = tMaster->get_basis( 5 );
                    break;
                }
                case( 6 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 2 );
                    mVertices[ 1 ] = tMaster->get_basis( 6 );
                    break;
                }
                case( 7 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 3 );
                    mVertices[ 1 ] = tMaster->get_basis( 7 );
                    break;
                }
                case( 8 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 5 );
                    mVertices[ 1 ] = tMaster->get_basis( 4 );
                    break;
                }
                case( 9 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 5 );
                    mVertices[ 1 ] = tMaster->get_basis( 6 );
                    break;
                }
                case( 10 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 6 );
                    mVertices[ 1 ] = tMaster->get_basis( 7 );
                    break;
                }
                case( 11 ) :
                {
                    mVertices[ 0 ] = tMaster->get_basis( 7 );
                    mVertices[ 1 ] = tMaster->get_basis( 4 );
                    break;
                }
                default :
                {
                     MORIS_ERROR( false, "Unknown edge index");
                }
            }
        }

// ----------------------------------------------------------------------------
    } /* namespace hmr */
} /* namespace moris */
#endif /* SRC_HMR_CL_HMR_LAGRANGE_EDGE2_HPP_ */
