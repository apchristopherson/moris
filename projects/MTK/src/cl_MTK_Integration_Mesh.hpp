/*
 * cl_MTK_Integration_Mesh.hpp
 *
 *  Created on: Apr 15, 2019
 *      Author: doble
 */

#ifndef PROJECTS_MTK_SRC_CL_MTK_INTEGRATION_MESH_HPP_
#define PROJECTS_MTK_SRC_CL_MTK_INTEGRATION_MESH_HPP_

#include "cl_MTK_Mesh_Core.hpp"
#include "cl_MTK_Cell_Cluster.hpp"
#include "cl_MTK_Side_Cluster.hpp"
#include "cl_MTK_Double_Side_Cluster.hpp"
#include "assert.hpp"
#include "cl_Matrix.hpp"

namespace moris
{
namespace mtk
{
class Integration_Mesh : public virtual Mesh
{
public:
    Integration_Mesh(){};
    // Functions only valid for integration meshes

    //##############################################
    // Cell Cluster Access
    //##############################################

    /*
     * Get a cell cluster related to an interpolation
     * cell
     */
    virtual
    Cell_Cluster const &
    get_cell_cluster(Cell const & aInterpCell) const = 0;

    /*
     * Get block set names
     */
    virtual
    moris::Cell<std::string>
    get_block_set_names() const = 0;

    /*
     * Get cell clusters within a block set
     */
    virtual
    moris::Cell<Cell_Cluster const *>
    get_cell_clusters_in_set(moris_index aBlockSetOrdinal) const = 0;

    //##############################################
    // Side Set Cluster Access
    //##############################################
    /*!
     * Get side clusters within a side set
     */
    virtual
    moris::Cell<Side_Cluster const *>
    get_side_set_cluster(moris_index aSideSetOrdinal) const = 0;


    //##############################################
    // Double Side Set Cluster Access
    //##############################################

    /*!
     * Returns the number of double sided side sets in the mesh
     */
    virtual
    uint
    get_num_double_sided_sets() const  = 0;

    /*!
     * Returns the label
     */
    virtual
    std::string
    get_double_sided_set_label(moris_index aSideSetOrdinal) const = 0;

    /*!
     * Returns the index given a label
     */
    virtual
    moris_index
    get_double_sided_set_index(std::string aDoubleSideSetLabel) const  = 0;

    /*!
     * Returns the double side clusters in the side set
     */
    virtual
    moris::Cell<Double_Side_Cluster> const &
    get_double_side_set_cluster(moris_index aSideSetOrdinal) const  = 0;


    //##############################################
    // Mesh Sets Access
    //##############################################




//    virtual
//    moris::Cell<std::string>
//    get_set_names(enum EntityRank aSetEntityRank) const
//    {
//        MORIS_ERROR(0," get_set_names has no base implementation");
//        return moris::Cell<std::string>(0);
//    }
//
//    virtual
//    Matrix< IndexMat >
//    get_set_entity_loc_inds( enum EntityRank aSetEntityRank,
//                             std::string     aSetName) const
//                             {
//        MORIS_ERROR(0," get_set_entity_ids has no base implementation");
//        return Matrix< IndexMat >(0,0);
//                             }
//
//    virtual
//    void
//    get_sideset_elems_loc_inds_and_ords(
//            const  std::string     & aSetName,
//            Matrix< IndexMat >     & aElemIndices,
//            Matrix< IndexMat >     & aSidesetOrdinals ) const
//    {
//        MORIS_ERROR(0," get_sideset_elems_loc_inds_and_ords has no base implementation");
//    }
//
//
//    virtual
//    void
//    get_sideset_cells_and_ords(
//            const  std::string & aSetName,
//            moris::Cell< mtk::Cell const * > & aCells,
//            Matrix< IndexMat > &       aSidesetOrdinals ) const
//    {
//        MORIS_ERROR(0,"get_sideset_cells_and_ords not implemented");
//    }

};
}
}


#endif /* PROJECTS_MTK_SRC_CL_MTK_INTEGRATION_MESH_HPP_ */
