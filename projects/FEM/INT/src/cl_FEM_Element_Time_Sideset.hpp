/*
 * cl_FEM_Element_Time_Sideset.hpp
 *
 *  Created on: Mar 19, 2019
 *      Author: noel
 */

#ifndef SRC_FEM_CL_FEM_ELEMENT_TIME_SIDESET_HPP_
#define SRC_FEM_CL_FEM_ELEMENT_TIME_SIDESET_HPP_

#include "assert.h"
#include "cl_FEM_Cluster.hpp" //FEM/INT/src

namespace moris
{
    namespace fem
    {
    class Element_Block;
//------------------------------------------------------------------------------
    /**
     * \brief Element_Sideset class
     */
    class Element_Time_Sideset : public Cluster
    {

//------------------------------------------------------------------------------
    protected:
//------------------------------------------------------------------------------
//        // a member list of time side ordinals
//        moris::Cell< moris_index > mListOfTimeOrdinals;

//------------------------------------------------------------------------------
    public:
//------------------------------------------------------------------------------

        /**
         * constructor
         *
         * @param[ in ]     pointer to mesh interface object
         * @param[ in ]     cell of pointers to integrand of weak form of governing eqs.
         * @param[ in ]     cell of pointer to fem nodes
         * @param[ in ]     Pointer to element block
         */
        Element_Time_Sideset( mtk::Cell            const * aCell,
                              moris::Cell< Node_Base* >  & aNodes,
                              Element_Block      * aElementBlock);
//------------------------------------------------------------------------------
        /**
         * destructor
         */
        ~Element_Time_Sideset();

//------------------------------------------------------------------------------
        /**
         * compute jacobian over the element
         */
        void compute_jacobian();

//------------------------------------------------------------------------------
        /**
         * compute residual over the element
         */
        void compute_residual();

//------------------------------------------------------------------------------
        /**
         * compute jacobian and residual over the element
         */
        void compute_jacobian_and_residual();

//------------------------------------------------------------------------------
    protected:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
    };

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */


#endif /* SRC_FEM_CL_FEM_ELEMENT_TIME_SIDESET_HPP_ */
