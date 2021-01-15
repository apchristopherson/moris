#include "cl_GEN_Circle.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        Circle::Circle(real aXCenter, real aYCenter, real aRadius, Field_Parameters aParameters)
                : Field(Matrix<DDRMat>({{aXCenter, aYCenter, aRadius}}), aParameters)
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        real Circle::get_field_value(const Matrix<DDRMat>& aCoordinates)
        {
            // Get variables
            real tXCenter = *(mFieldVariables(0));
            real tYCenter = *(mFieldVariables(1));
            real tRadius = *(mFieldVariables(2));

            // Evaluate field
            return sqrt(pow(aCoordinates(0) - tXCenter, 2) + pow(aCoordinates(1) - tYCenter, 2)) - tRadius;
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix<DDRMat>& Circle::get_field_sensitivities(const Matrix<DDRMat>& aCoordinates)
        {
            // Get variables
            real tXCenter = *(mFieldVariables(0));
            real tYCenter = *(mFieldVariables(1));

            // Calculate sensitivities
            real tConstant = sqrt(pow(aCoordinates(0) - tXCenter, 2) + pow(aCoordinates(1) - tYCenter, 2));
            tConstant = tConstant ? 1 / tConstant : 0.0;
            mSensitivities(0) = tConstant * (tXCenter - aCoordinates(0));
            mSensitivities(1) = tConstant * (tYCenter - aCoordinates(1));
            mSensitivities(2) = -1.0;

            return mSensitivities;
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}