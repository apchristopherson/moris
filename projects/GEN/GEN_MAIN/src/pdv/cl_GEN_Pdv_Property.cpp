#include "cl_GEN_Pdv_Property.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        Pdv_Property::Pdv_Property(std::shared_ptr<Property> aPropertyPointer)
        : mProperty(aPropertyPointer)
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        real Pdv_Property::get_value(uint aNodeIndex, const Matrix<DDRMat>& aCoordinates)
        {
            return mProperty->evaluate_field_value(aNodeIndex, aCoordinates);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Pdv_Property::get_sensitivity(uint aNodeIndex, const Matrix<DDRMat>& aCoordinates, Matrix<DDRMat>& aSensitivities)
        {
            mProperty->evaluate_sensitivity(aNodeIndex, aCoordinates, aSensitivities);
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}