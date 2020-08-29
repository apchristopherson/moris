/*
 * cl_XTK_Enriched_Integration_Mesh.hpp
 *
 *  Created on: Jul 22, 2019
 *      Author: doble
 */
#ifndef PROJECTS_XTK_SRC_XTK_CL_XTK_ENRICHED_INTEGRATION_MESH_HPP_
#define PROJECTS_XTK_SRC_XTK_CL_XTK_ENRICHED_INTEGRATION_MESH_HPP_

#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Vertex.hpp"
#include "typedefs.hpp"
#include "cl_Matrix.hpp"
#include "cl_Cell.hpp"
#include "cl_XTK_Field.hpp"
#include <unordered_map>
#include "cl_TOL_Memory_Map.hpp"

using namespace moris;

namespace xtk
{
class Model;
class Cell_Cluster;
class Side_Cluster;
class Interpolation_Cell_Unzipped;
class Ghost_Stabilization;


class Enriched_Integration_Mesh : public mtk::Integration_Mesh
{
public:
    //------------------------------------------------------------------------------
    Enriched_Integration_Mesh(Model*             aXTKModel,
                              moris::moris_index aInterpIndex);
    //------------------------------------------------------------------------------
    ~Enriched_Integration_Mesh();
    //------------------------------------------------------------------------------
    // MTK Mesh Core Functionality (see base class mtk::Mesh for documentation)
    //------------------------------------------------------------------------------
    MeshType                  get_mesh_type() const;
    moris::uint               get_spatial_dim() const;
    uint                      get_num_entities( enum EntityRank aEntityRank, const moris_index aIndex =0 ) const;
    Matrix< IndexMat >        get_entity_connected_to_entity_loc_inds(moris_index aEntityIndex, enum EntityRank aInputEntityRank, enum EntityRank aOutputEntityRank,const moris_index aIndex =0) const;
    Matrix< IndexMat >        get_elements_connected_to_element_and_face_ind_loc_inds(moris_index aElementIndex) const;
    moris_id                  get_glb_entity_id_from_entity_loc_index(moris_index aEntityIndex,enum EntityRank aEntityRank, const moris_index aIndex =0) const;
    moris_index               get_loc_entity_ind_from_entity_glb_id( moris_id aEntityId, enum EntityRank aEntityRank, const moris_index aIndex =0) const;
    Cell<mtk::Vertex const *> get_all_vertices() const;
    Matrix< IdMat >           get_entity_connected_to_entity_glob_ids( moris_id aEntityId, enum EntityRank aInputEntityRank, enum EntityRank aOutputEntityRank, const moris_index aIndex =0) const;
    Matrix< DDRMat >          get_node_coordinate( moris_index aNodeIndex ) const;
    mtk::Vertex &             get_mtk_vertex( moris_index aVertexIndex );
    mtk::Vertex const &       get_mtk_vertex( moris_index aVertexIndex ) const;
    mtk::Cell &               get_writable_mtk_cell( moris_index aElementIndex );
    mtk::Cell &               get_mtk_cell( moris_index aElementIndex );
    mtk::Cell const &         get_mtk_cell( moris_index aElementIndex ) const;
    Matrix< IdMat >           get_communication_table() const;
    moris::Cell<std::string>  get_set_names(enum EntityRank aSetEntityRank) const;
    enum CellTopology         get_blockset_topology(const  std::string & aSetName);
    Matrix< IndexMat >        get_set_entity_loc_inds( enum EntityRank aSetEntityRank, std::string     aSetName) const;
    void                      get_sideset_elems_loc_inds_and_ords( const  std::string     & aSetName, Matrix< IndexMat >     & aElemIndices, Matrix< IndexMat >     & aSidesetOrdinals ) const;
    moris_id                  get_max_entity_id( enum EntityRank aEntityRank,const moris_index aIndex =0 ) const;

