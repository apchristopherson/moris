#include "catch.hpp"
#include "cl_Matrix.hpp"
#include "cl_GEN_Geometry_Engine.hpp"
#include "fn_GEN_create_geometry.hpp"
#include "fn_PRM_GEN_Parameters.hpp"
#include "fn_Exec_load_user_library.hpp"
#include "cl_GEN_User_Defined_Geometry.hpp"


namespace moris
{

    // User-defined circle functions
    real circle_evaluate_field_value(const moris::Matrix< DDRMat >    & aCoordinates,
                                     const moris::Cell< moris::real* > & aParameters)
    {
        // Get variables
        Matrix<DDRMat> tCenter(1, 2);
        tCenter(0) = *(aParameters(0));
        tCenter(1) = *(aParameters(1));

        // Evaluate field
        moris::real tFunctionValue = norm(aCoordinates - tCenter) - *(aParameters(2));

        return tFunctionValue;
    }

    Matrix<DDRMat> circle_evaluate_sensitivity(const moris::Matrix< DDRMat >    & aCoordinates,
                                               const moris::Cell< moris::real* > & aParameters)
    {
        // Initialize sensitivity matrix
        moris::Matrix< moris::DDRMat > tSensitivityDxDp(3, 2, 0.0);

        // Get variables
        moris::real tXCenter = *(aParameters(0));
        moris::real tYCenter = *(aParameters(1));
        moris::real tRadius = *(aParameters(2));

        // dx/dr
        // Set sign based on value under square root
        moris::real sign = 0.0;
        moris::real tSqrt = tRadius * tRadius - std::pow((aCoordinates(1) - tYCenter), 2);
        if(tSqrt < 0.0)
        {
            sign = -1.0;
        }
        else if(tSqrt > 0.0)
        {
            sign = 1.0;
        }

        // Calculate
        tSensitivityDxDp(0, 0) = sign * tRadius / std::sqrt(std::abs(tSqrt));

        // dy/dr
        // Set sign based on value under square root
        tSqrt = tRadius * tRadius - std::pow((aCoordinates(0) - tXCenter),2);
        if(tSqrt < 0.0)
        {
            sign = -1.0;
        }
        else if(tSqrt > 0.0)
        {
            sign = 1.0;
        }

        // Calculate
        tSensitivityDxDp(0, 1) = sign * tRadius / std::sqrt(std::abs(tSqrt));

        // Fill remaining values in tSensitivity
        tSensitivityDxDp(1,0) = 1.0; // dx/dxc
        tSensitivityDxDp(1,1) = 0.0; // dy/dxc
        tSensitivityDxDp(2,0) = 0.0; // dx/dyc
        tSensitivityDxDp(2,1) = 1.0; // dy/dyc

        return tSensitivityDxDp;
    }

