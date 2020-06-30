#include "cl_Communication_Tools.hpp"
#include "cl_MTK_Exodus_IO_Helper.hpp"
#include "cl_Logger.hpp"
#include "fn_find_unique.hpp"

namespace moris
{
    namespace mtk
    {
        Exodus_IO_Helper::Exodus_IO_Helper(
                const char * aExodusFile,
                const int    aTimeStepIndex,
                const bool   aVerbose)
        {
            mVerbose       = aVerbose;
            mTimeStepIndex = aTimeStepIndex;

            mTitle = new char[MAX_LINE_LENGTH+1];
            int cpu_ws = sizeof( real );         // word size in bytes of the floating point variables used in moris
            int io_ws  = 0;                      // word size as stored in exodus
            float exoVersion = 0.0;

            get_file_name(aExodusFile, NULL, mTitle);

            mExoFileId = ex_open(
                    mTitle,
                    EX_WRITE,
                    &cpu_ws,
                    &io_ws,
                    &exoVersion);

            MORIS_ERROR(mExoFileId!=-1,"Cannot open exodus file: %s",mTitle);

            MORIS_ERROR( cpu_ws == io_ws,
                    "Word size of floating point variables stored in exodus file and used in moris are not the same.");

            get_init_mesh_data();
            get_init_global();
            get_eb_info_global();
            get_ns_param_global();
            get_load_bal_parameters();
            get_cmap_params();
            get_node_map();
            get_node_cmap();
            get_node_id_map();
            get_node_coords();
            get_elem_id_map();
            get_set_information();
            get_nodal_fields();
        }

        // ---------------------------------------------------------------------------------------------