    //------------------------------------------------------------------------------
    // end mesh core functions
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    // MTK Integration Mesh Functions
    // see base class mtk::Integration_Mesh for documentation
    //------------------------------------------------------------------------------
    mtk::Cell_Cluster const &         get_cell_cluster(mtk::Cell const & aInterpCell) const;
    Cell_Cluster const &              get_cell_cluster(moris_index aInterpCellIndex) const;
    moris::Cell<std::string>          get_block_set_names() const;
    std::string                       get_block_set_label(moris_index aBlockSetOrdinal) const;
    moris_index                       get_block_set_index(std::string aBlockSetLabel) const;
    moris::Cell<mtk::Cluster const *> get_cell_clusters_in_set(moris_index aBlockSetOrdinal) const;
    Matrix<IndexMat>                  get_block_set_colors(moris_index aBlockSetOrdinal) const;
    moris::Cell<mtk::Cluster const *> get_side_set_cluster(moris_index aSideSetOrdinal) const;
    Matrix<IndexMat>                  get_side_set_colors(moris_index aSideSetOrdinal) const;
    uint                              get_num_side_sets() const;
    std::string                       get_side_set_label(moris_index aSideSetOrdinal) const;
    moris_index                       get_side_set_index(std::string aSideSetLabel) const;
    uint                              get_num_double_sided_sets() const;
    std::string                       get_double_sided_set_label(moris_index aSideSetOrdinal) const;
    moris_index                       get_double_sided_set_index(std::string aDoubleSideSetLabel) const;
    moris::Cell<mtk::Cluster const*>  get_double_side_set_cluster(moris_index aSideSetOrdinal) const;
    Matrix<IndexMat>                  get_double_side_set_colors(moris_index aSideSetOrdinal) const;
    uint                              get_sidesets_num_faces( moris::Cell< moris_index > aSideSetIndex ) const;
    //------------------------------------------------------------------------------
    // end integration mesh functions
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    // Additional Set Functions
    //------------------------------------------------------------------------------
    moris::Cell<xtk::Cell_Cluster const  *>  const & get_xtk_cell_clusters_in_block_set(moris_index aBlockSetOrdinal) const;

    /*
     * Get the side set name of the interface.
     */
    std::string
    get_interface_side_set_name(moris_index aGeomIndex,
                                moris_index aBulkPhaseIndex0,
                                moris_index aBulkPhaseIndex1);

    std::string
    get_dbl_interface_side_set_name(moris_index aBulkPhaseIndex0,
                                    moris_index aBulkPhaseIndex1);

    /*!
     * Returns the primary cell local indices in a block set.
     */
    Matrix< IndexMat >
    get_block_entity_loc_inds( std::string     aSetName) const;

    /*!
     * This function creates additional dbl sided interfaces. By default,
     * the enriched integrztion mesh creates only the low-master high-slave
     * dbl sided interfaces. This functions allows creation of  low-slave high-master
     * interfaces.
     */
    void
    create_dbl_sided_interface_set(moris_index aMasterBulkPhaseIndex,
                                   moris_index aSlaveBulkPhaseIndex);


    //------------------------------------------------------------------------------
    // Output/ Viz Functions
    //------------------------------------------------------------------------------
    /*
     * For cleanup when writing to an exodus file (note: in general this should not be used because
     * sets may not be always empty through an optimization run)
     */
    void
    deactivate_empty_sets();

    void
    deactivate_empty_side_sets();

    void
    deactivate_empty_block_sets();

    moris::Cell<std::string>
    create_basis_support_fields();

    //------------------------------------------------------------------------------
    // Memory Map
    //------------------------------------------------------------------------------
    /*!
     * @brief get the memory usage map
     */
    moris::Memory_Map
    get_memory_usage();

