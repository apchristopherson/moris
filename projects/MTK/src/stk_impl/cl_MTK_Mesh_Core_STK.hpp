/*
 * cl_MTK_Mesh_Core_STK.hpp
 *
 *  Created on: Apr 15, 2019
 *      Author: doble
 */

#ifndef PROJECTS_MTK_SRC_STK_IMPL_CL_MTK_MESH_CORE_STK_HPP_
#define PROJECTS_MTK_SRC_STK_IMPL_CL_MTK_MESH_CORE_STK_HPP_


// Third-party header files.
#include <stk_io/StkMeshIoBroker.hpp>     // for StkMeshIoBroker
#include <stk_mesh/base/GetEntities.hpp>  // for count_entities
#include <stk_mesh/base/MetaData.hpp>     // for MetaData
#include <stk_mesh/base/BulkData.hpp>     // for BulkData
#include <stk_mesh/base/Selector.hpp>     // for Selector
#include <stk_mesh/base/FEMHelpers.hpp>   // for Selector
#include "stk_io/DatabasePurpose.hpp"     // for DatabasePurpose::READ_MESH
#include "stk_mesh/base/CoordinateSystems.hpp" // for Cartesian
#include "stk_mesh/base/CreateFaces.hpp"  // for handling faces
#include "stk_mesh/base/CreateEdges.hpp"  // for handling faces
#include "stk_mesh/base/Bucket.hpp"       // for buckets
#include "stk_mesh/base/Field.hpp"    // for coordinates
#include "stk_mesh/base/GetEntities.hpp"    // for coordinates
#include "stk_mesh/base/FieldParallel.hpp"  // for handling parallel fields


#include "cl_MTK_Sets_Info.hpp"
#include "cl_MTK_Mesh_Data_Input.hpp"
#include "cl_MTK_Block.hpp"
#include "cl_Mesh_Enums.hpp"
#include "cl_MTK_Mesh.hpp"
#include "cl_MTK_Scalar_Field_Info.hpp"
#include "cl_MTK_Matrix_Field_Info.hpp"
#include "cl_MTK_Exodus_IO_Helper.hpp"

#include "cl_Cell.hpp"

// For moris::Cell and vertex APi
#include "cl_MTK_Cell_STK.hpp"
#include "cl_MTK_Vertex_STK.hpp"

#include "cl_MTK_Mesh_Core.hpp"
#include "cl_MTK_Mesh_Data_STK.hpp"
namespace moris
{
namespace mtk
{

class Mesh_Core_STK : public virtual Mesh
{
    //! timestamp for stk output. Set in constructor over MtkMeshData
    double mTimeStamp = 0.0;

    //! timestamp for stk output. Set in constructor over MtkMeshData
    bool   mAutoAuraOption = true;

    std::shared_ptr<Mesh_Data_STK> mSTKMeshData;

    // give access to the member data to stk based interpolation and integration meshes
    friend class Interpolation_Mesh_STK;
    friend class Integration_Mesh_STK;

public:
    //##############################################
    // Build mesh functionality
    //##############################################
    /**
     * STK destructor.
     */
    ~Mesh_Core_STK();

    Mesh_Core_STK(){};

    /**
     * STK constructor (mesh generated internally or obtained from an Exodus file )
     *
     * @param[in] aFileName  .................    String with mesh file name.
     * @param[in] aSuppMeshData .............. Supplementary Mesh input data (currently used to declare extra fields only)
     */
    Mesh_Core_STK(
            std::string    aFileName,
            MtkMeshData*   aSuppMeshData,
            const bool     aCreateFacesAndEdges = true );

    /*!
     * pass in an already constructed mesh data base
     */
    Mesh_Core_STK(std::shared_ptr<Mesh_Data_STK> aSTKMeshData);

    /**
     * Create stk mesh with information given by the user.
     *
     * @param[in] aFileName   .................   String with mesh file name.
     *
     */
    void
    build_mesh(
            std::string    aFileName,
            MtkMeshData*   aSuppMeshData,
            const bool     aCreateFacesAndEdges = true  );

