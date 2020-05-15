#include "cl_GEN_Pdv_Host_Manager.hpp"
#include "fn_sum.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        Pdv_Host_Manager::Pdv_Host_Manager()
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        Pdv_Host_Manager::~Pdv_Host_Manager()
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ip_dv_types_for_set(const moris::moris_index aIPMeshSetIndex, Cell<Cell<PDV_Type>>& aPdvTypes)
        {
            if (mIpPdvTypes.size() > 0)
            {
                aPdvTypes = mIpPdvTypes(aIPMeshSetIndex);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ig_dv_types_for_set(const moris::moris_index aIGMeshSetIndex, Cell<Cell<PDV_Type>>& aPdvTypes)
        {
            if (mIgPdvTypes.size() > 0)
            {
                aPdvTypes = mIgPdvTypes(aIGMeshSetIndex);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ip_unique_dv_types_for_set(const moris_index aIPMeshSetIndex, Cell<PDV_Type>& aPdvTypes)
        {
            if (mUniqueIpPdvTypes.size() > 0)
            {
                aPdvTypes =  mUniqueIpPdvTypes(aIPMeshSetIndex);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ig_unique_dv_types_for_set(const moris::moris_index aIGMeshSetIndex, Cell<PDV_Type>& aPdvTypes)
        {
            if (mUniqueIgPdvTypes.size() > 0)
            {
                aPdvTypes =  mUniqueIgPdvTypes(aIGMeshSetIndex);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ip_pdv_value(const Matrix<IndexMat>&   aNodeIndices,
                                                const Cell<PDV_Type>&       aPdvTypes,
                                                Cell<Matrix<DDRMat>>&     aDvValues)
        {
            // get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // get the number of dv types requested
            uint tNumTypes = aPdvTypes.size();

            // set size for list of dv values
            aDvValues.resize(tNumTypes);

            // loop over the node indices
            for (uint iInd = 0; iInd < tNumIndices; iInd++)
            {
                // loop over the requested dv types
                for (uint iType = 0; iType < tNumTypes; iType++)
                {
                    aDvValues(iType).resize(tNumIndices, 1);
                    aDvValues(iType)(iInd) = mIpPdvHosts(aNodeIndices(iInd))->get_pdv_value(aPdvTypes(iType));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ip_pdv_value(const Matrix<IndexMat>&   aNodeIndices,
                                                const Cell<PDV_Type>&       aPdvTypes,
                                                Cell<Matrix<DDRMat>>&     aDvValues,
                                                Cell<Matrix<DDSMat>>&     aIsActiveDv)
        {
            get_ip_pdv_value(aNodeIndices, aPdvTypes, aDvValues);

            // get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // get the number of dv types requested
            uint tNumTypes = aPdvTypes.size();

            // set size for list of active flags
            aIsActiveDv.resize(tNumIndices);

            // loop over the node indices
            for (uint iInd = 0; iInd < tNumIndices; iInd++)
            {
                aIsActiveDv(iInd).resize(tNumTypes, 1);

                // loop over the requested dv types
                for (uint iType = 0; iType < tNumTypes; iType++)
                {
                    aIsActiveDv(iInd)(iType) = mIpPdvHosts(aNodeIndices(iInd))->is_active_type(aPdvTypes(iType));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ig_pdv_value(const Matrix<IndexMat>&   aNodeIndices,
                                                const Cell<PDV_Type>&       aPdvTypes,
                                                Cell<Matrix<DDRMat>>&     aDvValues)
        {
            // get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // get the number of dv types requested
            uint tNumTypes = aPdvTypes.size();

            // set size for list of dv values
            aDvValues.resize(tNumTypes);

            // loop over the node indices
            for (uint iInd = 0; iInd < tNumIndices; iInd++)
            {
                // loop over the requested dv types
                for (uint iType = 0; iType < tNumTypes; iType++)
                {
                    aDvValues(iType).resize(tNumIndices, 1);
                    aDvValues(iType)(iInd) = mIgPdvHosts(aNodeIndices(iInd))->get_pdv_value(aPdvTypes(iType));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ig_pdv_value(const Matrix<IndexMat>&   aNodeIndices,
                                                const Cell<PDV_Type>&       aPdvTypes,
                                                Cell<Matrix<DDRMat>>&     aDvValues,
                                                Cell<Matrix<DDSMat>>&     aIsActiveDv)
        {
            // get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // get the number of dv types requested
            uint tNumTypes = aPdvTypes.size();

            // set size for list of active flags
            aIsActiveDv.resize(tNumIndices);

            // loop over the node indices
            for (uint iInd = 0; iInd < tNumIndices; iInd++)
            {
                aIsActiveDv(iInd).resize(tNumTypes, 1);

                // loop over the requested dv types
                for (uint iType = 0; iType < tNumTypes; iType++)
                {
                    aIsActiveDv(iInd)(iType) = mIgPdvHosts(aNodeIndices(iInd))->is_active_type(aPdvTypes(iType));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix<DDSMat> & Pdv_Host_Manager::get_my_local_global_map()
        {
            return mGlobalPdvTypeMap;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ip_dv_ids_for_type_and_ind(const Matrix<IndexMat>&    aNodeIndices,
                                                              const Cell<PDV_Type>&         aPdvTypes,
                                                              Cell<Matrix<IdMat>>&        aDvIds)
        {
            // get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // get the number of dv types requested
            uint tNumTypes = aPdvTypes.size();

            // set size for list of dv values
            aDvIds.resize(tNumTypes);

            // loop over the node indices
            for (uint iInd = 0; iInd < tNumIndices; iInd++)
            {
                // loop over the requested dv types
                for (uint iType = 0; iType < tNumTypes; iType++)
                {
                    aDvIds(iType).resize(tNumIndices, 1);
                    aDvIds(iType)(iInd) = mIpPdvHosts(aNodeIndices(iInd))->get_global_index_for_pdv_type(aPdvTypes(iType));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ig_dv_ids_for_type_and_ind(const Matrix<IndexMat>&    aNodeIndices,
                                                              const Cell<PDV_Type>&         aPdvTypes,
                                                              Cell<Matrix<IdMat>>&        aDvIds)
        {
            // get the number of node indices requested
            uint tNumIndices = aNodeIndices.length();

            // get the number of dv types requested
            uint tNumTypes = aPdvTypes.size();

            // set size for list of dv values
            aDvIds.resize(tNumTypes);

            // loop over the node indices
            for (uint iInd = 0; iInd < tNumIndices; iInd++)
            {
                // loop over the requested dv types
                for (uint iType = 0; iType < tNumTypes; iType++)
                {
                    aDvIds(iType).resize(tNumIndices, 1);
                    aDvIds(iType)(iInd) = mIgPdvHosts(aNodeIndices(iInd))->get_global_index_for_pdv_type(aPdvTypes(iType));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ip_requested_dv_types( Cell< PDV_Type > & aPdvTypes )
        {
            aPdvTypes =  mRequestedIpPdvTypes;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::get_ig_requested_dv_types( Cell< PDV_Type > & aPdvTypes )
        {
            aPdvTypes =  mRequestedIgPdvTypes;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::assign_property_to_pdv_type_by_vertex_index(std::shared_ptr<GEN_Property> aPropertyPointer,
                                                                           PDV_Type                        aPdvType,
                                                                           moris_index                   aNodeIndex)
        {
            // Check if PDV_Type host exists
            MORIS_ASSERT(mIpPdvHosts(aNodeIndex) != nullptr, "PDV_Type attempted to be created via property when PDV_Type host doesn't exist yet.");

            // get the pdv host and create the pdv for dv type
            mIpPdvHosts(aNodeIndex)->create_pdv(aPdvType, aPropertyPointer);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::assign_field_to_pdv_type_by_vertex_index(std::shared_ptr<GEN_Field>   aFieldPointer,
                                                                        PDV_Type                       aPdvType,
                                                                        moris_index                  aNodeIndex)
        {
            // Check if PDV_Type host exists
            MORIS_ASSERT(mIpPdvHosts(aNodeIndex) != nullptr, "PDV_Type attempted to be created via field when PDV_Type host doesn't exist yet.");
        
            // get the pdv host and create the pdv for dv type
            mIpPdvHosts(aNodeIndex)->create_pdv(aPdvType, aFieldPointer, aNodeIndex);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::create_ip_pdv_hosts(uint aTotalNodes, Cell<Matrix<DDSMat>> aNodeIndicesPerSet, Cell<Cell<Cell<PDV_Type>>> aPdvTypes)
        {
            // Check that number of sets is consistent
            uint tNumSets = aPdvTypes.size();
            MORIS_ERROR(tNumSets == aNodeIndicesPerSet.size(),
                    "Information passed to Pdv_Host_Manager.create_ip_pdv_hosts() does not have a consistent number of sets!");

            // Set PDV_Type types
            mIpPdvTypes = aPdvTypes;
            mUniqueIpPdvTypes.resize(tNumSets);

            // Initialize PDV_Type hosts
            mIpPdvHosts.resize(aTotalNodes, nullptr);

            // Create PDV_Type hosts
            for (uint tMeshSetIndex = 0; tMeshSetIndex < tNumSets; tMeshSetIndex++)
            {
                // Get number of unique PDVs
                uint tNumUniquePdvs = 0;
                for (uint tGroupIndex = 0; tGroupIndex < mIpPdvTypes(tMeshSetIndex).size(); tGroupIndex++)
                {
                    tNumUniquePdvs += mIpPdvTypes(tMeshSetIndex)(tGroupIndex).size();
                }
                mUniqueIpPdvTypes(tMeshSetIndex).resize(tNumUniquePdvs);

                // Copy PDV_Type types over
                uint tUniquePdvIndex = 0;
                for (uint tGroupIndex = 0; tGroupIndex < mIpPdvTypes(tMeshSetIndex).size(); tGroupIndex++)
                {
                    for (uint tPdvIndex = 0; tPdvIndex < mIpPdvTypes(tMeshSetIndex)(tGroupIndex).size(); tPdvIndex++)
                    {
                        mUniqueIpPdvTypes(tMeshSetIndex)(tUniquePdvIndex++) = mIpPdvTypes(tMeshSetIndex)(tGroupIndex)(tPdvIndex);
                    }
                }

                // Create PDV_Type hosts
                for (uint tNodeIndexOnSet = 0; tNodeIndexOnSet < aNodeIndicesPerSet(tMeshSetIndex).length(); tNodeIndexOnSet++)
                {
                    // Create new host or add unique PDVs
                    uint tNumAddedPdvs = tNumUniquePdvs;
                    if (mIpPdvHosts(aNodeIndicesPerSet(tMeshSetIndex)(tNodeIndexOnSet)) == nullptr)
                    {
                        mIpPdvHosts(aNodeIndicesPerSet(tMeshSetIndex)(tNodeIndexOnSet))
                        = std::make_shared<Pdv_Host>(mUniqueIpPdvTypes(tMeshSetIndex), mGlobalPdvIndex);
                    }
                    else
                    {
                        tNumAddedPdvs = mIpPdvHosts(aNodeIndicesPerSet(tMeshSetIndex)(tNodeIndexOnSet))->add_pdv_types(mUniqueIpPdvTypes(tMeshSetIndex), mGlobalPdvIndex);
                    }

                    // Resize global map
                    mGlobalPdvTypeMap.resize(mGlobalPdvTypeMap.length() + tNumAddedPdvs, 1);

                    // Update global PDV_Type indices
                    for (uint tPdvIndex = 0; tPdvIndex < tNumAddedPdvs; tPdvIndex++)
                    {
                        mGlobalPdvTypeMap(mGlobalPdvIndex) = mGlobalPdvIndex;
                        mGlobalPdvIndex++;
                    }
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::create_ig_pdv_hosts(uint aTotalNodes, Cell<Matrix<DDSMat>> aNodeIndicesPerSet, Cell<Cell<Cell<PDV_Type>>> aPdvTypes)
        {
            // Check that number of sets is consistent
            uint tNumSets = aPdvTypes.size();
            MORIS_ERROR(tNumSets == aNodeIndicesPerSet.size(),
            "Information passed to Pdv_Host_Manager.create_ig_pdv_hosts() does not have a consistent number of sets!");

            // Set PDV_Type types
            mIgPdvTypes = aPdvTypes;
            mUniqueIgPdvTypes.resize(tNumSets);

            // Initialize PDV_Type hosts
            mIgPdvHosts.resize(aTotalNodes, nullptr);

            // Create PDV_Type hosts
            for (uint tMeshSetIndex = 0; tMeshSetIndex < tNumSets; tMeshSetIndex++)
            {
                // Get number of unique PDVs
                uint tNumUniquePdvs = 0;
                for (uint tGroupIndex = 0; tGroupIndex < mIgPdvTypes(tMeshSetIndex).size(); tGroupIndex++)
                {
                    tNumUniquePdvs += mIgPdvTypes(tMeshSetIndex)(tGroupIndex).size();
                }
                mUniqueIgPdvTypes(tMeshSetIndex).resize(tNumUniquePdvs);

                // Copy PDV_Type types over
                uint tUniquePdvIndex = 0;
                for (uint tGroupIndex = 0; tGroupIndex < mIgPdvTypes(tMeshSetIndex).size(); tGroupIndex++)
                {
                    for (uint tPdvIndex = 0; tPdvIndex < mIgPdvTypes(tMeshSetIndex)(tGroupIndex).size(); tPdvIndex++)
                    {
                        mUniqueIgPdvTypes(tMeshSetIndex)(tUniquePdvIndex++) = mIgPdvTypes(tMeshSetIndex)(tGroupIndex)(tPdvIndex);
                    }
                }

                // Create PDV_Type hosts
                for (uint tNodeIndexOnSet = 0; tNodeIndexOnSet < aNodeIndicesPerSet(tMeshSetIndex).length(); tNodeIndexOnSet++)
                {
                    // Create new host or add unique PDVs
                    uint tNumAddedPdvs = tNumUniquePdvs;
                    if (mIgPdvHosts(aNodeIndicesPerSet(tMeshSetIndex)(tNodeIndexOnSet)) == nullptr)
                    {
                        mIgPdvHosts(aNodeIndicesPerSet(tMeshSetIndex)(tNodeIndexOnSet))
                        = std::make_shared<Pdv_Host>(mUniqueIgPdvTypes(tMeshSetIndex), mGlobalPdvIndex);
                    }
                    else
                    {
                        tNumAddedPdvs = mIgPdvHosts(aNodeIndicesPerSet(tMeshSetIndex)(tNodeIndexOnSet))->add_pdv_types(mUniqueIgPdvTypes(tMeshSetIndex), mGlobalPdvIndex);
                    }

                    // Resize global map
                    mGlobalPdvTypeMap.resize(mGlobalPdvTypeMap.length() + tNumAddedPdvs, 1);

                    // Update global PDV_Type indices
                    for (uint tPdvIndex = 0; tPdvIndex < tNumAddedPdvs; tPdvIndex++)
                    {
                        mGlobalPdvTypeMap(mGlobalPdvIndex) = mGlobalPdvIndex;
                        mGlobalPdvIndex++;
                    }
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::mark_ip_pdv_as_inactive(moris_index aNodeIndex, PDV_Type aPdvType)
        {
            mIpPdvHosts(aNodeIndex)->mark_pdv_as_inactive(aPdvType);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::mark_ig_pdv_as_inactive(moris_index aNodeIndex, PDV_Type aPdvType)
        {
            mIgPdvHosts(aNodeIndex)->mark_pdv_as_inactive(aPdvType);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::set_ip_requested_dv_types(Cell<PDV_Type>& aPdvTypes)
        {
            mRequestedIpPdvTypes = aPdvTypes;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::set_ig_requested_dv_types(Cell<PDV_Type>& aPdvTypes)
        {
            mRequestedIgPdvTypes = aPdvTypes;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::create_ip_pdv(uint aNodeIndex, PDV_Type aPdvType, std::shared_ptr<GEN_Field> aFieldPointer)
        {
            mIpPdvHosts(aNodeIndex)->create_pdv(aPdvType, aFieldPointer, aNodeIndex);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::create_ip_pdv(uint aNodeIndex, PDV_Type aPdvType, std::shared_ptr<GEN_Property> aPropertyPointer)
        {
            mIpPdvHosts(aNodeIndex)->create_pdv(aPdvType, aPropertyPointer);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::create_ip_pdv(uint aNodeIndex, PDV_Type aPdvType, moris::real aPdvVal)
        {
            mIpPdvHosts(aNodeIndex)->create_pdv(aPdvType, aPdvVal);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Host_Manager::create_ig_pdv(uint aNodeIndex, PDV_Type aPdvType, moris::real aPdvVal)
        {
            mIgPdvHosts(aNodeIndex)->create_pdv(aPdvType, aPdvVal);
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
