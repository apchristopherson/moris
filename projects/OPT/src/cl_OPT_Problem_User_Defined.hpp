#ifndef MORIS_CL_OPT_PROBLEM_USER_DEFINED_HPP
#define MORIS_CL_OPT_PROBLEM_USER_DEFINED_HPP

#include "cl_OPT_Problem.hpp"
#include "cl_OPT_Criteria_Interface.hpp"
#include "cl_Param_List.hpp"
#include "cl_Library_IO.hpp"

namespace moris
{
    namespace opt
    {
        // User-defined function types
        typedef Matrix<DDRMat> ( *Objective_Constraint_Function ) (
                const moris::Matrix<DDRMat>&,
                const moris::Matrix<DDRMat>& );
        typedef Matrix<DDSMat> ( *Constraint_Types_Function ) ( );

        class Problem_User_Defined : public moris::opt::Problem
        {

        public:

            /**
             * Constructor
             *
             * @param aParameterList parameter list for this problem specifying the needed library
             * @param aInterface Interface class written for other module
             */
            Problem_User_Defined(
                    ParameterList                       & aParameterList,
                    std::shared_ptr<Criteria_Interface> & aInterface);

            /**
             * Alternate constructor where the user-defined functions are provided directly. Used for OPT tests.
             *
             * @param aParameterList Parameter list for the base Problem class
             * @param aInterface Interface class written for other module
             * @param aConstraintTypesFunction Function for getting constraint types
             * @param aObjectiveFunction Objective function
             * @param aConstraintFunction Constraint function
             * @param aObjectiveADVGradientFunction
             * @param aObjectiveCriteriaGradientFunction
             * @param aConstraintADVGradientFunction
             * @param aConstraintCriteriaGradientFunction
             */
            Problem_User_Defined(
                    ParameterList                       aParameterList,
                    std::shared_ptr<Criteria_Interface> aInterface,
                    Constraint_Types_Function     aConstraintTypesFunction,
                    Objective_Constraint_Function aObjectiveFunction,
                    Objective_Constraint_Function aConstraintFunction,
                    Objective_Constraint_Function aObjectiveADVGradientFunction,
                    Objective_Constraint_Function aObjectiveCriteriaGradientFunction,
                    Objective_Constraint_Function aConstraintADVGradientFunction,
                    Objective_Constraint_Function aConstraintCriteriaGradientFunction);

            /**
             * Gets the constraint types
             *
             * @return vector of integers, 0 = equality constraint, 1 = inequality constraint
             */
            Matrix<DDSMat> get_constraint_types();

        protected:

            /**
             * Gets the objective values
             *
             * @return vector of objectives
             */
            Matrix<DDRMat> compute_objectives();

            /**
             * Gets the constraint values
             *
             * @return vector of constraints
             */
            Matrix<DDRMat> compute_constraints();

            /**
             * Gets the derivative of the objectives with respect to the advs
             *
             * @return matrix d(objective)_i/d(adv)_j
             */
            Matrix<DDRMat> compute_dobjective_dadv();

            /**
             * Gets the derivative of the constraints with respect to the advs
             *
             * @return matrix d(constraints)_i/d(adv)_j
             */
            Matrix<DDRMat> compute_dconstraint_dadv();

            /**
             * Gets the derivative of the objective with respect to the criteria.
             *
             * @return matrix d(objective)_i/d(criteria)_j
             */
            Matrix<DDRMat> compute_dobjective_dcriteria();

            /**
             * Gets the derivative of the constraints with respect to the criteria.
             *
             * @return matrix d(constraint)_i/d(criteria)_j
             */
            Matrix<DDRMat> compute_dconstraint_dcriteria();

        private:
            Constraint_Types_Function get_constraint_types_user_defined;
            Objective_Constraint_Function compute_objectives_user_defined;
            Objective_Constraint_Function compute_constraints_user_defined;
            Objective_Constraint_Function compute_dobjective_dadv_user_defined;
            Objective_Constraint_Function compute_dobjective_dcriteria_user_defined;
            Objective_Constraint_Function compute_dconstraint_dadv_user_defined;
            Objective_Constraint_Function compute_dconstraint_dcriteria_user_defined;
        };
    }
}

#endif // MORIS_CL_OPT_PROBLEM_USER_DEFINED_HPP
