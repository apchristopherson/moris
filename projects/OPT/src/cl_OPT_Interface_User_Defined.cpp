#include "cl_OPT_Interface_User_Defined.hpp"
#include "fn_Exec_load_user_library.hpp"

namespace moris
{
    namespace opt
    {
        //--------------------------------------------------------------------------------------------------------------

        Interface_User_Defined::Interface_User_Defined(
                ParameterList aParameterList)
        : mLibrary(std::make_shared<Library_IO>(aParameterList.get<std::string>("library")))
        {
            // Set user-defined functions
            initialize_user_defined             = mLibrary->load_criteria_initialize_function("initialize");
            get_criteria_user_defined           = mLibrary->load_criteria_function("get_criteria");
            compute_dcriteria_dadv_user_defined = mLibrary->load_criteria_function("get_dcriteria_dadv");
        }

        //--------------------------------------------------------------------------------------------------------------

        Interface_User_Defined::Interface_User_Defined(
                MORIS_CRITERIA_INITIALIZE_FUNCTION aInitializationFunction,
                MORIS_CRITERIA_FUNCTION aCriteriaEvaluationFunction,
                MORIS_CRITERIA_FUNCTION aCriteriaGradientFunction)
        : initialize_user_defined(aInitializationFunction),
          get_criteria_user_defined(aCriteriaEvaluationFunction),
          compute_dcriteria_dadv_user_defined(aCriteriaGradientFunction)
        {
        }

        //--------------------------------------------------------------------------------------------------------------

        void Interface_User_Defined::initialize(
                Matrix<DDRMat>& aADVs,
                Matrix<DDRMat>& aLowerBounds,
                Matrix<DDRMat>& aUpperBounds)
        {
            initialize_user_defined(aADVs, aLowerBounds, aUpperBounds);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Interface_User_Defined::perform(const Matrix<DDRMat> & aNewADVs)
        {
            mADVs = aNewADVs;

            return this->get_criteria_user_defined(mADVs);
        }

        //--------------------------------------------------------------------------------------------------------------

        Matrix<DDRMat> Interface_User_Defined::compute_dcriteria_dadv()
        {
            return this->compute_dcriteria_dadv_user_defined(mADVs);
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
