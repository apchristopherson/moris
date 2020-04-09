//
// Created by christopherson on 3/4/20.
//

#include "fn_OPT_create_problem.hpp"
#include "cl_OPT_Problem_User_Defined.hpp"

namespace moris
{
    namespace opt
    {
        std::shared_ptr<Problem> create_problem(ParameterList aProblemParameterList, std::shared_ptr<Criteria_Interface> aInterface)
        {
            std::string tProblemType = aProblemParameterList.get<std::string>("problem");
            if (!tProblemType.compare("user_defined"))
            {
                return std::make_shared<Problem_User_Defined>(aProblemParameterList, aInterface);
            }
            else
            {
                MORIS_ERROR(false, tProblemType.append(" is not recognized as a valid Problem in fn_OPT_create_problem.").c_str());
                return nullptr;
            }
        }
    }
}
