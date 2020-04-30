//
// Created by christopherson on 4/13/20.
//

#include "fn_GEN_create_geometry.hpp"
#include "fn_Parsing_Tools.hpp"

#include "cl_GEN_Circle.hpp"
#include "cl_GEN_User_Defined_Geometry.hpp"

namespace moris
{
    namespace ge
    {
        std::shared_ptr<Geometry_Analytic> create_geometry(ParameterList aGeometryParameterList, Matrix<DDRMat>& aADVs, std::shared_ptr<moris::Library_IO> aLibrary)
        {
            // Geometry type
            std::string tGeometryType = aGeometryParameterList.get<std::string>("type");

            // Geometry inputs
            Matrix<DDUMat> tGeometryVariableIndices(1, 1);
            Matrix<DDUMat> tADVIndices(1, 1);
            Matrix<DDRMat> tConstantParameters(1, 1);

            // Get from parameter list
            string_to_mat(aGeometryParameterList.get<std::string>("geometry_variable_indices"), tGeometryVariableIndices);
            string_to_mat(aGeometryParameterList.get<std::string>("adv_indices"), tADVIndices);
            string_to_mat(aGeometryParameterList.get<std::string>("constant_parameters"), tConstantParameters);

            // Build Geometry
            if (tGeometryType == "circle")
            {
                return std::make_shared<Circle>(aADVs, tGeometryVariableIndices, tADVIndices, tConstantParameters);
            }
            else if (tGeometryType == "user_defined")
            {
                return std::make_shared<User_Defined_Geometry>(aADVs, tGeometryVariableIndices, tADVIndices, tConstantParameters,
                        aLibrary->load_geometry_function(aGeometryParameterList.get<std::string>("field_function_name")),
                        aLibrary->load_geometry_sensitivity_function(aGeometryParameterList.get<std::string>("sensitivity_function_name")));
            }
            else
            {
                MORIS_ERROR(false, tGeometryType.append(" is not recognized as a valid Geometry type in fn_GEN_create_geometry.").c_str());
                return nullptr;
            }
        }
    }
}
