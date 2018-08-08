/*
 * cl_FEM_Element.hpp
 *
 *  Created on: Jul 16, 2018
 *      Author: messe
 */

#ifndef SRC_FEM_CL_FEM_ELEMENT_HPP_
#define SRC_FEM_CL_FEM_ELEMENT_HPP_

#include "assert.h"


#include "cl_FEM_Interpolation_Matrix.hpp"
#include "typedefs.hpp" //MRS/COR/src
#include "cl_MTK_Enums.hpp" //MTK/src
#include "cl_MTK_Cell.hpp" //MTK/src
#include "cl_FEM_Enums.hpp" //FEM/INT/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------

    /**
     * \brief element class that communicates with the mesh interface
     */
    class Element
    {
        //! pointer to cell on mesh
        mtk::Cell * mCell;

//------------------------------------------------------------------------------
    public:
//------------------------------------------------------------------------------

        /**
         * constructor
         *
         * @param[ in ]     pointer to mesh interface object
         * @param[ in ]     proc local index of element
         */
        Element( mtk::Cell * aCell );

//------------------------------------------------------------------------------

        /**
         * trivial destructor
         */
        ~Element(){};
//------------------------------------------------------------------------------

        /**
         * returns the elememt geometry type
         */
        mtk::Geometry_Type
        get_geometry_type() const ;

//------------------------------------------------------------------------------

        /**
         * returns the node coordinates of the element
         */
        Mat<real>
        get_node_coords() const ;

//------------------------------------------------------------------------------

        /**
         * returns the interpolation order of the underlying cell
         */
        mtk::Interpolation_Order
        get_interpolation_order() const;

//------------------------------------------------------------------------------

        /**
         * returns a cell with the vertices that are connected to this element
         */
        moris::Cell< mtk::Vertex* >
        get_vertex_pointers();

//------------------------------------------------------------------------------

        /**
         * returns a moris::Mat with ids of vertices that are connected to this element
         */
        Mat< luint >
        get_vertex_ids() const;

//------------------------------------------------------------------------------

        void
        eval_mass( Mat< real > & aM );

//------------------------------------------------------------------------------

        /**
         * how many nodes are connected to this element
         */
        uint
        get_number_of_nodes() const;

//------------------------------------------------------------------------------
    };

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */




#endif /* SRC_FEM_CL_FEM_ELEMENT_HPP_ */