#include "cl_MTK_Mesh_Core.hpp"

namespace moris
{
    namespace mtk
    {
        
        //--------------------------------------------------------------------------------------------------------------
        
        Mesh::Mesh()
        {
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Mesh::~Mesh()
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_num_sets() const
        {
            MORIS_ASSERT( false ,"get_num_sets(), not implemented for base class");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Set* Mesh::get_set_by_index( uint aSetIndex ) const
        {
            MORIS_ASSERT( false ,"get_set_by_index(), not implemented for base class");
            return nullptr;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Set* Mesh::get_set_by_name( std::string aSetLabel ) const
        {
            MORIS_ASSERT( false ,"get_set_by_name(), not implemented for base class");
            return nullptr;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        uint Mesh::get_num_nodes() const
        {
            return get_num_entities(EntityRank::NODE);
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_num_edges() const
        {
            return get_num_entities(EntityRank::EDGE);
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_num_faces() const
        {
            return get_num_entities(EntityRank::FACE);
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_num_elems() const
        {
            return get_num_entities(EntityRank::ELEMENT);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< IndexMat >
        Mesh::get_elements_connected_to_element_and_face_ord_loc_inds(moris_index aElementIndex) const
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return Matrix<IndexMat>(0,0);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris::Cell<Vertex const *>
        Mesh::get_all_vertices() const
        {
            MORIS_ERROR(0,"No default implementation of get_all_vertices_no_aura");

            return moris::Cell<Vertex const *>(0,nullptr);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix < IndexMat >
        Mesh::get_elements_connected_to_node_loc_inds( moris_index aNodeIndex ) const
        {
            return get_entity_connected_to_entity_loc_inds(aNodeIndex,EntityRank::NODE, EntityRank::ELEMENT);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IndexMat >
        Mesh::get_faces_connected_to_node_loc_inds( moris_index aNodeIndex ) const
        {
            return get_entity_connected_to_entity_loc_inds(aNodeIndex,EntityRank::NODE, EntityRank::FACE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IndexMat >
        Mesh::get_edges_connected_to_node_loc_inds( moris_index aNodeIndex ) const
        {
            return get_entity_connected_to_entity_loc_inds(aNodeIndex,EntityRank::NODE, EntityRank::EDGE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IndexMat >
        Mesh::get_elements_connected_to_edge_loc_inds( moris_index aEdgeIndex ) const
        {
            return get_entity_connected_to_entity_loc_inds(aEdgeIndex,EntityRank::EDGE, EntityRank::ELEMENT);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IndexMat >
        Mesh::get_faces_connected_to_edge_loc_inds( moris_index aEdgeIndex ) const
        {
            return get_entity_connected_to_entity_loc_inds(aEdgeIndex,EntityRank::EDGE, EntityRank::FACE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IndexMat >
        Mesh::get_elements_connected_to_face_loc_inds( moris_index aFaceIndex ) const
        {
            return get_entity_connected_to_entity_loc_inds(aFaceIndex,EntityRank::FACE, EntityRank::ELEMENT);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IndexMat >
        Mesh::get_faces_connected_to_element_loc_inds(moris_index aElementIndex) const
        {
            return get_entity_connected_to_entity_loc_inds(aElementIndex,EntityRank::ELEMENT, EntityRank::FACE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IndexMat >
        Mesh::get_edges_connected_to_element_loc_inds(moris_index aElementIndex) const
        {
            return get_entity_connected_to_entity_loc_inds(aElementIndex,EntityRank::ELEMENT, EntityRank::EDGE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IndexMat >
        Mesh::get_nodes_connected_to_element_loc_inds(moris_index aElementIndex) const
        {
            return get_entity_connected_to_entity_loc_inds(aElementIndex,EntityRank::ELEMENT, EntityRank::NODE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris_index
        Mesh::get_loc_entity_ind_from_entity_glb_id(
                moris_id           aEntityId,
                enum EntityRank    aEntityRank,
                const moris_index  aBSPlineMeshIndex) const
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris_index
        Mesh::get_facet_ordinal_from_cell_and_facet_loc_inds(moris_index aFaceIndex,
                                                       moris_index aCellIndex) const
        {
            Matrix<IdMat> tElementFaces = get_entity_connected_to_entity_loc_inds(aCellIndex,EntityRank::ELEMENT, this->get_facet_rank());

            moris_index tOrdinal = MORIS_INDEX_MAX;
            for(moris_index iOrd = 0; iOrd<(moris_index)tElementFaces.numel(); iOrd++)
            {
                if(tElementFaces(iOrd) == aFaceIndex)
                {
                    tOrdinal = iOrd;
                    return tOrdinal;
                }
            }
            MORIS_ERROR(tOrdinal!=MORIS_INDEX_MAX," Facet ordinal not found");
            return tOrdinal;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< IdMat >
        Mesh::generate_unique_entity_ids(
                uint            aNumEntities,
                enum EntityRank aEntityRank) const
        {
            return Matrix<IdMat>(1,1,this->get_num_entities(aEntityRank)+1);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IdMat >
        Mesh::generate_unique_node_ids(uint aNumNodes)
        {
            return generate_unique_entity_ids(aNumNodes,EntityRank::NODE);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<IdMat>
        Mesh::get_entity_connected_to_entity_glob_ids(
                moris_id          aEntityId,
                enum EntityRank   aInputEntityRank,
                enum EntityRank   aOutputEntityRank,
                const moris_index aBSplineMeshIndex) const
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return Matrix<IdMat>(0,0);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< IdMat >
        Mesh::get_element_connected_to_element_glob_ids(moris_id aElementId) const
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return Matrix<IdMat>(0,0);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IdMat >
        Mesh::get_elements_connected_to_node_glob_ids( moris_id aNodeId )
        {
            return get_entity_connected_to_entity_glob_ids(aNodeId,EntityRank::NODE, EntityRank::ELEMENT);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IdMat >
        Mesh::get_faces_connected_to_node_glob_ids( moris_id aNodeId )
        {
            return get_entity_connected_to_entity_glob_ids(aNodeId,EntityRank::NODE, EntityRank::FACE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IdMat >
        Mesh::get_edges_connected_to_node_glob_ids( moris_id aNodeId )
        {
            return get_entity_connected_to_entity_glob_ids(aNodeId,EntityRank::NODE, EntityRank::EDGE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IdMat >
        Mesh::get_elements_connected_to_edge_glob_ids( moris_id aEdgeId )
        {
            return get_entity_connected_to_entity_glob_ids(aEdgeId,EntityRank::EDGE, EntityRank::ELEMENT);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix < IdMat >
        Mesh::get_faces_connected_to_edge_glob_ids( moris_id aEdgeId )
        {
            return get_entity_connected_to_entity_glob_ids(aEdgeId,EntityRank::EDGE, EntityRank::FACE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IdMat >
        Mesh::get_elements_connected_to_face_glob_ids( moris_id aFaceId )
        {
            return get_entity_connected_to_entity_glob_ids(aFaceId,EntityRank::FACE, EntityRank::ELEMENT);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IdMat >
        Mesh::get_faces_connected_to_element_glob_ids(moris_id aElementId)
        {
            return get_entity_connected_to_entity_glob_ids(aElementId,EntityRank::ELEMENT, EntityRank::FACE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IdMat >
        Mesh::get_edges_connected_to_element_glob_ids(moris_id aElementId)
        {
            return get_entity_connected_to_entity_glob_ids(aElementId,EntityRank::ELEMENT, EntityRank::EDGE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IdMat >
        Mesh::get_nodes_connected_to_element_glob_ids(moris_id aElementId)
        {
            return get_entity_connected_to_entity_glob_ids(aElementId,EntityRank::ELEMENT, EntityRank::NODE);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        void
        Mesh::get_elements_in_support_of_basis(
                const uint           aMeshIndex,
                const uint           aBasisIndex,
                Matrix< IndexMat > & aElementIndices )
        {
            MORIS_ERROR(0,"get_elements_in_support_of_basis not implemented");
        }

        void
        Mesh::get_nodes_indices_in_bounding_box(
                const Matrix< DDRMat >   & aPoint,
                const Matrix< DDRMat >   & aBoundingBoxSize,
                Matrix< IndexMat >       & aNodeIndices )
        {
            MORIS_ERROR(0,"get_nodes_in_bounding_box(), not implemented");
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< DDRMat >
        Mesh::get_entity_field_value_real_scalar(
                const Matrix<IndexMat> & aEntityIndices,
                const std::string      & aFieldName,
                enum EntityRank          aFieldEntityRank) const
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (get_entity_field_value_real_scalar is not implemented)");
            return Matrix< DDRMat >(0,0);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        void
        Mesh::add_mesh_field_real_scalar_data_loc_inds(
                const std::string     & aFieldName,
                const enum EntityRank & aFieldEntityRank,
                const Matrix<DDRMat>  & aFieldData)
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (add_mesh_field_real_scalar_data_loc_inds is not implemented)");

        }

        //--------------------------------------------------------------------------------------------------------------

        Facet*
        Mesh::get_facet(moris_index)
        {
            MORIS_ERROR(0,"get facet not implemented");
            return nullptr;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Cell  &
        Mesh::get_mtk_cell( moris_index aElementIndex)
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return *mDummyCells;
        }

        //--------------------------------------------------------------------------------------------------------------

        Cell const &
        Mesh::get_mtk_cell( moris_index aElementIndex) const
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return *mDummyCells;
        }

        //--------------------------------------------------------------------------------------------------------------

        Vertex &
        Mesh::get_mtk_vertex( moris_index aVertexIndex )
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return *mDummyVertex;
        }

        //--------------------------------------------------------------------------------------------------------------

        Vertex const &
        Mesh::get_mtk_vertex( moris_index aVertexIndex ) const
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return *mDummyVertex;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_base_node_index(uint aNodeIndex)
        {
            return this->get_mtk_vertex(aNodeIndex).get_base_vertex()->get_index();
        }

        //--------------------------------------------------------------------------------------------------------------

        Cell  &
        Mesh::get_writable_mtk_cell( moris_index aElementIndex )
        {
            MORIS_ERROR(0,"Entered function in Mesh base class, (function is not implemented)");
            return *mDummyCells;
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_id
        Mesh::get_max_entity_id( enum EntityRank aEntityRank,
                           const moris_index     aBSplineMeshIndex) const
        {
            MORIS_ERROR(0,"Entered virtual function in Mesh base class, (function is not implemented)");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_id
        Mesh::get_entity_owner(
                moris_index       aEntityIndex,
                enum EntityRank   aEntityRank,
                const moris_index aBSPlineMeshIndex) const
        {
            MORIS_ERROR(0," get entity owner has no base implementation");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        void
        Mesh::get_processors_whom_share_entity(
                moris_index       aEntityIndex,
                enum EntityRank   aEntityRank,
                Matrix< IdMat > & aProcsWhomShareEntity) const
        {
            MORIS_ERROR(0," get_processors_whom_share_entity has no base implementation");
        }

        //--------------------------------------------------------------------------------------------------------------

        uint
        Mesh::get_num_of_entities_shared_with_processor(
                moris_id        aProcessorRank,
                enum EntityRank aEntityRank,
                bool            aSendFlag) const
        {
            MORIS_ERROR(0," get_num_of_entities_shared_with_processor has no base implementation");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< IdMat >
        Mesh::get_communication_proc_ranks() const
        {
            MORIS_ERROR(0,"get_communication_proc_ranks not implemented");
            return Matrix< IdMat >(0,0);
        }

        //--------------------------------------------------------------------------------------------------------------

        moris::Cell<Matrix< IdMat >>
        Mesh::get_communication_vertex_pairing() const
        {
            MORIS_ERROR(0,"get_communication_vertex_pairing not implemented");
            return moris::Cell<Matrix< IdMat >>(0);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        void
        Mesh::create_output_mesh(
                std::string  &aFileName,
                bool          aAddElemCmap)
        {
            MORIS_ERROR(0,"Create output mesh not implemented");
        }

        //--------------------------------------------------------------------------------------------------------------
        
        uint
        Mesh::get_num_fields(
                const enum EntityRank aEntityRank,
                const moris_index     aBSPlineMeshIndex) const
        {
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_index Mesh::get_field_ind(
                const std::string     & aFieldLabel,
                const enum EntityRank   aEntityRank) const
        {
            MORIS_ERROR( false ,"get_field_ind() not implemented" );
            return gNoIndex;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris_index
        Mesh::create_scalar_field(
                const std::string   & aFieldLabel,
                const enum EntityRank aEntityRank )
        {
            MORIS_ERROR( false ,"create_scalar_field() not implemented" );
            return gNoIndex;
        }

        //--------------------------------------------------------------------------------------------------------------

        /**
         * add a vector field to the database
         */
        moris_index
        Mesh::create_vector_field(
                const std::string   & aFieldLabel,
                const enum EntityRank aEntityRank,
                const uint            aDimension )
        {
            MORIS_ERROR( false ,"create_vector_field() not implemented" );
            return gNoIndex;
        }

        //--------------------------------------------------------------------------------------------------------------

        /**
         * get value of entity
         */
        real &
        Mesh::get_value_of_scalar_field(
                const moris_index     aFieldIndex,
                const enum EntityRank aEntityRank,
                const uint            aEntityIndex,
                const moris_index     aBSPlineMeshIndex)
        {
            MORIS_ERROR( false ,"get_value_of_scalar_field() not implemented" );
            return mDummyReal;
        }

        //--------------------------------------------------------------------------------------------------------------

        /**
         * get value of entity ( const version )
         */
        const real &
        Mesh::get_value_of_scalar_field(
                const moris_index     aFieldIndex,
                const enum EntityRank aEntityRank,
                const uint            aEntityIndex,
                const moris_index     aBSPlineMeshIndex) const
        {
            MORIS_ERROR( false ,"get_value_of_scalar_field() const not implemented" );
            return mDummyReal;
        }

        //--------------------------------------------------------------------------------------------------------------

        /**
         * fixme: need opinion: sould we always return a DDRMat?
         *        should this be a row or column vector?
         */
        Matrix<DDRMat> &
        Mesh::get_value_of_vector_field(
                const moris_index     aFieldIndex,
                const enum EntityRank aEntityRank,
                const uint            aEntityIndex )
        {
            MORIS_ERROR( false ,"get_value_of_vector_field() not implemented" );
            return  mDummyMatrix;
        }

        //--------------------------------------------------------------------------------------------------------------

        /**
         * return the entry of a vector field ( const version )
         */
        const Matrix<DDRMat> &
        Mesh::get_value_of_vector_field(
                const moris_index     aFieldIndex,
                const enum EntityRank aEntityRank,
                const uint            aEntityIndex ) const
        {
            MORIS_ERROR( false ,"get_value_of_vector_field() not implemented" );
            return mDummyMatrix;
        }

        //--------------------------------------------------------------------------------------------------------------

        /**
         * returns a Matrix with the field
         * This function is specific to HMR, and called by the mapper
         * if HMR is used.
         */
        Matrix<DDRMat> &
        Mesh::get_field( const moris_index  aFieldIndex,
                   const enum EntityRank aEntityRank,
                   const moris_index     aBSPlineMeshIndex)
        {
            MORIS_ERROR( false ,"get_field() not implemented" );
            return mDummyMatrix;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        uint
        Mesh::get_max_level_of_entity(
                const enum EntityRank aEntityRank,
                const moris_index     aBSPlineMeshIndex)
        {
            // no error is thrown here
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        uint
        Mesh::get_level_of_entity_loc_ind(
                const enum EntityRank aEntityRank,
                const uint            aEntityIndex,
                const moris_index     aBSPlineMeshIndex)
        {
            // no error is thrown here
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        std::shared_ptr< hmr::Database > Mesh::get_HMR_database( )
        {
            MORIS_ERROR( this->get_mesh_type() == MeshType::HMR ,"Not HMR" );
            return mDatabase;
        }

        //--------------------------------------------------------------------------------------------------------------

        hmr::Lagrange_Mesh_Base * Mesh::get_HMR_lagrange_mesh( )
        {
            MORIS_ERROR( this->get_mesh_type() == MeshType::HMR ,"Not HMR" );
            MORIS_ERROR( mMesh != nullptr ,"get_HMR_lagrange_mesh(), Lagrange mesh is nullptr" );

            return mMesh;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        uint
        Mesh::get_num_coeffs(const uint aBSplineMeshIndex) const
        {
            return this->get_num_nodes();
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Mesh::node_has_interpolation(uint aNodeIndex, uint aBSplineMeshIndex)
        {
            return false;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        const Matrix< DDRMat > &
        Mesh::get_t_matrix_of_node_loc_ind(
                uint aNodeIndex,
                uint aBSplineMeshIndex)
        {
            mDummyMatrix.set_size(1, 1, 1.0);
            return mDummyMatrix;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< IndexMat >
        Mesh::get_bspline_inds_of_node_loc_ind(
                uint aNodeIndex,
                uint aBSplineMeshIndex )
        {
            return {{(sint)aNodeIndex}};
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< IdMat >
        Mesh::get_bspline_ids_of_node_loc_ind(
                uint aNodeIndex,
                uint aBSplineMeshIndex )
        {
            return {{get_glb_entity_id_from_entity_loc_index(aNodeIndex, EntityRank::NODE)}};
        }

        //--------------------------------------------------------------------------------------------------------------

        
        uint
        Mesh::get_num_basis_functions(const uint aMeshIndex)
        {
            return this->get_num_nodes();
        }

        //--------------------------------------------------------------------------------------------------------------
        
        void
        Mesh::get_adof_map(
                const uint                     aBSplineIndex,
                map< moris_id, moris_index > & aAdofMap ) const
        {
            MORIS_ERROR(false, "Entered function get_adof_map() in Mesh base class, (function is not implemented)");
        }

        //--------------------------------------------------------------------------------------------------------------

        /**
         * return the interpolation order of this field
         */
        uint
        Mesh::get_order_of_field(
                const moris_index     aFieldIndex,
                const enum EntityRank aEntityRank )
        {
            MORIS_ERROR(false , "get_order_of_field() not implemented" );
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris::Cell<std::string>
        Mesh::get_set_names(enum EntityRank aSetEntityRank) const
        {
            MORIS_ERROR(0," get_set_names has no base implementation");
            return moris::Cell<std::string>(0);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        Matrix< IndexMat >
        Mesh::get_set_entity_loc_inds( enum EntityRank aSetEntityRank,
                                 std::string     aSetName) const
        {
            MORIS_ERROR(0," get_set_entity_loc_inds has no base implementation");
            return Matrix< IndexMat >(0,0);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<IdMat> Mesh::get_element_ids_in_block_set(uint aSetIndex)
        {
            // Get element indices
            Matrix<IndexMat> tElementIndices = this->get_element_indices_in_block_set(aSetIndex);

            // Get element IDs using indices
            Matrix<IdMat> tElementIDs(tElementIndices.n_rows(), tElementIndices.n_cols());
            for (uint tElementNumber = 0; tElementNumber < tElementIndices.length(); tElementNumber++)
            {
                tElementIDs(tElementNumber) = this->get_glb_entity_id_from_entity_loc_index(tElementIndices(tElementNumber), EntityRank::ELEMENT);
            }

            return tElementIDs;
        }
        
        //--------------------------------------------------------------------------------------------------------------
        
        enum CellTopology
        Mesh::get_sideset_topology(const  std::string & aSetName)
        {
            MORIS_ERROR(0," get_sideset_topology has no base implementation");
            return CellTopology::INVALID;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris::Cell< Cell const * > Mesh::get_set_cells( std::string aSetLabel ) const
        {
            Set * tSet = this->get_set_by_name( aSetLabel );

            enum SetType tSetType = tSet->get_set_type();

            moris::Cell<Cell const *> tBlockSetCells;

            if ( tSetType == SetType::BULK )
            {
                Matrix< IndexMat > tBlockSetElementInd = this->get_set_entity_loc_inds( EntityRank::ELEMENT, aSetLabel );

                tBlockSetCells.resize(tBlockSetElementInd.numel());

                for ( luint k=0; k < tBlockSetElementInd.numel(); ++k )
                {
                    tBlockSetCells( k ) = & this->get_mtk_cell( tBlockSetElementInd(k) );
                }
            }
            else
            {
                MORIS_ERROR(false, "get_set_cells(), Only implemented for ELEMENT. Element for rest!!!") ;
            }

            return tBlockSetCells;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris::Cell<Cell const *>
        Mesh::get_block_set_cells( std::string     aSetName) const
        {
            Matrix< IndexMat > tBlockSetElementInd = this->get_set_entity_loc_inds( EntityRank::ELEMENT, aSetName );

            moris::Cell<Cell const *> tBlockSetCells(tBlockSetElementInd.numel());

            for( luint k=0; k < tBlockSetElementInd.numel(); ++k )
            {
                tBlockSetCells( k ) = & this->get_mtk_cell( tBlockSetElementInd(k) );
            }

            return tBlockSetCells;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        void Mesh::get_sideset_elems_loc_inds_and_ords(
                const  std::string & aSetName,
                Matrix< IndexMat > & aElemIndices,
                Matrix< IndexMat > & aSidesetOrdinals ) const
        {
            MORIS_ERROR(0," get_sideset_elems_loc_inds_and_ords has no base implementation");
        }

        //--------------------------------------------------------------------------------------------------------------

        
        void Mesh::get_sideset_cells_and_ords(
                const  std::string               & aSetName,
                moris::Cell< Cell const * > & aCells,
                Matrix< IndexMat >               & aSidesetOrdinals ) const
        {
            Matrix<IndexMat> tElemIndices;
            this->get_sideset_elems_loc_inds_and_ords( aSetName, tElemIndices, aSidesetOrdinals );

            // convert element indices to cell pointers
            uint tNumCellsInSet = tElemIndices.numel();
            aCells = moris::Cell< Cell const * >(tNumCellsInSet);

            for(uint i = 0 ; i < tNumCellsInSet; i++)
            {
                aCells(i) = &this->get_mtk_cell(tElemIndices(i));
            }
        }

        //--------------------------------------------------------------------------------------------------------------
        
        uint Mesh::get_sidesets_num_faces( moris::Cell< moris_index > aSideSetIndex ) const
        {
            uint tNumSideSetFaces = 0;

            moris::Cell<std::string> tSideSetsNames = this->get_set_names( this->get_facet_rank() );

            for( luint Ik=0; Ik < aSideSetIndex.size(); ++Ik )
            {
                // get the treated sideset name
                std::string tTreatedSideset = tSideSetsNames( aSideSetIndex ( Ik ) );

                // get the sideset face indices
                Matrix< IndexMat > tSideSetElementInd = this->get_set_entity_loc_inds( this->get_facet_rank(), tTreatedSideset );

                // add up the sideset number of faces
                tNumSideSetFaces = tNumSideSetFaces + tSideSetElementInd.numel();
            }

            return tNumSideSetFaces;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        moris::Cell<Vertex const *>
        Mesh::get_vertices_in_vertex_set_no_aura(std::string aSetName) const
        {
            MORIS_ERROR(0,"No default implementation of get_vertices_in_vertex_set");
            return moris::Cell<Vertex const *> (0);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        enum EntityRank
        Mesh::get_facet_rank() const
        {
            uint tSpatialDim = this->get_spatial_dim();
            if(tSpatialDim  == 1)
            {
                return EntityRank::NODE;
            }
            else if(tSpatialDim == 2)
            {
                return EntityRank::EDGE;
            }
            else if(tSpatialDim == 3)
            {
                return EntityRank::FACE;
            }
            else
            {
                MORIS_ASSERT(0,"Invalid Mesh dimension detected in get_facet_rank ");
                return EntityRank::INVALID;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        Mesh::get_mtk_cells(
                Matrix< IndexMat >          aCellInds,
                moris::Cell<Cell const *> & aCells)
        {
            aCells = moris::Cell< Cell const * >(aCellInds.numel());

            for(uint i = 0 ; i < aCellInds.numel(); i++)
            {
                aCells(i) = &this->get_mtk_cell(aCellInds(i));
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_num_interpolations()
        {
            return 1;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_max_level( const moris_index aInterpolationIndex )
        {
            MORIS_ERROR( false, "get_max_level(), not implemented for this mesh type.");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_num_basis( const moris_index aInterpolationIndex )
        {
            MORIS_ERROR( false, "get_num_basis(), not implemented for this mesh type.");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_basis_level( const moris_index aInterpolationIndex,
                                      const moris_index aBasisIndex )
        {
            MORIS_ERROR( false, "get_basis_level(), not implemented for this mesh type.");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_num_coarse_basis_of_basis(
                const moris_index aInterpolationIndex,
                const moris_index aBasisIndex )
        {
            MORIS_ERROR( false, "get_num_coarse_basis_of_basis(), not implemented for this mesh type.");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Mesh::get_coarse_basis_index_of_basis(
                const moris_index aInterpolationIndex,
                const moris_index aBasisIndex,
                const moris_index aCoarseParentIndex )
        {
            MORIS_ERROR( false, "get_coarse_basis_index_of_basis(), not implemented for this mesh type.");
            return 0;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< DDSMat > Mesh::get_fine_basis_inds_of_basis(
                const moris_index aInterpolationIndex,
                const moris_index aBasisIndex )
        {
            MORIS_ERROR( false, "get_fine_basis_inds_of_basis(), not implemented for this mesh type.");
            return mDummyMatrix2;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix< DDRMat > Mesh::get_fine_basis_weights_of_basis(
                const moris_index aInterpolationIndex,
                const moris_index aBasisIndex )
        {
            MORIS_ERROR( false, "get_fine_basis_weights_of_basis(), not implemented for this mesh type.");
            return mDummyMatrix;
        }

        //--------------------------------------------------------------------------------------------------------------

#ifdef DEBUG

        Matrix< DDRMat > Mesh::get_basis_coords(
                const moris_index aInterpolationIndex,
                const moris_index aBasisIndex )
        {
            MORIS_ERROR( false, "get_basis_coords(), not implemented for this mesh type.");
            return mDummyMatrix;
        }

        //--------------------------------------------------------------------------------------------------------------

        sint Mesh::get_basis_status(
                const moris_index aInterpolationIndex,
                const moris_index aBasisIndex )
        {
            MORIS_ERROR( false, "get_basis_status(), not implemented for this mesh type.");
            return 0;
        }

#endif

        //--------------------------------------------------------------------------------------------------------------
                
    }
}