    namespace ge
    {
        TEST_CASE("Geometry_Analytic with ADVs test", "[GE], [GE_GEOM_ADV]")
        {
            // Test parameter list
            // Set up default parameter lists
            moris::Cell<moris::Cell<ParameterList>> tParameterLists(2);
            tParameterLists(0).resize(1);
            tParameterLists(1).resize(2);
            tParameterLists(0)(0) = moris::prm::create_gen_parameter_list();
            tParameterLists(1)(0) = moris::prm::create_geometry_parameter_list();
            tParameterLists(1)(1) = moris::prm::create_geometry_parameter_list();

            // Modify parameters
            tParameterLists(0)(0).set("initial_advs", "0.0, 1.0, 2.0");

            tParameterLists(1)(0).set("type", "circle");
            tParameterLists(1)(0).set("geometry_variable_indices", "0, 2");
            tParameterLists(1)(0).set("adv_indices", "0, 1");
            tParameterLists(1)(0).set("constant_parameters", "1.0");

            tParameterLists(1)(1).set("type", "circle");
            tParameterLists(1)(1).set("geometry_variable_indices", "0, 2");
            tParameterLists(1)(1).set("adv_indices", "0, 2");
            tParameterLists(1)(1).set("constant_parameters", "2.0");

            // Create geometry separately
            Matrix<DDRMat> tADVs(3, 1);
            tADVs(0) = 0.0;
            tADVs(1) = 1.0;
            tADVs(2) = 2.0;
            std::shared_ptr<Geometry_Analytic> tCircle1 = create_geometry(tParameterLists(1)(0), tADVs);
            std::shared_ptr<Geometry_Analytic> tCircle2 = create_geometry(tParameterLists(1)(1), tADVs);

            // Check field values
            Matrix<DDRMat> tCoordinates(1, 2);
            tCoordinates(0) = 0.0;
            tCoordinates(1) = 0.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);
            tCoordinates(0) = 1.0;
            tCoordinates(1) = 1.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == sqrt(2.0) - 2.0);
            tCoordinates(0) = 2.0;
            tCoordinates(1) = 2.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == sqrt(5.0) - 1.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);

            // Change ADVs and check again
            tADVs(0) = 1.0;
            tADVs(1) = 2.0;
            tADVs(2) = 3.0;

            tCoordinates(0) = 1.0;
            tCoordinates(1) = -1.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);
            tCoordinates(0) = 3.0;
            tCoordinates(1) = 1.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == sqrt(5.0) - 3.0);
            tCoordinates(0) = 4.0;
            tCoordinates(1) = 2.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == sqrt(10.0) - 2.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);

        }   // end test case

        TEST_CASE("User-Defined Geometry Test", "[GE], [GE_USER_GEOM]")
        {
            // Create user-defined geometry
            Matrix<DDRMat> tADVs(3, 1);
            tADVs(0) = 0.0;
            tADVs(1) = 1.0;
            tADVs(2) = 2.0;

            Matrix<DDUMat> tGeometryVariableIndices(2, 1);
            tGeometryVariableIndices(0) = 0;
            tGeometryVariableIndices(1) = 2;

            Matrix<DDUMat> tADVIndices(2, 1);
            tADVIndices(0) = 0;
            tADVIndices(1) = 1;

            Matrix<DDRMat> tConstantParameters(1, 1);
            tConstantParameters(0) = 1.0;

            std::shared_ptr<Geometry_Analytic> tCircle1 = std::make_shared<User_Defined_Geometry>(tADVs,
                                                                                tGeometryVariableIndices,
                                                                                tADVIndices,
                                                                                tConstantParameters,
                                                                                &circle_evaluate_field_value,
                                                                                &circle_evaluate_sensitivity);

            tADVIndices(1) = 2;
            tConstantParameters(0) = 2.0;
            std::shared_ptr<Geometry_Analytic> tCircle2 = std::make_shared<User_Defined_Geometry>(tADVs,
                                                                                  tGeometryVariableIndices,
                                                                                  tADVIndices,
                                                                                  tConstantParameters,
                                                                                  &circle_evaluate_field_value,
                                                                                  &circle_evaluate_sensitivity);

            // Check field values
            Matrix<DDRMat> tCoordinates(1, 2);
            tCoordinates(0) = 0.0;
            tCoordinates(1) = 0.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);
            tCoordinates(0) = 1.0;
            tCoordinates(1) = 1.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == sqrt(2.0) - 2.0);
            tCoordinates(0) = 2.0;
            tCoordinates(1) = 2.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == sqrt(5.0) - 1.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);

            // Change ADVs and check again
            tADVs(0) = 1.0;
            tADVs(1) = 2.0;
            tADVs(2) = 3.0;

            tCoordinates(0) = 1.0;
            tCoordinates(1) = -1.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);
            tCoordinates(0) = 3.0;
            tCoordinates(1) = 1.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == 0.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == sqrt(5.0) - 3.0);
            tCoordinates(0) = 4.0;
            tCoordinates(1) = 2.0;
            CHECK(tCircle1->evaluate_field_value(tCoordinates) == sqrt(10.0) - 2.0);
            CHECK(tCircle2->evaluate_field_value(tCoordinates) == 0.0);

        }   // end test case

    }   // end ge namespace
}   // end moris namespace