    /**
     * Create a MORIS mesh with information given by the user.
     *
     * @param[in] aMeshData   .................   struct with the following mesh information
     * @param[in] SpatialDim   ...............   problem dimensions (1D = 1, 2D = 2 , or 3D = 3).
     * @param[in] ElemConn     ...............   table containing element connectivity.
     * @param[in] NodeCoords         .........   node coordinates
     * @param[in] LocaltoGlobalNodeMap   .....   node local ind to global id map
     * @param[in] LocaltoGlobalElemMap   .....   element local ind to global id map
     * @param[in] EntProcOwner................   number of processors required.
     * @param[in] PartNames   ................   information of parts where elements will be stored.
     * @param[in] FieldsData  ................   vectors with field data for all fields
     * @param[in] FieldsRank  ................   entity types on which the fields are acting
     * @param[in] FieldsName  ................   names for all fields
     *
     */
    Mesh_Core_STK(MtkMeshData & aMeshData );


    /*!
     * Get pointer of stk mesh data
     */
    std::shared_ptr<Mesh_Data_STK>
    get_mesh_data()
    {
        return mSTKMeshData;
    }

    MeshType
    get_mesh_type() const
    {
        return MeshType::STK;
    }

    //##############################################
    // General mesh information access
    //##############################################

    /*
     * Get spatial dimension of the mesh
     */
    uint
    get_spatial_dim() const
    {
        return (uint)mSTKMeshData->mMtkMeshMetaData->spatial_dimension();
    }

    /*
     * Get number of entities for specified rank in the STK universal part
     */
    uint
    get_num_entities(
            enum EntityRank aEntityRank) const;


    //##############################################
    // Access Mesh Data by index Functions
    //##############################################
    /*
     * Generic get local index of entities connected to
     * entity using an entities local index
     */
    Matrix<IndexMat>
    get_entity_connected_to_entity_loc_inds(moris_index     aEntityIndex,
                                            enum EntityRank aInputEntityRank,
                                            enum EntityRank aOutputEntityRank) const;

    /*
     * Since the connectivity between entities of the same rank are considered
     * invalid by STK standards, we need a seperate function for element to element
     * specifically
     *
     * @param[in]  aElementId - element id
     * @param[out] Element to element connectivity and face ordinal shared
     *                   (where elements are all by index)
     */
    Matrix< IndexMat >
    get_elements_connected_to_element_and_face_ord_loc_inds(moris_index aElementIndex) const;


    /*
     * Since the connectivity between entities of the same rank are considered
     * invalid by STK standards, we need a seperate function for element to element
     * specifically
     *
     * @param[in]  aElementId - element id
     * @param[out] Element to element connectivity and face index shared
     *                   (where elements are all by index)
     */
    Matrix< IndexMat >
    get_elements_connected_to_element_and_face_ind_loc_inds(moris_index aElementIndex) const;

    Matrix< IndexMat >
    get_elements_in_support_of_basis(moris_index aBasisIndex)
    {
        return get_entity_connected_to_entity_loc_inds(aBasisIndex, EntityRank::NODE, EntityRank::ELEMENT);
    }


    //##############################################
    // global id functions
    //##############################################

    /*
     * Get global identifier of an entity from a local index and entity rank
     */
    moris_id
    get_glb_entity_id_from_entity_loc_index(moris_index     aEntityIndex,
                                            enum EntityRank aEntityRank) const;

    /*
     * Get local indec of an entity from a global index
     */
    moris_index
    get_loc_entity_ind_from_entity_glb_id(moris_id        aEntityId,
                                          enum EntityRank aEntityRank) const;

    /*
     * Generic get global id of entities connected to
     * entity using an entities global id
     */
    Matrix< IdMat >
    get_entity_connected_to_entity_glob_ids( moris_id     aEntityId,
                                             enum EntityRank aInputEntityRank,
                                             enum EntityRank aOutputEntityRank) const;

    /*
     * Since the connectivity between entities of the same rank are considered
     * invalid by STK standards, we need a seperate function for element to element
     * specifically
     *
     * @param[in]  aElementId - element id
     * @param[out] Element to element connectivity and face ordinal shared
     */
    Matrix< IdMat >
    get_element_connected_to_element_glob_ids(moris_index aElementId) const;

    /*
     * Get the ordinal of a facet relative to a cell, using global identifiers
     * @param[in] aFaceId - Global face id
     * @param[in] aCellId - Global cell id
     * @param[out] Side Ordinal
     */
    moris_index
    get_facet_ordinal_from_cell_and_facet_id_glob_ids(moris_id aFaceId,
                                                      moris_id aCellId) const;

