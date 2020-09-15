/*
 * cl_FEM_Cluster.hpp
 *
 *  Created on: Apr 20, 2019
 *      Author: schmidt
 */

#ifndef SRC_FEM_CL_FEM_CLUSTER_HPP_
#define SRC_FEM_CL_FEM_CLUSTER_HPP_

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
#include "cl_FEM_Element_Factory.hpp"       //FEM/INT/src
#include "cl_FEM_Set.hpp"                   //FEM/INT/src

namespace moris
{
    namespace fem
    {
        class Set;
        //------------------------------------------------------------------------------

        class Cluster
        {

            protected:

                // pointer to the mesh cluster
                const mtk::Cluster* mMeshCluster = nullptr;

                MSI::Equation_Object* mInterpolationElement = nullptr;

                // time sideset information
                Matrix< IndexMat > mListOfTimeOrdinals;

                // list of pointers to the master and slave mesh integration cells
                moris::Cell<const mtk::Cell *> mMasterIntegrationCells;
                moris::Cell<const mtk::Cell *> mSlaveIntegrationCells;

                // master and slave side ordinal information
                Matrix< IndexMat > mMasterListOfSideOrdinals;
                Matrix< IndexMat > mSlaveListOfSideOrdinals;

                // list of pointers to element
                moris::Cell< fem::Element * > mElements;

                // pointer to the fem set
                Set * mSet;

                // element type
                Element_Type mElementType;

                friend class Element_Bulk;
                friend class Element_Sideset;
                friend class Element_Double_Sideset;
                friend class Element_Time_Sideset;
                friend class Element_Time_Boundary;
                friend class Element;

                //------------------------------------------------------------------------------
            public:
                //------------------------------------------------------------------------------
                /**
                 * trivial constructor
                 */
                Cluster(){};

                /**
                 * constructor
                 * @param[ in ] aElementType    enum for element type (BULK, SIDESET, ...)
                 * @param[ in ] aMeshCluster    cluster pointer from mtk mesh
                 * @param[ in ] aSet            a fem set
                 * @param[ in ] aEquationObject pointer to the corresponding interpolation element
                 */
                Cluster(
                        const Element_Type     aElementType,
                        const mtk::Cluster   * aMeshCluster,
                        Set                  * aSet,
                        MSI::Equation_Object * aEquationObject );

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~Cluster();

                //------------------------------------------------------------------------------
                /**
                 * get mesh cluster
                 * @param[ out ] mMeshCluster a mesh cluster
                 */
                const mtk::Cluster * get_mesh_cluster()
                {
                    return mMeshCluster;
                }

                //------------------------------------------------------------------------------
                /**
                 * get side ordinal information
                 * @param[ out ] mMeshCluster a mesh cluster
                 */
                Matrix< IndexMat > & get_side_ordinal_info(
                        mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER )
                {
                    switch( aIsMaster )
                    {
                        case mtk::Master_Slave::MASTER :
                            return mMasterListOfSideOrdinals;

                        case mtk::Master_Slave::SLAVE :
                            return mSlaveListOfSideOrdinals;

                        default:
                            MORIS_ERROR( false, "Cluster::get_side_ordinal_info - can only be master or slave." );
                            return mMasterListOfSideOrdinals;
                    }
                }

                //------------------------------------------------------------------------------
                /**
                 * get the vertices local coordinates on the IP cell
                 */
                moris::Matrix< moris::DDRMat > get_vertices_local_coordinates_wrt_interp_cell();

                //------------------------------------------------------------------------------
                /**
                 * get the vertices indices in cluster
                 */
                moris::Cell< moris_index > get_vertex_indices_in_cluster();

                //------------------------------------------------------------------------------
                /**
                 * get the vertices indices in cluster
                 */
                void get_vertex_indices_in_cluster_for_sensitivity(
                        moris::Matrix< moris::IndexMat > & aVerticesIndices );

                //------------------------------------------------------------------------------
                /**
                 * get the IG cell local coordinates on the side wrt to the IP cell
                 * @param[ in ] aCellIndexInCluster index of the IG cell within the cluster
                 * @param[ in ] aSideOrdinal        ordinal for the side
                 * @param[ in ] aIsMaster           enum for master or slave
                 */
                moris::Matrix< moris::DDRMat > get_cell_local_coords_on_side_wrt_interp_cell(
                        moris::moris_index aCellIndexInCluster,
                        moris::moris_index aSideOrdinal,
                        mtk::Master_Slave  aIsMaster = mtk::Master_Slave::MASTER );