    //------------------------------------------------------------------------------
    // Additional Field Functions
    //------------------------------------------------------------------------------
    /*!
     * Create a field in the enriched integration mesh. aBulkphaseIndex of MORIS_
     * INDEX_MAX results in a field over all phases. (returns the field index)
     */
    moris::moris_index
    create_field(std::string            aLabel,
                 enum moris::EntityRank aEntityRank,
                 moris::moris_index     aBulkPhaseIndex = MORIS_INDEX_MAX);
    //------------------------------------------------------------------------------
    /*!
     * Returns the field index in the member data vector, more efficient to do
     * this once and access the field directly through the index
     */
    moris::moris_index
    get_field_index(std::string              aLabel,
                    enum moris::EntityRank   aEntityRank);
    //------------------------------------------------------------------------------
    /*!
     * Add field data to created field.
     */
    void
    add_field_data(moris::moris_index       aFieldIndex,
                   enum moris::EntityRank   aEntityRank,
                   Matrix<DDRMat>  const  & aFieldData);

    /*!
     * return field data on a specified field
     */
    Matrix<DDRMat> const   &
    get_field_data(moris::moris_index       aFieldIndex,
                   enum moris::EntityRank   aEntityRank) const;
    //------------------------------------------------------------------------------
    /*
     * Convert a entity indices to entity ids
     */
    Matrix<IdMat> convert_indices_to_ids(Matrix<IndexMat> const & aIndices,
                                         enum EntityRank          aEntityRank) const;
    //------------------------------------------------------------------------------
    /*
     * Convert a entity ids to entity indices
     */
    Matrix<IndexMat> convert_ids_to_indices(Matrix<IdMat> const & aIds,
                                            enum EntityRank       aEntityRank) const;
    //------------------------------------------------------------------------------

    /*
     * Get multple mtk cells from cell index matrix
     */
    moris::Cell<moris::mtk::Cell const *>
    get_mtk_cells_loc_inds(Matrix<IndexMat> const & aCellIndices);
    //------------------------------------------------------------------------------
    /*
     * Get multple mtk vertices from vertex index matrix
     */
    moris::Cell<moris::mtk::Vertex const *>
    get_mtk_vertices_loc_inds(Matrix<IndexMat> const & aVertexIndices);
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    // Accessor functions of XTK specific data structures
    //------------------------------------------------------------------------------
    /*!
     * get the xtk cell cluster associated with an interpolation cell
     */
    xtk::Cell_Cluster const &
    get_xtk_cell_cluster(mtk::Cell const & aInterpCell) const;
    //------------------------------------------------------------------------------
    // Debug
    //------------------------------------------------------------------------------


    //------------------------------------------------------------------------------
    // Printing
    //------------------------------------------------------------------------------
    void print() const;
    void print_cell_clusters(moris::uint aVerbosityLevel = 0) const;
    void print_block_sets(moris::uint aVerbosityLevel = 0) const;
    void print_side_sets(moris::uint aVerbosityLevel = 0) const;
    void print_double_side_sets(moris::uint aVerbosityLevel = 0) const;
    void print_double_side_clusters(moris::uint aVerbosityLevel = 0) const;

    //--------------------------------------------------------------------------------
    // Utilities for manipulating sets
    //--------------------------------------------------------------------------------
    moris_index
    create_side_set_from_dbl_side_set(moris_index const & aDblSideSetIndex,
                                      std::string const & aSideSetName,
                                      bool aCollectSets = true);

    moris_index
    create_block_set_from_cells_of_side_set(moris_index const & aSideSetIndex,
                                            std::string const & aSideSetName,
                                            enum CellTopology   aCellTopo);

    friend class Enrichment;
    friend class Ghost_Stabilization;
protected:
    Model* mModel;

    //mesh index
    moris::moris_index mMeshIndexInModel;

    // Cell Clusters
    moris::Cell< std::shared_ptr<xtk::Cell_Cluster> > mCellClusters;

    // Vertex Set
    std::unordered_map<std::string, moris_index>  mVertexSetLabelToOrd;
    moris::Cell<std::string>                      mVertexSetNames;
    moris::Cell<moris::Cell<moris::mtk::Vertex*>> mVerticesInVertexSet;
    moris::Cell<moris::Matrix<IndexMat>>          mVertexSetColors;

