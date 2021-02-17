#include "cl_GEN_Field_Discrete_Integration.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        Field_Discrete_Integration::Field_Discrete_Integration(uint aNumOriginalNodes)
                : mNumOriginalNodes(aNumOriginalNodes)
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        real Field_Discrete_Integration::get_field_value(
                uint                  aNodeIndex,
                const Matrix<DDRMat>& aCoordinates)
        {
            if (aNodeIndex < mNumOriginalNodes)
            {
                return this->get_field_value(aNodeIndex);
            }
            else
            {
                MORIS_ASSERT((aNodeIndex - mNumOriginalNodes) < mChildNodes.size(),
                        "A discrete field value was requested from a node that this field doesn't know. "
                        "Perhaps a child node was not added to this field?");
                return mChildNodes(aNodeIndex - mNumOriginalNodes)->interpolate_field_value(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix<DDRMat>& Field_Discrete_Integration::get_field_sensitivities(
                uint                  aNodeIndex,
                const Matrix<DDRMat>& aCoordinates)
        {
            if (aNodeIndex < mNumOriginalNodes)
            {
                return this->get_field_sensitivities(aNodeIndex);
            }
            else
            {
                MORIS_ASSERT((aNodeIndex - mNumOriginalNodes) < mChildNodes.size(),
                        "A discrete field sensitivity was requested from a node that this field doesn't know. "
                        "Perhaps a child node was not added to this field?");
                return mChildNodes(aNodeIndex - mNumOriginalNodes)->join_field_sensitivities(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDSMat> Field_Discrete_Integration::get_determining_adv_ids(
                uint                  aNodeIndex,
                const Matrix<DDRMat>& aCoordinates)
        {
            if (aNodeIndex < mNumOriginalNodes)
            {
                return this->get_determining_adv_ids(aNodeIndex);
            }
            else
            {
                MORIS_ASSERT((aNodeIndex - mNumOriginalNodes) < mChildNodes.size(),
                             "A discrete field sensitivity was requested from a node that this field doesn't know. "
                             "Perhaps a child node was not added to this field?");
                return mChildNodes(aNodeIndex - mNumOriginalNodes)->join_determining_adv_ids(this);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDSMat> Field_Discrete_Integration::get_determining_adv_ids(uint aNodeIndex)
        {
            return Field::get_determining_adv_ids(aNodeIndex, {{}});
        }

        //--------------------------------------------------------------------------------------------------------------

        void Field_Discrete_Integration::add_child_node(uint aNodeIndex, std::shared_ptr<Child_Node> aChildNode)
        {
            MORIS_ASSERT(aNodeIndex == mNumOriginalNodes + mChildNodes.size(),
                    "Child nodes must be added to a level set field in order by node index.");
            mChildNodes.push_back(aChildNode);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Field_Discrete_Integration::reset_nodal_information()
        {
            mChildNodes.resize(0);
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
