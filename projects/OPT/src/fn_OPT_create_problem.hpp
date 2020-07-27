#ifndef MORIS_FN_OPT_CREATE_PROBLEM_HPP
#define MORIS_FN_OPT_CREATE_PROBLEM_HPP

#include "cl_OPT_Problem.hpp"
#include "cl_Param_List.hpp"

namespace moris
{
    namespace opt
    {
        /**
         * Creates an instance of the specified Problem class and returns a shared pointer to it.
         *
         * @param aProblemParameterList Parameter list for the optimization problem
         * @param aInterface Pointer to already-built criteria interface
         * @return Problem class
         */
        std::shared_ptr<Problem> create_problem(ParameterList aProblemParameterList, std::shared_ptr<Criteria_Interface> aInterface);
    }
}

#endif //MORIS_FN_OPT_CREATE_PROBLEM_HPP
