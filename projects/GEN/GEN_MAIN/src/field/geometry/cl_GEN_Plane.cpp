#include "cl_GEN_Plane.hpp"

namespace moris
{
    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        Plane::Plane(real             aXCenter,
                     real             aYCenter,
                     real             aZCenter,
                     real             aXNormal,
                     real             aYNormal,
                     real             aZNormal,
                     Geometry_Field_Parameters aParameters)
                : Field(Matrix<DDRMat>({{aXCenter, aYCenter, aZCenter, aXNormal, aYNormal, aZNormal}}), aParameters)
                , Geometry(aParameters)
        {
            m_eval_field = &Plane::eval_field_3d;
            m_eval_sensitivity = &Plane::eval_sensitivity_3d;
        }

        //--------------------------------------------------------------------------------------------------------------
    
        Plane::Plane(real aXCenter, real aYCenter, real aXNormal, real aYNormal, Geometry_Field_Parameters aParameters)
                : Field(Matrix<DDRMat>({{aXCenter, aYCenter, aXNormal, aYNormal}}), aParameters)
                , Geometry(aParameters)
        {
            m_eval_field = &Plane::eval_field_2d;
            m_eval_sensitivity = &Plane::eval_sensitivity_2d;
        }

        //--------------------------------------------------------------------------------------------------------------

        real Plane::get_field_value(const Matrix<DDRMat>& aCoordinates)
        {
            return (this->*m_eval_field)(aCoordinates);
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix<DDRMat>& Plane::get_field_sensitivities(const Matrix<DDRMat>& aCoordinates)
        {
            return (this->*m_eval_sensitivity)(aCoordinates);
        }

        //--------------------------------------------------------------------------------------------------------------

        real Plane::eval_field_2d(const Matrix<DDRMat>& aCoordinates)
        {
            // Get variables
            real tXCenter = *(mFieldVariables(0));
            real tYCenter = *(mFieldVariables(1));
            real tXNormal = *(mFieldVariables(2));
            real tYNormal = *(mFieldVariables(3));

            // Evaluate field value
            return tXNormal * (aCoordinates(0) - tXCenter) + tYNormal * (aCoordinates(1) - tYCenter);
        }

        //--------------------------------------------------------------------------------------------------------------
        
        real Plane::eval_field_3d(const Matrix<DDRMat>& aCoordinates)
        {
            // Get variables
            real tXCenter = *(mFieldVariables(0));
            real tYCenter = *(mFieldVariables(1));
            real tZCenter = *(mFieldVariables(2));
            real tXNormal = *(mFieldVariables(3));
            real tYNormal = *(mFieldVariables(4));
            real tZNormal = *(mFieldVariables(5));

            // Evaluate field value
            return tXNormal * (aCoordinates(0) - tXCenter) + tYNormal * (aCoordinates(1) - tYCenter) + tZNormal * (aCoordinates(2) - tZCenter);
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix<DDRMat>& Plane::eval_sensitivity_2d(const Matrix<DDRMat>& aCoordinates)
        {
            // Get variables
            real tXCenter = *(mFieldVariables(0));
            real tYCenter = *(mFieldVariables(1));
            real tXNormal = *(mFieldVariables(2));
            real tYNormal = *(mFieldVariables(3));

            // Evaluate sensitivities
            mSensitivities(0) = -tXNormal;
            mSensitivities(1) = -tYNormal;
            mSensitivities(2) = aCoordinates(0) - tXCenter;
            mSensitivities(3) = aCoordinates(1) - tYCenter;

            return mSensitivities;
        }

        //--------------------------------------------------------------------------------------------------------------

        const Matrix<DDRMat>& Plane::eval_sensitivity_3d(const Matrix<DDRMat>& aCoordinates)
        {
            MORIS_ERROR(false, "Sensitivities not implemented for 3d plane.");
            return mSensitivities;
        }

        //--------------------------------------------------------------------------------------------------------------
        
    }
}