    moris::moris_index
    get_facet_ordinal_from_cell_and_facet_loc_inds(moris::moris_index aFaceIndex,
                                                   moris::moris_index aCellIndex) const;

    /*
     * Returns a list of globally unique entity ids for entities
     * of the provided rank
     * @param[in]  aNumNodes - number of node ids requested
     * @param[in]  aEntityRank - Entity rank to assign ids for
     * @param[out] aAvailableNodeIDs - list of globally unique node IDs
     */
    Matrix< IdMat >
    generate_unique_entity_ids( uint            aNumEntities,
                                enum EntityRank aEntityRank) const;

    //##############################################
    // Coordinate Field Functions
    //##############################################
    Matrix< DDRMat >
    get_node_coordinate( moris_index aNodeIndex ) const;


    //##############################################
    // Entity Ownership Functions
    //##############################################
    moris_id
    get_entity_owner(  moris_index     aEntityIndex,
                       enum EntityRank aEntityRank ) const;


    //TODO: function get_processors_whom_share_entity

    Matrix< IdMat >
    get_processors_whom_share_entity_glob_ids(moris_id aEntityId,
                                              enum EntityRank aEntityRank) const;

    //TODO: function get_num_of_entities_shared_with_processor

    //##############################################
    // Mesh Sets Access
    //##############################################
    moris::Cell<std::string>
    get_set_names(enum EntityRank aSetEntityRank) const;

    Matrix< IndexMat >
    get_set_entity_loc_inds( enum EntityRank aSetEntityRank,
                             std::string     aSetName) const;

    void
    get_sideset_elems_loc_inds_and_ords(
            const  std::string & aSetName,
            Matrix< IndexMat > & aElemIndices,
            Matrix< IndexMat > & aSidesetOrdinals ) const;

    void
    get_sideset_cells_and_ords(
            const  std::string & aSetName,
            moris::Cell< mtk::Cell const* > & aCells,
            Matrix< IndexMat > &       aSidesetOrdinals ) const;



    //##############################################
    // Field Access
    //##############################################
    uint
    get_num_fields(  const enum EntityRank aEntityRank ) const;

    //------------------------------------------------------------------------------

    /**
     * return the index of the field of this label
     * return gNoIndex if not found
     */
    moris_index
    get_field_ind(
            const std::string & aFieldLabel,
            const enum EntityRank aEntityRank ) const;



    /*
     * Access an entity
     *
     */
    Matrix< DDRMat >
    get_entity_field_value_real_scalar(Matrix< IndexMat > const & aEntityIndices,
                                       std::string        const & aFieldName,
                                       enum EntityRank            aFieldEntityRank) const;


    /*
     * Given a field name and rank associated with field, add the field data
     * For now, this is just for real type single component fields
     */
    void
    add_mesh_field_real_scalar_data_loc_inds(std::string      const & aFieldName,
                                             enum EntityRank  const & aFieldEntityRank,
                                             Matrix< DDRMat > const & aFieldData);



    //##############################################
    // moris::Cell and Vertex Pointer Functions
    //##############################################

    /*
     * Get an mtk cell by index
     */
    mtk::Cell &
    get_mtk_cell(moris_index aCellIndex);

    mtk::Cell const &
    get_mtk_cell(moris_index aCellIndex) const ;

    /*
     * get an mtk vertex by index
     */
    mtk::Vertex &
    get_mtk_vertex(moris_index aVertexIndex);

    mtk::Vertex const &
    get_mtk_vertex(moris_index aVertexIndex) const;

    /*
     * Returns the vertex as a child class
     */
    mtk::Vertex_Core_STK &
    get_mtk_vertex_stk(moris_index aVertexIndex);

    //------------------------------------------------------------------------------

    //fixme: this function needs to go
    /**
     * populates the member variables of the relevant nodes
     * with their T-Matrices
     */
    void
    finalize()
    {
        MORIS_ERROR(0,"Not implemented in STK");
    }

    //------------------------------------------------------------------------------

    /**
     * provides a moris::Mat<uint> containing the IDs this mesh has
     * to communicate with
     */
    Matrix< IdMat >
    get_communication_table() const
    {
        MORIS_ERROR(0,"Not implemented in STK");
        return mSTKMeshData->mEntityLocaltoGlobalMap(0);
    }

