#include "cl_XTK_Integration_Mesh_Generator.hpp"

#include "cl_XTK_Regular_Subdivision_Interface.hpp"
#include "cl_XTK_Cell_No_CM.hpp"

namespace xtk
{   
    Integration_Mesh_Generator::Integration_Mesh_Generator( 
        xtk::Model*                    aXTKModelPtr,
        Cell<enum Subdivision_Method>  aMethods,
        moris::Matrix<moris::IndexMat> aActiveGeometries):
    mXTKModel(aXTKModelPtr),
    mGeometryEngine(mXTKModel->get_geom_engine()),
    mActiveGeometries(aActiveGeometries)
    {
        this->setup_subdivision_methods(aMethods);

        if(mActiveGeometries.numel() == 0)
        {
            mAllActiveGeometries = true;
        }
    }
// ----------------------------------------------------------------------------------

    Integration_Mesh_Generator::~Integration_Mesh_Generator()
    {
    
    }
// ----------------------------------------------------------------------------------

    bool
    Integration_Mesh_Generator::perform()
    {
        Tracer tTracer( "XTK", "Integration_Mesh_Generator", "perform" );
        
        // data structure to pass things around
        Integration_Mesh_Generation_Data tGenerationData;

        // pointer to the background mesh
        moris::mtk::Mesh* tBackgroundMesh = &mXTKModel->get_background_mesh().get_mesh_data();

        // the cut integration mesh
        Cut_Integration_Mesh tCutIntegrationMesh(tBackgroundMesh);

        // figure out which background cells are intersected and by which geometry they are intersected
        this->determine_intersected_background_cells(tGenerationData,&tCutIntegrationMesh,tBackgroundMesh);

        // Allocate all child meshes in one go, we link them to background cells in this step
        this->allocate_child_meshes(tGenerationData,&tCutIntegrationMesh,tBackgroundMesh);

        // Work In progress, allocate the first Decomposition algorithm
        // this decomposition algorithm depends only needs the child meshes to be linked with background cells
        Decomposition_Algorithm* tRegSubMethod = new Regular_Subdivision_Interface();

        // allocate a data structure for the new nodes created during decomposition to be constructed
        Decomposition_Data tDecompositionData;
        tRegSubMethod->perform(&tGenerationData, &tDecompositionData,&tCutIntegrationMesh, tBackgroundMesh, this);
        delete tRegSubMethod;

        
        
        // the next decomposition_algorithm requires the ig edges that are intersected
        // this->determine_intersected_ig_edges(tGenerationData,&tCutIntegrationMesh, tBackgroundMesh);

        // // initialize the node hier method
        // Decomposition_Algorithm* tNodeHierMethod = new Conformal_Edge_Based_Decomposition_Algorithm();
        // tNodeHierMethod = tNodeHierMethod->perform();

        return true;
    }

// ----------------------------------------------------------------------------------
    bool
    Integration_Mesh_Generator::determine_intersected_background_cells( 
        Integration_Mesh_Generation_Data & aMeshGenerationData, 
        Cut_Integration_Mesh*              aCutIntegrationMesh,
        moris::mtk::Mesh*                  aBackgroundMesh )
        {
            uint tNumGeometries = mAllActiveGeometries? mXTKModel->mGeometryEngine->get_num_geometries() : mActiveGeometries.numel();

            uint tNumCells = aBackgroundMesh->get_num_elems();
            
            aMeshGenerationData.mIntersectedBackgroundCellIndex.resize(tNumGeometries);
            aMeshGenerationData.mIntersectedBackgroundCellIndex.reserve(tNumGeometries*tNumCells);
            aMeshGenerationData.mBackgroundCellGeometryIndices.resize(tNumCells);
            aMeshGenerationData.mBackgroundCellGeometryIndices.reserve(tNumGeometries*tNumCells);

            // Initialize geometric query
            Geometric_Query_XTK tGeometricQuery;

            // say I am just interested in a yes or no answer
            tGeometricQuery.set_query_type(moris::ge::Query_Type::INTERSECTION_NO_LOCATION);

            // large coord matrix that I want to keep in scope for a long time avoid copying coordinate all the time.
            tGeometricQuery.set_coordinates_matrix(&aCutIntegrationMesh->mVertexCoordinates);

            for(moris::uint iCell = 0; iCell < tNumCells; iCell++)
            {
                // setup geometric query with this current cell information
                tGeometricQuery.set_parent_cell(&aBackgroundMesh->get_mtk_cell((moris_index) iCell));
                tGeometricQuery.set_query_parent_cell(&aBackgroundMesh->get_mtk_cell((moris_index) iCell));

                for(moris::size_t iGeom = 0; iGeom<tNumGeometries; iGeom++)
                {
                    // current index for this geometry
                    moris_index tGeometryIndex = mAllActiveGeometries? iGeom : mActiveGeometries(iGeom);

                    // tell the query which geometric index we are working on
                    tGeometricQuery.set_geometric_index(tGeometryIndex);

                    if(mXTKModel->get_geom_engine()->geometric_query(&tGeometricQuery))
                    {
                        // add background cell to the list for iGEOM
                        aMeshGenerationData.mIntersectedBackgroundCellIndex(iGeom).push_back(iCell);

                        // add the geometry to the background cell list
                        aMeshGenerationData.mBackgroundCellGeometryIndices(iCell).push_back(iGeom);

                        // if this one is intersected for the first time give it a child mesh index
                        if(aMeshGenerationData.mIntersectedBackgroundCellIndexToChildMeshIndex.find(iCell) == aMeshGenerationData.mIntersectedBackgroundCellIndexToChildMeshIndex.end())
                        {
                            aMeshGenerationData.mIntersectedBackgroundCellIndexToChildMeshIndex[iCell] = aMeshGenerationData.tNumChildMeshes;
                            aMeshGenerationData.tNumChildMeshes++;
                        }
                    }
                }
            }

            // remove the excess space
            aMeshGenerationData.mIntersectedBackgroundCellIndex.shrink_to_fit();
            aMeshGenerationData.mBackgroundCellGeometryIndices.shrink_to_fit();

            return true;
        }
// ----------------------------------------------------------------------------------