        Exodus_IO_Helper::~Exodus_IO_Helper()
        {
            ex_close(mExoFileId);
            delete [] mTitle;
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::create_new_exo_with_elem_cmaps_from_existing_exo(
                std::string    & aFileName,
                Matrix<IdMat>  & aElementIds,
                Matrix<IdMat>  & aElementSideOrds,
                Matrix<IdMat>  & aSharedProcIds)
        {
            // retrieve initialization parameters from existing exodus file
            ex_init_params init_params;
            ex_get_init_ext(mExoFileId, &init_params);

            if (mVerbose)
            {
                MORIS_LOG_INFO( " \n===================================================");
                MORIS_LOG_INFO( " Parameters read from %s \n", aFileName.c_str());
                MORIS_LOG_INFO( " Exodus ID:      %d",mExoFileId);
                MORIS_LOG_INFO( " Title:          %s",init_params.title);
                MORIS_LOG_INFO( " num_dim:        %d",init_params.num_dim);
                MORIS_LOG_INFO( " num_nodes:      %d",init_params.num_nodes);
                MORIS_LOG_INFO( " num_elem:       %d",init_params.num_elem);
                MORIS_LOG_INFO( " num_elem_blk:   %d",init_params.num_elem_blk);
                MORIS_LOG_INFO( " num_node_sets:  %d",init_params.num_node_sets);
                MORIS_LOG_INFO( " num_side_sets:  %d",init_params.num_side_sets);
            }

            // Word sizes
            int cpu_ws = sizeof( moris::real );         // word size in bytes of the floating point variables used in moris
            int io_ws  = sizeof( moris::real );         // word size as stored in exodus

            // Create a new exodus
            char * tNewTitle = new char[MAX_LINE_LENGTH+1];

            // Create the file name
            get_file_name(aFileName.c_str(), NULL, tNewTitle);

            // Create a new exodus file (clobber if already there)
            int tNewExoFileId = ex_create(tNewTitle,
                    EX_CLOBBER,
                    &cpu_ws,
                    &io_ws);

            // Put file initialization information
            mErrFlag = ex_put_init_info(tNewExoFileId,
                    par_size(),
                    1,
                    const_cast<char *>("p"));

            MORIS_ASSERT(!mErrFlag,"ex_put_init_info failed");

            // Put global information on the file
            mErrFlag = ex_put_init_global( tNewExoFileId,
                    mNumNodesGlobal,
                    mNumElemsGlobal,
                    mNumElemBlksGlobal, /* I.   */
                    mNumNodeSetsGlobal, /* II.  */
                    mNumSideSetsGlobal );

            MORIS_ASSERT(!mErrFlag,"ex_put_init_global failed");

            // Initialize information about the mesh (local information)
            mErrFlag = ex_put_init(tNewExoFileId,
                    init_params.title,
                    init_params.num_dim,
                    init_params.num_nodes,
                    init_params.num_elem,
                    init_params.num_elem_blk,
                    init_params.num_node_sets,
                    init_params.num_side_sets);

            MORIS_ASSERT(!mErrFlag,"ex_put_init failed");

            // Put information about global blk ids and blk counts
            mErrFlag = ex_put_eb_info_global(tNewExoFileId,
                    this->mGlobalElemBlkIds.data(),
                    this->mGlobalElemBlkCnts.data());

            MORIS_ASSERT(!mErrFlag,"ex_put_eb_info_global failed");

            // Put the global node sets on the new file
            if (mGlobalNodesetIds.size())
            {
                mErrFlag = ex_put_ns_param_global(tNewExoFileId,
                        this->mGlobalNodesetIds.data(),
                        this->mNumGlobalNodeCounts.data(),
                        this->mNumGlobalNodeDfCounts.data());

                MORIS_ASSERT(!mErrFlag,"ex_put_ns_param_global failed");
            }

            // TODO: SIDE SETS and BLOCKSET copying

            // Specify new load balance parameters but add the num_elem_cmaps of 1
            mNumElemCmaps = 1;

            mErrFlag = ex_put_loadbal_param( tNewExoFileId,
                    this->mNumInternalNodes,
                    this->mNumBorderNodes,
                    this->mNumExternalNodes,
                    this->mNumInternalElems,
                    this->mNumBorderElems,
                    this->mNumNodeCmaps,
                    this->mNumElemCmaps,
                    par_rank());

            MORIS_ASSERT(!mErrFlag,"ex_put_loadbal_param failed");

            mElemCmapIds.resize(1,1);
            mElemCmapIds(0,0) = 2;

            mElemCmapElemCnts.resize(1,1);
            mElemCmapElemCnts(0,0) = aElementIds.numel();

            // Put communication map parameters
            mErrFlag = ex_put_cmap_params(
                    tNewExoFileId,
                    mNodeCmapIds.data(),
                    mNodeCmapNodeCnts.data(),
                    mElemCmapIds.data(),
                    mElemCmapElemCnts.data(),
                    par_rank());

            // Put node communication map
            mErrFlag = ex_put_node_cmap(
                    tNewExoFileId,
                    mNodeCmapIds(0),
                    mNodeCmapNodeIds(0).data(),
                    mNodeCmapProcIds(0).data(),
                    par_rank());

            // put node map
            mErrFlag =ne_put_node_map(
                    tNewExoFileId,
                    mNodeMapi.data(),
                    mNodeMapb.data(),
                    mNodeMape.data(),
                    par_rank());

            // put node id map
            mErrFlag = ex_put_id_map(
                    tNewExoFileId,
                    EX_NODE_MAP,
                    mNodeNumMap.data());

            // put the element communication maps
            mErrFlag = ex_put_elem_cmap(
                    tNewExoFileId,
                    2,
                    aElementIds.data(),
                    aElementSideOrds.data(),
                    aSharedProcIds.data(),
                    par_rank());

            // put the element id map
            mErrFlag = ex_put_id_map(
                    tNewExoFileId,
                    EX_ELEM_MAP,
                    mElemNumMap.data());

            // Coordinates
            copy_coordinates(tNewExoFileId);

            // Copy sets
            copy_node_sets( tNewExoFileId );
            copy_side_sets( tNewExoFileId );
            copy_block_sets(tNewExoFileId );

            // Copy fields
            copy_nodal_fields(tNewExoFileId,  init_params);

            ex_close(tNewExoFileId);
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_init_mesh_data()
        {
            mErrFlag = ex_get_init(
                    mExoFileId,
                    mTitle,
                    &mNumDim,
                    &mNumNodes,
                    &mNumElem,
                    &mNumElemBlk,
                    &mNumNodeSets,
                    &mNumSideSets);

            MORIS_ERROR(!mErrFlag, "Error reading initial global data!");

            if (mVerbose)
            {
                int tRank = par_rank();

                MORIS_LOG_INFO( " [%d] Parameters read from exodus file",tRank);
                MORIS_LOG_INFO( " [%d] Title:          %s",tRank,mTitle);
                MORIS_LOG_INFO( " [%d] num_dim:        %d",tRank,mNumDim);
                MORIS_LOG_INFO( " [%d] num_nodes:      %d",tRank,mNumNodes);
                MORIS_LOG_INFO( " [%d] num_elem:       %d",tRank,mNumElem);
                MORIS_LOG_INFO( " [%d] num_elem_blk:   %d",tRank,mNumElemBlk);
                MORIS_LOG_INFO( " [%d] num_node_sets:  %d",tRank,mNumNodeSets);
                MORIS_LOG_INFO( " [%d] num_side_sets:  %d",tRank,mNumSideSets);
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_load_bal_parameters()
        {
            if (par_size() > 1 )
            {
                mErrFlag =ex_get_loadbal_param(
                        mExoFileId,
                        &mNumInternalNodes,
                        &mNumBorderNodes,
                        &mNumExternalNodes,
                        &mNumInternalElems,
                        &mNumBorderElems,
                        &mNumNodeCmaps,
                        &mNumElemCmaps,
                        par_rank() // The ID of the processor for which info is to be read
                );

                if (mErrFlag)
                {
                    printf("after ex_get_init, error = %d\n", mErrFlag);
                }

                if (mVerbose)
                {
                    int tRank = par_rank();

                    MORIS_LOG_INFO( " [%d] num_internal_nodes = %d",tRank,mNumInternalNodes);
                    MORIS_LOG_INFO( " [%d] num_border_nodes   = %d",tRank,mNumBorderNodes);
                    MORIS_LOG_INFO( " [%d] num_external_nodes = %d",tRank,mNumExternalNodes);
                    MORIS_LOG_INFO( " [%d] num_internal_elems = %d",tRank,mNumInternalElems);
                    MORIS_LOG_INFO( " [%d] num_border_elems   = %d",tRank,mNumBorderElems);
                    MORIS_LOG_INFO( " [%d] num_node_cmaps     = %d",tRank,mNumNodeCmaps);
                    MORIS_LOG_INFO( " [%d] num_elem_cmaps     = %d",tRank,mNumElemCmaps);
                }
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_cmap_params()
        {
            if (par_size() > 1 )
            {
                // Allocate space based on information from load balance parameter calls
                mNodeCmapIds.resize(1,mNumNodeCmaps);
                mNodeCmapNodeCnts.resize(1,mNumNodeCmaps);
                mElemCmapIds.resize(1,mNumElemCmaps);
                mElemCmapElemCnts.resize(1,mNumElemCmaps);

                // get the cmap parameters
                mErrFlag = ex_get_cmap_params(
                        mExoFileId,
                        mNodeCmapIds.data(),
                        mNodeCmapNodeCnts.data(),
                        mElemCmapIds.data(),
                        mElemCmapElemCnts.data(),
                        par_rank());

                MORIS_ERROR(!mErrFlag, "Error reading cmap parameters!");

                if (mVerbose)
                {
                    //               print(node_cmap_ids,"node_cmap_ids");
                    //               print(node_cmap_node_cnts,"node_cmap_node_cnts");
                    //               print(elem_cmap_ids,"elem_cmap_ids");
                    //               print(elem_cmap_elem_cnts,"elem_cmap_elem_cnts");
                }
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_node_cmap()
        {
            if (par_size() > 1 )
            {
                mNodeCmapNodeIds.resize(mNumNodeCmaps);
                mNodeCmapProcIds.resize(mNumNodeCmaps);

                for (unsigned int i=0; i<mNodeCmapNodeIds.size(); ++i)
                {
                    mNodeCmapNodeIds(i).resize(1,mNodeCmapNodeCnts(i));
                    mNodeCmapProcIds(i).resize(1,mNodeCmapNodeCnts(i));

                    mErrFlag = ex_get_node_cmap(
                            mExoFileId,
                            mNodeCmapIds(i),
                            mNodeCmapNodeIds(i).data(),
                            mNodeCmapProcIds(i).data(),
                            par_rank());

                    MORIS_ERROR(!mErrFlag, "Error reading node cmap node and processor ids!");

                    if (mVerbose)
                    {
                        for (unsigned int j=0; j<mNodeCmapNodeIds.size(); ++j)
                            print(mNodeCmapNodeIds(j),"node_cmap_node_ids(j)");

                        // This is basically a vector, all entries of which are = node_cmap_ids[i]
                        // Not sure if it's always guaranteed to be that or what...
                        for (unsigned int j=0; j<mNodeCmapProcIds.size(); ++j)
                            print(mNodeCmapNodeIds(j),"node_cmap_node_ids(j)");
                    }
                }
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_init_global()
        {
            if (par_size() > 1 )
            {
                mErrFlag =ex_get_init_global(
                        mExoFileId,
                        &mNumNodesGlobal,
                        &mNumElemsGlobal,
                        &mNumElemBlksGlobal,
                        &mNumNodeSetsGlobal,
                        &mNumSideSetsGlobal);

                MORIS_ERROR(!mErrFlag, "Error reading initial global data!");

                if (mVerbose)
                {
                    int tRank = par_rank();

                    MORIS_LOG_INFO( " [%d] num_nodes_global     = %d",tRank,mNumNodesGlobal);
                    MORIS_LOG_INFO( " [%d] num_elems_global     = %d",tRank,mNumElemsGlobal);
                    MORIS_LOG_INFO( " [%d] num_elem_blks_global = %d",tRank,mNumElemBlksGlobal);
                    MORIS_LOG_INFO( " [%d] num_node_sets_global = %d",tRank,mNumNodeSetsGlobal);
                    MORIS_LOG_INFO( " [%d] num_side_sets_global = %d",tRank,mNumSideSetsGlobal);
                }
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_eb_info_global()
        {
            if (par_size() > 1 )
            {
                mGlobalElemBlkIds.resize(mNumElemBlksGlobal);
                mGlobalElemBlkCnts.resize(mNumElemBlksGlobal);

                if (mNumElemBlksGlobal > 0)
                {
                    mErrFlag = ex_get_eb_info_global(
                            mExoFileId,
                            mGlobalElemBlkIds.data(),
                            mGlobalElemBlkCnts.data());

                    MORIS_ERROR(!mErrFlag, "Error reading global element block info!");
                }

                if (mVerbose)
                {
                    std::cout << "[" << par_rank() << "] " << "Global Element Block IDs and Counts:" << std::endl;
                    for (std::size_t bn=0; bn<mGlobalElemBlkIds.size(); ++bn)
                    {
                        std::cout << "  [" << par_rank() << "] " <<
                                "global_elem_blk_ids["<<bn<<"]=" << mGlobalElemBlkIds[bn] <<
                                ", global_elem_blk_cnts["<<bn<<"]=" << mGlobalElemBlkCnts[bn] <<
                                std::endl;
                    }
                }
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_node_coords()
        {
            mX.resize(mNumNodes,1);
            mY.resize(mNumNodes,1);

            if (mNumDim < 3)
            {
                mErrFlag = ex_get_coord(mExoFileId,
                        mX.data(),
                        mY.data(),
                        nullptr);
            }
            else
            {
                mZ.resize(mNumNodes,1);

                mErrFlag = ex_get_coord(mExoFileId,
                        mX.data(),
                        mY.data(),
                        mZ.data());
            }

            MORIS_ERROR(!mErrFlag, "get_node_coords filed");
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_ns_param_global()
        {
            if (par_size() > 1 )
            {
                if (mNumNodeSetsGlobal > 0)
                {
                    mGlobalNodesetIds.resize(mNumNodeSetsGlobal);
                    mNumGlobalNodeCounts.resize(mNumNodeSetsGlobal);
                    mNumGlobalNodeDfCounts.resize(mNumNodeSetsGlobal);

                    mErrFlag = ex_get_ns_param_global(
                            mExoFileId,
                            mGlobalNodesetIds.data(),
                            mNumGlobalNodeCounts.data(),
                            mNumGlobalNodeDfCounts.data());

                    MORIS_ERROR(!mErrFlag, "Error reading global nodeset parameters!");

                    if (mVerbose)
                    {
                        std::cout << "[" << par_rank() << "] " << "Global Nodeset IDs, Node Counts, and DF counts:" << std::endl;
                        for (std::size_t bn=0; bn<mGlobalNodesetIds.size(); ++bn)
                        {
                            std::cout << "  [" << par_rank() << "] "
                                    << "global_nodeset_ids["<<bn<<"]=" << mGlobalNodesetIds[bn] <<
                                    ", num_global_node_counts["<<bn<<"]=" << mNumGlobalNodeCounts[bn] <<
                                    ", num_global_node_df_counts["<<bn<<"]=" << mNumGlobalNodeDfCounts[bn] <<
                                    std::endl;
                        }
                    }
                }
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_node_map()
        {
            if (par_size() > 1 )
            {
                mNodeMapi.resize(mNumInternalNodes,1);
                mNodeMapb.resize(mNumBorderNodes,1);
                mNodeMape.resize(mNumExternalNodes,1);

                mErrFlag =ne_get_node_map(
                        mExoFileId,
                        mNodeMapi.data(),
                        mNodeMapb.data(),
                        mNodeMape.data(),
                        par_rank());

                MORIS_ERROR(!mErrFlag, "ne_get_node_map failed!");
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_node_id_map()
        {
            mNodeNumMap.resize(1,mNumNodes);

            mErrFlag = ex_get_id_map(
                    mExoFileId,
                    EX_NODE_MAP,
                    mNodeNumMap.data());

            MORIS_ERROR(!mErrFlag, "get_node_id_map failed.");
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_elem_id_map()
        {
            mElemNumMap.resize(1,mNumElem);

            mErrFlag = ex_get_id_map(
                    mExoFileId,
                    EX_ELEM_MAP,
                    mElemNumMap.data());
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_set_information()
        {
            // Read node sets
            if (mNumNodeSets > 0)
            {
                // Node set information
                mNodeSetIds.resize(mNumNodeSets);

                mErrFlag = ex_get_ids(
                        mExoFileId,
                        EX_NODE_SET,
                        mNodeSetIds.data());

                MORIS_ASSERT(!mErrFlag,"ex_get_ids for node sets sets failed");

                setup_names(
                        int(mNumNodeSets),
                        mNodeSetNamesMemory,
                        mNodeSetNamePtrs);

                mErrFlag = ex_get_names(
                        mExoFileId,
                        EX_NODE_SET,
                        mNodeSetNamePtrs.data());

                MORIS_ASSERT(!mErrFlag,"ex_get_names for node sets sets failed");

                mNodeSetNEntries.resize(mNumNodeSets);
                mNodeSetNodeIds.resize(mNumNodeSets);
                mNodeSetDistFactors.resize(mNumNodeSets);

                for (size_t i = 0; i < mNodeSetIds.size(); ++i)
                {
                    mErrFlag = ex_get_set_param(
                            mExoFileId,
                            EX_NODE_SET,
                            mNodeSetIds[i],
                            &mNodeSetNEntries[i],
                            &mNodeSetDistFactors[i]);

                    MORIS_ASSERT(!mErrFlag,"ex_get_set_param for node sets failed");

                    if (mVerbose) {
                        MORIS_LOG_INFO( " [%d] node set #%d | %s has %d sides, will be surface %d",
                                par_rank(),mNodeSetIds[i],mNodeSetNamePtrs[i],mNodeSetNEntries[i],mNodeSetIds[i]);
                    }

                    mNodeSetNodeIds[i] = Matrix<IndexMat>(1,mNodeSetNEntries[i]);

                    mErrFlag = ex_get_set(
                            mExoFileId,
                            EX_NODE_SET,
                            mNodeSetIds[i],
                            mNodeSetNodeIds[i].data(),
                            nullptr);

                    MORIS_ASSERT(!mErrFlag,"ex_get_set for node sets failed");
                }
            }

            // Read side sets
            if ( mNumSideSets > 0 )
            {
                mSideSetIds.resize(mNumSideSets);

                mErrFlag = ex_get_ids(
                        mExoFileId,
                        EX_SIDE_SET,
                        mSideSetIds.data());

                MORIS_ASSERT(!mErrFlag,"ex_get_ids for side sets failed");

                // get/put side set names
                setup_names(
                        int(mNumSideSets),
                        SideSetNamesMemory,
                        mSideSetNamePtrs);

                mErrFlag = ex_get_names(
                        mExoFileId,
                        EX_SIDE_SET,
                        mSideSetNamePtrs.data());

                MORIS_ASSERT(!mErrFlag,"ex_get_names for side sets failed");

                // Put the side set data into the new file

                mSideSetNEntries.resize(mNumSideSets);
                mSideSetElemIds.resize(mNumSideSets);
                mSideSetSideOrd.resize(mNumSideSets);
                mSideSetDistFactors.resize(mNumSideSets);

                for (size_t i = 0; i < mSideSetIds.size(); ++i)
                {
                    mErrFlag = ex_get_set_param(
                            mExoFileId,
                            EX_SIDE_SET,
                            mSideSetIds[i],
                            &mSideSetNEntries[i],
                            &mSideSetDistFactors[i] );

                    MORIS_ASSERT(!mErrFlag,"ex_get_set_param for side sets failed");

                    if (mVerbose) {
                        MORIS_LOG_INFO( " [%d] side set #%d | %s has %d sides, will be surface %d",
                                par_rank(),mSideSetIds[i],mSideSetNamePtrs[i],mSideSetNEntries[i],mSideSetIds[i]);
                    }

                    mSideSetElemIds[i] = Matrix<IndexMat> (1,mSideSetNEntries[i]);
                    mSideSetSideOrd[i] = Matrix<IndexMat> (1,mSideSetNEntries[i]);

                    mErrFlag = ex_get_set(
                            mExoFileId,
                            EX_SIDE_SET,
                            mSideSetIds[i],
                            mSideSetElemIds[i].data(),
                            mSideSetSideOrd[i].data());

                    MORIS_ASSERT(!mErrFlag,"ex_get_set for side sets failed");
                }
            }

            // Read block sets
            if (mNumElemBlk > 0)
            {
                mBlockIds.resize(mNumElemBlk);

                mErrFlag = ex_get_ids(
                        mExoFileId,
                        EX_ELEM_BLOCK,
                        mBlockIds.data());

                MORIS_ASSERT(!mErrFlag,"ex_get_ids for element blocks failed");

                setup_names(
                        int(mNumElemBlk),
                        mBlockNamesMemory,
                        mBlockNamesPtrs);

                mErrFlag = ex_get_names(
                        mExoFileId,
                        EX_ELEM_BLOCK,
                        mBlockNamesPtrs.data());

                MORIS_ASSERT(!mErrFlag,"ex_get_names for element blocks failed");

                setup_names(
                        int(mNumElemBlk),
                        mBlockElemTypeNamesMemory,
                        mBlockElemTypeNamesPtrs);

                mBlockSetNEntries.resize(mNumElemBlk);
                mBlockSetNNodesPerEntry.resize(mNumElemBlk);
                mBlockSetNedgesPerEntry.resize(mNumElemBlk);
                mBlockSetNfacesPerEntry.resize(mNumElemBlk);
                mBlockSetNattrPerEntry.resize(mNumElemBlk);
                mBlockSetNodeConn.resize(mNumElemBlk);
                mBlockSetEdgeConn.resize(mNumElemBlk);
                mBlockSetFaceConn.resize(mNumElemBlk);

                for (size_t i = 0; i < mBlockIds.size(); ++i)
                {
                    mErrFlag = ex_get_block(
                            mExoFileId,
                            EX_ELEM_BLOCK,
                            mBlockIds[i],
                            mBlockElemTypeNamesPtrs[i],
                            &mBlockSetNEntries[i],
                            &mBlockSetNNodesPerEntry[i],
                            &mBlockSetNedgesPerEntry[i],
                            &mBlockSetNfacesPerEntry[i],
                            &mBlockSetNattrPerEntry[i]);

                    MORIS_ASSERT(!mErrFlag,"ex_get_block failed");

                    if (mVerbose) {
                        MORIS_LOG_INFO( " [%d] block set #%d | %s has %d elements of type %s",
                                par_rank(),mBlockIds[i],mBlockNamesPtrs[i],mBlockSetNEntries[i],mBlockElemTypeNamesPtrs[i]);
                    }

                    mBlockSetNodeConn[i] = Matrix<IndexMat>(1,mBlockSetNEntries[i]*mBlockSetNNodesPerEntry[i]);
                    mBlockSetEdgeConn[i] = Matrix<IndexMat>(1,mBlockSetNEntries[i]*mBlockSetNedgesPerEntry[i]);
                    mBlockSetFaceConn[i] = Matrix<IndexMat>(1,mBlockSetNEntries[i]*mBlockSetNfacesPerEntry[i]);

                    mErrFlag = ex_get_conn(
                            mExoFileId,
                            EX_ELEM_BLOCK,
                            mBlockIds[i],
                            mBlockSetNodeConn[i].data(),
                            mBlockSetEdgeConn[i].data(),
                            mBlockSetFaceConn[i].data());

                    MORIS_ASSERT(!mErrFlag,"ex_get_conn failed");
                }
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_nodal_fields()
        {
            mErrFlag = ex_get_variable_param(
                    mExoFileId,
                    EX_NODAL,
                    &mNumNodalVars);

            MORIS_ASSERT(!mErrFlag,"ex_get_variable_param failed");

            // get number of time steps stored in exodus file
            mNumTimeSteps = ex_inquire_int(mExoFileId, EX_INQ_TIME);

            MORIS_ASSERT(mNumTimeSteps >= 0,"ex_inquire_int with EX_INQ_TIME failed.");

            if(mVerbose)
            {
                MORIS_LOG_INFO( " [%d] number of nodal variables = %d",par_rank(),mNumNodalVars);
                MORIS_LOG_INFO( " [%d] number of time steps      = %d",par_rank(),mNumTimeSteps);
            }

            if(mNumNodalVars == 0)
            {
                return ;
            }

            setup_names(
                    mNumNodalVars,
                    mNodeFieldNamesMemory,
                    mNodeFieldNamePtrs);

            mErrFlag = ex_get_variable_names(
                    mExoFileId,
                    EX_NODAL,
                    mNumNodalVars,
                    mNodeFieldNamePtrs.data());

            MORIS_ASSERT(!mErrFlag,"ex_get_variable_names failed");

            mFieldsNodalVars.resize(mNumNodalVars);

            // loop over all nodal variable field and allocate memory
            for (int i = 0; i < mNumNodalVars; ++i)
            {
                auto name = mNodeFieldNamePtrs[std::size_t(i)];

                if (mVerbose)
                {
                    MORIS_LOG_INFO( " [%d] Nodal variables #%d : %s",par_rank(),i,name);
                }

                mFieldsNodalVars[i] = Matrix<DDRMat>(mNumNodes, 1);
            }

            // read nodal variable fields
            this->reload_nodal_fields();
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::reload_nodal_fields()
        {
            MORIS_ERROR( mTimeStepIndex < mNumTimeSteps, "Requested time step does not exist." );

            for (int i = 0; i < mNumNodalVars; ++i)
            {
                mErrFlag = ex_get_var(
                        mExoFileId,
                        mTimeStepIndex + 1,
                        EX_NODAL, i + 1,
                        0,
                        mNumNodes,
                        mFieldsNodalVars[i].data());

                MORIS_ASSERT(!mErrFlag,"ex_get_var failed");

                mErrFlag = ex_get_time(
                        mExoFileId,
                        mTimeStepIndex + 1,
                        &mTimeValue );

                MORIS_ASSERT(!mErrFlag,"ex_get_time failed");
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::copy_coordinates(int tNewExoFileId)
        {
            const char *tmp[3] = {"x", "y", "z"};
            char *coord_names[3];

            memcpy(coord_names, tmp, sizeof coord_names);

            mErrFlag = ex_put_coord_names(
                    tNewExoFileId,
                    coord_names);

            mErrFlag = ex_put_coord(tNewExoFileId,
                    mX.data(),
                    mY.data(),
                    mZ.data());
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::copy_node_sets(int aNewExoFileId)
        {
            // Put the names onto the new file
            ex_put_names(
                    aNewExoFileId,
                    EX_NODE_SET,
                    mNodeSetNamePtrs.data());

            // Put the side set data into the new file
            for (size_t i = 0; i < mNodeSetIds.size(); ++i) {

                ex_put_set_param(
                        aNewExoFileId,
                        EX_NODE_SET,
                        mNodeSetIds[i],
                        mNodeSetNEntries[i],
                        mNodeSetDistFactors[i]);

                ex_put_set(
                        aNewExoFileId,
                        EX_NODE_SET,
                        mNodeSetIds[i],
                        mNodeSetNodeIds[i].data(),
                        nullptr);
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::copy_side_sets(int aNewExoFileId)
        {
            // Copy over the side sets
            ex_put_names(
                    aNewExoFileId,
                    EX_SIDE_SET,
                    mSideSetNamePtrs.data());

            // Put the side set data into the new file
            for (size_t i = 0; i < mSideSetIds.size(); ++i) {

                ex_put_set_param(
                        aNewExoFileId,
                        EX_SIDE_SET,
                        mSideSetIds[i],
                        mSideSetNEntries[i],
                        mSideSetDistFactors[i]);

                if (mVerbose)
                {
                    MORIS_LOG_INFO( " Side set # %d | %d has %d sides, will be surface %d",mSideSetIds[i],mSideSetNamePtrs[i],mSideSetNEntries[i],mSideSetIds[i]);
                }

                ex_put_set(
                        aNewExoFileId,
                        EX_SIDE_SET,
                        mSideSetIds[i],
                        mSideSetElemIds[i].data(),
                        mSideSetSideOrd[i].data());
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::copy_block_sets(int aNewExoFileId)
        {
            ex_put_names(aNewExoFileId, EX_ELEM_BLOCK, mBlockNamesPtrs.data());

            for (size_t i = 0; i < mBlockIds.size(); ++i)
            {
                mErrFlag = ex_put_block(
                        aNewExoFileId,
                        EX_ELEM_BLOCK,
                        mBlockIds[i],
                        mBlockElemTypeNamesPtrs[i],
                        mBlockSetNEntries[i],
                        mBlockSetNNodesPerEntry[i],
                        mBlockSetNedgesPerEntry[i],
                        mBlockSetNfacesPerEntry[i],
                        mBlockSetNattrPerEntry[i]);

                MORIS_ASSERT(!mErrFlag,"ex_put_block failed");

                mErrFlag = ex_put_conn(
                        aNewExoFileId,
                        EX_ELEM_BLOCK,
                        mBlockIds[i],
                        mBlockSetNodeConn[i].data(),
                        mBlockSetEdgeConn[i].data(),
                        mBlockSetFaceConn[i].data());

                MORIS_ASSERT(!mErrFlag,"ex_put_conn failed");
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::copy_nodal_fields(
                int              aNewExoFileId,
                ex_init_params & init_params)
        {
            if(mVerbose)
            {
                std::cout << mNumNodalVars << " nodal variables\n";
            }

            if(mNumNodalVars == 0)
            {
                return ;
            }

            int time_step = 0;

            ex_put_variable_param(
                    aNewExoFileId,
                    EX_NODAL,
                    mNumNodalVars);

            ex_put_variable_names(
                    aNewExoFileId,
                    EX_NODAL,
                    mNumNodalVars,
                    mNodeFieldNamePtrs.data());

            for (int i = 0; i < mNumNodalVars; ++i) {

                auto name = mNodeFieldNamePtrs[std::size_t(i)];

                if (mVerbose)
                {
                    std::cout << "Loading nodal variable \"" << name << "\" at time step "
                            << time_step << '\n';
                }

                ex_put_var(
                        aNewExoFileId,
                        time_step + 1,
                        EX_NODAL,
                        i + 1,
                        /*obj_id*/ 0,
                        mNumNodes,
                        mFieldsNodalVars[i].data());
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::get_file_name(
                const char *base,
                const char *other,
                char       *output)
        {
            int nprocs = par_size();
            int rank = par_rank();
            int  i1, iTemp1;
            int  iMaxDigit = 0, iMyDigit = 0;
            char cTemp[128];

            output[0] = '\0';

            strcpy(output, base);

            if (other != NULL)
            {
                strcat(output, ".");
                strcat(output, other);
            }

            if (nprocs > 1)
            {
                /*
                 * Find out the number of digits needed to specify the processor ID.
                 * This allows numbers like 01-99, i.e., prepending zeros to the
                 * name to preserve proper alphabetic sorting of the files.
                 */

                iTemp1 = nprocs;
                do {
                    iTemp1 /= 10;
                    iMaxDigit++;
                } while (iTemp1 >= 1);

                iTemp1 = rank;
                do {
                    iTemp1 /= 10;
                    iMyDigit++;
                } while (iTemp1 >= 1);

                strcat(output, ".");
                sprintf(cTemp, "%d", nprocs);
                strcat(output, cTemp);
                strcat(output, ".");

                /*
                 * Append the proper number of zeros to the filename.
                 */
                for (i1 = 0; i1 < iMaxDigit - iMyDigit; i1++) {
                    strcat(output, "0");
                }

                sprintf(cTemp, "%d", rank);
                strcat(output, cTemp);
            }
        }

        // ---------------------------------------------------------------------------------------------

        void
        Exodus_IO_Helper::setup_names(
                int nnames, std::vector<char>& storage,
                std::vector<char*>           & ptrs)
        {
            constexpr auto max_name_length = MAX_STR_LENGTH + 1;

            storage = std::vector<char>(std::size_t(nnames * max_name_length), '\0');
            ptrs    = std::vector<char*>(std::size_t(nnames), nullptr);

            for (int i = 0; i < nnames; ++i)
            {
                ptrs[std::size_t(i)] = storage.data() + max_name_length * i;
            }
        }

        // ---------------------------------------------------------------------------------------------

        moris::Matrix<DDRMat>
        Exodus_IO_Helper::get_nodal_coordinate( uint aNodeId )
        {
            // find index of node given its nodeId
            uint tIndex = this->get_node_index_by_Id(aNodeId);

            // initialize nodal coordinate vector
            Matrix<DDRMat> tNodalVec(mNumDim,1,0);

            // fill nodal coordinate vector
            tNodalVec(0,0) = mX( tIndex );
            tNodalVec(1,0) = mY( tIndex );

            if (mNumDim == 3)
            {
                tNodalVec(2,0) = mZ( tIndex );
            }

            return tNodalVec;
        }

        // ---------------------------------------------------------------------------------------------

        real
        Exodus_IO_Helper::get_nodal_field_value( uint aNodeId, uint aFieldIndex, uint aTimeStepIndex)
        {
            // find index of node given its nodeId
            uint tIndex = this->get_node_index_by_Id(aNodeId);

            // check that field exists
            MORIS_ERROR( aFieldIndex < mFieldsNodalVars.size(), "Nodal field index out of bounds.");

            // check if loaded time step is requested time step; if not load new time step
            if ( aTimeStepIndex != (uint) mTimeStepIndex )
            {
                mTimeStepIndex = aTimeStepIndex;

                this->reload_nodal_fields();
            }

            return mFieldsNodalVars[aFieldIndex](tIndex);
        }
    }
}