    //##############################################
    //  Output Mesh To a File
    //##############################################
    /*
     * Create an exodus mesh database with the specified
     * filename.
     *
     * @param[in] filename The full pathname to the file which will be
     *   created and the mesh data written to. If the file already
     *   exists, it will be overwritten.
     *
     *   Description from create_output_mesh() in StkMeshIoBroker.hpp
     */
    void
    create_output_mesh(
            std::string  &aFileName,
            bool          aAddElemCmap = false);

    /*
     * Given an exodus file, add the element communication maps
     * @param[in] Exodus_IO_Helper & aExoIO -  an mtk exodus io helper
     */
    void
    add_element_cmap_to_exodus( std::string  & aFileName,
                                Exodus_IO_Helper & aExoIO);


public:


    // Dummy Block
    //    Block* mDummyBlock;

    //##############################################
    // Private functions to build mesh
    //##############################################
    /*
     *
     * Create communication list
     */
    void
    create_communication_lists_and_local_to_global_map(enum EntityRank aEntityRank);
    //------------------------------------------------------------------------------

    /*
     * sets up the vertex and cell api
     */
    void
    setup_vertices_and_cell();
    //------------------------------------------------------------------------------

    //##############################################
    // Private functions to access mesh information
    //##############################################

    /*
     * Return the STK rank of an entity given the moris
     * entity rank
     */
    stk::mesh::EntityRank
    get_stk_entity_rank(enum EntityRank aMRSEntityRank) const;
    //------------------------------------------------------------------------------

    /*
     * Returns the local entity index of entities owned and shared by current proc
     *  @return
     */
    Matrix< IdMat >
    get_entities_owned_and_shared_by_current_proc(
            EntityRank   aEntityRank ) const;
    //------------------------------------------------------------------------------

    /*
     * Return the number of a entities
     */
    Matrix< IdMat >
    get_entities_universal_glob_id(
            EntityRank   aEntityRank ) const
            {
        return this->get_entities_in_selector_interface_glob_id( aEntityRank, mSTKMeshData->mMtkMeshMetaData->universal_part() );
            }
    //------------------------------------------------------------------------------

    /*
     * Returns the local entity index of entities owned and shared by current proc
     *  @return
     */
    Matrix< IdMat >
    get_entities_in_selector_interface_glob_id(
            EntityRank            aEntityRank,
            stk::mesh::Selector   aSelectedEntities ) const;
    //------------------------------------------------------------------------------

    /*
     * Returns
     * @param[in]  aEntityRank       - entity rank
     * @param[in]  aEntityRank       - entity ID
     * @param[out] list of entities shared
     */
    Matrix< IdMat >
    get_procs_sharing_entity_by_id(
            moris_id          aEntityID,
            enum EntityRank   aEntityRank ) const;


    //------------------------------------------------------------------------------
    moris::Cell < moris::Cell < uint > >
    get_shared_info_by_entity( uint aNumActiveSharedProcs, enum EntityRank  aEntityRank );

    //------------------------------------------------------------------------------


    //------------------------------------------------------------------------------

    void
    get_processors_whom_share_entity(moris_id          aEntityIndex,
                                     enum EntityRank   aEntityRank,
                                     Matrix< IdMat > & aProcsWhomShareEntity) const;

    //------------------------------------------------------------------------------
    uint
    get_num_of_entities_shared_with_processor(moris_id        aProcessorRank,
                                              enum EntityRank aEntityRank,
                                              bool aSendFlag) const;

    /*
     * Returns the local entity index of entities globally shared by current process
     *
     */
    Matrix< IdMat >
    get_entities_glb_shared_current_proc(
            EntityRank   aEntityRank ) const
            {
        return this->get_entities_in_selector_interface_glob_id( aEntityRank, mSTKMeshData->mMtkMeshMetaData->globally_shared_part() );
            }

    //##############################################
    // internal id functions
    //##############################################
    std::vector<stk::mesh::Entity>
    entities_connected_to_entity_stk(stk::mesh::Entity*    const aInputEntity,
                                     const stk::mesh::EntityRank aInputEntityRank,
                                     const stk::mesh::EntityRank aOutputEntityRank) const;

    bool
    is_aura_cell(moris_id aElementId) const;


