#include "cl_GEN_Scaled_Field.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        real Scaled_Field::get_base_field_value(
                uint                  aNodeIndex,
                const Matrix<DDRMat>& aCoordinates)
        {
            return *mFieldVariables(0) * mField->get_field_value(aNodeIndex, aCoordinates);
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix<DDRMat>& Scaled_Field::get_base_field_sensitivities(
                uint                  aNodeIndex,
                const Matrix<DDRMat>& aCoordinates)
        {
            mSensitivities = *mFieldVariables(0) *
                    mField->get_field_sensitivities(aNodeIndex, aCoordinates);
            return mSensitivities;
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDSMat> Scaled_Field::get_base_determining_adv_ids(
                uint                  aNodeIndex,
                const Matrix<DDRMat>& aCoordinates)
        {
            return mField->get_determining_adv_ids(aNodeIndex, aCoordinates);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Scaled_Field::set_dependencies(Cell<std::shared_ptr<Field>> aDependencyFields)
        {
            MORIS_ERROR(aDependencyFields.size() == 1, "A scaled field only depends on one field.");
            mField = aDependencyFields(0);
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