    void
    Integration_Mesh_Generator::commit_new_ig_cells_to_cut_mesh(
        Integration_Mesh_Generation_Data*  aMeshGenerationData,
        Decomposition_Data *               aDecompositionData,
        Cut_Integration_Mesh *             aCutIntegrationMesh,
        moris::mtk::Mesh *                 aBackgroundMesh,
        Decomposition_Algorithm*           aDecompositionAlgorithm)
    {
        // iterate through cells that the decomposition constructed
        moris::uint tNumNewCells = aDecompositionAlgorithm->mNumNewCells;

        MORIS_ERROR(aDecompositionAlgorithm->mNewCellToVertexConnectivity.size() == aDecompositionAlgorithm->mNewCellChildMeshIndex.size()  &&
                    aDecompositionAlgorithm->mNewCellChildMeshIndex.size() == aDecompositionAlgorithm->mNewCellCellIndexToReplace.size()
                    ,"Inconsistent size from decomposition algorithm");

        // add space to the mesh
        moris::uint tNumStartingCellsControlled = aCutIntegrationMesh->mControlledIgCells.size();
        moris::uint tNumStartingTotalIgCells    = aCutIntegrationMesh->mIntegrationCells.size();

        aCutIntegrationMesh->mControlledIgCells.resize(tNumStartingCellsControlled + tNumNewCells);
        aCutIntegrationMesh->mIntegrationCells.resize(tNumStartingTotalIgCells + tNumNewCells);

        // current index
        moris_index tCellIndex = tNumStartingTotalIgCells;

        // iterate through new and add to the mesh
        for(moris::uint iCell = 0;  iCell < aDecompositionAlgorithm->mNewCellToVertexConnectivity.size(); iCell++ )
        {
            // child mesh pointer
            std::shared_ptr<Child_Mesh_Experimental> tCM = aCutIntegrationMesh->get_child_mesh(aDecompositionAlgorithm->mNewCellChildMeshIndex(iCell));

            // parent cell owner
            moris_index tOwner = tCM->get_parent_cell()->get_owner();

            // collect the vertex pointers for the cell
            moris::Cell<moris::mtk::Vertex*> tVertexPointers(aDecompositionAlgorithm->mNewCellToVertexConnectivity(iCell).size());
            for(moris::uint iV = 0; iV < aDecompositionAlgorithm->mNewCellToVertexConnectivity(iCell).size(); iV++)
            {
                tVertexPointers(iV) = aCutIntegrationMesh->get_mtk_vertex_pointer( aDecompositionAlgorithm->mNewCellToVertexConnectivity(iCell)(iV));
            }

            bool tReplaceExistingCell = aDecompositionAlgorithm->mNewCellCellIndexToReplace(iCell) != MORIS_INDEX_MAX;

            // cell index (if I replace one that is the index of this cell)
            moris_index tNewCellIndex = tReplaceExistingCell? aDecompositionAlgorithm->mNewCellCellIndexToReplace(iCell) : tCellIndex++;

            // create the new cell no id
            std::shared_ptr<moris::mtk::Cell> tNewCell = std::make_shared<xtk::Cell_XTK_No_CM>(
                MORIS_ID_MAX,tNewCellIndex,tOwner,aDecompositionAlgorithm->mNewCellCellInfo(iCell),tVertexPointers);

            // add the cell to the mesh
            aCutIntegrationMesh->set_integration_cell(tNewCellIndex,tNewCell,!tReplaceExistingCell);

            // add the cell to a child mesh
            aCutIntegrationMesh->add_cell_to_integration_mesh(tNewCellIndex,tCM->mChildMeshIndex);
            
        }        
        
    }

// ----------------------------------------------------------------------------------
    void
    Integration_Mesh_Generator::create_edges_from_element_to_node( 
        moris::Cell<moris::mtk::Cell*> aCells,
        std::shared_ptr<Edge_Based_Connectivity> aEdgeConnectivity)
    {        
        // assume homogenous cell type
        if(aCells.size() > 0)
        {
            // cell information
            moris::mtk::Cell_Info  const * tCellInfo = aCells(0)->get_cell_info();

            //hard-coded values
            moris::uint tMaxEdgePerNode   = 20;
            // moris::uint tMaxEdgeToElement = 10;

            // Initialize
            moris::uint tNumEdgePerElem = tCellInfo->get_num_edges();

            // nodes to edge map
            moris::Matrix< moris::IdMat > tElementEdgeToNodeMap = tCellInfo->get_node_to_edge_map();

            moris::uint tNumElements       = aCells.size();
            moris::uint tNumNodesPerEdge   = 2;
            MORIS_ERROR(tNumNodesPerEdge == 2,"Only works on two node edges at the moment");
            MORIS_ERROR(tNumNodesPerEdge == tElementEdgeToNodeMap.n_cols(),"Mismatch in number of nodes per edge (only operating on two node edges)");
            // moris::uint tNumEdgeCreated    = 0;
            moris::uint tMaxNumEdges       = tNumElements*tNumEdgePerElem;

            moris::uint tNumNodes = 0;
            moris::Cell<moris::mtk::Vertex*> tVertices;
            std::unordered_map<moris_index,moris_index> tVertexIndexToLocalIndexMap;
            for(moris::uint i = 0; i < aCells.size(); i++)
            {
               moris::Cell<moris::mtk::Vertex*> tCellVerts = aCells(i)->get_vertex_pointers();

               for(moris::uint iV = 0; iV < tCellVerts.size(); iV++)
               {
                   if(tVertexIndexToLocalIndexMap.find(tCellVerts(iV)->get_index()) == tVertexIndexToLocalIndexMap.end())
                   {
                       tVertexIndexToLocalIndexMap[tCellVerts(iV)->get_index()] = (moris_index) tNumNodes;
                       tVertices.push_back(tCellVerts(iV));
                       tNumNodes++;
                   }
               }
            }
            // Allocate outputs
            aEdgeConnectivity->mCellToEdge.resize(aCells.size());
            aEdgeConnectivity->mEdgeToCell.reserve(tMaxNumEdges);
            aEdgeConnectivity->mEdgeToCellEdgeOrdinal.reserve(tMaxNumEdges);
            aEdgeConnectivity->mEdgeVertices.reserve(tMaxNumEdges*tNumNodesPerEdge);

            moris::Cell<moris::Cell<uint>> tVertexToEdgeIndex(tNumNodes);
            tVertexToEdgeIndex.reserve(tMaxNumEdges*tNumNodesPerEdge*tMaxEdgePerNode);

            moris::Cell<moris::mtk::Vertex*> tVerticesOnEdge(tNumNodesPerEdge,nullptr);

            for(moris::uint i = 0; i < aCells.size(); i++)
            {
               moris::Cell<moris::mtk::Vertex*> tCellVerts = aCells(i)->get_vertex_pointers();

               // iterate through edges
               for(moris::uint iEdge = 0; iEdge < tElementEdgeToNodeMap.n_rows(); iEdge++)
               {
                    // get the vertices on the edge
                    for(moris::uint iVOnE = 0; iVOnE < tElementEdgeToNodeMap.n_cols(); iVOnE++)
                    {
                        tVerticesOnEdge(iVOnE) = tCellVerts(tElementEdgeToNodeMap(iEdge,iVOnE));
                    }

                    // figure out if the edge exists and if so where
                    moris_index tEdgeIndex = this->edge_exists(tVerticesOnEdge,tVertexIndexToLocalIndexMap,tVertexToEdgeIndex,aEdgeConnectivity->mEdgeVertices);
                    
                    // add it new
                    if(tEdgeIndex == MORIS_INDEX_MAX)
                    {
                        tEdgeIndex = aEdgeConnectivity->mEdgeVertices.size();
                        aEdgeConnectivity->mEdgeVertices.push_back(tVerticesOnEdge);
                        aEdgeConnectivity->mEdgeToCell.push_back(moris::Cell<moris::mtk::Cell*>());
                        aEdgeConnectivity->mEdgeToCellEdgeOrdinal.push_back(moris::Cell<moris::moris_index>());

                        // local vertex index
                        auto tIter = tVertexIndexToLocalIndexMap.find(tVerticesOnEdge(0)->get_index());
                        MORIS_ERROR(tIter != tVertexIndexToLocalIndexMap.end(),"Invalid vertex detected.");
                        moris_index tLocalVertexIndex = tIter->second;

                        tVertexToEdgeIndex(tLocalVertexIndex).push_back(tEdgeIndex);
                    }

                    aEdgeConnectivity->mCellToEdge(i).push_back(tEdgeIndex);
                    aEdgeConnectivity->mEdgeToCell(i).push_back(aCells(i));
                    aEdgeConnectivity->mEdgeToCellEdgeOrdinal(i).push_back(iEdge);
               }
            }
        }
    }
// ----------------------------------------------------------------------------------
    void
    Integration_Mesh_Generator::commit_new_ig_vertices_to_cut_mesh(
        Integration_Mesh_Generation_Data*  aMeshGenerationData,
        Decomposition_Data *               aDecompositionData,
        Cut_Integration_Mesh *             aCutIntegrationMesh,
        moris::mtk::Mesh *                 aBackgroundMesh,
        Decomposition_Algorithm*           aDecompAlg)
    {
        // current index
        moris_index tControlledVertexIndex = aCutIntegrationMesh->mControlledIgVerts.size();

        // allocate new vertices
        moris::uint tNumNewIgVertices = aDecompositionData->tNewNodeIndex.size();
        aCutIntegrationMesh->mControlledIgVerts.resize(aCutIntegrationMesh->mControlledIgVerts.size() + tNumNewIgVertices);
        aCutIntegrationMesh->mIntegrationVertices.resize(aCutIntegrationMesh->mIntegrationVertices.size() + tNumNewIgVertices);
        aCutIntegrationMesh->mVertexCoordinates.resize(aCutIntegrationMesh->mControlledIgVerts.size() + tNumNewIgVertices,nullptr);
        
        // iterate and create new vertices
        for(moris::uint iV = 0; iV < aDecompositionData->tNewNodeId.size(); iV++ )
        {   
            // construct coordinate matrix
            aCutIntegrationMesh->mVertexCoordinates(aDecompositionData->tNewNodeIndex(iV)) = std::make_shared<Matrix<DDRMat>> (aDecompositionData->tNewNodeCoordinate(iV));

            // create a controlled vertex (meaning I need to manage memory of it)
            aCutIntegrationMesh->mControlledIgVerts(tControlledVertexIndex) = std::make_shared<moris::mtk::Vertex_XTK>( 
                aDecompositionData->tNewNodeId(iV),
                aDecompositionData->tNewNodeIndex(iV),
                aCutIntegrationMesh->mVertexCoordinates(aDecompositionData->tNewNodeIndex(iV)).get());

            // add vertex coordinates to the mesh data
                
            aCutIntegrationMesh->mIntegrationVertices(aDecompositionData->tNewNodeIndex(iV)) = aCutIntegrationMesh->mControlledIgVerts(tControlledVertexIndex).get();
            tControlledVertexIndex++;
        }
        

        // iterate through child meshes and commit the vertices to their respective vertex groups
        for(moris::uint iCM = 0; iCM < (moris::uint) aMeshGenerationData->tNumChildMeshes; iCM++)
        {
            MORIS_ERROR(aDecompositionData->tCMNewNodeLoc.size() == (moris::uint) aCutIntegrationMesh->mChildMeshes.size(),"Mismatch in child mesh sizes. All child meshes need to be present in the decomposition data");

            // add the vertices to child mesh groups
            moris_index tStartIndex = (moris_index) aCutIntegrationMesh->mIntegrationVertexGroups(iCM)->mIgVertexGroup.size();
            moris_index tNumNewVertices = (moris_index) aDecompositionData->tCMNewNodeLoc(iCM).size();

            // resize the vertices in the group
            aCutIntegrationMesh->mIntegrationVertexGroups(iCM)->mIgVertexGroup.resize(tNumNewVertices + tStartIndex,nullptr);

            for(moris::moris_index iCMVerts = 0; iCMVerts < tNumNewVertices; iCMVerts++)
            {
                moris_index tNewNodeLocInDecomp = aDecompositionData->tCMNewNodeLoc(iCM)(iCMVerts);
                moris_index tNewNodeIndex = aDecompositionData->tNewNodeIndex(tNewNodeLocInDecomp);

                aCutIntegrationMesh->mIntegrationVertexGroups(iCM)->mIgVertexGroup(iCMVerts + tStartIndex) = aCutIntegrationMesh->mIntegrationVertices(tNewNodeIndex);
            }
        }

        // construct a relationship for the geometry engine to have geomtric information for this vertex  
        this->link_new_vertices_to_geometry_engine( aDecompositionData, aDecompAlg );

    }

