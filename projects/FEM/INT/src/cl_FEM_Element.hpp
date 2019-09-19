/*
 * cl_FEM_Element.hpp
 *
 *  Created on: Apr 20, 2019
 *      Author: Schmidt
 */

#ifndef SRC_FEM_CL_FEM_ELEMENT_HPP_
#define SRC_FEM_CL_FEM_ELEMENT_HPP_

#include "assert.h"
#include <cmath>

#include "typedefs.hpp"                     //MRS/COR/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_Cell.hpp"
#include "cl_MTK_Cell.hpp"                  //MTK/src
#include "cl_MSI_Equation_Object.hpp"       //FEM/MSI/src
#include "cl_FEM_Enums.hpp"                 //FEM/INT/src
#include "cl_FEM_Node.hpp"                  //FEM/INT/src
#include "cl_FEM_IWG.hpp"                   //FEM/INT/src
#include "cl_FEM_Geometry_Interpolator.hpp" //FEM/INT/src
#include "cl_FEM_Field_Interpolator.hpp"    //FEM/INT/src
#include "cl_FEM_Integrator.hpp"            //FEM/INT/src

#include "cl_FEM_Set.hpp"   //FEM/INT/src
#include "cl_FEM_Cluster.hpp"   //FEM/INT/src

namespace moris
{
    namespace fem
    {
    class Set;
//------------------------------------------------------------------------------
    /**
     * \brief element class that communicates with the mesh interface
     */
    class Element
    {

    protected:

        //! pointer to master and slave integration cells on mesh
        const mtk::Cell * mMasterCell;
        const mtk::Cell * mSlaveCell;

        moris::moris_index mCellIndexInCluster;

        //! node indices of this element
        //  @node: MTK interface returns copy of vertices. T
        //         storing the indices in private matrix is faster,
        //         but might need more memory
        moris::Matrix< IndexMat > mNodeIndices;

        Set      * mSet     = nullptr;
        Cluster  * mCluster = nullptr;
//------------------------------------------------------------------------------
    public:
//------------------------------------------------------------------------------

        Element( const mtk::Cell * aCell,
                 Set             * aSet,
                 Cluster         * aCluster,
                 moris::moris_index tCellIndexInCluster ) : mSet( aSet ),
                                                            mCluster( aCluster )
        {
            // fill the cell index in cluster
            mCellIndexInCluster = tCellIndexInCluster;

            // fill the bulk mtk::Cell pointer //FIXME
            mMasterCell = aCell;

            // select the element nodes from aNodes and fill mNodeObj
            // get vertices from cell
            moris::Cell< mtk::Vertex* > tVertices = mMasterCell->get_vertex_pointers();

            // get number of nodes from cell
            uint tNumOfNodes = tVertices.size();

            // set size of Weak BCs
            mCluster->get_weak_bcs().set_size( tNumOfNodes, 1 );             // FIXME
        };

        Element( const mtk::Cell  * aMasterCell,
                 const mtk::Cell  * aSlaveCell,
                 Set              * aSet,
                 Cluster          * aCluster,
                 moris::moris_index tCellIndexInCluster ) : mSet( aSet ),
                                                            mCluster( aCluster )
        {
            // fill the cell index in cluster
            mCellIndexInCluster = tCellIndexInCluster;

            // fill the master and slave cell pointers
            mMasterCell = aMasterCell;
            mSlaveCell  = aSlaveCell;
        };
//------------------------------------------------------------------------------
        /**
         * trivial destructor
         */
        virtual ~Element(){};

//------------------------------------------------------------------------------

        virtual void compute_jacobian() = 0;

//------------------------------------------------------------------------------

        virtual void compute_residual() = 0;

//------------------------------------------------------------------------------

        virtual void compute_jacobian_and_residual() = 0;

//------------------------------------------------------------------------------
    protected:
//------------------------------------------------------------------------------
        /**
         * compute element volume
         */
        real compute_element_volume( Geometry_Interpolator* aGeometryInterpolator )
        {
            //get number of integration points
            uint tNumOfIntegPoints = mSet->get_number_of_integration_points();

            // init volume
            real tVolume = 0;

            // loop over integration points
            for( uint iGP = 0; iGP < tNumOfIntegPoints; iGP++ )
            {
                // set integration point for geometry interpolator
                aGeometryInterpolator->set_space_time( mSet->get_integration_points().get_column( iGP ) );

                // compute integration point weight x detJ
                real tWStar = aGeometryInterpolator->det_J()
                            * mSet->get_integration_weights()( iGP );

                // add contribution to jacobian from evaluation point
                //FIXME: include a thickness if 2D
                tVolume += tWStar;
            }

            // FIXME: compute the element size + switch 1D, 2D, 3D
            //real he = std::pow( 6*tVolume/M_PI, 1.0/3.0 );
            //real he = std::pow( 4*tVolume/M_PI, 1.0/2.0 );
            //std::cout<<he<<std::endl;

            return tVolume;
        }

//------------------------------------------------------------------------------
    };

//------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_ELEMENT_HPP_ */
