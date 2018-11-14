/*
 * MTK_Tools.hpp
 *
 *  Created on: Nov 13, 2018
 *      Author: messe
 */

#ifndef PROJECTS_MTK_SRC_MTK_TOOLS_HPP_
#define PROJECTS_MTK_SRC_MTK_TOOLS_HPP_

#include "assert.hpp"
#include "typedefs.hpp"
#include "cl_MTK_Enums.hpp"
namespace moris
{
    namespace mtk
    {

// ----------------------------------------------------------------------------

        /**
         * converts an interpolation order to a numeric value
         */
        uint
        interpolation_order_to_uint( const Interpolation_Order& aOrder )
        {
            switch( aOrder )
            {
                case( Interpolation_Order::CONSTANT ) :
                {
                    return 0;
                    break;
                }
                case( Interpolation_Order::LINEAR ) :
                {
                    return 1;
                    break;
                }
                case( Interpolation_Order::SERENDIPITY ) :
                {
                    MORIS_ERROR( false, "Interpolation_Order::SERENDIPITY cannot be converted to uint" );
                    return 0;
                    break;
                }
                case( Interpolation_Order::QUADRATIC ) :
                {
                    return 2;
                    break;

                }
                case( Interpolation_Order::CUBIC ) :
                {
                    return 3;
                    break;
                }
                default :
                {
                    MORIS_ERROR( false, "unknown interpolation order" );
                    return 0;
                    break;
                }
            }
        }

// ----------------------------------------------------------------------------

        /**
         * returns an interpolation order for a given dimension and number of nodes
         */
        Interpolation_Order
        get_interpolation_order_from_element_nodes_and_dimension(
                const uint aNumberOfNodes,
                const uint aNumberOfDimensions )
        {
            switch( aNumberOfDimensions )
            {
                case( 1 ) :
                {
                     switch( aNumberOfNodes )
                     {
                        case( 1 ) :
                        {
                            return Interpolation_Order::CONSTANT;
                            break;
                        }
                        case( 2 ) :
                        {
                            return Interpolation_Order::LINEAR;
                            break;
                        }
                        case( 3 ) :
                        {
                            return Interpolation_Order::QUADRATIC;
                            break;
                        }
                        case( 4 ) :
                        {
                            return Interpolation_Order::CUBIC;
                            break;
                        }
                        default :
                        {
                            MORIS_ERROR( false, "Unknown number of nodes");
                            return Interpolation_Order::UNDEFINED;
                            break;
                        }
                     }
                     break;
                }
                case( 2 ) :
                {
                    switch( aNumberOfNodes )
                    {
                        case( 1 ) : // vertex
                        {
                            return Interpolation_Order::CONSTANT;
                            break;
                        }
                        case( 3 ) : // tri 3
                        case( 4 ) : // quad 4
                        {
                            return Interpolation_Order::LINEAR;
                            break;
                        }
                        case( 8 ) : // quad 8
                        {
                            return Interpolation_Order::SERENDIPITY;
                            break;
                        }
                        case( 6 ) : // tri 6
                        case( 9 ) : // quad 9
                        {
                            return Interpolation_Order::QUADRATIC;
                            break;
                        }
                        case( 10 ) : // tri 10
                        case( 16 ) : // quad 16
                        {
                            return Interpolation_Order::CUBIC;
                            break;
                        }
                        default :
                        {
                            MORIS_ERROR( false, "Unknown number of nodes");
                            return Interpolation_Order::UNDEFINED;
                            break;
                        }
                    }
                    break;
                }
                case( 3 ) :
                {
                    switch( aNumberOfNodes )
                    {
                        case( 1 ) : // vertex
                        {
                            return Interpolation_Order::CONSTANT;
                            break;
                        }
                        case( 4 ) : // tet 4
                        case( 8 ) : // hex 8
                        {
                            return Interpolation_Order::LINEAR;
                            break;
                        }
                        case( 20 ) : // hex 20
                        {
                            return Interpolation_Order::SERENDIPITY;
                            break;
                        }
                        case( 10 ) : // tet 10
                        case( 27 ) : // hex 27
                        {
                            return Interpolation_Order::QUADRATIC;
                            break;
                        }
                        // case( 20 ) : tet 20
                        case( 64 ) : // hex 64
                        {
                            return Interpolation_Order::CUBIC;
                            break;
                        }
                        default :
                        {
                            MORIS_ERROR( false, "Unknown number of nodes");
                            return Interpolation_Order::UNDEFINED;
                            break;
                        }
                    }
                    break;
                }
                default :
                {
                    MORIS_ERROR( false, "Unknown number of dimensions" );
                    return Interpolation_Order::UNDEFINED;
                    break;
                }
            }
        }

// ----------------------------------------------------------------------------
    } /* namespace mtk */
} /* namespace moris */



#endif /* PROJECTS_MTK_SRC_MTK_TOOLS_HPP_ */