    void
    Integration_Mesh_Generator::link_new_vertices_to_geometry_engine(
        Decomposition_Data *               aDecompositionData,
        Decomposition_Algorithm*           aDecompAlg)
    
    {
        if(aDecompAlg->has_geometric_independent_vertices())
        {
            // pass the data in decomposition data to the geometry engine so it can keep track of these newly constructed vertices
            mGeometryEngine->create_new_child_nodes(
                &aDecompositionData->tNewNodeIndex,
                &aDecompositionData->mNewNodeParentCells,
                &aDecompositionData->mNewVertexLocalCoordWRTParentCell,
                &aDecompositionData->tNewNodeCoordinate);
        }
    }

    

// ----------------------------------------------------------------------------------
    bool
    Integration_Mesh_Generator::allocate_child_meshes(
        Integration_Mesh_Generation_Data & aMeshGenerationData, 
        Cut_Integration_Mesh*              aCutIntegrationMesh,
        moris::mtk::Mesh*                  aBackgroundMesh  )
    {
        
        aCutIntegrationMesh->mChildMeshes.resize(aMeshGenerationData.tNumChildMeshes);
        aCutIntegrationMesh->mIntegrationCellGroups.resize(aMeshGenerationData.tNumChildMeshes,nullptr);
        aCutIntegrationMesh->mIntegrationVertexGroups.resize(aMeshGenerationData.tNumChildMeshes,nullptr);
        aCutIntegrationMesh->mIntegrationCellGroupsParentCell.resize(aMeshGenerationData.tNumChildMeshes,nullptr);

        // create the child meshes
        for (auto& it: aMeshGenerationData.mIntersectedBackgroundCellIndexToChildMeshIndex) 
        {
            moris_index tCMIndex = it.second;
            moris::mtk::Cell* tParentCell = &aBackgroundMesh->get_mtk_cell(it.first);
            aCutIntegrationMesh->mChildMeshes(tCMIndex)                     = std::make_shared<Child_Mesh_Experimental>();
            aCutIntegrationMesh->mIntegrationCellGroups(tCMIndex)           = std::make_shared<IG_Cell_Group>(0);
            aCutIntegrationMesh->mChildMeshes(tCMIndex)->mIgCells           = aCutIntegrationMesh->mIntegrationCellGroups(tCMIndex);
            aCutIntegrationMesh->mIntegrationCellGroupsParentCell(tCMIndex) = tParentCell;
            aCutIntegrationMesh->mChildMeshes(tCMIndex)->mParentCell        = tParentCell;
            aCutIntegrationMesh->mChildMeshes(tCMIndex)->mChildMeshIndex    = tCMIndex;


            moris_index tNumGeometricVertices = 8;
            moris::Cell<moris::mtk::Vertex*> tParentCellVerts = tParentCell->get_vertex_pointers();
            aCutIntegrationMesh->mIntegrationVertexGroups(tCMIndex) = std::make_shared<IG_Vertex_Group>(tNumGeometricVertices);
            aCutIntegrationMesh->mChildMeshes(tCMIndex)->mIgVerts = aCutIntegrationMesh->mIntegrationVertexGroups(tCMIndex);
            //FIXME: GET GEOMETRIC VERTICES FROM MTK CELL HARDCODED TO HEXFAMILY
            for(moris::moris_index i = 0 ; i < tNumGeometricVertices; i++)
            {
                aCutIntegrationMesh->mIntegrationVertexGroups(tCMIndex)->mIgVertexGroup(i) = tParentCellVerts(i);
            }
            
        }

        return true;
    }
// ----------------------------------------------------------------------------------
    void
    Integration_Mesh_Generator::setup_subdivision_methods(Cell<enum Subdivision_Method> aMethods)
    {
        MORIS_ERROR(aMethods.size() == 2,  "Invalid methods provided to integration mesh generation, to be extended to support more cases");

        if(aMethods.size() == 1)
        {
            if(aMethods(0) == Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4 
            || aMethods(0) == Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8)
            {
                mRegularSubdivision = aMethods(0);
                mConformalSubdivision = Subdivision_Method::NO_METHOD;
            }
            else
            {
                mRegularSubdivision = Subdivision_Method::NO_METHOD;
                mConformalSubdivision = aMethods(0);
            }
        }

        else if(aMethods.size() == 2)
        {
            MORIS_ERROR(aMethods(0) == Subdivision_Method::NC_REGULAR_SUBDIVISION_QUAD4 || aMethods(0) == Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8,"Regular subdivision method must come first");
            MORIS_ERROR(aMethods(1) == Subdivision_Method::C_TRI3 || aMethods(1) == Subdivision_Method::C_HIERARCHY_TET4,"Recursive conformal method must come second");
            mRegularSubdivision   = aMethods(0);
            mConformalSubdivision = aMethods(1);
        }
    }
// ----------------------------------------------------------------------------------

