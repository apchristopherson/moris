#include "fn_OPT_create_algorithm.hpp"
#include "cl_OPT_Algorithm_GCMMA.hpp"
#include "cl_OPT_Algorithm_LBFGS.hpp"
#include "cl_OPT_Algorithm_SQP.hpp"
#include "cl_OPT_Algorithm_Sweep.hpp"

namespace moris
{
    namespace opt
    {
        std::shared_ptr<Algorithm> create_algorithm(ParameterList aAlgorithmParameterList)
        {
            std::string tAlgorithmName = aAlgorithmParameterList.get<std::string>("algorithm");
            if (tAlgorithmName == "gcmma")
            {
                return std::make_shared<OptAlgGCMMA>(aAlgorithmParameterList);
            }
            else if (tAlgorithmName == "sqp")
            {
                return std::make_shared<Algorithm_SQP>(aAlgorithmParameterList);
            }
            else if (tAlgorithmName == "lbfgs")
            {
                MORIS_ERROR(false, "LBFGS currently is not working.");
                return nullptr;
            }
            else if (tAlgorithmName == "sweep")
            {
                return std::make_shared<Algorithm_Sweep>(aAlgorithmParameterList);
            }
            else
            {
                MORIS_ERROR(false, tAlgorithmName.append(" is not recognized as a valid Algorithm type in fn_OPT_create_algorithm.").c_str());
                return nullptr;
            }
        }
    }
}