    //##############################################
    // Build mesh from data functions internal
    //##############################################
    /**
     * Verifies that the information given by the user makes sense.
     *
     * @param[in] aMeshData   .................   struct with the following mesh information
     * @param[in] SpatialDim   ...............   problem dimensions (1D = 1, 2D = 2 , or 3D = 3).
     * @param[in] ElemConn     ...............   table containing element connectivity.
     * @param[in] NodeCoords         .........   node coordinates
     * @param[in] LocaltoGlobalNodeMap   .....   node local ind to global id map
     * @param[in] LocaltoGlobalElemMap   .....   element local ind to global id map
     * @param[in] EntProcOwner................   number of processors required.
     * @param[in] PartNames   ................   information of parts where elements will be stored.
     * @param[in] FieldsData  ................   vectors with field data for all fields
     * @param[in] FieldsRank  ................   entity types on which the fields are acting
     * @param[in] FieldsName  ................   names for all fields
     *
     */
    void
    check_and_update_input_data(
            MtkMeshData &   aMeshData );

    /*
     * @param[in]  aMeshData
     */
    void
    check_and_update_fields_data( MtkMeshData &   aMeshData );

    /*
     * @param[in]  aMeshData
     */
    void
    check_and_update_sets_data(
            MtkMeshData &   aMeshData );


    //##############################################
    // internal build functions
    //##############################################
    /**
     * Create a MORIS mesh with information given by the user.
     *
     * @param[in] aMeshData   .................   struct with the following mesh information
     * @param[in] SpatialDim   ...............   problem dimensions (1D = 1, 2D = 2 , or 3D = 3).
     * @param[in] ElemConn     ...............   table containing element connectivity.
     * @param[in] NodeCoords         .........   node coordinates
     * @param[in] LocaltoGlobalNodeMap   .....   node local ind to global id map
     * @param[in] LocaltoGlobalElemMap   .....   element local ind to global id map
     * @param[in] EntProcOwner................   number of processors required.
     * @param[in] PartNames   ................   information of parts where elements will be stored.
     * @param[in] FieldsData  ................   vectors with field data for all fields
     * @param[in] FieldsRank  ................   entity types on which the fields are acting
     * @param[in] FieldsName  ................   names for all fields
     *
     */
    void
    build_mesh(
            MtkMeshData & aMeshData );

    //------------------------------------------------------------------------------

    /*
     * Returns
     * @param[in]  aNumModelDims
     * @param[in]  aNumNodesPerElem
     * @param[in]  aPartNames
     */
    void
    declare_mesh_parts(
            MtkMeshData &  aMeshData );

    //------------------------------------------------------------------------------

    /*
     * Returns
     * @param[in]  aMeshData
     */
    void
    declare_mesh_fields(
            MtkMeshData & aMeshData );
    //------------------------------------------------------------------------------
    /*
     * Add list of supplementary fields to add to output step
     */
    void
    add_supplementary_fields_to_declare_at_output(MtkMeshData &  aMeshData);

    //------------------------------------------------------------------------------

    /*
     * Returns
     * @param[in]  aMeshData
     * @param[in]  aFieldsInfo
     */
    void
    internal_declare_mesh_field(
            MtkMeshData &  aMeshData,
            uint          iField );

    //------------------------------------------------------------------------------

    /*
     * Returns
     * @param[in]  aMeshData
     */
    void
    populate_mesh_database(
            MtkMeshData &  aMeshData );

    //------------------------------------------------------------------------------

    void
    setup_cell_global_to_local_map(
            MtkMeshData &   aMeshData );

    //------------------------------------------------------------------------------
    void
    setup_vertex_global_to_local_map(
            MtkMeshData &   aMeshData );

    //------------------------------------------------------------------------------
    void
    setup_entity_global_to_local_map(enum EntityRank aEntityRank);


    //------------------------------------------------------------------------------

    /*
     * Returns
     * @param[in]  aMeshData
     */
    void
    process_block_sets(
            MtkMeshData &   aMeshData );
    //------------------------------------------------------------------------------

    Matrix< IndexMat >
    process_cell_block_membership( MtkMeshData &  aMeshData);
    //------------------------------------------------------------------------------
    stk::mesh::PartVector
    get_block_set_part_vector(MtkMeshData &  aMeshData);
    //------------------------------------------------------------------------------

    void
    process_node_sharing_information(MtkMeshData& aMeshData);

    /*
     * Returns
     * @param[in]  aMeshData
     */
    void
    populate_mesh_fields(
            MtkMeshData &   aMeshData );
    //------------------------------------------------------------------------------
    /*
     * Returns
     * @param[in]  aModelDim
     * @param[in]  aNumNodesInElem
     */
    stk::topology::topology_t
    get_mesh_topology(
            uint   aModelDim,
            uint   aNumNodesInElem );