    void
    Integration_Mesh_Generator::assign_node_requests_identifiers( 
            Decomposition_Data &  aDecompData,
            Cut_Integration_Mesh* aCutIntegrationMesh,
            moris::mtk::Mesh*     aBackgroundMesh,
            moris::moris_index    aMPITag)
    {
        moris_index tNodeIndex = aCutIntegrationMesh->get_first_available_index(EntityRank::NODE);

        for(moris::uint i = 0; i < aDecompData.tNewNodeIndex.size(); i++)
        {
            // set the new node index
            aDecompData.tNewNodeIndex(i) = tNodeIndex;
            tNodeIndex++;
        }

        barrier();
        // asserts
        MORIS_ERROR(aDecompData.tNewNodeId.size() == aDecompData.tNewNodeIndex.size(),
                "Dimension mismatch in assign_node_requests_identifiers");
        MORIS_ERROR(aDecompData.tNewNodeId.size() == aDecompData.tNewNodeParentRank.size(),
                "Dimension mismatch in assign_node_requests_identifiers");
        MORIS_ERROR(aDecompData.tNewNodeId.size() == aDecompData.tNewNodeParentIndex.size(),
                "Dimension mismatch in assign_node_requests_identifiers");

        // owned requests and shared requests sorted by owning proc
        Cell<uint> tOwnedRequest;
        Cell<Cell<uint>> tNotOwnedRequests;
        Cell<uint> tProcRanks;
        std::unordered_map<moris_id,moris_id> tProcRankToDataIndex;
        this->sort_new_node_requests_by_owned_and_not_owned(
                aDecompData,
                aBackgroundMesh,
                tOwnedRequest,
                tNotOwnedRequests,
                tProcRanks,
                tProcRankToDataIndex);

        // allocate ids for nodes I own
        moris::moris_id tNodeId  = aCutIntegrationMesh->allocate_entity_ids(aDecompData.tNewNodeId.size(), EntityRank::NODE);

        // Assign owned request identifiers
        this->assign_owned_request_id(aDecompData, tOwnedRequest, tNodeId);

        // prepare node information request data
        Cell<Matrix<IndexMat>> tOutwardRequests;
        this->setup_outward_requests(aDecompData, aBackgroundMesh, tNotOwnedRequests, tProcRanks, tProcRankToDataIndex, tOutwardRequests);

        // send requests to owning processor
        mXTKModel->send_outward_requests(aMPITag,tProcRanks,tOutwardRequests);

        // hold on to make sure everyone has sent all their information
        barrier();

        // receive the requests
        Cell<Matrix<IndexMat>> tReceivedRequests;
        Cell<uint> tProcsReceivedFrom;
        mXTKModel->inward_receive_requests(aMPITag, 3, tReceivedRequests, tProcsReceivedFrom);

        // Prepare request answers
        Cell<Matrix<IndexMat>> tRequestAnwers;
        this->prepare_request_answers(aDecompData,aBackgroundMesh,tReceivedRequests,tRequestAnwers);

        // send the answers back
        mXTKModel->return_request_answers(aMPITag+1, tRequestAnwers, tProcsReceivedFrom);

        barrier();

        // receive the answers
        Cell<Matrix<IndexMat>> tReceivedRequestsAnswers;
        mXTKModel->inward_receive_request_answers(aMPITag+1,1,tProcRanks,tReceivedRequestsAnswers);

        // handle received information
        this->handle_received_request_answers(aDecompData, aBackgroundMesh, tOutwardRequests,tReceivedRequestsAnswers,tNodeId);

        MORIS_ERROR(mXTKModel->verify_successful_node_assignment(aDecompData),
                "Unsuccesssful node assignment detected.");

        barrier();
    }
// ----------------------------------------------------------------------------------
   void
   Integration_Mesh_Generator::sort_new_node_requests_by_owned_and_not_owned(
            Decomposition_Data                    & tDecompData,
            moris::mtk::Mesh*                       aBackgroundMesh,
            Cell<uint>                            & aOwnedRequests,
            Cell<Cell<uint>>                      & aNotOwnedRequests,
            Cell<uint>                            & aProcRanks,
            std::unordered_map<moris_id,moris_id> & aProcRankToIndexInData)
    {
        // access the communication
        Matrix<IdMat> tCommTable = aBackgroundMesh->get_communication_table();

        // number of new nodes
        moris::uint tNumNewNodes = tDecompData.tNewNodeParentIndex.size();

        // Par rank
        moris::moris_index tParRank = par_rank();

        // resize proc ranks and setup map to comm table
        aProcRanks.resize(tCommTable.numel());
        for(moris::uint i = 0; i <tCommTable.numel(); i++)
        {
            aProcRankToIndexInData[tCommTable(i)] = i;
            aProcRanks(i) = (tCommTable(i));
            aNotOwnedRequests.push_back(Cell<uint>(0));
        }

        // iterate through each node request and figure out the owner
        for(moris::uint i = 0; i <tNumNewNodes; i++)
        {
            // Parent Rank
            enum EntityRank    tParentRank  = tDecompData.tNewNodeParentRank(i);
            moris::moris_index tParentIndex = tDecompData.tNewNodeParentIndex(i);

            // get the owner processor
            moris::moris_index tOwnerProc = aBackgroundMesh->get_entity_owner(tParentIndex,tParentRank);

            // If i own the request keep track of the index
            if(tOwnerProc == tParRank)
            {
                aOwnedRequests.push_back(i);
            }
            else
            {
                moris_index tIndex = aProcRankToIndexInData[tOwnerProc];

                aNotOwnedRequests(tIndex).push_back(i);
            }
        }
    }
// ----------------------------------------------------------------------------------
    void
    Integration_Mesh_Generator::assign_owned_request_id(
            Decomposition_Data & aDecompData,
            Cell<uint> const &   aOwnedRequest,
            moris::moris_id &    aNodeId)
    {
        for(moris::uint i = 0; i < aOwnedRequest.size(); i++)
        {
            moris_index tRequestIndex = aOwnedRequest(i);

            // set the new node id
            aDecompData.tNewNodeId(tRequestIndex) = aNodeId;
            aNodeId++;

            // increment number of new nodes with set ids (for assertion purposes)
            aDecompData.mNumNewNodesWithIds++;
        }
    }
// ----------------------------------------------------------------------------------
    void
    Integration_Mesh_Generator::setup_outward_requests(
            Decomposition_Data              const & aDecompData,
            moris::mtk::Mesh*                       aBackgroundMesh,
            Cell<Cell<uint>>                const & aNotOwnedRequests,
            Cell<uint>                      const & aProcRanks,
            std::unordered_map<moris_id,moris_id> & aProcRankToIndexInData,
            Cell<Matrix<IndexMat>>                & aOutwardRequests)
    {
        // size data
        aOutwardRequests.resize(aProcRanks.size());

        // iterate through the processors we need information from and package the matrix
        for(moris::uint i = 0; i < aProcRanks.size(); i++)
        {
            uint tProcRank = aProcRanks(i);

            MORIS_ASSERT(aProcRankToIndexInData.find(tProcRank) != aProcRankToIndexInData.end(),"Proc rank not in map");
            uint tIndexInData = aProcRankToIndexInData[tProcRank];

            uint tNumRequests = aNotOwnedRequests(tIndexInData).size();

            // size the sending matrix
            // column - request
            //   r0 - parent entity id
            //   r1 - parent entity rank
            //   r2 - Secondary id
            if(tNumRequests > 0)
            {
                aOutwardRequests(i) = moris::Matrix<IndexMat>(3,tNumRequests);
            }

            else
            {
                aOutwardRequests(i) = moris::Matrix<IndexMat>(3,1,MORIS_INDEX_MAX);
            }

            // populate matrix to send;
            for(moris::uint j = 0; j < tNumRequests; j++)
            {
                moris_index     tRequestIndex = aNotOwnedRequests(tIndexInData)(j);
                moris_index     tParentIndex  = aDecompData.tNewNodeParentIndex(tRequestIndex);
                moris_index     tSecondaryId  = aDecompData.tSecondaryIdentifiers(tRequestIndex);
                enum EntityRank tParentRank   = aDecompData.tNewNodeParentRank(tRequestIndex);

                aOutwardRequests(i)(0,j) = aBackgroundMesh->get_glb_entity_id_from_entity_loc_index(tParentIndex,tParentRank);
                aOutwardRequests(i)(1,j) = (moris_index)tParentRank;
                aOutwardRequests(i)(2,j) = tSecondaryId;
            }
        }
    }
// ----------------------------------------------------------------------------------
    void
    Integration_Mesh_Generator::prepare_request_answers(
            Decomposition_Data           & aDecompData,
            moris::mtk::Mesh*              aBackgroundMesh,
            Cell<Matrix<IndexMat>> const & aReceiveData,
            Cell<Matrix<IndexMat>>       & aRequestAnswers)
    {
        // allocate answer size
        aRequestAnswers.resize(aReceiveData.size());

        // iterate through received data
        for(moris::uint i = 0; i < aReceiveData.size(); i++)
        {
            uint tNumReceivedReqs = aReceiveData(i).n_cols();

            aRequestAnswers(i).resize(1,tNumReceivedReqs);

            aRequestAnswers(i)(0) = MORIS_INDEX_MAX;

            // avoid the dummy message
            if(aReceiveData(i)(0,0) != MORIS_INDEX_MAX)
            {
                // iterate through received requests
                for(moris::uint j = 0; j < tNumReceivedReqs; j++)
                {
                    moris_id        tParentId      = aReceiveData(i)(0,j);
                    enum EntityRank tParentRank    = (enum EntityRank) aReceiveData(i)(1,j);
                    moris_id        tSecondaryId   = aReceiveData(i)(2,j);
                    moris_index     tParentInd     = aBackgroundMesh->get_loc_entity_ind_from_entity_glb_id(tParentId,tParentRank);
                    bool            tRequestExists = false;
                    moris_index     tRequestIndex  = MORIS_INDEX_MAX;

                    if(aDecompData.mHasSecondaryIdentifier)
                    {
                        tRequestExists = aDecompData.request_exists(
                                tParentInd,
                                tSecondaryId,
                                (EntityRank)tParentRank,
                                tRequestIndex);
                    }
                    else
                    {
                        tRequestExists = aDecompData.request_exists(
                                tParentInd,
                                (EntityRank)tParentRank,
                                tRequestIndex);
                    }

                    if(tRequestExists)
                    {
                        moris_id tNodeId =aDecompData.tNewNodeId(tRequestIndex);

                        aRequestAnswers(i)(j) = tNodeId;

                        if(tNodeId == MORIS_ID_MAX)
                        {
                            std::cout<<"tParentId = "<<tParentId<<" | Rank "<<(uint)tParentRank<<std::endl;
                            //                    MORIS_ERROR(0,"Max node");
                        }
                    }
                    else
                    {
                        aRequestAnswers(i)(j) = MORIS_ID_MAX;
                    }
                }
            }
        }
    }
// ----------------------------------------------------------------------------------    
    void
    Integration_Mesh_Generator::handle_received_request_answers(
            Decomposition_Data           & aDecompData,
            moris::mtk::Mesh*              aBackgroundMesh,
            Cell<Matrix<IndexMat>> const & aRequests,
            Cell<Matrix<IndexMat>> const & aRequestAnswers,
            moris::moris_id              & aNodeId)
    {
        Cell<moris_index> tUnhandledRequestIndices;

        // iterate through received data
        for(moris::uint i = 0; i < aRequests.size(); i++)
        {
            uint tNumReceivedReqs = aRequests(i).n_cols();

            // avoid the dummy message
            if(aRequests(i)(0,0) != MORIS_INDEX_MAX)
            {
                // iterate through received requests
                for(moris::uint j = 0; j < tNumReceivedReqs; j++)
                {
                    moris_id        tParentId      = aRequests(i)(0,j);
                    enum EntityRank tParentRank    = (enum EntityRank) aRequests(i)(1,j);
                    moris_id        tSecondaryId   = aRequests(i)(2,j);
                    moris_index     tParentInd     = aBackgroundMesh->get_loc_entity_ind_from_entity_glb_id(tParentId,tParentRank);
                    bool            tRequestExists = false;
                    moris_index     tRequestIndex  = MORIS_INDEX_MAX;

                    if(aDecompData.mHasSecondaryIdentifier)
                    {
                        tRequestExists = aDecompData.request_exists(tParentInd,tSecondaryId,(EntityRank)tParentRank,tRequestIndex);
                    }
                    else
                    {
                        tRequestExists = aDecompData.request_exists(tParentInd,(EntityRank)tParentRank,tRequestIndex);
                    }

                    if(tRequestExists && aRequestAnswers(i)(j))
                    {
                        moris_id tNodeId =aRequestAnswers(i)(j);

                        // meaning the owning processor expected this and gave an answer
                        if(tNodeId < MORIS_ID_MAX && aDecompData.tNewNodeId(tRequestIndex) == MORIS_INDEX_MAX)
                        {
                            // set the new node id
                            aDecompData.tNewNodeId(tRequestIndex) = tNodeId;

                            aDecompData.mNumNewNodesWithIds++;
                        }
                        // The owner did not expect and did not return an answer
                        else
                        {   
                            // keep track of unhandled
                            tUnhandledRequestIndices.push_back(tRequestIndex);
                            // moris_index tNodeIndex = mBackgroundMesh.get_first_available_index(EntityRank::NODE);

                            // aDecompData.tNewNodeOwner(tRequestIndex) = par_rank();

                            // aDecompData.tNewNodeId(tRequestIndex) = tNodeId;
                            // aDecompData.tNewNodeIndex(tRequestIndex) = tNodeIndex;
                            // tNodeIndex++;

                            // // set the new node id
                            // aDecompData.tNewNodeId(tRequestIndex) = tNodeId;

                            // aDecompData.mNumNewNodesWithIds++;

                            // mBackgroundMesh.update_first_available_index(tNodeIndex, EntityRank::NODE);

                        }
                    }
                    else
                    {
                        MORIS_ASSERT(0,"Request does not exist.");
                    }
                }
            }
        }
    }
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

}