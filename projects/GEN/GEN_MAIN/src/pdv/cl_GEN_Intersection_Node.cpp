#include "cl_GEN_Intersection_Node.hpp"
#include "cl_GEN_Geometry.hpp"
#include "fn_norm.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        Intersection_Node::Intersection_Node(
                real                               aLocalCoordinate,
                std::shared_ptr<Intersection_Node> aFirstParentNode,
                std::shared_ptr<Intersection_Node> aSecondParentNode,
                real                               aFirstParentNodeIndex,
                real                               aSecondParentNodeIndex,
                const Matrix<DDRMat>&              aFirstParentNodeLocalCoordinates,
                const Matrix<DDRMat>&              aSecondParentNodeLocalCoordinates,
                Matrix<DDUMat>                     aAncestorNodeIndices,
                Cell<Matrix<DDRMat>>               aAncestorNodeCoordinates,
                const xtk::Basis_Function&         aAncestorBasisFunction,
                std::shared_ptr<Geometry>          aInterfaceGeometry,
                real                               aIsocontourThreshold,
                real                               aIsocontourTolerance,
                real                               aIntersectionTolerance)
                : Child_Node(
                        aAncestorNodeIndices,
                        aAncestorNodeCoordinates,
                        aAncestorBasisFunction,
                        0.5 * ((1 - aLocalCoordinate) * aFirstParentNodeLocalCoordinates + (1 + aLocalCoordinate) * aSecondParentNodeLocalCoordinates)),
                  mLocalCoordinate(aLocalCoordinate),
                  mIsIntersected(false),
                  mInterfaceGeometry(aInterfaceGeometry),
                  mIsocontourThreshold(aIsocontourThreshold),
                  mFirstParentNode(aFirstParentNode),
                  mSecondParentNode(aSecondParentNode)
        {
            // Parent basis
            Matrix<DDRMat> tFirstParentBasisValues;
            Matrix<DDRMat> tSecondParentBasisValues;
            aAncestorBasisFunction.evaluate_basis_function(aFirstParentNodeLocalCoordinates, tFirstParentBasisValues);
            aAncestorBasisFunction.evaluate_basis_function(aSecondParentNodeLocalCoordinates, tSecondParentBasisValues);

            // Global coordinates of intersection and parents
            mGlobalCoordinates = mBasisValues(0) * aAncestorNodeCoordinates(0);
            Matrix<DDRMat> tFirstParentGlobalCoordinates = tFirstParentBasisValues(0) * aAncestorNodeCoordinates(0);
            Matrix<DDRMat> tSecondParentGlobalCoordinates = tSecondParentBasisValues(0) * aAncestorNodeCoordinates(0);
            for (uint tBasisIndex = 1; tBasisIndex < mBasisValues.length(); tBasisIndex++)
            {
                mGlobalCoordinates += mBasisValues(tBasisIndex) * aAncestorNodeCoordinates(tBasisIndex);
                tFirstParentGlobalCoordinates += tFirstParentBasisValues(tBasisIndex) * aAncestorNodeCoordinates(tBasisIndex);
                tSecondParentGlobalCoordinates += tSecondParentBasisValues(tBasisIndex) * aAncestorNodeCoordinates(tBasisIndex);
            }

            // Parents on interface
            real tFirstParentPhi = aInterfaceGeometry->get_field_value(aFirstParentNodeIndex, tFirstParentGlobalCoordinates);
            real tSecondParentPhi = aInterfaceGeometry->get_field_value(aSecondParentNodeIndex, tSecondParentGlobalCoordinates);

            real tParentLength = norm(tSecondParentGlobalCoordinates - tFirstParentGlobalCoordinates);
            mFirstParentOnInterface = std::abs(tFirstParentPhi) < aIsocontourTolerance or
                    0.5 * tParentLength * std::abs(1 + aLocalCoordinate) < aIntersectionTolerance;
            mSecondParentOnInterface = std::abs(tSecondParentPhi) < aIsocontourTolerance or
                    0.5 * tParentLength * std::abs(1 - aLocalCoordinate) < aIntersectionTolerance;

            if (mFirstParentOnInterface or mSecondParentOnInterface)
            {
                mIsIntersected = true;
            }
            else
            {
                mIsIntersected = (std::abs(mLocalCoordinate) <= 1.0);
            }

        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Intersection_Node::get_dcoordinate_dadv()
        {
            // Set size
            mCoordinateSensitivities.set_size(0, 0);

            // Locked interface geometry
            std::shared_ptr<Geometry> tLockedInterfaceGeometry = mInterfaceGeometry.lock();

            // Get sensitivity values from other ancestors
            Matrix<DDRMat> tSensitivitiesToAdd;
            for (uint tAncestorNode = 0; tAncestorNode < mAncestorNodeIndices.length(); tAncestorNode++)
            {
                // Get geometry field sensitivity with respect to ADVs
                const Matrix<DDRMat>& tFieldSensitivities = tLockedInterfaceGeometry->get_field_sensitivities(
                        mAncestorNodeIndices(tAncestorNode),
                        mAncestorNodeCoordinates(tAncestorNode));

                // Ancestor sensitivities
                tSensitivitiesToAdd = 0.5 * this->get_dcoordinate_dfield_from_ancestor(tAncestorNode) *
                        trans( trans(mAncestorNodeCoordinates(1) - mAncestorNodeCoordinates(0)) * tFieldSensitivities );

                // Join sensitivities
                this->join_coordinate_sensitivities(tSensitivitiesToAdd);
            }

            // Add first parent sensitivities, if needed
            if (mFirstParentNode)
            {
                tSensitivitiesToAdd = 0.5 * (1 - mLocalCoordinate) * mFirstParentNode->get_dcoordinate_dadv();
                this->join_coordinate_sensitivities(tSensitivitiesToAdd);
            }

            // Add second parent sensitivities, if needed
            if (mSecondParentNode)
            {
                tSensitivitiesToAdd = 0.5 * (1 + mLocalCoordinate) * mSecondParentNode->get_dcoordinate_dadv();
                this->join_coordinate_sensitivities(tSensitivitiesToAdd);
            }

            return mCoordinateSensitivities;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDSMat> Intersection_Node::get_coordinate_determining_adv_ids()
        {
            // Set size
            mCoordinateDeterminingADVIDs.set_size(0, 0);

            // Locked interface geometry
            std::shared_ptr<Geometry> tLockedInterfaceGeometry = mInterfaceGeometry.lock();

            // Get sensitivity values from other ancestors
            for (uint tAncestorNode = 0; tAncestorNode < mAncestorNodeIndices.length(); tAncestorNode++)
            {
                // Get geometry field sensitivity with respect to ADVs
                const Matrix<DDSMat>& tAncestorADVIDs = tLockedInterfaceGeometry->get_determining_adv_ids(
                        mAncestorNodeIndices(tAncestorNode),
                        mAncestorNodeCoordinates(tAncestorNode));

                // Join IDs
                this->join_adv_ids(tAncestorADVIDs);
            }

            // Add first parent IDs, if needed
            if (mFirstParentNode)
            {
                this->join_adv_ids( mFirstParentNode->get_coordinate_determining_adv_ids() );
            }

            // Add second parent IDs, if needed
            if (mSecondParentNode)
            {
                this->join_adv_ids( mSecondParentNode->get_coordinate_determining_adv_ids() );
            }

            return mCoordinateDeterminingADVIDs;
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Intersection_Node::parent_edge_is_intersected()
        {
            return mIsIntersected;
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Intersection_Node::first_parent_on_interface()
        {
            return mFirstParentOnInterface;
        }

        //--------------------------------------------------------------------------------------------------------------

        bool Intersection_Node::second_parent_on_interface()
        {
            return mSecondParentOnInterface;
        }

        //--------------------------------------------------------------------------------------------------------------

        real Intersection_Node::get_local_coordinate()
        {
            return mLocalCoordinate;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Intersection_Node::get_global_coordinates()
        {
            return mGlobalCoordinates;
        }

        //--------------------------------------------------------------------------------------------------------------

        real Intersection_Node::get_coordinate_value(uint aCoordinateIndex)
        {
            return mGlobalCoordinates(aCoordinateIndex);
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Intersection_Node::get_num_pdvs()
        {
            return mGlobalCoordinates.numel();
        }

        //--------------------------------------------------------------------------------------------------------------

        void Intersection_Node::set_starting_pdv_id(moris_id aPDVStartingID)
        {
            mPDVStartingID = aPDVStartingID;
            mPDVStartingIDSet = true;
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_id Intersection_Node::get_starting_pdv_id()
        {
            MORIS_ASSERT(mPDVStartingIDSet, "PDV Starting ID must be set for an intersection.");
            return mPDVStartingID;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Intersection_Node::set_id(moris_id aNodeID)
        {
            mNodeID = aNodeID;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Intersection_Node::set_owner(moris_index aNodeOwner)
        {
            mNodeOwner = aNodeOwner;
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_id Intersection_Node::get_id()
        {
            return mNodeID;
        }

        //--------------------------------------------------------------------------------------------------------------

        moris_index Intersection_Node::get_owner()
        {
            return mNodeOwner;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Intersection_Node::join_coordinate_sensitivities(const Matrix<DDRMat>& aSensitivitiesToAdd)
        {
            // Resize sensitivities
            uint tJoinedSensitivityLength = mCoordinateSensitivities.n_rows();
            mCoordinateSensitivities.resize(
                    tJoinedSensitivityLength + aSensitivitiesToAdd.n_rows(),
                    aSensitivitiesToAdd.n_cols());

            // Join sensitivities
            for (uint tAddedSensitivity = 0; tAddedSensitivity < aSensitivitiesToAdd.n_rows(); tAddedSensitivity++)
            {
                for (uint tCoordinateIndex = 0; tCoordinateIndex < aSensitivitiesToAdd.n_cols(); tCoordinateIndex++)
                {
                    mCoordinateSensitivities(tJoinedSensitivityLength + tAddedSensitivity, tCoordinateIndex) =
                            aSensitivitiesToAdd(tAddedSensitivity, tCoordinateIndex);
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Intersection_Node::join_adv_ids(const Matrix<DDSMat>& aIDsToAdd)
        {
            // Resize IDs
            uint tJoinedSensitivityLength = mCoordinateDeterminingADVIDs.n_rows();
            mCoordinateDeterminingADVIDs.resize(tJoinedSensitivityLength + aIDsToAdd.n_rows(), 1);

            // Join IDs
            for (uint tAddedSensitivity = 0; tAddedSensitivity < aIDsToAdd.n_rows(); tAddedSensitivity++)
            {
                mCoordinateDeterminingADVIDs(tJoinedSensitivityLength + tAddedSensitivity) = aIDsToAdd(tAddedSensitivity);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
