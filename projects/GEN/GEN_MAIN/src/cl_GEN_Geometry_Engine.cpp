// MRS
#include "fn_Parsing_Tools.hpp"
#include "cl_Tracer.hpp"

// GEN
#include "cl_GEN_Geometry_Engine.hpp"
#include "GEN_typedefs.hpp"
#include "fn_GEN_create_geometries.hpp"
#include "cl_GEN_BSpline_Geometry.hpp"
#include "cl_GEN_BSpline_Property.hpp"
#include "cl_GEN_Stored_Geometry.hpp"
#include "fn_GEN_create_properties.hpp"
#include "cl_GEN_Interpolation.hpp"
#include "cl_GEN_Child_Node.hpp"
#include "cl_GEN_Intersection_Node_Linear.hpp"
#include "cl_GEN_Intersection_Node_Bilinear.hpp"

// MTK
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"
#include "cl_MTK_Writer_Exodus.hpp"

// XTK FIXME
#include "cl_XTK_Topology.hpp"

// SOL FIXME
#include "cl_SOL_Matrix_Vector_Factory.hpp"
#include "cl_SOL_Dist_Map.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------
        // PUBLIC
        //--------------------------------------------------------------------------------------------------------------

        Geometry_Engine::Geometry_Engine(
                Cell<Cell<ParameterList>> aParameterLists,
                std::shared_ptr<Library_IO> aLibrary,
                std::shared_ptr< mtk::Mesh_Manager >  aMTKMesh)

                // Phase table
                : mPhaseTable( create_phase_table(aParameterLists, aLibrary) )

                // Level set options
                , mIsocontourThreshold( aParameterLists(0)(0).get<real>("isocontour_threshold") )
                , mIsocontourTolerance( aParameterLists(0)(0).get<real>("isocontour_tolerance") )
                , mIntersectionTolerance( aParameterLists(0)(0).get<real>("intersection_tolerance") )

                // Requested IQIs
                , mRequestedIQIs( string_to_cell<std::string>( aParameterLists(0)(0).get<std::string>("IQI_types") ) )

                // Library
                , mLibrary(aLibrary)

                // Geometries
                , mGeometryFieldFile( aParameterLists(0)(0).get<std::string>("geometry_field_file") )
                , mOutputMeshFile( aParameterLists(0)(0).get<std::string>("output_mesh_file") )
                , mTimeOffset( aParameterLists(0)(0).get<real>("time_offset") )
        {
            // Tracer
            Tracer tTracer("GEN", "N/A", "Create geometry engine");

            // Read ADVs
            if ( aParameterLists(0)(0).get<sint>("advs_size") )
            {
                mADVs        = Matrix<DDRMat>(aParameterLists(0)(0).get<sint>("advs_size"), 1, aParameterLists(0)(0).get<real>("initial_advs_fill"));
                mLowerBounds = Matrix<DDRMat>(aParameterLists(0)(0).get<sint>("advs_size"), 1, aParameterLists(0)(0).get<real>("lower_bounds_fill"));
                mUpperBounds = Matrix<DDRMat>(aParameterLists(0)(0).get<sint>("advs_size"), 1, aParameterLists(0)(0).get<real>("upper_bounds_fill"));
            }
            else
            {
                mADVs        = string_to_mat<DDRMat>(aParameterLists(0)(0).get<std::string>("initial_advs"));
                mLowerBounds = string_to_mat<DDRMat>(aParameterLists(0)(0).get<std::string>("lower_bounds"));
                mUpperBounds = string_to_mat<DDRMat>(aParameterLists(0)(0).get<std::string>("upper_bounds"));

                // check that advs and bounds are vectors
                MORIS_ERROR ( isvector(mADVs),        "ADVs need to be of type vector.\n");
                MORIS_ERROR ( isvector(mLowerBounds), "ADV lower bounds need to be of type vector.\n");
                MORIS_ERROR ( isvector(mUpperBounds), "ADV upper bounds need to be of type vector.\n");

                // ensure that advs and bounds are column vectors
                mADVs        = mADVs.n_rows()        == 1 ? trans(mADVs) : mADVs;
                mLowerBounds = mLowerBounds.n_rows() == 1 ? trans(mLowerBounds) : mLowerBounds;
                mUpperBounds = mUpperBounds.n_rows() == 1 ? trans(mUpperBounds) : mUpperBounds;
            }

            // Geometries
            mGeometries = create_geometries(
                    aParameterLists(1),
                    mADVs,
                    mLibrary,
                    aMTKMesh == nullptr ? nullptr : aMTKMesh->get_interpolation_mesh(0));



            MORIS_ERROR(mGeometries.size() <= MAX_GEOMETRIES,
                    "Number of geometries exceeds MAX_GEOMETRIES, please change this in GEN_typedefs.hpp");

            // Properties
            mProperties = create_properties(
                    aParameterLists(2),
                    mADVs,
                    mGeometries,
                    mLibrary);

            // Get intersection mode
            std::string tIntersectionModeString = aParameterLists(0)(0).get<std::string>("intersection_mode");
            map< std::string, Intersection_Mode > tIntersectionModeMap = get_intersection_mode_map();
            mIntersectionMode = tIntersectionModeMap[ tIntersectionModeString ];

            // Set requested PDVs
            Cell<std::string> tRequestedPdvNames = string_to_cell<std::string>(aParameterLists(0)(0).get<std::string>("PDV_types"));
            Cell<PDV_Type> tRequestedPdvTypes(tRequestedPdvNames.size());
            map<std::string, PDV_Type> tPdvTypeMap = get_pdv_type_map();
            for (uint tPdvTypeIndex = 0; tPdvTypeIndex < tRequestedPdvTypes.size(); tPdvTypeIndex++)
            {
                tRequestedPdvTypes(tPdvTypeIndex) = tPdvTypeMap[tRequestedPdvNames(tPdvTypeIndex)];
            }
            mPdvHostManager.set_requested_interpolation_pdv_types(tRequestedPdvTypes);

            // Initialize PDV type list
            this->initialize_pdv_type_list();

            // Print the phase table if requested
            if (aParameterLists(0)(0).get<bool>("print_phase_table") and par_rank() == 0)
            {
                mPhaseTable.print();
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        Geometry_Engine::Geometry_Engine(
                mtk::Interpolation_Mesh*   aMesh,
                Geometry_Engine_Parameters aParameters)
                : mGeometries(aParameters.mGeometries)
                , mProperties(aParameters.mProperties)
                , mPhaseTable( create_phase_table(aParameters.mGeometries.size(), aParameters.mBulkPhases) )
                , mIntersectionMode(aParameters.mIntersectionMode)
                , mIsocontourThreshold(aParameters.mIsocontourThreshold)
                , mIsocontourTolerance(aParameters.mIsocontourTolerance)
                , mIntersectionTolerance(aParameters.mIntersectionTolerance)
                , mTimeOffset(aParameters.mTimeOffset)
        {
            // Tracer
            Tracer tTracer("GEN", "N/A", "Create geometry engine");

            this->distribute_advs(aMesh);
        }

        //--------------------------------------------------------------------------------------------------------------

        Geometry_Engine::~Geometry_Engine()
        {
            delete mOwnedADVs;
            delete mPrimitiveADVs;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::set_advs(const Matrix<DDRMat> & aNewADVs)
        {
            // Set new ADVs
            mOwnedADVs->vec_put_scalar(0);
            mOwnedADVs->replace_global_values(mFullADVIds, aNewADVs);
            mOwnedADVs->vector_global_assembly();
            mPrimitiveADVs->import_local_to_global(*mOwnedADVs);

            // Reset info related to the mesh
            mPdvHostManager.reset();
            mActiveGeometryIndex = 0;
            for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
            {
                mGeometries(tGeometryIndex)->import_advs(mOwnedADVs);
                mGeometries(tGeometryIndex)->reset_nodal_information();
            }
            for (uint tPropertyIndex = 0; tPropertyIndex < mProperties.size(); tPropertyIndex++)
            {
                mProperties(tPropertyIndex)->import_advs(mOwnedADVs);
                mProperties(tPropertyIndex)->reset_nodal_information();
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat>& Geometry_Engine::get_advs()
        {
            // Create full ADVs
            sol::Matrix_Vector_Factory tDistributedFactory;

            sol::Dist_Map* tFullMap       = tDistributedFactory.create_map(mFullADVIds);
            sol::Dist_Vector* tFullVector = tDistributedFactory.create_vector(tFullMap, 1, false, true);

            // Import ADVs
            tFullVector->import_local_to_global(*mOwnedADVs);

            // Extract copy
            tFullVector->extract_copy(mADVs);

            // Delete full ADVs/map
            delete tFullVector;

            return mADVs;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat>& Geometry_Engine::get_lower_bounds()
        {
            return mLowerBounds;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat>& Geometry_Engine::get_upper_bounds()
        {
            return mUpperBounds;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::communicate_requested_IQIs()
        {
            mPdvHostManager.set_requested_IQIs(mRequestedIQIs);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::communicate_requested_IQIs(Cell<std::string> aIQINames)
        {
            mPdvHostManager.set_requested_IQIs(aIQINames);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Geometry_Engine::get_dcriteria_dadv()
        {
            return mPdvHostManager.compute_diqi_dadv(mFullADVIds);
        }

        //--------------------------------------------------------------------------------------------------------------

        MSI::Design_Variable_Interface* Geometry_Engine::get_design_variable_interface()
        {
            return &mPdvHostManager;
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Geometry_Engine::is_intersected(const Matrix<IndexMat>& aNodeIndices, const Matrix<DDRMat>& aNodeCoordinates)
        {
            // Check input
            MORIS_ASSERT(aNodeIndices.length() == aNodeCoordinates.n_rows(),
                    "Geometry engine must be provided the same number of node indices as node coordinates for "
                    "determining if an element is intersected or not.");
            MORIS_ASSERT(aNodeIndices.length() > 0,
                    "Geometry engine must be provided at least 1 node to determine if an element is intersected or not.");

            bool tIsIntersected = false;

            switch (mIntersectionMode)
            {
                case Intersection_Mode::LEVEL_SET:
                {
                    // Initialize by evaluating the first node
                    real tMin = mGeometries(mActiveGeometryIndex)->get_field_value(aNodeIndices(0), aNodeCoordinates.get_row(0));
                    real tMax = tMin;

                    // Evaluate the rest of the nodes
                    for (uint tNodeCount = 0; tNodeCount < aNodeIndices.length(); tNodeCount++)
                    {
                        real tEval = mGeometries(mActiveGeometryIndex)->get_field_value(
                                aNodeIndices(tNodeCount),
                                aNodeCoordinates.get_row(tNodeCount));
                        tMin = std::min(tMin, tEval);
                        tMax = std::max(tMax, tEval);
                    }

                    tIsIntersected = (tMax >= mIsocontourThreshold and tMin <= mIsocontourThreshold);

                    break;
                }
                case Intersection_Mode::COLORING:
                {
                    real tFieldValue = mGeometries(mActiveGeometryIndex)->
                            get_field_value(aNodeIndices(0), aNodeCoordinates.get_row(0));

                    // Evaluate the rest of the nodes
                    for (uint Ik = 0; Ik < aNodeIndices.length(); Ik++)
                    {
                        real tEval = mGeometries(mActiveGeometryIndex)->get_field_value(
                                aNodeIndices( Ik ),
                                aNodeCoordinates.get_row( Ik ));

                        if( tFieldValue != tEval )
                        {
                            tIsIntersected = true;
                            break;
                        }
                    }

                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "Geometry_Engine::is_intersected(), unknown intersection type." );
                }
            }

            // Return result
            return tIsIntersected;
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Geometry_Engine::queue_intersection(
                uint                        aFirstNodeIndex,
                uint                        aSecondNodeIndex,
                const Matrix<DDRMat>&       aFirstNodeLocalCoordinates,
                const Matrix<DDRMat>&       aSecondNodeLocalCoordinates,
                const Matrix<DDRMat>&       aFirstNodeGlobalCoordinates,
                const Matrix<DDRMat>&       aSecondNodeGlobalCoordinates,
                const Matrix<DDUMat>&       aBackgroundElementNodeIndices,
                const Cell<Matrix<DDRMat>>& aBackgroundElementNodeCoordinates)
        {
            // Queue an intersection node
            switch (mIntersectionMode)
            {
                case Intersection_Mode::LEVEL_SET:
                {
                    switch (mGeometries(mActiveGeometryIndex)->get_intersection_interpolation())
                    {
                        case Intersection_Interpolation::LINEAR:
                        {
                            mQueuedIntersectionNode = std::make_shared<Intersection_Node_Linear>(
                                    aFirstNodeIndex,
                                    aSecondNodeIndex,
                                    aFirstNodeGlobalCoordinates,
                                    aSecondNodeGlobalCoordinates,
                                    mGeometries(mActiveGeometryIndex),
                                    mIsocontourThreshold,
                                    mIsocontourTolerance,
                                    mIntersectionTolerance);
                            break;
                        }
                        case Intersection_Interpolation::MULTILINEAR:
                        {
                            if (mNumSpatialDimensions == 2)
                            {
                                mQueuedIntersectionNode = std::make_shared<Intersection_Node_Bilinear>(
                                        aFirstNodeIndex,
                                        aSecondNodeIndex,
                                        aFirstNodeLocalCoordinates,
                                        aSecondNodeLocalCoordinates,
                                        aBackgroundElementNodeIndices,
                                        aBackgroundElementNodeCoordinates,
                                        mGeometries(mActiveGeometryIndex),
                                        mIsocontourThreshold,
                                        mIsocontourTolerance,
                                        mIntersectionTolerance);
                            }
                            else
                            {
                                MORIS_ERROR(false, "Only bilinear intersections have been implemented.");
                            }

                            break;
                        }
                        default:
                        {
                            MORIS_ERROR(false, "Intersection interpolation type not implemented yet.");
                        }
                    }
                    break;
                }
                case Intersection_Mode::COLORING:
                {
                    // Determine if edge is intersected
                    if (mGeometries(mActiveGeometryIndex)->get_field_value(aFirstNodeIndex , aFirstNodeGlobalCoordinates ) !=
                        mGeometries(mActiveGeometryIndex)->get_field_value(aSecondNodeIndex, aSecondNodeGlobalCoordinates))
                    {
                        mQueuedIntersectionNode = std::make_shared<Intersection_Node_Linear>(
                                aFirstNodeIndex,
                                aSecondNodeIndex,
                                aFirstNodeGlobalCoordinates,
                                aSecondNodeGlobalCoordinates,
                                mGeometries(mActiveGeometryIndex),
                                mIsocontourThreshold,
                                mIsocontourTolerance);
                    }
                    else
                    {
                        return false;
                    }

                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "Geometry_Engine::queue_intersection(), unknown intersection type." );
                }
            }

            return mQueuedIntersectionNode->parent_edge_is_intersected();
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Geometry_Engine::queued_intersection_first_parent_on_interface()
        {
            return mQueuedIntersectionNode->first_parent_on_interface();
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Geometry_Engine::queued_intersection_second_parent_on_interface()
        {
            return mQueuedIntersectionNode->second_parent_on_interface();
        }

        //--------------------------------------------------------------------------------------------------------------

        real Geometry_Engine::get_queued_intersection_local_coordinate()
        {
            return mQueuedIntersectionNode->get_local_coordinate();
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Geometry_Engine::get_queued_intersection_global_coordinates()
        {
            return mQueuedIntersectionNode->get_global_coordinates();
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::admit_queued_intersection(uint aNodeIndex)
        {
            // Assign as PDV host
            if (mGeometries(mActiveGeometryIndex)->depends_on_advs())
            {
                mPdvHostManager.set_intersection_node(aNodeIndex, mQueuedIntersectionNode);
            }
            else
            {
                mPdvHostManager.set_intersection_node(aNodeIndex, nullptr);
            }

            // Assign as child node
            for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
            {
                // tGeomProx.set_geometric_proximity();
                mGeometries(tGeometryIndex)->add_child_node(aNodeIndex, mQueuedIntersectionNode);
            }

            // admit the queued intersection geometric proximity
            this->admit_queued_intersection_geometric_proximity(aNodeIndex);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::update_queued_intersection(
                const moris_index & aNodeIndex,
                const moris_index & aNodeId,
                const moris_index & aNodeOwner )
        {
            mPdvHostManager.update_intersection_node(aNodeIndex, aNodeId, aNodeOwner);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::create_new_child_nodes(
                const Cell<moris_index>&    aNewNodeIndices,
                const Cell<xtk::Topology*>& aParentTopo,
                const Cell<Matrix<DDRMat>>& aParamCoordRelativeToParent,
                const Matrix<DDRMat>&       aGlobalNodeCoord )
        {
            // resize proximities
            mVertexGeometricProximity.resize(mVertexGeometricProximity.size() + aNewNodeIndices.size(), Geometric_Proximity(mGeometries.size()));

            // Loop over nodes
            for (uint tNode = 0; tNode < aNewNodeIndices.size(); tNode++)
            {
                Matrix<DDUMat> tParentNodeIndices(aParentTopo(tNode)->get_node_indices().length(), 1);
                Cell<Matrix<DDRMat>> tParentNodeCoordinates(tParentNodeIndices.length());
                for (uint tParentNode = 0; tParentNode < tParentNodeIndices.length(); tParentNode++)
                {
                    tParentNodeIndices(tParentNode) = aParentTopo(tNode)->get_node_indices()(tParentNode);
                    tParentNodeCoordinates(tParentNode) = aGlobalNodeCoord.get_row(tParentNodeIndices(tParentNode));
                }
                std::shared_ptr<Child_Node> tChildNode = std::make_shared<Child_Node>(
                        tParentNodeIndices, tParentNodeCoordinates, aParentTopo(tNode)->get_basis_function(), aParamCoordRelativeToParent(tNode));

                mVertexGeometricProximity(aNewNodeIndices(tNode)).mAssociatedVertexIndex = aNewNodeIndices(tNode);

                Matrix<DDRMat> tCoord = aGlobalNodeCoord.get_row(aNewNodeIndices(tNode));

                // Assign to geometries
                for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
                {
                    mGeometries(tGeometryIndex)->add_child_node(aNewNodeIndices(tNode), tChildNode);

                    real tVertGeomVal = mGeometries(tGeometryIndex)->get_field_value(aNewNodeIndices(tNode), tCoord);

                    moris_index tGeomProxIndex = this->get_geometric_proximity_index(tVertGeomVal);

                    mVertexGeometricProximity(aNewNodeIndices(tNode)).set_geometric_proximity(tGeomProxIndex,tGeometryIndex);
                }
            }

            // Set max node index
            if (aNewNodeIndices.size() > 0)
            {
                mPdvHostManager.set_num_background_nodes(aNewNodeIndices(aNewNodeIndices.size() - 1) + 1);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        size_t Geometry_Engine::get_num_phases()
        {
            return mPhaseTable.get_num_phases();
        }

        //--------------------------------------------------------------------------------------------------------------

        size_t Geometry_Engine::get_phase_index(
                moris_index            aNodeIndex,
                const Matrix<DDRMat> & aCoordinates)
        {
            // Initialize bitset of geometry signs
            Geometry_Bitset tGeometrySigns(0);

            // Flip bits as needed
            for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
            {
                moris_index tProxIndex = mVertexGeometricProximity(aNodeIndex).get_geometric_proximity((moris_index)tGeometryIndex);
                tGeometrySigns.set(tGeometryIndex, tProxIndex == 2);
            }

            return mPhaseTable.get_phase_index(tGeometrySigns);
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_index Geometry_Engine::get_elem_phase_index(Matrix< IndexMat > const & aElemOnOff)
        {
            // FIXME
            Geometry_Bitset tGeometrySigns(0);
            for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
            {
                tGeometrySigns.set(tGeometryIndex, aElemOnOff(tGeometryIndex));
            }

            return mPhaseTable.get_phase_index(tGeometrySigns);
        }

        //--------------------------------------------------------------------------------------------------------------

        size_t Geometry_Engine::get_node_phase_index_wrt_a_geometry(
                uint                   aNodeIndex,
                const Matrix<DDRMat> & aCoordinates,
                uint                   aGeometryIndex)
        {

            moris_index tProxIndex = mVertexGeometricProximity(aNodeIndex).get_geometric_proximity(aGeometryIndex);


            size_t tPhaseOnOff = 0;

            if (tProxIndex == 2)
            {
                tPhaseOnOff = 1;
            }

            return tPhaseOnOff;
        }

        //--------------------------------------------------------------------------------------------------------------

        size_t Geometry_Engine::get_num_geometries()
        {
            return mGeometries.size();
        }

        //--------------------------------------------------------------------------------------------------------------

        size_t Geometry_Engine::get_num_bulk_phase()
        {
            return mPhaseTable.get_num_phases();
        }

        //--------------------------------------------------------------------------------------------------------------

        size_t Geometry_Engine::get_active_geometry_index()
        {
            return mActiveGeometryIndex;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::advance_geometry_index()
        {
            MORIS_ASSERT(mActiveGeometryIndex < mGeometries.size(),
                    "Trying to advance past the number of geometries in the geometry engine");
            mActiveGeometryIndex += 1;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Geometry_Engine::get_num_refinement_fields()
        {
            return mGeometries.size();
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix< DDSMat > & Geometry_Engine::get_num_refinements(uint aFieldIndex )
        {
            return mGeometries(aFieldIndex)->get_num_refinements();
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix< DDSMat > & Geometry_Engine::get_refinement_mesh_indices(uint aFieldIndex )
        {
            return mGeometries(aFieldIndex)->get_refinement_mesh_indices();
        }

        //--------------------------------------------------------------------------------------------------------------

        real Geometry_Engine::get_field_value(
                uint                  aFieldIndex,
                uint                  aNodeIndex,
                const Matrix<DDRMat>& aCoordinates)
        {
            // TODO can return property field too
            return mGeometries(aFieldIndex)->get_field_value(aNodeIndex, aCoordinates);
        }

        //--------------------------------------------------------------------------------------------------------------

        sint Geometry_Engine::get_refinement_function_index(
                uint aFieldIndex,
                uint aRefinementIndex)
        {
            return mGeometries(aFieldIndex)->get_refinement_function_index();
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::create_pdvs(std::shared_ptr<mtk::Mesh_Manager> aMeshManager)
        {
            // Tracer
            Tracer tTracer( "GEN", "N/A", "Create PDVs" );

            // Get meshes
            mtk::Integration_Mesh* tIntegrationMesh = aMeshManager->get_integration_mesh(0);
            mtk::Interpolation_Mesh* tInterpolationMesh = aMeshManager->get_interpolation_mesh(0);

            // Initialize PDV type groups and mesh set info
            Cell<Cell<Cell<PDV_Type>>> tPdvTypes(tIntegrationMesh->get_num_sets());
            Cell<PDV_Type> tPDVTypeGroup(1);
            Cell<Matrix<DDUMat>> tMeshSetIndicesPerProperty(mProperties.size());

            // Loop over properties to create PDVs
            for (uint tPropertyIndex = 0; tPropertyIndex < mProperties.size(); tPropertyIndex++)
            {
                // PDV type and mesh set names/indices from parameter list
                tPDVTypeGroup(0) = mProperties(tPropertyIndex)->get_pdv_type();

                tMeshSetIndicesPerProperty(tPropertyIndex) = mProperties(tPropertyIndex)->get_pdv_mesh_set_indices();
                Cell<std::string> tMeshSetNames = mProperties(tPropertyIndex)->get_pdv_mesh_set_names();

                // Convert mesh set names to indices
                uint tNumSetIndices = tMeshSetIndicesPerProperty(tPropertyIndex).length();
                tMeshSetIndicesPerProperty(tPropertyIndex).resize(tNumSetIndices + tMeshSetNames.size(), 1);

                for (uint tIndex = tNumSetIndices; tIndex < tMeshSetIndicesPerProperty(tPropertyIndex).length(); tIndex++)
                {
                    tMeshSetIndicesPerProperty(tPropertyIndex)(tIndex) = tIntegrationMesh->get_set_index_by_name(tMeshSetNames(tIndex - tNumSetIndices));
                }

                // Assign PDV types
                for (uint tIndex = 0; tIndex < tMeshSetIndicesPerProperty(tPropertyIndex).length(); tIndex++)
                {
                    tPdvTypes(tMeshSetIndicesPerProperty(tPropertyIndex)(tIndex)).push_back(tPDVTypeGroup);
                }
            }

            Matrix< IdMat > tCommTable = tInterpolationMesh->get_communication_table();
            std::unordered_map<moris_id,moris_index> tIPVertexGlobaToLocalMap =
                    tInterpolationMesh->get_vertex_glb_id_to_loc_vertex_ind_map();
            std::unordered_map<moris_id,moris_index> tIGVertexGlobaToLocalMap =
                    tIntegrationMesh->get_vertex_glb_id_to_loc_vertex_ind_map();

            mPdvHostManager.set_communication_table( tCommTable );
            mPdvHostManager.set_vertex_global_to_local_maps( tIPVertexGlobaToLocalMap, tIGVertexGlobaToLocalMap);

            // Create PDV hosts
            this->create_interpolation_pdv_hosts(
                    tInterpolationMesh,
                    tIntegrationMesh,
                    tPdvTypes);

            // Set integration PDV types
            if (mShapeSensitivities)
            {
                this->set_integration_pdv_types(tIntegrationMesh);
            }

            // Loop over properties to assign PDVs
            for (uint tPropertyIndex = 0; tPropertyIndex < mProperties.size(); tPropertyIndex++)
            {
                // Assign PDVs
                if (mProperties(tPropertyIndex)->is_interpolation_pdv())
                {
                    this->assign_property_to_pdv_hosts(
                            mProperties(tPropertyIndex),
                            tPDVTypeGroup(0),
                            tIntegrationMesh,
                            tMeshSetIndicesPerProperty(tPropertyIndex));
                }
                else
                {
                    MORIS_ERROR(false, "Assignment of PDVs is only supported with an interpolation mesh right now.");
                }
            }

            // Create PDV IDs
            mPdvHostManager.create_pdv_ids();
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::initialize_pdv_type_list()
        {
            // Reserve of temporary pdv type list
            Cell< enum PDV_Type > tTemporaryPdvTypeList;
            tTemporaryPdvTypeList.reserve( static_cast< int >( PDV_Type::UNDEFINED ) + 1 );
            Matrix< DDUMat > tListToCheckIfEnumExist( (static_cast< int >(PDV_Type::UNDEFINED) + 1), 1, 0 );

            // PDV type map
            map< std::string, PDV_Type > tPdvTypeMap = get_pdv_type_map();

            // Loop over properties to build parallel consitent pdv list
             for (uint tPropertyIndex = 0; tPropertyIndex < mProperties.size(); tPropertyIndex++)
             {
                 // PDV type and mesh set names/indices from parameter list
                 PDV_Type tPdvType = mProperties(tPropertyIndex)->get_pdv_type();

                 if ( tListToCheckIfEnumExist(static_cast< int >(tPdvType) , 0) == 0)
                 {
                     // Set 1 at position of the enum value
                     tListToCheckIfEnumExist( static_cast< int >(tPdvType) ,0 ) = 1;

                     tTemporaryPdvTypeList.push_back( tPdvType );
                 }
             }

            // Shrink pdv type list to fit
             tTemporaryPdvTypeList.shrink_to_fit();

            // Communicate dof types so that all processors have the same unique list
             mPdvHostManager.communicate_dof_types( tTemporaryPdvTypeList );

            // Create a map
             mPdvHostManager.create_dv_type_map();
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::distribute_advs( std::shared_ptr< mtk::Mesh_Manager> aMeshManager )
        {
            // Tracer
            Tracer tTracer("GEN", "N/A", "Distribute ADVs");

            mtk::Interpolation_Mesh* aMesh = aMeshManager->get_interpolation_mesh(0);

            // Register spatial dimension
            mNumSpatialDimensions = aMesh->get_spatial_dim();

            // Set number of background nodes
            mPdvHostManager.set_num_background_nodes(aMesh->get_num_nodes());

            // allocate proximity data
            this->setup_initial_geometric_proximities(aMesh);

            for( uint Ik = 0; Ik <mGeometries.size(); Ik++ )
            {
                mGeometries( Ik )->set_mesh( aMeshManager );
                mGeometries( Ik )->set_mesh_index( 0 );
            }
            for( uint Ik = 0; Ik <mProperties.size(); Ik++ )
            {
                mProperties( Ik )->set_mesh( aMeshManager );
                mProperties( Ik )->set_mesh_index( 0 );
            }

            // Gather all fields
            Cell<std::shared_ptr<Field>> tFields(mGeometries.size() + mProperties.size());
            std::copy(mGeometries.begin(), mGeometries.end(), tFields.begin());
            std::copy(mProperties.begin(), mProperties.end(), tFields.begin() + mGeometries.size());

            // Build distributed ADVs
            if (mOwnedADVs == nullptr)
            {
                //------------------------------------//
                // Determine owned and shared ADV IDs //
                //------------------------------------//

                // Loop over all geometries to get number of new ADVs
                uint tNumNewOwnedADVs = 0;
                for (uint tFieldIndex = 0; tFieldIndex < tFields.size(); tFieldIndex++)
                {
                    // Determine if level set will be created
                    if (tFields(tFieldIndex)->conversion_to_bsplines())
                    {
                        // Get number of coefficients
                        sint tBSplineMeshIndex = tFields(tFieldIndex)->get_discretization_mesh_index();
                        uint tNumCoefficients = aMesh->get_num_coeffs(tBSplineMeshIndex);

                        // Loop over B-spline coefficients
                        for (uint tBSplineIndex = 0; tBSplineIndex < tNumCoefficients; tBSplineIndex++)
                        {
                            // If this processor owns this coefficient
                            if ((uint) par_rank() == aMesh->get_entity_owner(tBSplineIndex, EntityRank::BSPLINE, tBSplineMeshIndex))
                            {
                                // New ADV needs to be created on this processor
                                tNumNewOwnedADVs++;
                            }
                        }
                    }
                }

                // Set number of total owned ADVs
                uint tNumOwnedADVs = tNumNewOwnedADVs;
                if (par_rank() == 0)
                {
                    tNumOwnedADVs += mADVs.length();
                }

                // Set primitive IDs
                Matrix<DDSMat> tPrimitiveADVIds(mADVs.length(), 1);
                for (uint tADVIndex = 0; tADVIndex < mADVs.length(); tADVIndex++)
                {
                    tPrimitiveADVIds(tADVIndex) = tADVIndex;
                }

                // Start with primitive IDs for owned IDs on processor 0
                Matrix<DDSMat> tOwnedADVIds(0, 0);
                if (par_rank() == 0)
                {
                    tOwnedADVIds = tPrimitiveADVIds;
                }

                // Check for proper dimensions
                MORIS_ASSERT( mLowerBounds.numel() > 0 ? mLowerBounds.n_cols() == 1 : true,
                        "ADV lower bound vector needs to be a column vector.\n");

                MORIS_ASSERT( mUpperBounds.numel() > 0 ? mUpperBounds.n_cols() == 1 : true,
                        "ADV upper bound vector needs to be a column vector.\n");

                // Resize owned IDs and bounds
                tOwnedADVIds.resize(tNumOwnedADVs, 1);
                mLowerBounds.resize(tNumOwnedADVs, 1);
                mUpperBounds.resize(tNumOwnedADVs, 1);

                // Cell of shared ADV IDs
                Cell<Matrix<DDSMat>> tSharedADVIds(mGeometries.size() + mProperties.size());

                // Loop over all geometry parameter lists to set B-spline ADV bounds and IDs
                uint tOwnedADVIndex = mADVs.length();
                uint tIdOffset = tOwnedADVIndex; // FIXME this needs to be updated for multiple B-spline fields
                for (uint tFieldIndex = 0; tFieldIndex < tFields.size(); tFieldIndex++)
                {
                    // Determine if level set will be created
                    if (tFields(tFieldIndex)->conversion_to_bsplines())
                    {
                        // Get bounds
                        real tBSplineLowerBound = tFields(tFieldIndex)->get_bspline_lower_bound();
                        real tBSplineUpperBound = tFields(tFieldIndex)->get_bspline_upper_bound();

                        // Get number of coefficients
                        sint tBSplineMeshIndex = tFields(tFieldIndex)->get_discretization_mesh_index();
                        uint tNumCoefficients = aMesh->get_num_coeffs(tBSplineMeshIndex);

                        // Resize shared ADV IDs
                        tSharedADVIds(tFieldIndex).resize(tNumCoefficients, 1);

                        // Loop over B-spline coefficients
                        for (uint tBSplineIndex = 0; tBSplineIndex < tNumCoefficients; tBSplineIndex++)
                        {
                            // Get new ADV ID and set as shared ID
                            sint tNewADVId = tIdOffset + aMesh->get_glb_entity_id_from_entity_loc_index(
                                    tBSplineIndex,
                                    EntityRank::BSPLINE,
                                    tBSplineMeshIndex);
                            tSharedADVIds(tFieldIndex)(tBSplineIndex) = tNewADVId;

                            // If this processor owns this coefficient set to owned list and set bounds
                            if ((uint) par_rank() == aMesh->get_entity_owner(tBSplineIndex, EntityRank::BSPLINE, tBSplineMeshIndex))
                            {
                                // Bounds
                                mLowerBounds(tOwnedADVIndex) = tBSplineLowerBound;
                                mUpperBounds(tOwnedADVIndex) = tBSplineUpperBound;

                                // New ADV ID
                                tOwnedADVIds(tOwnedADVIndex++) = tNewADVId;
                            }
                        }
                    }
                }

                // Set owned ADV IDs
                mPdvHostManager.set_owned_adv_ids(tOwnedADVIds);

                //----------------------------------------//
                // Create owned ADV vector                //
                //----------------------------------------//

                // Create factory for distributed ADV vector
                sol::Matrix_Vector_Factory tDistributedFactory;

                // Create map for distributed vectors
                sol::Dist_Map* tOwnedADVMap = tDistributedFactory.create_map(tOwnedADVIds);
                sol::Dist_Map* tPrimitiveADVMap = tDistributedFactory.create_map(tPrimitiveADVIds);

                // Create vectors
                mOwnedADVs = tDistributedFactory.create_vector(tOwnedADVMap, 1, false, true);
                mPrimitiveADVs = tDistributedFactory.create_vector(tPrimitiveADVMap, 1, false, true);

                // Assign primitive ADVs
                if (par_rank() == 0)
                {
                    mOwnedADVs->replace_global_values(tPrimitiveADVIds, mADVs);
                }

                // Global assembly
                mOwnedADVs->vector_global_assembly();

                // Get primitive ADVs from owned vector
                mPrimitiveADVs->import_local_to_global(*mOwnedADVs);

                // Set field ADVs using distributed vector
                if (mADVs.length() > 0)
                {
                    for (uint tFieldIndex = 0; tFieldIndex < tFields.size(); tFieldIndex++)
                    {
                        tFields(tFieldIndex)->set_advs(mPrimitiveADVs);
                    }
                }

                //----------------------------------------//
                // Convert to B-spline fields             //
                //----------------------------------------//

                // Loop to find B-spline geometries
                for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
                {
                    // Shape sensitivities logic
                    mShapeSensitivities = (mShapeSensitivities or mGeometries(tGeometryIndex)->depends_on_advs());

                    // Convert to B-spline field
                    if (mGeometries(tGeometryIndex)->conversion_to_bsplines())
                    {
                        // Always have shape sensitivities if B-spline field
                        mShapeSensitivities = true;

                        // Create B-spline geometry FIXME Multiple B-spline fields
                        mGeometries(tGeometryIndex) = std::make_shared<BSpline_Geometry>(
                                mOwnedADVs,
                                tOwnedADVIds,
                                tSharedADVIds(tGeometryIndex),
                                tPrimitiveADVIds.length(),
                                aMesh,
                                mGeometries(tGeometryIndex));
                    }

                    // Store field values if needed
                    else if (mGeometries(tGeometryIndex)->store_field_values())
                    {
                        // Create stored geometry
                        mGeometries(tGeometryIndex) = std::make_shared<Stored_Geometry>(
                                aMesh,
                                mGeometries(tGeometryIndex));
                    }
                }

                // Loop to find B-spline properties
                for (uint tPropertyIndex = 0; tPropertyIndex < mProperties.size(); tPropertyIndex++)
                {
                    // Convert to B-spline field
                    if (mProperties(tPropertyIndex)->conversion_to_bsplines())
                    {
                        // Create B-spline property FIXME Multiple B-spline fields
                        mProperties(tPropertyIndex) = std::make_shared<BSpline_Property>(
                                mOwnedADVs,
                                tOwnedADVIds,
                                tSharedADVIds(mGeometries.size() + tPropertyIndex),
                                tPrimitiveADVIds.length(),
                                aMesh,
                                mProperties(tPropertyIndex));
                    }
                }

                //----------------------------------------//
                // Communicate all ADV IDs to processor 0 //
                //----------------------------------------//

                // Sending mats
                Cell<Matrix<DDSMat>> tSendingIDs(0);
                Cell<Matrix<DDRMat>> tSendingLowerBounds(0);
                Cell<Matrix<DDRMat>> tSendingUpperBounds(0);

                // Receiving mats
                Cell<Matrix<DDSMat>> tReceivingIDs(0);
                Cell<Matrix<DDRMat>> tReceivingLowerBounds(0);
                Cell<Matrix<DDRMat>> tReceivingUpperBounds(0);

                // Set up communication list for communicating ADV IDs
                Matrix<IdMat> tCommunicationList(1, 1, 0);
                if (par_rank() == 0)
                {
                    // Resize communication list and sending mats
                    tCommunicationList.resize(par_size() - 1, 1);
                    tSendingIDs.resize(par_size() - 1);
                    tSendingLowerBounds.resize(par_size() - 1);
                    tSendingUpperBounds.resize(par_size() - 1);

                    // Assign communication list
                    for (uint tProcessorIndex = 1; tProcessorIndex < (uint)par_size(); tProcessorIndex++)
                    {
                        tCommunicationList(tProcessorIndex - 1) = tProcessorIndex;
                    }
                }
                else
                {
                    tSendingIDs = {tOwnedADVIds};
                    tSendingLowerBounds = {mLowerBounds};
                    tSendingUpperBounds = {mUpperBounds};
                }

                // Communicate mats
                communicate_mats(tCommunicationList, tSendingIDs, tReceivingIDs);
                communicate_mats(tCommunicationList, tSendingLowerBounds, tReceivingLowerBounds);
                communicate_mats(tCommunicationList, tSendingUpperBounds, tReceivingUpperBounds);

                // Assemble full ADVs/bounds
                if (par_rank() == 0)
                {
                    // Start full IDs with owned IDs on processor 0
                    mFullADVIds = tOwnedADVIds;

                    // Assemble additional IDs/bounds from other processors
                    for (uint tProcessorIndex = 1; tProcessorIndex < (uint)par_size(); tProcessorIndex++)
                    {
                        // Get number of received ADVs
                        uint tFullADVsFilled = mFullADVIds.length();
                        uint tNumReceivedADVs = tReceivingIDs(tProcessorIndex - 1).length();

                        // Resize full ADV IDs and bounds
                        mFullADVIds.resize(tFullADVsFilled + tNumReceivedADVs, 1);
                        mLowerBounds.resize(tFullADVsFilled + tNumReceivedADVs, 1);
                        mUpperBounds.resize(tFullADVsFilled + tNumReceivedADVs, 1);

                        // Assign received ADV IDs
                        for (uint tADVIndex = 0; tADVIndex < tNumReceivedADVs; tADVIndex++)
                        {
                            mFullADVIds(tFullADVsFilled + tADVIndex) = tReceivingIDs(tProcessorIndex - 1)(tADVIndex);
                            mLowerBounds(tFullADVsFilled + tADVIndex) =
                                    tReceivingLowerBounds(tProcessorIndex - 1)(tADVIndex);
                            mUpperBounds(tFullADVsFilled + tADVIndex) =
                                    tReceivingUpperBounds(tProcessorIndex - 1)(tADVIndex);
                        }
                    }
                }
                else
                {
                    mLowerBounds.set_size(0, 0);
                    mUpperBounds.set_size(0, 0);
                }
            }
        }

        void Geometry_Engine::distribute_advs(mtk::Interpolation_Mesh* aMesh)
        {
            // Tracer
            Tracer tTracer("GEN", "N/A", "Distribute ADVs");

            // Register spatial dimension
            mNumSpatialDimensions = aMesh->get_spatial_dim();

            // Set number of background nodes
            mPdvHostManager.set_num_background_nodes(aMesh->get_num_nodes());

            // allocate proximity data
            this->setup_initial_geometric_proximities(aMesh);

            // Gather all fields
            Cell<std::shared_ptr<Field>> tFields(mGeometries.size() + mProperties.size());
            std::copy(mGeometries.begin(), mGeometries.end(), tFields.begin());
            std::copy(mProperties.begin(), mProperties.end(), tFields.begin() + mGeometries.size());

            // Build distributed ADVs
            if (mOwnedADVs == nullptr)
            {
                //------------------------------------//
                // Determine owned and shared ADV IDs //
                //------------------------------------//

                // Loop over all geometries to get number of new ADVs
                uint tNumNewOwnedADVs = 0;
                for (uint tFieldIndex = 0; tFieldIndex < tFields.size(); tFieldIndex++)
                {
                    // Determine if level set will be created
                    if (tFields(tFieldIndex)->conversion_to_bsplines())
                    {
                        // Get number of coefficients
                        sint tBSplineMeshIndex = tFields(tFieldIndex)->get_discretization_mesh_index();
                        uint tNumCoefficients = aMesh->get_num_coeffs(tBSplineMeshIndex);

                        // Loop over B-spline coefficients
                        for (uint tBSplineIndex = 0; tBSplineIndex < tNumCoefficients; tBSplineIndex++)
                        {
                            // If this processor owns this coefficient
                            if ((uint) par_rank() == aMesh->get_entity_owner(tBSplineIndex, EntityRank::BSPLINE, tBSplineMeshIndex))
                            {
                                // New ADV needs to be created on this processor
                                tNumNewOwnedADVs++;
                            }
                        }
                    }
                }

                // Set number of total owned ADVs
                uint tNumOwnedADVs = tNumNewOwnedADVs;
                if (par_rank() == 0)
                {
                    tNumOwnedADVs += mADVs.length();
                }

                // Set primitive IDs
                Matrix<DDSMat> tPrimitiveADVIds(mADVs.length(), 1);
                for (uint tADVIndex = 0; tADVIndex < mADVs.length(); tADVIndex++)
                {
                    tPrimitiveADVIds(tADVIndex) = tADVIndex;
                }

                // Start with primitive IDs for owned IDs on processor 0
                Matrix<DDSMat> tOwnedADVIds(0, 0);
                if (par_rank() == 0)
                {
                    tOwnedADVIds = tPrimitiveADVIds;
                }

                // Check for proper dimensions
                MORIS_ASSERT( mLowerBounds.numel() > 0 ? mLowerBounds.n_cols() == 1 : true,
                        "ADV lower bound vector needs to be a column vector.\n");

                MORIS_ASSERT( mUpperBounds.numel() > 0 ? mUpperBounds.n_cols() == 1 : true,
                        "ADV upper bound vector needs to be a column vector.\n");

                // Resize owned IDs and bounds
                tOwnedADVIds.resize(tNumOwnedADVs, 1);
                mLowerBounds.resize(tNumOwnedADVs, 1);
                mUpperBounds.resize(tNumOwnedADVs, 1);

                // Cell of shared ADV IDs
                Cell<Matrix<DDSMat>> tSharedADVIds(mGeometries.size() + mProperties.size());

                // Loop over all geometry parameter lists to set B-spline ADV bounds and IDs
                uint tOwnedADVIndex = mADVs.length();
                uint tIdOffset = tOwnedADVIndex; // FIXME this needs to be updated for multiple B-spline fields
                for (uint tFieldIndex = 0; tFieldIndex < tFields.size(); tFieldIndex++)
                {
                    // Determine if level set will be created
                    if (tFields(tFieldIndex)->conversion_to_bsplines())
                    {
                        // Get bounds
                        real tBSplineLowerBound = tFields(tFieldIndex)->get_bspline_lower_bound();
                        real tBSplineUpperBound = tFields(tFieldIndex)->get_bspline_upper_bound();

                        // Get number of coefficients
                        sint tBSplineMeshIndex = tFields(tFieldIndex)->get_discretization_mesh_index();
                        uint tNumCoefficients = aMesh->get_num_coeffs(tBSplineMeshIndex);

                        // Resize shared ADV IDs
                        tSharedADVIds(tFieldIndex).resize(tNumCoefficients, 1);

                        // Loop over B-spline coefficients
                        for (uint tBSplineIndex = 0; tBSplineIndex < tNumCoefficients; tBSplineIndex++)
                        {
                            // Get new ADV ID and set as shared ID
                            sint tNewADVId = tIdOffset + aMesh->get_glb_entity_id_from_entity_loc_index(
                                    tBSplineIndex,
                                    EntityRank::BSPLINE,
                                    tBSplineMeshIndex);
                            tSharedADVIds(tFieldIndex)(tBSplineIndex) = tNewADVId;

                            // If this processor owns this coefficient set to owned list and set bounds
                            if ((uint) par_rank() == aMesh->get_entity_owner(tBSplineIndex, EntityRank::BSPLINE, tBSplineMeshIndex))
                            {
                                // Bounds
                                mLowerBounds(tOwnedADVIndex) = tBSplineLowerBound;
                                mUpperBounds(tOwnedADVIndex) = tBSplineUpperBound;

                                // New ADV ID
                                tOwnedADVIds(tOwnedADVIndex++) = tNewADVId;
                            }
                        }
                    }
                }

                // Set owned ADV IDs
                mPdvHostManager.set_owned_adv_ids(tOwnedADVIds);

                //----------------------------------------//
                // Create owned ADV vector                //
                //----------------------------------------//

                // Create factory for distributed ADV vector
                sol::Matrix_Vector_Factory tDistributedFactory;

                // Create map for distributed vectors
                sol::Dist_Map* tOwnedADVMap = tDistributedFactory.create_map(tOwnedADVIds);
                sol::Dist_Map* tPrimitiveADVMap = tDistributedFactory.create_map(tPrimitiveADVIds);

                // Create vectors
                mOwnedADVs = tDistributedFactory.create_vector(tOwnedADVMap, 1, false, true);
                mPrimitiveADVs = tDistributedFactory.create_vector(tPrimitiveADVMap, 1, false, true);

                // Assign primitive ADVs
                if (par_rank() == 0)
                {
                    mOwnedADVs->replace_global_values(tPrimitiveADVIds, mADVs);
                }

                // Global assembly
                mOwnedADVs->vector_global_assembly();

                // Get primitive ADVs from owned vector
                mPrimitiveADVs->import_local_to_global(*mOwnedADVs);

                // Set field ADVs using distributed vector
                if (mADVs.length() > 0)
                {
                    for (uint tFieldIndex = 0; tFieldIndex < tFields.size(); tFieldIndex++)
                    {
                        tFields(tFieldIndex)->set_advs(mPrimitiveADVs);
                    }
                }

                //----------------------------------------//
                // Convert to B-spline fields             //
                //----------------------------------------//

                // Loop to find B-spline geometries
                for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
                {
                    // Shape sensitivities logic
                    mShapeSensitivities = (mShapeSensitivities or mGeometries(tGeometryIndex)->depends_on_advs());

                    // Convert to B-spline field
                    if (mGeometries(tGeometryIndex)->conversion_to_bsplines())
                    {
                        // Always have shape sensitivities if B-spline field
                        mShapeSensitivities = true;

                        // Create B-spline geometry FIXME Multiple B-spline fields
                        mGeometries(tGeometryIndex) = std::make_shared<BSpline_Geometry>(
                                mOwnedADVs,
                                tOwnedADVIds,
                                tSharedADVIds(tGeometryIndex),
                                tPrimitiveADVIds.length(),
                                aMesh,
                                mGeometries(tGeometryIndex));
                    }

                    // Store field values if needed
                    else if (mGeometries(tGeometryIndex)->store_field_values())
                    {
                        // Create stored geometry
                        mGeometries(tGeometryIndex) = std::make_shared<Stored_Geometry>(
                                aMesh,
                                mGeometries(tGeometryIndex));
                    }
                }

                // Loop to find B-spline properties
                for (uint tPropertyIndex = 0; tPropertyIndex < mProperties.size(); tPropertyIndex++)
                {
                    // Convert to B-spline field
                    if (mProperties(tPropertyIndex)->conversion_to_bsplines())
                    {
                        // Create B-spline property FIXME Multiple B-spline fields
                        mProperties(tPropertyIndex) = std::make_shared<BSpline_Property>(
                                mOwnedADVs,
                                tOwnedADVIds,
                                tSharedADVIds(mGeometries.size() + tPropertyIndex),
                                tPrimitiveADVIds.length(),
                                aMesh,
                                mProperties(tPropertyIndex));
                    }
                }

                //----------------------------------------//
                // Communicate all ADV IDs to processor 0 //
                //----------------------------------------//

                // Sending mats
                Cell<Matrix<DDSMat>> tSendingIDs(0);
                Cell<Matrix<DDRMat>> tSendingLowerBounds(0);
                Cell<Matrix<DDRMat>> tSendingUpperBounds(0);

                // Receiving mats
                Cell<Matrix<DDSMat>> tReceivingIDs(0);
                Cell<Matrix<DDRMat>> tReceivingLowerBounds(0);
                Cell<Matrix<DDRMat>> tReceivingUpperBounds(0);

                // Set up communication list for communicating ADV IDs
                Matrix<IdMat> tCommunicationList(1, 1, 0);
                if (par_rank() == 0)
                {
                    // Resize communication list and sending mats
                    tCommunicationList.resize(par_size() - 1, 1);
                    tSendingIDs.resize(par_size() - 1);
                    tSendingLowerBounds.resize(par_size() - 1);
                    tSendingUpperBounds.resize(par_size() - 1);

                    // Assign communication list
                    for (uint tProcessorIndex = 1; tProcessorIndex < (uint)par_size(); tProcessorIndex++)
                    {
                        tCommunicationList(tProcessorIndex - 1) = tProcessorIndex;
                    }
                }
                else
                {
                    tSendingIDs = {tOwnedADVIds};
                    tSendingLowerBounds = {mLowerBounds};
                    tSendingUpperBounds = {mUpperBounds};
                }

                // Communicate mats
                communicate_mats(tCommunicationList, tSendingIDs, tReceivingIDs);
                communicate_mats(tCommunicationList, tSendingLowerBounds, tReceivingLowerBounds);
                communicate_mats(tCommunicationList, tSendingUpperBounds, tReceivingUpperBounds);

                // Assemble full ADVs/bounds
                if (par_rank() == 0)
                {
                    // Start full IDs with owned IDs on processor 0
                    mFullADVIds = tOwnedADVIds;

                    // Assemble additional IDs/bounds from other processors
                    for (uint tProcessorIndex = 1; tProcessorIndex < (uint)par_size(); tProcessorIndex++)
                    {
                        // Get number of received ADVs
                        uint tFullADVsFilled = mFullADVIds.length();
                        uint tNumReceivedADVs = tReceivingIDs(tProcessorIndex - 1).length();

                        // Resize full ADV IDs and bounds
                        mFullADVIds.resize(tFullADVsFilled + tNumReceivedADVs, 1);
                        mLowerBounds.resize(tFullADVsFilled + tNumReceivedADVs, 1);
                        mUpperBounds.resize(tFullADVsFilled + tNumReceivedADVs, 1);

                        // Assign received ADV IDs
                        for (uint tADVIndex = 0; tADVIndex < tNumReceivedADVs; tADVIndex++)
                        {
                            mFullADVIds(tFullADVsFilled + tADVIndex) = tReceivingIDs(tProcessorIndex - 1)(tADVIndex);
                            mLowerBounds(tFullADVsFilled + tADVIndex) =
                                    tReceivingLowerBounds(tProcessorIndex - 1)(tADVIndex);
                            mUpperBounds(tFullADVsFilled + tADVIndex) =
                                    tReceivingUpperBounds(tProcessorIndex - 1)(tADVIndex);
                        }
                    }
                }
                else
                {
                    mLowerBounds.set_size(0, 0);
                    mUpperBounds.set_size(0, 0);
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::output_fields(mtk::Mesh* aMesh)
        {
            // Tracer
            Tracer tTracer("GEN", "N/A", "Output fields");

            this->output_fields_on_mesh(aMesh, mOutputMeshFile);
            this->write_geometry_fields(aMesh, mGeometryFieldFile);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::output_fields_on_mesh(
                mtk::Mesh* aMesh,
                std::string aExodusFileName)
        {
            // time shift
            real tTimeShift = 0.0;

            if (aExodusFileName != "")
            {
                if ( mTimeOffset > 0)
                {
                    // get optimization iteration
                    uint tOptIter = gLogger.get_opt_iteration();

                    // set name
                    std::string tOptIterStrg = std::to_string(tOptIter);
                    aExodusFileName += ".e-s." + std::string(4-tOptIterStrg.length(),'0') + tOptIterStrg;

                    // determine time shift
                    tTimeShift = tOptIter * mTimeOffset;
                }

                // Write mesh
                mtk::Writer_Exodus tWriter(aMesh);
                tWriter.write_mesh("./", aExodusFileName, "./", "gen_temp.exo");

                // Setup field names
                uint tNumGeometries = mGeometries.size();
                uint tNumProperties = mProperties.size();
                Cell<std::string> tFieldNames(tNumGeometries + tNumProperties);

                // Geometry field names
                for (uint tGeometryIndex = 0; tGeometryIndex < tNumGeometries; tGeometryIndex++)
                {
                    tFieldNames(tGeometryIndex) = mGeometries(tGeometryIndex)->get_label();
                    if (tFieldNames(tGeometryIndex) == "")
                    {
                        tFieldNames(tGeometryIndex) = "Geometry " + std::to_string(tGeometryIndex);
                    }
                }

                // Property field names
                for (uint tPropertyIndex = 0; tPropertyIndex < tNumProperties; tPropertyIndex++)
                {
                    tFieldNames(tNumGeometries + tPropertyIndex) = mProperties(tPropertyIndex)->get_label();
                    if (tFieldNames(tNumGeometries + tPropertyIndex) == "")
                    {
                        tFieldNames(tNumGeometries + tPropertyIndex) = "Property " + std::to_string(tPropertyIndex);
                    }
                }

                // write time to file
                tWriter.set_time( tTimeShift );

                // Set nodal fields based on field names
                tWriter.set_nodal_fields(tFieldNames);

                // Get all node coordinates
                Cell<Matrix<DDRMat>> tNodeCoordinates(aMesh->get_num_nodes());
                for (uint tNodeIndex = 0; tNodeIndex < aMesh->get_num_nodes(); tNodeIndex++)
                {
                    tNodeCoordinates(tNodeIndex) = aMesh->get_node_coordinate(tNodeIndex);
                }

                // Loop over geometries
                for (uint tGeometryIndex = 0; tGeometryIndex < tNumGeometries; tGeometryIndex++)
                {
                    // Create field vector
                    Matrix<DDRMat> tFieldData(aMesh->get_num_nodes(), 1);

                    // Assign field to vector
                    for (uint tNodeIndex = 0; tNodeIndex < aMesh->get_num_nodes(); tNodeIndex++)
                    {
                        tFieldData(tNodeIndex) = mGeometries(tGeometryIndex)->get_field_value(
                                tNodeIndex,
                                tNodeCoordinates(tNodeIndex));
                    }

                    // Create field on mesh
                    tWriter.write_nodal_field(tFieldNames(tGeometryIndex), tFieldData);
                }

                // Loop over properties
                for (uint tPropertyIndex = 0; tPropertyIndex < tNumProperties; tPropertyIndex++)
                {
                    // Create field vector
                    Matrix<DDRMat> tFieldData(aMesh->get_num_nodes(), 1);

                    // Assign field to vector
                    for (uint tNodeIndex = 0; tNodeIndex < aMesh->get_num_nodes(); tNodeIndex++)
                    {
                        tFieldData(tNodeIndex) = mProperties(tPropertyIndex)->get_field_value(
                                tNodeIndex,
                                tNodeCoordinates(tNodeIndex));
                    }

                    // Create field on mesh
                    tWriter.write_nodal_field(tFieldNames(tNumGeometries + tPropertyIndex), tFieldData);
                }

                // Finalize
                tWriter.close_file(true);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::write_geometry_fields(
                mtk::Mesh* aMesh,
                std::string aBaseFileName)
        {
            if (aBaseFileName != "")
            {
                // Get all node coordinates
                Cell<Matrix<DDRMat>> tNodeCoordinates(aMesh->get_num_nodes());
                for (uint tNodeIndex = 0; tNodeIndex < aMesh->get_num_nodes(); tNodeIndex++)
                {
                    tNodeCoordinates(tNodeIndex) = aMesh->get_node_coordinate(tNodeIndex);
                }

                // Loop over geometries
                for (uint tGeometryIndex = 0; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
                {
                    // Create file
                    std::ofstream tOutFile(aBaseFileName + "_" + std::to_string(tGeometryIndex) + ".txt");

                    // Write to file
                    for (uint tNodeIndex = 0; tNodeIndex < aMesh->get_num_nodes(); tNodeIndex++)
                    {
                        // Coordinates
                        for (uint tDimension = 0; tDimension < mNumSpatialDimensions; tDimension++)
                        {
                            tOutFile << tNodeCoordinates(tNodeIndex)(tDimension) << ", ";
                        }

                        // Fill unused dimensions with zeros
                        for (uint tDimension = mNumSpatialDimensions; tDimension < 3; tDimension++)
                        {
                            tOutFile << 0.0 << ", ";
                        }

                        // Level-set field
                        tOutFile << mGeometries(tGeometryIndex)->get_field_value(
                                tNodeIndex,
                                tNodeCoordinates(tNodeIndex)) << std::endl;
                    }

                    // Close file
                    tOutFile.close();
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------
        // PRIVATE
        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::create_interpolation_pdv_hosts(
                mtk::Interpolation_Mesh     * aInterpolationMesh,
                mtk::Integration_Mesh       * aIntegrationMesh,
                Cell<Cell<Cell<PDV_Type>>>    aPdvTypes)
        {
            // Get information from integration mesh
            //uint tNumSets = tInterpolationMesh->get_num_sets(); FIXME
            uint tNumSets  = aPdvTypes.size();
            uint tNumNodes = aInterpolationMesh->get_num_nodes();

            Cell<Matrix<DDSMat>> tNodeIndicesPerSet(tNumSets);
            Cell<Matrix<DDSMat>> tNodeIdsPerSet(tNumSets);
            Cell<Matrix<DDSMat>> tNodeOwnersPerSet(tNumSets);
            Cell<Matrix<DDRMat>> tNodeCoordinates(tNumNodes);

            // Determine if we need to do the below loop
            bool tDoJankLoop = false;
            for (uint tMeshSetIndex = 0; tMeshSetIndex < tNumSets; tMeshSetIndex++)
            {
                if (aPdvTypes(tMeshSetIndex).size() > 0)
                {
                    tDoJankLoop = true;
                }
            }

            if (tDoJankLoop)
            {
                // Loop through sets
                for (uint tMeshSetIndex = 0; tMeshSetIndex < tNumSets; tMeshSetIndex++)
                {
                    uint tCurrentNode = 0;
                    mtk::Set* tSet = aIntegrationMesh->get_set_by_index(tMeshSetIndex);

                    // Clusters per set
                    for (uint tClusterIndex = 0; tClusterIndex < tSet->get_num_clusters_on_set(); tClusterIndex++)
                    {
                        const mtk::Cluster* tCluster = tSet->get_clusters_by_index(tClusterIndex);

                        // Indices on cluster // FIXME this is really bad and slow. especially when building the pdvs
                        Matrix<IndexMat> tNodeIndicesInCluster = tCluster->get_interpolation_cell().get_base_cell()->get_vertex_inds();
                        Matrix<IndexMat> tNodeIdsInCluster     = tCluster->get_interpolation_cell().get_base_cell()->get_vertex_ids();
                        Matrix<IndexMat> tNodeOwnersInCluster  = tCluster->get_interpolation_cell().get_base_cell()->get_vertex_owners();

                        // FIXME don't undersand this resize. it's really slow
                        tNodeIndicesPerSet(tMeshSetIndex).resize(tNodeIndicesPerSet(tMeshSetIndex).length() + tNodeIndicesInCluster.length(), 1);
                        tNodeIdsPerSet(tMeshSetIndex).resize(tNodeIdsPerSet(tMeshSetIndex).length() + tNodeIdsInCluster.length(), 1);
                        tNodeOwnersPerSet(tMeshSetIndex).resize(tNodeOwnersPerSet(tMeshSetIndex).length() + tNodeOwnersInCluster.length(), 1);

                        // FIXME we have nodes up to 8 tims in this list in 3d
                        for (uint tNodeInCluster = 0; tNodeInCluster < tNodeIndicesInCluster.length(); tNodeInCluster++)
                        {
                            tNodeIndicesPerSet(tMeshSetIndex)(tCurrentNode)   = tNodeIndicesInCluster(tNodeInCluster);
                            tNodeIdsPerSet(tMeshSetIndex)(tCurrentNode)   = tNodeIdsInCluster(tNodeInCluster);
                            tNodeOwnersPerSet(tMeshSetIndex)(tCurrentNode++) = tNodeOwnersInCluster(tNodeInCluster);
                        }
                    }
                }
            }

            // Get node coordinates
            for (uint tNodeIndex = 0; tNodeIndex < tNumNodes; tNodeIndex++)
            {
                tNodeCoordinates(tNodeIndex) = aInterpolationMesh->get_node_coordinate(tNodeIndex);
            }

            // Create hosts
            mPdvHostManager.create_interpolation_pdv_hosts(
                    tNodeIndicesPerSet,
                    tNodeIdsPerSet,
                    tNodeOwnersPerSet,
                    tNodeCoordinates,
                    aPdvTypes);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::set_integration_pdv_types(mtk::Integration_Mesh* aIntegrationMesh)
        {
            // Get information from integration mesh
            uint tNumSets = aIntegrationMesh->get_num_sets();

            // Cell of IG PDV_Type types
            Cell<PDV_Type> tCoordinatePdvs(mNumSpatialDimensions);

            switch(mNumSpatialDimensions)
            {
                case 2:
                {
                    tCoordinatePdvs(0) = PDV_Type::X_COORDINATE;
                    tCoordinatePdvs(1) = PDV_Type::Y_COORDINATE;
                    break;
                }
                case 3:
                {
                    tCoordinatePdvs(0) = PDV_Type::X_COORDINATE;
                    tCoordinatePdvs(1) = PDV_Type::Y_COORDINATE;
                    tCoordinatePdvs(2) = PDV_Type::Z_COORDINATE;
                    break;
                }
                default:
                {
                    MORIS_ERROR( false, "Geometry Engine only works for 2D and 3D models." );
                }
            }

            // Loop through sets
            Cell<Cell<Cell<PDV_Type>>> tPdvTypes(tNumSets);
            for (uint tMeshSetIndex = 0; tMeshSetIndex < tNumSets; tMeshSetIndex++)
            {
                // PDV_Type types per set
                tPdvTypes(tMeshSetIndex).resize(1);
                tPdvTypes(tMeshSetIndex)(0) = tCoordinatePdvs;
            }

            // Set PDV types
            mPdvHostManager.set_integration_pdv_types(tPdvTypes);
            mPdvHostManager.set_requested_integration_pdv_types(tCoordinatePdvs);

        }

        //--------------------------------------------------------------------------------------------------------------

        void Geometry_Engine::assign_property_to_pdv_hosts(
                std::shared_ptr<Property> aPropertyPointer,
                PDV_Type                  aPdvType,
                mtk::Integration_Mesh*    aIntegrationMesh,
                Matrix<DDUMat>            aSetIndices)
        {
            for (uint tSet = 0; tSet < aSetIndices.length(); tSet++)
            {
                // get the mesh set from index
                mtk::Set* tSetPointer = aIntegrationMesh->get_set_by_index( aSetIndices(tSet) );

                // get the list of cluster on mesh set
                Cell< mtk::Cluster const * > tClusterPointers = tSetPointer->get_clusters_on_set();

                // get number of clusters on mesh set
                uint tNumClusters = tClusterPointers.size();

                // loop over the clusters on mesh set
                for(uint iClust=0; iClust<tNumClusters; iClust++)
                {
                    // get the IP cell from cluster
                    mtk::Cell const & tIPCell = tClusterPointers(iClust)->get_interpolation_cell();

                    // get the vertices from IP cell
                    Cell< mtk::Vertex * > tVertices = tIPCell.get_base_cell()->get_vertex_pointers();

                    // get the number of vertices on IP cell
                    uint tNumVerts = tVertices.size();

                    // loop over vertices on IP cell
                    for(uint iVert = 0; iVert < tNumVerts; iVert++)
                    {
                        // get the vertex index
                        moris_index tVertIndex = tVertices(iVert)->get_index();

                        // ask pdv host manager to assign to vertex a pdv type and a property
                        mPdvHostManager.create_interpolation_pdv( uint(tVertIndex), aPdvType, aPropertyPointer );
                    }
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void
        Geometry_Engine::setup_initial_geometric_proximities(mtk::Interpolation_Mesh* aMesh)
        {
            mVertexGeometricProximity = Cell<Geometric_Proximity>(aMesh->get_num_nodes(),Geometric_Proximity(mGeometries.size()));

            // iterate through vertices then geometries
            for(uint iV = 0; iV < aMesh->get_num_nodes(); iV++)
            {
                Matrix<DDRMat> tCoords = aMesh->get_node_coordinate(moris_index(iV));

                 mVertexGeometricProximity(iV).mAssociatedVertexIndex = (moris_index) iV;

                for (uint iGeometryIndex = 0; iGeometryIndex < mGeometries.size(); iGeometryIndex++)
                {            
                    real tVertGeomVal = mGeometries(iGeometryIndex)->get_field_value(iV, tCoords);

                    moris_index tGeomProxIndex = this->get_geometric_proximity_index(tVertGeomVal);

                    mVertexGeometricProximity(iV).set_geometric_proximity(tGeomProxIndex,iGeometryIndex);

                }
            }

        }

        //--------------------------------------------------------------------------------------------------------------

        moris_index
        Geometry_Engine::get_geometric_proximity_index(real const & aGeometricVal)
        {
            moris_index tGeometricProxIndex = MORIS_INDEX_MAX;
            if(aGeometricVal < mIsocontourThreshold)
            {
                tGeometricProxIndex = 0;
            }
            else if(aGeometricVal == mIsocontourThreshold)
            {
                tGeometricProxIndex = 1;
            }
            else if(aGeometricVal > mIsocontourThreshold)
            {
                tGeometricProxIndex = 2;
            }
            
            return tGeometricProxIndex;
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_index
        Geometry_Engine::get_queued_intersection_geometric_proximity_index(moris_index const & aGeomIndex)
        {
            // parent vertex
            moris_index tParentVertexIndex0 = mQueuedIntersectionNode->mAncestorNodeIndices(0);
            moris_index tParentVertexIndex1 = mQueuedIntersectionNode->mAncestorNodeIndices(1);

            // parent vertex proximity wrt aGeomIndex
            moris_index tParentProx0 = mVertexGeometricProximity(tParentVertexIndex0).get_geometric_proximity(aGeomIndex);
            moris_index tParentProx1 = mVertexGeometricProximity(tParentVertexIndex1).get_geometric_proximity(aGeomIndex);

            // 0 - G(x) < threshold
            // 1 - G(x) == threshold
            // 2 - G(x) > threshold
            // verify we dont transition across interface
            // MORIS_ERROR((tParentProx0 == 0 && tParentProx1 == 0) || 
            //             (tParentProx0 == 0 && tParentProx1 == 1) || 
            //             (tParentProx0 == 1 && tParentProx1 == 0) || 
            //             (tParentProx0 == 1 && tParentProx1 == 1) || 
            //             (tParentProx0 == 2 && tParentProx1 == 2) || 
            //             (tParentProx0 == 2 && tParentProx1 == 1) || 
            //             (tParentProx0 == 1 && tParentProx1 == 2),   "Invalid proximity data");
            // 
            // add them together
            moris_index tSum = tParentProx0 + tParentProx1;

            // proximity value
            moris_index tProxIndex = MORIS_INDEX_MAX;
            if(tSum == 0)
            {
                tProxIndex = 0;
            }
            else if(tSum == 1)
            {
                tProxIndex = 0;
            }
            else if(tSum == 2)
            {
                tProxIndex = 1;
            }
            else if(tSum == 3)
            {
                tProxIndex = 2;
            }
            else if(tSum == 4)
            {
                tProxIndex = 2;
            }
            else
            {
                MORIS_ASSERT(0,"Proximity determination failed.");
            }

             return tProxIndex;
        }

        //--------------------------------------------------------------------------------------------------------------
        
        void
        Geometry_Engine::admit_queued_intersection_geometric_proximity(uint aNodeIndex)
        {

            MORIS_ERROR(aNodeIndex == mVertexGeometricProximity.size(),"Index mismatch");
            // initialize proximity data
            mVertexGeometricProximity.push_back(Geometric_Proximity(mGeometries.size()));

            // node index associated with this proximity
            mVertexGeometricProximity(aNodeIndex).mAssociatedVertexIndex = aNodeIndex;

            // geometry iteration through previous geometries
            for (uint tGeometryIndex = 0; tGeometryIndex < this->get_active_geometry_index(); tGeometryIndex++)
            {
                moris_index tProxIndex = this->get_queued_intersection_geometric_proximity_index(tGeometryIndex);
                mVertexGeometricProximity(aNodeIndex).set_geometric_proximity(tProxIndex,tGeometryIndex);
            }

            // place the current one on the interface
            mVertexGeometricProximity(aNodeIndex).set_geometric_proximity(1,this->get_active_geometry_index());

            if(this->get_active_geometry_index() != mGeometries.size() - 1)
            {
                // iterate through following geometries (here we just compute the vertex value to determine proximity)
                for (uint tGeometryIndex = this->get_active_geometry_index() + 1; tGeometryIndex < mGeometries.size(); tGeometryIndex++)
                {
                    real tVertGeomVal = mGeometries(tGeometryIndex)->get_field_value(aNodeIndex, mQueuedIntersectionNode->get_global_coordinates());

                    moris_index tGeomProxIndex = this->get_geometric_proximity_index(tVertGeomVal);

                    mVertexGeometricProximity(aNodeIndex).set_geometric_proximity(tGeomProxIndex,tGeometryIndex);
                }
            }

        }

        //--------------------------------------------------------------------------------------------------------------

        Phase_Table Geometry_Engine::create_phase_table(
                Cell<Cell<ParameterList>> aParameterLists,
                std::shared_ptr<Library_IO> aLibrary)
        {
            // Get number of geometries
            uint tNumGeometries = aParameterLists(1).size();

            // Recreate phase table via different methods if needed
            std::string tPhaseFunctionName = aParameterLists(0)(0).get<std::string>("phase_function_name");
            if (tPhaseFunctionName != "")
            {
                // User-defined phase function
                return Phase_Table(
                        aLibrary->load_function<PHASE_FUNCTION>(tPhaseFunctionName),
                        aParameterLists(0)(0).get<sint>("number_of_phases"));
            }
            else if (aParameterLists(0)(0).get<std::string>("phase_table") != "")
            {
                // User-defined bulk phases
                return Phase_Table(tNumGeometries, string_to_mat<DDUMat>(aParameterLists(0)(0).get<std::string>("phase_table")));
            }
            else
            {
                // Unique phase per geometry combination
                return Phase_Table(tNumGeometries);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        Phase_Table Geometry_Engine::create_phase_table(
                uint                     aNumGeometries,
                Matrix<DDUMat>           aBulkPhases,
                PHASE_FUNCTION aPhaseFunction,
                uint                     aNumPhases)
        {
            if (aPhaseFunction)
            {
                return Phase_Table(aPhaseFunction, aNumPhases);
            }
            else if (aBulkPhases.length() > 0)
            {
                return Phase_Table(aNumGeometries, aBulkPhases);
            }
            else
            {
                return Phase_Table(aNumGeometries);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