    // Block sets containing Cell Clusters
    std::unordered_map<std::string, moris_index>        mBlockSetLabelToOrd;
    moris::Cell<std::string>                            mBlockSetNames;
    moris::Cell<enum CellTopology>                      mBlockSetTopology;
    moris::Cell<moris::Cell<xtk::Cell_Cluster const *>> mPrimaryBlockSetClusters;
    moris::Cell<moris::Matrix<IndexMat>>                mBlockSetColors; /*Bulk phases*/
    moris::Cell<moris::Cell<moris_index>>               mColorsBlockSets; /*transpose of mBlockSetColors*/

    // side sets
    std::unordered_map<std::string, moris_index>                 mSideSideSetLabelToOrd;
    moris::Cell<std::string>                                     mSideSetLabels;
    moris::Cell<moris::Cell<std::shared_ptr<xtk::Side_Cluster>>> mSideSets;
    moris::Cell<moris::Matrix<IndexMat>>                         mSideSetColors; /*Bulk phases of cells attached to side*/
    moris::Cell<moris::Cell<moris_index>>                        mColorsSideSets; /*transpose of mSideSetColors*/

    // double side sets
    std::unordered_map<std::string, moris_index>        mDoubleSideSetLabelToOrd;
    moris::Cell<std::string>                            mDoubleSideSetLabels;
    moris::Cell<moris::Cell<mtk::Double_Side_Cluster*>> mDoubleSideSets;
    moris::Cell<moris::Cell<moris_index>>               mDoubleSideSetsMasterIndex;
    moris::Cell<moris::Cell<moris_index>>               mDoubleSideSetsSlaveIndex;
    moris::Cell<mtk::Double_Side_Cluster*>              mDoubleSideClusters;
    moris::Cell<std::shared_ptr<xtk::Side_Cluster>>     mDoubleSideSingleSideClusters; /*lefts and rights of the double side sets*/
    moris::Matrix<moris::IndexMat>                      mBulkPhaseToDblSideIndex;
    moris::Cell<moris::Matrix<IndexMat>>                mMasterDoubleSideSetColor;
    moris::Cell<moris::Matrix<IndexMat>>                mSlaveDoubleSideSetColor;
    moris::Cell<moris::Cell<moris_index>>               mColorMasterDoubleSideSet; /*transpose of mMasterDoubleSideSetColor*/
    moris::Cell<moris::Cell<moris_index>>               mColorSlaveDoubleSideSet; /*transpose of mSlaveDoubleSideSetColor*/

    // Fields
    moris::Cell<xtk::Field> mFields;   /*Structure Node (0), Cell(1)*/
    moris::Cell<std::unordered_map<std::string, moris_index>> mFieldLabelToIndex;

    // Sub phase index to Cell Cluster Index (these only include the standard cluster i.e. non-ghost clusters.)
    moris::Matrix<moris::IndexMat> mSubphaseIndexToClusterIndex;

    // a connectivity pointer used by all transition cells
    moris::mtk::Cell_Info* mCellInfo;

    //------------------------------------------------------------------------------
    // Parallel functions
    //------------------------------------------------------------------------------
    moris_id allocate_entity_ids(moris::size_t  aNumReqs, enum EntityRank aEntityRank);

    void
    commit_double_side_set(moris_index const & aDoubleSideSetIndex);

    void
    commit_side_set(moris_index const & aSideSetIndex);