    //    stk::topology::topology_t
    //    get_stk_entity_rank(enum EntityRank aMTKEntityRank);

    stk::topology::topology_t
    get_stk_topo( enum CellTopology aMTKCellTopo );
    //------------------------------------------------------------------------------
    void
    create_additional_communication_lists_from_data();
    //------------------------------------------------------------------------------
    void
    create_facets_communication_lists();
    //------------------------------------------------------------------------------
    void
    create_owners_communication_lists();
    //------------------------------------------------------------------------------
    void
    create_shared_communication_lists();
    //------------------------------------------------------------------------------
    /*
     * Returns
     * @param[in]  aMeshData
     */
    void
    process_node_sets(
            MtkMeshData   aMeshData );
    //------------------------------------------------------------------------------
    /*
     * Returns
     * @param[in]  aMeshData
     */
    void
    process_side_sets(
            MtkMeshData   aMeshData );
    //------------------------------------------------------------------------------
    /*
     * Returns
     * @param[in]  aStkElemConn
     * @param[in]  aElemParts
     * @param[in]  aMeshData
     */
    void
    populate_mesh_database_serial(
            moris::uint  aElementTypeInd,
            MtkMeshData                            aMeshData,
            std::vector< stk::mesh::PartVector >   aElemParts,
            Matrix< IdMat >                       aOwnerPartInds);
    //------------------------------------------------------------------------------
    /*
     * Returns
     * @param[in]  aStkElemConn
     * @param[in]  aElemParts
     * @param[in]  aMeshData
     */
    void
    populate_mesh_database_parallel(
            MtkMeshData                            aMeshData,
            std::vector< stk::mesh::PartVector >   aElemParts,
            Matrix< IdMat >                       aOwnerPartInds);
    //------------------------------------------------------------------------------
    /*
     * Returns
     * @param[in]  aEntityRank
     * @param[in]  aFieldName
     * @param[out] field values
     */
    Matrix< IdMat >
    get_set_entity_glob_ids(
            stk::mesh::EntityRank   aEntityRank,
            std::string             aFieldName ) const;

    //------------------------------------------------------------------------------


    /*
     * Returns
     * @param[in]  aMeshData
     * @param[in]  aFieldsInfo
     */
    void
    internal_declare_mesh_real_matrix_fields(
            std::string         aFieldName,
            enum EntityRank     aFieldRank,
            stk::mesh::Selector aFieldPart,
            uint                aNumCols,
            uint                aNumRows);


    template<typename Field_Matrix_Type>
    void
    populate_field_data_scalar_field(Scalar_Field_Info<Field_Matrix_Type>* aScalarField)
    {
        typedef typename  Scalar_Field_Info<Field_Matrix_Type>::Field_Data_Type FDT;
        enum EntityRank                            tFieldEntityRank = aScalarField->get_field_entity_rank();
        std::string                                tFieldName       = aScalarField->get_field_name();
        moris::Matrix< Field_Matrix_Type > const & tFieldData       = aScalarField->get_field_data();
        moris::Matrix< IdMat  > const &            tFieldEntityIds  = aScalarField->get_field_entity_ids();
        stk::mesh::EntityRank                      tStkFieldRank    = this->get_stk_entity_rank( tFieldEntityRank );
        stk::mesh::FieldBase *                     tFieldBase       = mSTKMeshData->mMtkMeshMetaData->get_field( tStkFieldRank, tFieldName );

        // Loop over field entities
        for ( uint iEntityInd = 0; iEntityInd < tFieldEntityIds.numel(); ++iEntityInd )
        {
            // Get global Id of current entity based on rank (use map provided by the user).
            // This is done only if the field is not contained in any set.
            stk::mesh::Entity aEntity = mSTKMeshData->mMtkMeshBulkData->get_entity( tStkFieldRank, tFieldEntityIds( iEntityInd ) );

            // Store the coordinates of the current entity
            if ( mSTKMeshData->mMtkMeshBulkData->is_valid( aEntity ) )
            {
                // Get the pointer to the field data
                FDT* tFieldEntityData = static_cast <FDT*> ( stk::mesh::field_data ( *tFieldBase, aEntity ) );

                // Set the field data
                tFieldEntityData[0] = tFieldData(iEntityInd);
            }
        }
    }


