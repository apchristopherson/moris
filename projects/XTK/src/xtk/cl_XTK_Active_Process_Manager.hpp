/*
 * cl_XTK_Active_Process_Manager.hpp
 *
 *  Created on: Jun 28, 2017
 *      Author: ktdoble
 */

#ifndef SRC_XTK_CL_XTK_ACTIVE_PROCESS_MANAGER_HPP_
#define SRC_XTK_CL_XTK_ACTIVE_PROCESS_MANAGER_HPP_

// Standard Include
#include <limits>

// XTKL: Mesh Includes
#include "mesh/cl_Mesh_Data.hpp"
#include "mesh/cl_Mesh_Enums.hpp"

// XTKL: Container Includes
#include "linalg/cl_XTK_Matrix.hpp"
#include "containers/cl_XTK_Cell.hpp"

// XTKL: Logging and Assertion Includes
#include "assert/fn_xtk_assert.hpp"

namespace xtk
{
template<typename Real, typename Integer, typename Real_Matrix, typename Integer_Matrix>
class Active_Process_Manager
{
public:
    // Forward declaration
    Integer INTEGER_MAX = std::numeric_limits<Integer>::max();

    // this class minimizes the amount of information needed to ship regarding the mesh
    // This function does not check to see if
    Active_Process_Manager(bool aSendFlag,
                           Integer aNumChildren,
                           Integer aNumProcessors,
                           enum EntityRank aEntityRank,
                           mesh::Mesh_Data<Real, Integer, Real_Matrix, Integer_Matrix> & aMeshRef) :
    mSendFlag(aSendFlag),
    mCondensedFlag(false),
    mHasInformation(false),
    mNumChildren(aNumChildren),
    mActiveCount(0),
    mNumProcessors(aNumProcessors-1),
    mEntityRank(aEntityRank),
    mMeshReference(aMeshRef),
    mProcTracker(aNumProcessors, 3, 0),
    mActiveProcTracker(aNumProcessors-1, 2, 0)
    {
        XTK_ASSERT(mActiveCount==0,"The active count needs to start at 0.");
        mActiveInfoToCommunicate = Cell<moris::Matrix<Integer, Integer_Matrix>>(aNumProcessors-1);

        for( Integer i = 0; i<aNumProcessors-1; i++)
        {
            mActiveInfoToCommunicate(i) = moris::Matrix<Integer, Integer_Matrix>(0, 0, 0);
        }

    }

    ~Active_Process_Manager()
    {
    }

    void set_communication_info(Integer aEntityIndex, Integer aSecondaryTag, Integer aFirstAvailableID, Integer aOtherProcRank)
    {

        // If the entity is equivalent to the one in the communication list
        // NOTE: this is the parent entity Id
        Integer tGlbEntityId = mMeshReference.get_glb_entity_id_from_entity_loc_index(aEntityIndex, mEntityRank);

        // Check to see if processor is active
        // If it is not active yet, activate it
        if (mProcTracker(aOtherProcRank, 0) == 0)
        {
            // Mark the flag as true so that this data structure believes it has important things to send
            mHasInformation = true;

            // Get the size of the relevant communication list
            Integer tNumInCommListInd = mMeshReference.get_num_of_entities_shared_with_processor(aOtherProcRank, mEntityRank, mSendFlag);

            // Initialize communication matrix to maximum
            if (mNumChildren == 1)
            {
                moris::Matrix<Integer, Integer_Matrix> tCommunicationAllocator(3, tNumInCommListInd + 1, INTEGER_MAX);
                mActiveInfoToCommunicate(mActiveCount) =  tCommunicationAllocator;
            }

            else
            {
                moris::Matrix<Integer, Integer_Matrix> tCommunicationAllocator(3, (tNumInCommListInd + 1) * mNumChildren, INTEGER_MAX);
                mActiveInfoToCommunicate(mActiveCount) =  tCommunicationAllocator;
            }

            // Tell process tracker which index process has been assigned
            mProcTracker(aOtherProcRank, 1) = mActiveCount;

            // Mark as active
            mProcTracker(aOtherProcRank, 0) = 1;

            // Tell Active process tracker which other process rank
            mActiveProcTracker(mActiveCount, 0) = aOtherProcRank;

            // Advance active count
            mActiveCount++;
        }

        // Get active index
        Integer tActInd = mProcTracker(aOtherProcRank, 1);

        // Tell communication matrix entity Index and Id assigned on it
        mActiveInfoToCommunicate(tActInd)(0, mActiveProcTracker(tActInd, 1)) = tGlbEntityId;
        mActiveInfoToCommunicate(tActInd)(1, mActiveProcTracker(tActInd, 1)) = aSecondaryTag;
        mActiveInfoToCommunicate(tActInd)(2, mActiveProcTracker(tActInd, 1)) = aFirstAvailableID;

        // Advance counter
        mActiveProcTracker(tActInd, 1)++;}

