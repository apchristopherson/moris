/*
 * cl_MDL_Node_Proxy.hpp
 *
 *  Created on: Mai 22, 2019
 *      Author: Schmidt
 */
#ifndef PROJECTS_GEN_SRC_CL_MDL_NODE_PROXY_HPP_
#define PROJECTS_GEN_SRC_CL_MDL_NODE_PROXY_HPP_

#include "cl_Matrix.hpp"
#include "cl_MTK_Vertex.hpp"

namespace moris
{
    namespace mdl
    {
        class Node : public mtk::Vertex
        {
//------------------------------------------------------------------------------
        private:
//            moris::Matrix< DDRMat > coord;
            moris::sint mIndex = -1;
            moris::sint mID = -1;

//------------------------------------------------------------------------------

        public:
//------------------------------------------------------------------------------

            Node( const sint aIndex, const sint aID ) : mIndex( aIndex ),
                                                        mID( aID )
            {};

//------------------------------------------------------------------------------

            ~Node(){};

        };

    } /* namespace mdl */

} /* namespace moris */




#endif /* PROJECTS_GEN_SRC_CL_MDL_NODE_PROXY_HPP_ */
