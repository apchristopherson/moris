#ifndef MORIS_CL_GEN_CONSTANT_PROPERTY_HPP
#define MORIS_CL_GEN_CONSTANT_PROPERTY_HPP

#include "cl_GEN_Property.hpp"

namespace moris
{
    namespace ge
    {
        class Constant_Property : public Property
        {

        public:
            /**
             * Constructor
             *
             * @tparam Vector_Type Type of vector where ADVs are stored
             * @param aADVs ADV vector
             * @param aPropertyVariableIndices Indices of property variables to be filled by the ADVs
             * @param aADVIndices The indices of the ADV vector to fill in the property variables
             * @param aConstants The constant field variables not filled by ADVs
             * @param aFieldDependencies Other created fields that this property depends on
             * @param aParameters Additional parameters
             */
            template <typename Vector_Type>
            Constant_Property(
                    Vector_Type&              aADVs,
                    Matrix<DDUMat>            aPropertyVariableIndices,
                    Matrix<DDUMat>            aADVIndices,
                    Matrix<DDRMat>            aConstants,
                    Property_Field_Parameters aParameters = {})
                    : Field(aADVs, aPropertyVariableIndices, aADVIndices, aConstants, aParameters)
                    , Property(aParameters)
            {
                MORIS_ERROR(mFieldVariables.size() == 1, "A constant property has only one variable.");
            }

            /**
             * Given a node index, returns the field value.
             *
             * @param aNodeIndex Node index
             * @param aCoordinates Node coordinates
             * @return Property value
             */
            real get_field_value(
                    uint                  aNodeIndex,
                    const Matrix<DDRMat>& aCoordinates);

            /**
             * Given a node index, evaluates the sensitivity of the property field with respect to all of the
             * property variables.
             *
             * @param aNodeIndex Node index
             * @param aCoordinates Node coordinates
             * @return Vector of sensitivities
             */
            const Matrix<DDRMat>& get_field_sensitivities(
                    uint                  aNodeIndex,
                    const Matrix<DDRMat>& aCoordinates);
        };
    }
}

#endif //MORIS_CL_GEN_SCALED_FIELD_HPP