    template<typename Field_Matrix_Type>
    void
    populate_field_data_matrix_field(Matrix_Field_Info<Field_Matrix_Type>* aMatrixField)
    {
        typedef typename  Matrix_Field_Info<Field_Matrix_Type>::Field_Data_Type FDT;

        enum EntityRank                                          tFieldEntityRank  = aMatrixField->get_field_entity_rank();
        std::string                                              tFieldName        = aMatrixField->get_field_name();
        moris::Cell<moris::Matrix< Field_Matrix_Type >*> const & tFieldData        = aMatrixField->get_field_data();
        moris::Matrix< IdMat  > const &                          tFieldEntityIds   = aMatrixField->get_field_entity_ids();
        uint                                                     tNumRows          = aMatrixField->get_num_rows();
        uint                                                     tNumCols          = aMatrixField->get_num_cols();
        stk::mesh::EntityRank                                    tStkFieldRank     = this->get_stk_entity_rank( tFieldEntityRank );

        stk::mesh::FieldBase * aFieldBase = mSTKMeshData->mMtkMeshMetaData->get_field( tStkFieldRank, tFieldName );

        // Loop over field entities
        for ( uint iEntityInd = 0; iEntityInd < tFieldEntityIds.numel(); ++iEntityInd )
        {
            // Get global Id of current entity based on rank (use map provided by the user).
            // This is done only if the field is not contained in any set.
            stk::mesh::Entity aEntity = mSTKMeshData->mMtkMeshBulkData->get_entity( tStkFieldRank, tFieldEntityIds( iEntityInd ) );

            // Store the coordinates of the current entity
            if ( mSTKMeshData->mMtkMeshBulkData->is_valid( aEntity ) )
            {
                // Get the pointer to the field data
                FDT* tFieldEntityData = static_cast <FDT*> ( stk::mesh::field_data ( *aFieldBase, aEntity ) );

                uint tCount = 0 ;
                for ( uint j = 0; j < tNumCols; ++j )
                {
                    for ( uint i = 0; i < tNumRows; ++i )
                    {
                        tFieldEntityData[tCount] = (*tFieldData(iEntityInd))(i,j);
                        tCount++;
                    }
                }
            }
        }
    }


    /**
     * returns the option of auto aura based on internally set flag
     */
    stk::mesh::BulkData::AutomaticAuraOption
    get_aura_option() const
    {
        if( mAutoAuraOption )
        {
            return stk::mesh::BulkData::AutomaticAuraOption::AUTO_AURA;
        }
        else
        {
            return stk::mesh::BulkData::AutomaticAuraOption::NO_AUTO_AURA;
        }
    }

    //------------------------------------------------------------------------------

    enum EntityRank
    get_facet_rank() const
    {
        if(mSTKMeshData->mNumDims == 1)
        {
            return EntityRank::NODE;
        }
        else if(mSTKMeshData->mNumDims == 2)
        {
            return EntityRank::EDGE;
        }
        else if(mSTKMeshData->mNumDims == 3)
        {
            return EntityRank::FACE;
        }
        else
        {
            MORIS_ASSERT(0,"Invalid Mesh dimension detected in get_facet_rank ");
            return EntityRank::INVALID;
        }

    }

    typedef stk::mesh::Field<real>    Field1CompReal;
    typedef stk::mesh::Field<real,stk::mesh::Cartesian2d>  Field2CompReal;
    typedef stk::mesh::Field<real,stk::mesh::Cartesian3d>  Field3CompReal;
    typedef stk::mesh::Field<real,stk::mesh::FullTensor22> Field4CompReal;
    typedef stk::mesh::Field<real,stk::mesh::FullTensor>   Field9CompReal;
    typedef stk::mesh::Field<sint>    Field1CompInt;
    typedef stk::mesh::Field<sint,stk::mesh::Cartesian2d>  Field2CompInt;
    typedef stk::mesh::Field<sint,stk::mesh::Cartesian3d>  Field3CompInt;
    typedef stk::mesh::Field<sint,stk::mesh::FullTensor22> Field4CompInt;
    typedef stk::mesh::Field<sint,stk::mesh::FullTensor>   Field9CompInt;
};
}
}


#endif /* PROJECTS_MTK_SRC_STK_IMPL_CL_MTK_MESH_CORE_STK_HPP_ */