    void condense_info()
    {
        XTK_ASSERT(!mCondensedFlag,"Data in active process manager has already been condensed. Multiple Calls to condense_info detected. ");
        mCondensedFlag=true;

        // Loop over communication matrices
        // Resize first
        // Then communicate using MPI tools
        if (!mHasInformation)
        {
            mActiveProcTracker.set_size(1, 1);
        }
        else
        {

            for (Integer tp = 0; tp < mNumProcessors; tp++)
            {
                if (tp < mActiveCount)
                {
                    // Condense communication matrix
                    mActiveInfoToCommunicate(tp).resize(3, mActiveProcTracker(tp, 1));
                }
                else
                {
                    mActiveInfoToCommunicate.pop_back();
                }
            }

            mActiveProcTracker.resize(mActiveCount, 1);
        }
    }

    /**
     *Specifies whether or not this active process manager has any information
     */
    bool has_information()
    {
        return mHasInformation;
    }

   moris::Matrix<Integer, Integer_Matrix> &
    get_comm_info(Integer aInformationIndex)
    {
        XTK_ASSERT(mCondensedFlag,"Data in active process manager has not been condensed. Before using this function condense_info must be called");
        XTK_ASSERT(aInformationIndex<get_num_active_processors(),"Index out of bounds error in get_comm_info. Attempted to access information which does not exist");
        return mActiveInfoToCommunicate(aInformationIndex);
    }

    int
    get_active_processor_rank(Integer aInformationIndex)
    {
        XTK_ASSERT(mCondensedFlag,"Data in active process manager has not been condensed. Before using this function condense_info must be called");
        XTK_ASSERT(aInformationIndex<get_num_active_processors(),"Index out of bounds error in get_active_processor. Attempted to access information which does not exist");
        return (int)mActiveProcTracker(aInformationIndex,0);
    }

    Integer
    get_num_active_processors()
    {
        XTK_ASSERT(mCondensedFlag,"Data in active process manager has not been condensed. Before using this function condense_info must be called");
        return mActiveProcTracker.n_rows();
    }


    void print_communication_matrix()
    {

    }

private:
    bool mSendFlag;
    bool mCondensedFlag;
    bool mHasInformation;
    Integer mNumChildren;
    Integer mActiveCount;
    Integer mNumProcessors;
    enum EntityRank mEntityRank;
    mesh::Mesh_Data<Real, Integer, Real_Matrix, Integer_Matrix> & mMeshReference;
    moris::Matrix<Integer, Integer_Matrix> mProcTracker;
    moris::Matrix<Integer, Integer_Matrix> mActiveProcTracker;
    Cell<moris::Matrix<Integer, Integer_Matrix>> mActiveInfoToCommunicate;


};
}


#endif /* SRC_XTK_CL_XTK_ACTIVE_PROCESS_MANAGER_HPP_ */