                //------------------------------------------------------------------------------
                /**
                 * get the IG cell local coordinates wrt IP cell
                 * @param[ in ] aPrimaryCellIndexInCluster index of the IG cell within the cluster
                 */
                moris::Matrix< moris::DDRMat > get_primary_cell_local_coords_on_side_wrt_interp_cell(
                        moris::moris_index aPrimaryCellIndexInCluster );

                //------------------------------------------------------------------------------
                /**
                 * get side normal
                 * @param[ in ] aCell        mesh cell pointer
                 * @param[ in ] aSideOrdinal ordinal of the side where normal is evaluated
                 */
                Matrix< DDRMat > get_side_normal(
                        const mtk::Cell    * aCell,
                        moris::moris_index   aSideOrdinal );

                //------------------------------------------------------------------------------
                /**
                 * get the index of the vertex associated with a given master vertex
                 * @param[ in ] aLeftVertex mesh vertex pointer
                 */
                const moris::mtk::Vertex * get_left_vertex_pair(
                        const moris::mtk::Vertex * aLeftVertex );

                //------------------------------------------------------------------------------
                /**
                 * get the ordinal of the right vertex on the facet
                 * @param[ in ] aCellIndexInCluster an index for the cell in the cluster
                 * @param[ in ] aVertex             a vertex pointer
                 */
                moris::moris_index get_right_vertex_ordinal_on_facet(
                        moris_index                aCellIndexInCluster,
                        const moris::mtk::Vertex * aVertex );

                //------------------------------------------------------------------------------
                /**
                 * compute the jacobian on cluster
                 */
                void compute_jacobian();

                //------------------------------------------------------------------------------
                /**
                 * compute the residual on cluster
                 */
                void compute_residual();

                //------------------------------------------------------------------------------
                /**
                 * compute the jacobian and the residual on cluster
                 */
                void compute_jacobian_and_residual();

                //------------------------------------------------------------------------------
                /**
                 * compute the quantity of interest on cluster
                 * @param[ in ] aOutputType an enum for output type
                 * @param[ in ] aFieldType  an enum for computation/return type
                 *                          GLOBAL, NODAL, ELEMENTAL
                 */
                void compute_quantity_of_interest(
                        const uint             aMeshIndex,
                        const std::string    & aQIName,
                        enum vis::Field_Type   aFieldType );

                //------------------------------------------------------------------------------
                /**
                 * compute dRdp by analytical formulation
                 */
                void compute_dRdp();

                //------------------------------------------------------------------------------
                /**
                 * compute the quantities of interest on cluster
                 */
                void compute_QI();

                //------------------------------------------------------------------------------
                /**
                 * compute dQIdp by analytical formulation
                 */
                void compute_dQIdp_explicit();

                //------------------------------------------------------------------------------
                /**
                 * compute dQIdu
                 */
                void compute_dQIdu();

                //------------------------------------------------------------------------------
                /**
                 * compute the cluster volume
                 */
                real compute_volume();

                //------------------------------------------------------------------------------
                /*
                 * Compute the measure (volume 3d or area 2d) of the cells in the void or primary phase
                 */
                moris::real compute_cluster_cell_measure(
                        const mtk::Primary_Void aPrimaryOrVoid = mtk::Primary_Void::PRIMARY,
                        const mtk::Master_Slave aIsMaster      = mtk::Master_Slave::MASTER ) const;

                //------------------------------------------------------------------------------
                /*
                 * Compute the side measure (surface area 3d or length 2d) of the cells in the void or primary phase on the side set.
                 * Only valid on side cluster type mtk clusters
                 */
                moris::real compute_cluster_cell_side_measure(
                        const mtk::Primary_Void aPrimaryOrVoid = mtk::Primary_Void::PRIMARY,
                        const mtk::Master_Slave aIsMaster      = mtk::Master_Slave::MASTER ) const;

                //------------------------------------------------------------------------------
                /*
                 * Compute the element size (length) of the cells in the void or primary phase
                 */
                moris::real compute_cluster_cell_length_measure(
                        const mtk::Primary_Void aPrimaryOrVoid = mtk::Primary_Void::PRIMARY,
                        const mtk::Master_Slave aIsMaster      = mtk::Master_Slave::MASTER ) const;

                //------------------------------------------------------------------------------
            protected:

                //------------------------------------------------------------------------------
        };

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_CLUSTER_HPP_ */
