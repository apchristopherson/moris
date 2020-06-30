//
// Created by christopherson on 4/13/20.
//

#include "fn_GEN_create_geometry.hpp"
#include "fn_Parsing_Tools.hpp"

#include "cl_GEN_Circle.hpp"
#include "cl_GEN_Sphere.hpp"
#include "cl_GEN_User_Defined_Geometry.hpp"

namespace moris
{
    namespace ge
    {
        std::shared_ptr<Geometry> create_geometry(ParameterList aGeometryParameterList, Matrix<DDRMat>& aADVs, std::shared_ptr<moris::Library_IO> aLibrary)
        {
            // Geometry type
            std::string tGeometryType = aGeometryParameterList.get<std::string>("type");

            // Geometry inputs
            Matrix<DDUMat> tGeometryVariableIndices(0, 0);
            Matrix<DDUMat> tADVIndices(0, 0);
            Matrix<DDRMat> tConstantParameters(0, 0);
            bool tFillVariables = false;
            bool tFillADVs = false;

            // Determine if variable or ADV indices need to be filled (specified by "all")
            if (aGeometryParameterList.get<std::string>("geometry_variable_indices") == "all")
            {
                tFillVariables = true;
            }
            else
            {
                string_to_mat(aGeometryParameterList.get<std::string>("geometry_variable_indices"), tGeometryVariableIndices);
            }
            if (aGeometryParameterList.get<std::string>("adv_indices") == "all")
            {
                tFillADVs = true;
            }
            else
            {
                string_to_mat(aGeometryParameterList.get<std::string>("adv_indices"), tADVIndices);
            }

            // Perform fill
            if (tFillVariables and tFillADVs)
            {
                tGeometryVariableIndices.resize(aADVs.length(), 1);
                tADVIndices.resize(aADVs.length(), 1);
                for (uint tIndex = 0; tIndex < aADVs.length(); tIndex++)
                {
                    tGeometryVariableIndices(tIndex) = tIndex;
                    tADVIndices(tIndex) = tIndex;
                }
            }
            else if (tFillVariables)
            {
                tGeometryVariableIndices.resize(tADVIndices.length(), 1);
                for (uint tIndex = 0; tIndex < tADVIndices.length(); tIndex++)
                {
                    tGeometryVariableIndices(tIndex) = tIndex;
                }
            }
            else if (tFillADVs)
            {
                tADVIndices.resize(tGeometryVariableIndices.length(), 1);
                for (uint tIndex = 0; tIndex < tGeometryVariableIndices.length(); tIndex++)
                {
                    tADVIndices(tIndex) = tIndex;
                }
            }

            // Get constant parameters
            string_to_mat(aGeometryParameterList.get<std::string>("constant_parameters"), tConstantParameters);

            // Get refinement info
            sint tNumRefinements = aGeometryParameterList.get<sint>("number_of_refinements");
            sint tRefinementFunctionIndex = aGeometryParameterList.get<sint>("refinement_function_index");

            // Build Geometry
            if (tGeometryType == "circle")
            {
                return std::make_shared<Circle>(aADVs,
                                                tGeometryVariableIndices,
                                                tADVIndices,
                                                tConstantParameters,
                                                tNumRefinements,
                                                tRefinementFunctionIndex);
            }
            else if (tGeometryType == "sphere")
            {
                return std::make_shared<Sphere>(aADVs,
                                                tGeometryVariableIndices,
                                                tADVIndices,
                                                tConstantParameters,
                                                tNumRefinements,
                                                tRefinementFunctionIndex);
            }
            else if (tGeometryType == "user_defined")
            {
                std::string tSensitivityFunctionName = aGeometryParameterList.get<std::string>("sensitivity_function_name");
                if (tSensitivityFunctionName == "")
                {
                    return std::make_shared<User_Defined_Geometry>(aADVs, tGeometryVariableIndices, tADVIndices, tConstantParameters,
                                                                   aLibrary->load_gen_field_function(aGeometryParameterList.get<std::string>("field_function_name")),
                                                                   nullptr,
                                                                   tNumRefinements,
                                                                   tRefinementFunctionIndex);
                }
                else
                {
                    return std::make_shared<User_Defined_Geometry>(aADVs, tGeometryVariableIndices, tADVIndices, tConstantParameters,
                                                                   aLibrary->load_gen_field_function(aGeometryParameterList.get<std::string>("field_function_name")),
                                                                   aLibrary->load_gen_sensitivity_function(tSensitivityFunctionName),
                                                                   tNumRefinements,
                                                                   tRefinementFunctionIndex);
                }
            }
            else
            {
                MORIS_ERROR(false, tGeometryType.append(" is not recognized as a valid Geometry type in fn_GEN_create_geometry.").c_str());
                return nullptr;
            }
        }
    }
}