    void
    commit_block_set(moris_index const & aBlockSetIndex);

private:
    //------------------------------------------------------------------------------
    void
    setup_cell_clusters();
    //------------------------------------------------------------------------------
    void
    setup_blockset_with_cell_clusters();
    //------------------------------------------------------------------------------
    void
    setup_side_set_clusters();
    //------------------------------------------------------------------------------
    void
    setup_double_side_set_clusters();
    //------------------------------------------------------------------------------
    void
    setup_color_to_set();
    //------------------------------------------------------------------------------
    void
    setup_double_sided_interface_sides();
    //------------------------------------------------------------------------------
    void
    declare_interface_double_side_sets();
    //------------------------------------------------------------------------------
    moris_index
    get_dbl_side_set_index(moris_index aPhase0,
                           moris_index aPhase1);
    //------------------------------------------------------------------------------
    void
    create_interface_double_side_sets_and_clusters();
    //------------------------------------------------------------------------------
    moris::Cell<std::string>
    split_set_name_by_bulk_phase(std::string aBaseName);
    //------------------------------------------------------------------------------
    moris::Cell<std::string>
    split_set_name_by_child_no_child(std::string aBaseName);

    //------------------------------------------------------------------------------
    Cell<moris_index>
    register_vertex_set_names(moris::Cell<std::string> const & aVertexSetNames);
    //------------------------------------------------------------------------------
    Cell<moris_index>
    register_block_set_names_with_cell_topo(moris::Cell<std::string> const & aBlockSetNames,
                                            enum CellTopology                aBlockTopology);
    //------------------------------------------------------------------------------
    void
    set_block_set_colors(moris_index const &    aBlockSetIndex,
                         Matrix<IndexMat> const & aBlockSetColors);
    //------------------------------------------------------------------------------
    Cell<moris_index>
    register_side_set_names(moris::Cell<std::string> const & aSideSetNames);
    //------------------------------------------------------------------------------
    void
    set_side_set_colors(moris_index const &    aSideSetIndex,
                         Matrix<IndexMat> const & aSideSetColors);
    //------------------------------------------------------------------------------
    Cell<moris_index>
    register_double_side_set_names(moris::Cell<std::string> const & aDblSideSetNames);
    //------------------------------------------------------------------------------
    void
    set_double_side_set_colors(moris_index const &      aDblSideSetIndex,
                               Matrix<IndexMat> const & aMasterSideColors,
                               Matrix<IndexMat> const & aSlaveSideColors);
    //------------------------------------------------------------------------------
    void
    setup_interface_side_sets();
    //------------------------------------------------------------------------------
    void
    declare_interface_side_sets();
    //------------------------------------------------------------------------------
    void
    create_interface_side_sets_and_clusters();

    //------------------------------------------------------------------------------
    void
    setup_interface_vertex_sets();

    //------------------------------------------------------------------------------
    Cell<moris_index>
    declare_interface_vertex_sets();
    //------------------------------------------------------------------------------
    void
    create_interface_vertex_sets(Cell<moris_index> const & aInterfaceVertexSetOrds);
    //------------------------------------------------------------------------------
    void
    set_vertex_set_color(moris_index     const & aVertexSetIndex,
                        Matrix<IndexMat> const & aVertexSetColors);
    //------------------------------------------------------------------------------
    void
    construct_color_to_set_relationship(moris::Cell<moris::Matrix<IndexMat>> const & aSetColors,
                                        moris::Cell<moris::Cell<moris_index>> & aColorToSetIndex);

    //------------------------------------------------------------------------------
    void
    create_interface_side_sets_from_interface_double_side_set(moris_index const & aBulkphase0,
                                                              moris_index const & aBulkphase1);

    //------------------------------------------------------------------------------
    // Internal Additional Field Functions
    //------------------------------------------------------------------------------
    /*
     * Returns an index in the data structure for a given entity rank (i.e. NODE = 0)
     */
    moris_index
    get_entity_rank_field_index(enum moris::EntityRank   aEntityRank);
    //------------------------------------------------------------------------------
    /*
     * Returns whether a field exists or not
     */
    bool
    field_exists(std::string              aLabel,
                 enum moris::EntityRank   aEntityRank);
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    // Parallel functions
    //------------------------------------------------------------------------------
};
}




#endif /* PROJECTS_XTK_SRC_XTK_CL_XTK_ENRICHED_INTEGRATION_MESH_HPP_ */
