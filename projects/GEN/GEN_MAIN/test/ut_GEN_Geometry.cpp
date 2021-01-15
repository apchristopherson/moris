#include "catch.hpp"
#include "cl_Matrix.hpp"
#include "fn_Parsing_Tools.hpp"
#include "fn_Exec_load_user_library.hpp"
#include "fn_trans.hpp"

#include "cl_GEN_Geometry_Engine_Test.hpp"
#include "cl_GEN_User_Defined_Field.hpp"
#include "fn_GEN_create_geometries.hpp"
#include "fn_PRM_GEN_Parameters.hpp"

#include "fn_GEN_create_simple_mesh.hpp"
#include "fn_GEN_check_equal.hpp"

#include "cl_SOL_Matrix_Vector_Factory.hpp"

namespace moris
{

    //------------------------------------------------------------------------------------------------------------------

    // Dummy user-defined functions
    uint user_defined_phase_function(const ge::Geometry_Bitset& aGeometrySigns);

    real user_defined_geometry_field(const Matrix<DDRMat>& aCoordinates,
                                     const Cell<real*>&    aParameters);

    void user_defined_geometry_sensitivity(const Matrix<DDRMat>& aCoordinates,
                                           const Cell<real*>&    aParameters,
                                           Matrix<DDRMat>&       aSensitivities);

    //--------------------------------------------------------------------------------------------------------------

    namespace ge
    {
        // Check for ellipse location in a swiss cheese
        void check_swiss_cheese(std::shared_ptr<Geometry> aSwissCheese,
                                real aXCenter,
                                real aYCenter,
                                real aXSemidiameter,
                                real aYSemidiameter,
                                bool aCheck = true);

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Phase Table", "[gen], [geometry], [phase table]")
        {
            // Create phase table using number of geometries
            Phase_Table tPhaseTable(3);

            // Check number of phases
            CHECK(tPhaseTable.get_num_phases() == 8);

            // Check individual phases
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(0)) == 0);
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(4)) == 1);
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(2)) == 2);
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(6)) == 3);
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(1)) == 4);
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(5)) == 5);
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(3)) == 6);
            CHECK(tPhaseTable.get_phase_index(Geometry_Bitset(7)) == 7);

            // Create phase custom phase table
            Phase_Table tPhaseTableCustom(3, Matrix<DDUMat>({{3, 2, 1, 0, 0, 1, 2, 3}}));

            // Check number of phases
            CHECK(tPhaseTableCustom.get_num_phases() == 4);

            // Check individual phases
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(0)) == 3);
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(4)) == 2);
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(2)) == 1);
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(6)) == 0);
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(1)) == 0);
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(5)) == 1);
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(3)) == 2);
            CHECK(tPhaseTableCustom.get_phase_index(Geometry_Bitset(7)) == 3);

            // Create phase custom phase table
            Phase_Table tPhaseTableFunction(&user_defined_phase_function, 4);

            // Check number of phases
            CHECK(tPhaseTableFunction.get_num_phases() == 4);

            // Check individual phases
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(0)) == 3);
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(4)) == 2);
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(2)) == 1);
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(6)) == 2);
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(1)) == 0);
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(5)) == 2);
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(3)) == 1);
            CHECK(tPhaseTableFunction.get_phase_index(Geometry_Bitset(7)) == 2);
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Circle", "[gen], [geometry], [distributed advs], [circle]")
        {
            // Set up geometry
            ParameterList tCircle1ParameterList = prm::create_geometry_parameter_list();
            tCircle1ParameterList.set("type", "circle");
            tCircle1ParameterList.set("field_variable_indices", "all");
            tCircle1ParameterList.set("adv_indices", "0, 1, 3");

            ParameterList tCircle2ParameterList = prm::create_geometry_parameter_list();
            tCircle2ParameterList.set("type", "circle");
            tCircle2ParameterList.set("field_variable_indices", "all");
            tCircle2ParameterList.set("adv_indices", "0, 2, 4");

            // ADV vector
            Matrix<DDRMat> tADVs;

            // Distributed ADVs
            sol::Matrix_Vector_Factory tDistributedFactory;
            Matrix<DDSMat> tADVIds = {{0}, {1}, {2}, {3}, {4}};
            sol::Dist_Map* tADVMap = tDistributedFactory.create_map(tADVIds);
            sol::Dist_Vector* tDistributedADVs = tDistributedFactory.create_vector(tADVMap, 1, false, true);

            // Define circles
            std::shared_ptr<Geometry> tCircle1;
            std::shared_ptr<Geometry> tCircle2;

            // Loop over ADV types
            for (bool tDistributed : {false, true})
            {
                // Set ADVs
                tADVs = {{0.0, 1.0, 2.0, 1.0, 2.0}};
                tDistributedADVs->replace_global_values(tADVIds, tADVs);

                // Create circles
                if (tDistributed)
                {
                    tCircle1 = create_geometry(tCircle1ParameterList, tDistributedADVs);
                    tCircle2 = create_geometry(tCircle2ParameterList, tDistributedADVs);
                }
                else
                {
                    tCircle1 = create_geometry(tCircle1ParameterList, tADVs);
                    tCircle2 = create_geometry(tCircle2ParameterList, tADVs);
                }

                // Set coordinates for checking
                Matrix<DDRMat> tCoordinates0 = {{0.0, 0.0}};
                Matrix<DDRMat> tCoordinates1 = {{1.0, 1.0}};
                Matrix<DDRMat> tCoordinates2 = {{2.0, 2.0}};

                // Check field values
                CHECK(tCircle1->get_field_value(0, tCoordinates0) == Approx(0.0));
                CHECK(tCircle2->get_field_value(0, tCoordinates0) == Approx(0.0));
                CHECK(tCircle1->get_field_value(0, tCoordinates1) == Approx(0.0));
                CHECK(tCircle2->get_field_value(0, tCoordinates1) == Approx(sqrt(2.0) - 2.0));
                CHECK(tCircle1->get_field_value(0, tCoordinates2) == Approx(sqrt(5.0) - 1.0));
                CHECK(tCircle2->get_field_value(0, tCoordinates2) == Approx(0.0));

                // Check sensitivity values
                check_equal(tCircle1->get_field_sensitivities(0, tCoordinates0), {{0.0, 1.0, -1.0}});
                check_equal(tCircle2->get_field_sensitivities(0, tCoordinates0), {{0.0, 1.0, -1.0}});
                check_equal(tCircle1->get_field_sensitivities(0, tCoordinates1), {{-1.0, 0.0, -1.0}});
                check_equal(tCircle2->get_field_sensitivities(0, tCoordinates1), {{-sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, -1.0}});
                check_equal(tCircle1->get_field_sensitivities(0, tCoordinates2), {{-2.0 / sqrt(5.0), -1.0 / sqrt(5.0), -1.0}});
                check_equal(tCircle2->get_field_sensitivities(0, tCoordinates2), {{-1.0, 0.0, -1.0}});

                // Change ADVs and coordinates
                tADVs = {{1.0, 1.0, 2.0, 2.0, 3.0}};
                tDistributedADVs->replace_global_values(tADVIds, tADVs);
                tCoordinates0(0) = 1.0;
                tCoordinates0(1) = -1.0;
                tCoordinates1(0) = 3.0;
                tCoordinates1(1) = 1.0;
                tCoordinates2(0) = 4.0;
                tCoordinates2(1) = 2.0;

                // Check field values
                CHECK(tCircle1->get_field_value(0, tCoordinates0) == Approx(0.0));
                CHECK(tCircle2->get_field_value(0, tCoordinates0) == Approx(0.0));
                CHECK(tCircle1->get_field_value(0, tCoordinates1) == Approx(0.0));
                CHECK(tCircle2->get_field_value(0, tCoordinates1) == Approx(sqrt(5.0) - 3.0));
                CHECK(tCircle1->get_field_value(0, tCoordinates2) == Approx(sqrt(10.0) - 2.0));
                CHECK(tCircle2->get_field_value(0, tCoordinates2) == Approx(0.0));

                // Check sensitivity values
                check_equal(tCircle1->get_field_sensitivities(0, tCoordinates0), {{0.0, 1.0, -1.0}});
                check_equal(tCircle2->get_field_sensitivities(0, tCoordinates0), {{0.0, 1.0, -1.0}});
                check_equal(tCircle1->get_field_sensitivities(0, tCoordinates1), {{-1.0, 0.0, -1.0}});
                check_equal(tCircle2->get_field_sensitivities(0, tCoordinates1), {{-2.0 / sqrt(5.0), 1.0 / sqrt(5.0), -1.0}});
                check_equal(tCircle1->get_field_sensitivities(0, tCoordinates2), {{-3.0 / sqrt(10.0), -1.0 / sqrt(10.0), -1.0}});
                check_equal(tCircle2->get_field_sensitivities(0, tCoordinates2), {{-1.0, 0.0, -1.0}});
            }

            // Clean up
            delete tDistributedADVs;
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Superellipse", "[gen], [geometry], [superellipse]")
        {
            // Set up geometry
            ParameterList tSuperellipseParameterList = prm::create_geometry_parameter_list();
            tSuperellipseParameterList.set("type", "superellipse");
            tSuperellipseParameterList.set("field_variable_indices", "all");
            tSuperellipseParameterList.set("adv_indices", "all");

            // Create circles
            Matrix<DDRMat> tADVs = {{3.0, 4.0, 1.0, 2.0, 2.0, 1.0, 0.0, 0.0}};
            std::shared_ptr<Geometry> tSuperellipse = create_geometry(tSuperellipseParameterList, tADVs);

            // Set coordinates for checking
            Matrix<DDRMat> tCoordinates0 = {{2.0, 2.0}};
            Matrix<DDRMat> tCoordinates1 = {{3.0, 3.0}};
            Matrix<DDRMat> tCoordinates2 = {{4.0, 4.0}};

            // Check field values
            CHECK(tSuperellipse->get_field_value(0, tCoordinates0) == Approx(0.414214));
            CHECK(tSuperellipse->get_field_value(0, tCoordinates1) == Approx(-0.5));
            CHECK(tSuperellipse->get_field_value(0, tCoordinates2) == Approx(0.0));

            // Check sensitivity values
            check_equal(tSuperellipse->get_field_sensitivities(0, tCoordinates0),
                    {{ 7.071067811865476e-01, 3.535533905932738e-01, -7.071067811865476e-01, -3.535533905932738e-01,
                    MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX}});

            check_equal(tSuperellipse->get_field_sensitivities(0, tCoordinates1),
                    {{-0.000000000000000e+00, 5.000000000000000e-01, -0.000000000000000e+00, -2.500000000000000e-01,
                    MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX}});

            check_equal(tSuperellipse->get_field_sensitivities(0, tCoordinates2),
                    {{-1.000000000000000e+00, 0.000000000000000e+00, -1.000000000000000e+00, -0.000000000000000e+00,
                    MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX}});

            // Change ADVs and coordinates
            tADVs = {{2.0, 1.0, 4.0, 3.0, 4.0, 1.0, 0.0, 0.0}};
            tCoordinates0 = {{-2.0, 1.0}};
            tCoordinates1 = {{0.0, 2.5}};
            tCoordinates2 = {{2.0, 5.0}};

            // Check field values
            CHECK(tSuperellipse->get_field_value(0, tCoordinates0) == Approx(0.0));
            CHECK(tSuperellipse->get_field_value(0, tCoordinates1) == Approx(pow(2.0, -0.75) - 1.0));
            CHECK(tSuperellipse->get_field_value(0, tCoordinates2) == Approx(1.0 / 3.0));

            // Check sensitivity values
            check_equal(tSuperellipse->get_field_sensitivities(0, tCoordinates0),
                    {{0.25, 0.0, -0.25, 0.0,
                    MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX}});

            check_equal(tSuperellipse->get_field_sensitivities(0, tCoordinates1),
                    {{pow(2.0, 0.25) / 8.0, -pow(2.0, -0.75) / 3.0, -pow(2.0, -0.75) / 8.0, -pow(2.0, -0.75) / 6.0,
                    MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX}});

            check_equal(tSuperellipse->get_field_sensitivities(0, tCoordinates2),
                    {{0.0, -1.0 / 3.0, 0.0, -4.0 / 9.0,
                    MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX, MORIS_REAL_MAX}});
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Sphere", "[gen], [geometry], [sphere]")
        {
            // Set up geometry
            ParameterList tSphereParameterList = prm::create_geometry_parameter_list();
            tSphereParameterList.set("type", "sphere");
            tSphereParameterList.set("field_variable_indices", "all");
            tSphereParameterList.set("adv_indices", "all");

            // Create sphere
            Matrix<DDRMat> tADVs = {{-1.0, 0.0, 1.0, 2.0}};
            std::shared_ptr<Geometry> tSphere = create_geometry(tSphereParameterList, tADVs);

            // Set coordinates for checking
            Matrix<DDRMat> tCoordinates0 = {{0.0, 0.0, 0.0}};
            Matrix<DDRMat> tCoordinates1 = {{1.0, 1.0, 1.0}};
            Matrix<DDRMat> tCoordinates2 = {{2.0, 2.0, 2.0}};

            // Check field values
            CHECK(tSphere->get_field_value(0, tCoordinates0) == Approx(sqrt(2.0) - 2.0));
            CHECK(tSphere->get_field_value(0, tCoordinates1) == Approx(sqrt(5.0) - 2.0));
            CHECK(tSphere->get_field_value(0, tCoordinates2) == Approx(sqrt(14.0) - 2.0));

            // Check sensitivity values
            check_equal(tSphere->get_field_sensitivities(0, tCoordinates0), {{-sqrt(2.0)/ 2.0, 0.0, sqrt(2.0) / 2.0, -1.0}});
            check_equal(tSphere->get_field_sensitivities(0, tCoordinates1), {{-2.0 / sqrt(5.0), -1.0 / sqrt(5.0), 0.0, -1.0}});
            check_equal(tSphere->get_field_sensitivities(0, tCoordinates2), {{-3.0 / sqrt(14.0), -sqrt(2.0 / 7.0), -1.0 / sqrt(14.0), -1.0}});

            // Change ADVs and coordinates
            tADVs = {{0.0, 0.0, 1.0, 1.0}};
            tCoordinates1 = {{1.0, 1.0, -1.0}};
            tCoordinates2 = {{2.0, -2.0, 2.0}};

            // Check field values
            CHECK(tSphere->get_field_value(0, tCoordinates0) == Approx(0.0));
            CHECK(tSphere->get_field_value(0, tCoordinates1) == Approx(sqrt(6.0) - 1.0));
            CHECK(tSphere->get_field_value(0, tCoordinates2) == Approx(2.0));

            // Check sensitivity values
            check_equal(tSphere->get_field_sensitivities(0, tCoordinates0), {{0.0, 0.0, 1.0, -1.0}});
            check_equal(tSphere->get_field_sensitivities(0, tCoordinates1), {{-1.0 / sqrt(6.0), -1.0 / sqrt(6.0), sqrt(2.0 / 3.0), -1.0}});
            check_equal(tSphere->get_field_sensitivities(0, tCoordinates2), {{-2.0 / 3.0, 2.0 / 3.0, -1.0 / 3.0, -1.0}});
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Superellipsoid", "[gen], [geometry], [superellipsoid]")
        {
            // Set up geometry
            ParameterList tSuperellipsoidParameterList = prm::create_geometry_parameter_list();
            tSuperellipsoidParameterList.set("type", "superellipsoid");
            tSuperellipsoidParameterList.set("field_variable_indices", "all");
            tSuperellipsoidParameterList.set("adv_indices", "all");

            // Create circles
            Matrix<DDRMat> tADVs = {{3.0, 4.0, 5.0, 1.0, 2.0, 4.0, 3.0}};
            std::shared_ptr<Geometry> tSuperellipsoid = create_geometry(tSuperellipsoidParameterList, tADVs);

            // Set coordinates for checking
            Matrix<DDRMat> tCoordinates0 = {{2.0, 2.0, 5.0}};
            Matrix<DDRMat> tCoordinates1 = {{3.0, 3.0, 5.0}};
            Matrix<DDRMat> tCoordinates2 = {{4.0, 4.0, 5.0}};

            // Check field values
            CHECK(tSuperellipsoid->get_field_value(0, tCoordinates0) == Approx(pow(2.0, 1.0/3.0) - 1.0));
            CHECK(tSuperellipsoid->get_field_value(0, tCoordinates1) == Approx(-0.5));
            CHECK(tSuperellipsoid->get_field_value(0, tCoordinates2) == Approx(0.0));

            // Check sensitivity values
            check_equal(tSuperellipsoid->get_field_sensitivities(0, tCoordinates0),
                    {{pow(2.0, -2.0 / 3.0), pow(2.0, -5.0 / 3.0), 0.0,
                    -pow(2.0, -2.0 / 3.0), -pow(2.0, -5.0 / 3.0), 0.0, -0.0970335}});
            check_equal(tSuperellipsoid->get_field_sensitivities(0, tCoordinates1),
                    {{0.0, 0.5, 0.0, 0.0, -0.25, 0.0, 0.0}});
            check_equal(tSuperellipsoid->get_field_sensitivities(0, tCoordinates2),
                    {{-1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0}});

            // Change ADVs and coordinates
            tADVs = {{2.0, 1.0, 0.0, 5.0, 4.0, 3.0, 4.0}};
            tCoordinates0 = {{2.0, -3.0, 0.0}};
            tCoordinates1 = {{2.0, -1.0, 1.5}};
            tCoordinates2 = {{2.0, 1.0, 4.0}};

            // Check field values
            CHECK(tSuperellipsoid->get_field_value(0, tCoordinates0) == Approx(0.0));
            CHECK(tSuperellipsoid->get_field_value(0, tCoordinates1) == Approx(pow(2.0, -0.75) - 1.0));
            CHECK(tSuperellipsoid->get_field_value(0, tCoordinates2) == Approx(1.0 / 3.0));

            // Check sensitivity values
            check_equal(tSuperellipsoid->get_field_sensitivities(0, tCoordinates0),
                    {{0.0, 0.25, 0.0, 0.0, -0.25, 0.0, 0.0}});
            check_equal(tSuperellipsoid->get_field_sensitivities(0, tCoordinates1),
                    {{0.0, pow(2.0, 0.25) / 8.0, -pow(2.0, -0.75) / 3.0,
                    0.0, -pow(2.0, -0.75) / 8.0, -pow(2.0, -0.75) / 6.0, -0.0257572}});
            check_equal(tSuperellipsoid->get_field_sensitivities(0, tCoordinates2),
                    {{0.0, 0.0, -1.0 / 3.0, 0.0, 0.0, -4.0 / 9.0, 0.0}});
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("User-defined Geometry", "[gen], [geometry], [user-defined geometry]")
        {
            // Create user-defined geometry
            Matrix<DDRMat> tADVs = {{-1.0, 0.5}};
            std::shared_ptr<Geometry> tUserDefinedGeometry = std::make_shared<User_Defined_Field>(
                    tADVs,
                    Matrix<DDUMat>({{1, 0}}),
                    Matrix<DDUMat>({{0, 1}}),
                    Matrix<DDRMat>({{}}),
                    &user_defined_geometry_field,
                    &user_defined_geometry_sensitivity);

            // Set coordinates for checking
            Matrix<DDRMat> tCoordinates1 = {{1.0, 1.0}};
            Matrix<DDRMat> tCoordinates2 = {{2.0, 2.0}};

            // Check field values
            CHECK(tUserDefinedGeometry->get_field_value(0, tCoordinates1) == Approx(-0.75));
            CHECK(tUserDefinedGeometry->get_field_value(0, tCoordinates2) == Approx(-1.5));

            // Check sensitivity values
            check_equal(tUserDefinedGeometry->get_field_sensitivities(0, tCoordinates1), {{1.0, 3.0}});
            check_equal(tUserDefinedGeometry->get_field_sensitivities(0, tCoordinates2), {{2.0, 6.0}});

            // Change ADVs and coordinates
            tADVs = {{2.0, 0.5}};
            tCoordinates1 = {{0.0, 1.0}};
            tCoordinates2 = {{2.0, -1.0}};

            // Check field values
            CHECK(tUserDefinedGeometry->get_field_value(0, tCoordinates1) == Approx(8.0));
            CHECK(tUserDefinedGeometry->get_field_value(0, tCoordinates2) == Approx(-7.5));

            // Check sensitivity values
            check_equal(tUserDefinedGeometry->get_field_sensitivities(0, tCoordinates1), {{0.0, 12.0}});
            check_equal(tUserDefinedGeometry->get_field_sensitivities(0, tCoordinates2), {{2.0, -12.0}});
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("B-spline Geometry", "[gen], [geometry], [distributed advs], [B-spline geometry]")
        {
            // Loop over possible cases
            for (uint tCaseNumber = 0; tCaseNumber < 4; tCaseNumber++)
            {
                // Determine mesh orders
                uint tLagrangeOrder = 1;
                uint tBSplineOrder = 1;
                switch (tCaseNumber)
                {
                    case 1:
                    {
                        tLagrangeOrder = 2;
                        break;
                    }
                    case 2:
                    {
                        tBSplineOrder = 2;
                        break;
                    }
                    case 3:
                    {
                        tLagrangeOrder = 2;
                        tBSplineOrder = 2;
                        break;
                    }
                    default:
                    {
                        // Do nothing
                    }
                }

                // Create mesh
                uint tNumElementsPerDimension = 10;
                mtk::Interpolation_Mesh* tMesh = create_simple_mesh(
                        tNumElementsPerDimension,
                        tNumElementsPerDimension,
                        tLagrangeOrder,
                        tBSplineOrder);

                // B-spline circle parameter list
                real tRadius = 0.5;
                ParameterList tCircleParameterList = prm::create_geometry_parameter_list();
                tCircleParameterList.set("type", "circle");
                tCircleParameterList.set("constant_parameters", "0.0, 0.0, " + std::to_string(tRadius));
                tCircleParameterList.set("bspline_mesh_index", 0);
                tCircleParameterList.set("bspline_lower_bound", -1.0);
                tCircleParameterList.set("bspline_upper_bound", 1.0);

                // Set up geometry
                Matrix<DDRMat> tADVs(0, 0);
                std::shared_ptr<Geometry> tBSplineCircle = create_geometry(tCircleParameterList, tADVs);

                // Create geometry engine
                Geometry_Engine_Parameters tGeometryEngineParameters;
                tGeometryEngineParameters.mGeometries = {tBSplineCircle};
                Geometry_Engine_Test tGeometryEngine(tMesh, tGeometryEngineParameters);

                // Get ADVs and upper/lower bounds
                tADVs = tGeometryEngine.get_advs();
                Matrix<DDRMat> tLowerBounds = tGeometryEngine.get_lower_bounds();
                Matrix<DDRMat> tUpperBounds = tGeometryEngine.get_upper_bounds();

                // Set epsilon for checking
                real tEpsilon = std::numeric_limits<real>::epsilon() * 1000;

                // Check that ADVs were created and L2 was performed
                if (par_rank() == 0)
                {
                    uint tNumADVs = pow(tNumElementsPerDimension + tBSplineOrder, 2);
                    REQUIRE(tADVs.length() == tNumADVs);
                    REQUIRE(tLowerBounds.length() == tNumADVs);
                    REQUIRE(tUpperBounds.length() == tNumADVs);
                    for (uint tBSplineIndex = 0; tBSplineIndex < tNumADVs; tBSplineIndex++)
                    {
                        CHECK(tADVs(tBSplineIndex) != Approx(0.0).epsilon(tEpsilon));
                        CHECK(tLowerBounds(tBSplineIndex) == Approx(-1.0));
                        CHECK(tUpperBounds(tBSplineIndex) == Approx(1.0));
                    }
                }
                else
                {
                    REQUIRE(tADVs.length() == 0);
                    REQUIRE(tLowerBounds.length() == 0);
                    REQUIRE(tUpperBounds.length() == 0);
                }

                // Epsilon must be larger for a quadratic Lagrange mesh
                if (tLagrangeOrder > 1)
                {
                    tEpsilon = 0.04;
                }

                // Get geometry back
                tBSplineCircle = tGeometryEngine.get_geometry(0);

                // Check field values and sensitivities at all nodes
                Matrix<DDRMat> tTargetSensitivities;
                for (uint tNodeIndex = 0; tNodeIndex < tMesh->get_num_nodes(); tNodeIndex++)
                {
                    // Get node coordinates
                    Matrix<DDRMat> tNodeCoordinates = tMesh->get_node_coordinate(tNodeIndex);

                    // Set approximate field target
                    Approx tApproxTarget =
                            Approx(sqrt(pow(tNodeCoordinates(0), 2) + pow(tNodeCoordinates(1), 2)) - tRadius)
                            .scale(2.0)
                            .epsilon(tEpsilon);

                    // Check field value
                    CHECK(tBSplineCircle->get_field_value(tNodeIndex, {{}}) == tApproxTarget);

                    // Check sensitivities
                    if ((uint) par_rank() == tMesh->get_entity_owner(tNodeIndex, EntityRank::NODE, 0))
                    {
                        Matrix<DDRMat> tMatrix = trans(tMesh->get_t_matrix_of_node_loc_ind(tNodeIndex, 0));
                        check_equal(tBSplineCircle->get_field_sensitivities(tNodeIndex, {{}}), tMatrix);
                        check_equal(
                                tBSplineCircle->get_determining_adv_ids(tNodeIndex, {{}}),
                                tMesh->get_bspline_ids_of_node_loc_ind(tNodeIndex, 0));
                    }
                }

                // Set new ADVs
                tADVs = tADVs + (tRadius / 2.0);
                tGeometryEngine.set_advs(tADVs);

                // Check field values at all nodes again
                for (uint tNodeIndex = 0; tNodeIndex < tMesh->get_num_nodes(); tNodeIndex++)
                {
                    // Get node coordinates
                    Matrix<DDRMat> tNodeCoordinates = tMesh->get_node_coordinate(tNodeIndex);

                    // Set approximate target
                    Approx tApproxTarget =
                            Approx(sqrt(pow(tNodeCoordinates(0), 2) + pow(tNodeCoordinates(1), 2)) - (tRadius / 2.0))
                            .scale(2.0)
                            .epsilon(tEpsilon);

                    // Check field value
                    CHECK(tGeometryEngine.get_geometry_field_value(tNodeIndex, {{}}, 0) == tApproxTarget);
                }

                // Delete mesh pointer
                delete tMesh;
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Stored Geometry", "[gen], [geometry], [stored geometry]")
        {
            // Create mesh
            mtk::Interpolation_Mesh* tMesh = create_simple_mesh(6, 6);

            // Level set circle parameter list
            ParameterList tCircleParameterList = prm::create_geometry_parameter_list();
            tCircleParameterList.set("type", "circle");
            tCircleParameterList.set("field_variable_indices", "0, 1, 2");
            tCircleParameterList.set("adv_indices", "0, 1, 2");
            tCircleParameterList.set("bspline_mesh_index", -1);

            // Set up geometry
            Matrix<DDRMat> tADVs = {{0.0, 0.0, 0.5}};
            std::shared_ptr<Geometry> tCircle = create_geometry(tCircleParameterList, tADVs);

            // Create geometry engine
            Geometry_Engine_Parameters tGeometryEngineParameters;
            tGeometryEngineParameters.mGeometries = {tCircle};
            Geometry_Engine_Test tGeometryEngine(tMesh, tGeometryEngineParameters);

            // Get geometry back
            std::shared_ptr<Geometry> tStoredCircle = tGeometryEngine.get_geometry(0);

            // Check field values at all nodes
            for (uint tNodeIndex = 0; tNodeIndex < tMesh->get_num_nodes(); tNodeIndex++)
            {
                // Get node coordinates
                Matrix<DDRMat> tNodeCoordinates = tMesh->get_node_coordinate(tNodeIndex);

                // Check field value
                CHECK(tStoredCircle->get_field_value(tNodeIndex, {{}}) ==
                        Approx(tCircle->get_field_value(tNodeIndex, tNodeCoordinates)));

                // Check sensitivities
                check_equal(
                        tStoredCircle->get_field_sensitivities(tNodeIndex, {{}}),
                        tCircle->get_field_sensitivities(tNodeIndex, tNodeCoordinates));
                check_equal(
                        tStoredCircle->get_determining_adv_ids(tNodeIndex, {{}}),
                        tCircle->get_determining_adv_ids(tNodeIndex, tNodeCoordinates));
            }

            // Set new ADVs
            tADVs = {{1.0, 1.0, 1.0}};
            tGeometryEngine.set_advs(tADVs);

            // Check field values at all nodes again
            for (uint tNodeIndex = 0; tNodeIndex < tMesh->get_num_nodes(); tNodeIndex++)
            {
                // Get node coordinates
                Matrix<DDRMat> tNodeCoordinates = tMesh->get_node_coordinate(tNodeIndex);

                // Check field value
                CHECK(tStoredCircle->get_field_value(tNodeIndex, {{}}) ==
                        Approx(tCircle->get_field_value(tNodeIndex, tNodeCoordinates)));

                // Check sensitivities
                check_equal(
                        tStoredCircle->get_field_sensitivities(tNodeIndex, {{}}),
                        tCircle->get_field_sensitivities(tNodeIndex, tNodeCoordinates));
                check_equal(
                        tStoredCircle->get_determining_adv_ids(tNodeIndex, {{}}),
                        tCircle->get_determining_adv_ids(tNodeIndex, tNodeCoordinates));
            }

            // Delete mesh pointer
            delete tMesh;
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("MultiGeometry", "[gen], [geometry], [multigeometry]")
        {
            // ADV indices
            std::string tADVIndices1 = "0, 1, 3";
            std::string tADVIndices2 = "0, 2, 4";
            Matrix<DDSMat> tADVIndicesMat1 = string_to_mat<DDSMat>(tADVIndices1);
            Matrix<DDSMat> tADVIndicesMat2 = string_to_mat<DDSMat>(tADVIndices2);

            // Set up 2 circles
            Cell<ParameterList> tCircleParameterLists(2);
            tCircleParameterLists(0) = prm::create_geometry_parameter_list();
            tCircleParameterLists(0).set("type", "circle");
            tCircleParameterLists(0).set("field_variable_indices", "all");
            tCircleParameterLists(0).set("adv_indices", tADVIndices1);
            tCircleParameterLists(0).set("name", "circles");

            tCircleParameterLists(1) = prm::create_geometry_parameter_list();
            tCircleParameterLists(1).set("type", "circle");
            tCircleParameterLists(1).set("field_variable_indices", "all");
            tCircleParameterLists(1).set("adv_indices", tADVIndices2);
            tCircleParameterLists(1).set("name", "circles");

            // Create multigeometry
            Matrix<DDRMat> tADVs = {{0.0, 1.0, 2.0, 1.0, 2.0}};
            Cell<std::shared_ptr<Geometry>> tGeometries = create_geometries(tCircleParameterLists, tADVs);

            // Should be only one total geometry
            REQUIRE(tGeometries.size() == 1);
            std::shared_ptr<Geometry> tMultigeometry = tGeometries(0);

            // Set coordinates for checking
            Matrix<DDRMat> tCoordinates0 = {{0.0, 0.0}};
            Matrix<DDRMat> tCoordinates1 = {{1.0, 1.0}};
            Matrix<DDRMat> tCoordinates2 = {{2.0, 2.0}};

            // Check field values
            CHECK(tMultigeometry->get_field_value(0, tCoordinates0) == Approx(0.0));
            CHECK(tMultigeometry->get_field_value(0, tCoordinates1) == Approx(sqrt(2.0) - 2.0));
            CHECK(tMultigeometry->get_field_value(0, tCoordinates2) == Approx(0.0));

            // Check sensitivity values TODO determining IDs
            check_equal(tMultigeometry->get_field_sensitivities(0, tCoordinates0), {{0.0, 1.0, -1.0}});
            check_equal(tMultigeometry->get_field_sensitivities(0, tCoordinates1), {{-sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, -1.0}});
            check_equal(tMultigeometry->get_field_sensitivities(0, tCoordinates2), {{-1.0, 0.0, -1.0}});

            // Change ADVs and coordinates
            tADVs(0) = 1.0;
            tADVs(3) = 2.0;
            tADVs(4) = 3.0;
            tCoordinates0(0) = 1.0;
            tCoordinates0(1) = -1.0;
            tCoordinates1(0) = 3.0;
            tCoordinates1(1) = 1.0;
            tCoordinates2(0) = 4.0;
            tCoordinates2(1) = 2.0;

            // Check field values
            CHECK(tMultigeometry->get_field_value(0, tCoordinates0) == Approx(0.0));
            CHECK(tMultigeometry->get_field_value(0, tCoordinates1) == Approx(sqrt(5.0) - 3.0));
            CHECK(tMultigeometry->get_field_value(0, tCoordinates2) == Approx(0.0));

            // Check sensitivity values
            check_equal(tMultigeometry->get_field_sensitivities(0, tCoordinates0), {{0.0, 1.0, -1.0}});
            check_equal(tMultigeometry->get_field_sensitivities(0, tCoordinates1), {{-2.0 / sqrt(5.0), 1.0 / sqrt(5.0), -1.0}});
            check_equal(tMultigeometry->get_field_sensitivities(0, tCoordinates2), {{-1.0, 0.0, -1.0}});
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Swiss Cheese Slice", "[gen], [geometry], [swiss cheese slice]")
        {
            // Set up geometry
            ParameterList tSwissCheeseParameterList = prm::create_swiss_cheese_slice_parameter_list();
            tSwissCheeseParameterList.set("left_bound", -2.0);
            tSwissCheeseParameterList.set("right_bound", 2.0);
            tSwissCheeseParameterList.set("bottom_bound", -1.0);
            tSwissCheeseParameterList.set("top_bound", 1.0);
            tSwissCheeseParameterList.set("hole_x_semidiameter", 0.2);
            tSwissCheeseParameterList.set("hole_y_semidiameter", 0.1);
            tSwissCheeseParameterList.set("number_of_x_holes", 3);
            tSwissCheeseParameterList.set("number_of_y_holes", 5);

            // Create swiss cheese
            Matrix<DDRMat> tADVs = {{}};
            std::shared_ptr<Geometry> tSwissCheese = create_geometry(tSwissCheeseParameterList, tADVs);

            // Check holes
            check_swiss_cheese(tSwissCheese, -2.0, -1.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, -2.0, -0.5, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, -2.0, 0.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, -2.0, -0.5, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, -2.0, 1.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, -1.0, -1.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, -1.0, -0.5, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, -1.0, 0.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, -1.0, -0.5, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, -1.0, 1.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 0.0, -1.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 0.0, -0.5, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 0.0, 0.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 0.0, -0.5, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 0.0, 1.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 1.0, -1.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 1.0, -0.5, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 1.0, 0.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 1.0, -0.5, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 1.0, 1.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 2.0, -1.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 2.0, -0.5, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 2.0, 0.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 2.0, -0.5, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 2.0, 1.0, 0.2, 0.1);
            check_swiss_cheese(tSwissCheese, 3.0, -1.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 3.0, -0.5, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 3.0, 0.0, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 3.0, -0.5, 0.2, 0.1, false);
            check_swiss_cheese(tSwissCheese, 3.0, 1.0, 0.2, 0.1, false);

            // Create swiss cheese
            tSwissCheeseParameterList.set("number_of_x_holes", 0);
            tSwissCheeseParameterList.set("number_of_y_holes", 0);
            tSwissCheeseParameterList.set("hole_x_semidiameter", 0.1);
            tSwissCheeseParameterList.set("hole_y_semidiameter", 0.2);
            tSwissCheeseParameterList.set("target_x_spacing", 2.1);
            tSwissCheeseParameterList.set("target_y_spacing", 0.55);
            tSwissCheeseParameterList.set("row_offset", 1.0);
            tSwissCheese = create_geometry(tSwissCheeseParameterList, tADVs);
            
            // Check holes
            check_swiss_cheese(tSwissCheese, -2.0, -1.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, -2.0, -0.5, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, -2.0, 0.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, -2.0, -0.5, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, -2.0, 1.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, -1.0, -1.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, -1.0, -0.5, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, -1.0, 0.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, -1.0, -0.5, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, -1.0, 1.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 0.0, -1.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 0.0, -0.5, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 0.0, 0.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 0.0, -0.5, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 0.0, 1.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 1.0, -1.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 1.0, -0.5, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 1.0, 0.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 1.0, -0.5, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 1.0, 1.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 2.0, -1.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 2.0, -0.5, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 2.0, 0.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 2.0, -0.5, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 2.0, 1.0, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 3.0, -1.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 3.0, -0.5, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 3.0, 0.0, 0.1, 0.2, false);
            check_swiss_cheese(tSwissCheese, 3.0, -0.5, 0.1, 0.2);
            check_swiss_cheese(tSwissCheese, 3.0, 1.0, 0.1, 0.2, false);
            
            // Create swiss cheese
            tSwissCheeseParameterList.set("hole_x_semidiameter", 0.45);
            tSwissCheeseParameterList.set("hole_y_semidiameter", 0.45);
            tSwissCheeseParameterList.set("target_y_spacing", 1.1);
            tSwissCheeseParameterList.set("allow_less_than_target_spacing", false);
            tSwissCheeseParameterList.set("row_offset", 0.0);
            tSwissCheese = create_geometry(tSwissCheeseParameterList, tADVs);
            
            // Check holes
            check_swiss_cheese(tSwissCheese, -2.0, -1.0, 0.45, 0.45);
            check_swiss_cheese(tSwissCheese, -2.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, -2.0, 0.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, -2.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, -2.0, 1.0, 0.45, 0.45);
            check_swiss_cheese(tSwissCheese, -1.0, -1.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, -1.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, -1.0, 0.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, -1.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, -1.0, 1.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 0.0, -1.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 0.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 0.0, 0.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 0.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 0.0, 1.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 1.0, -1.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 1.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 1.0, 0.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 1.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 1.0, 1.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 2.0, -1.0, 0.45, 0.45);
            check_swiss_cheese(tSwissCheese, 2.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 2.0, 0.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 2.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 2.0, 1.0, 0.45, 0.45);
            check_swiss_cheese(tSwissCheese, 3.0, -1.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 3.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 3.0, 0.0, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 3.0, -0.5, 0.45, 0.45, false);
            check_swiss_cheese(tSwissCheese, 3.0, 1.0, 0.45, 0.45, false);
        }

        //------------------------------------------------------------------------------------------------------------------

        void check_swiss_cheese(std::shared_ptr<Geometry> aSwissCheese,
                                real aXCenter,
                                real aYCenter,
                                real aXSemidiameter,
                                real aYSemidiameter,
                                bool aCheck)
        {
            CHECK((aSwissCheese->get_field_value(0, {{aXCenter + aXSemidiameter, aYCenter}}) == Approx(0.0)) == aCheck);
            CHECK((aSwissCheese->get_field_value(0, {{aXCenter, aYCenter + aYSemidiameter}}) == Approx(0.0)) == aCheck);
            CHECK((aSwissCheese->get_field_value(0, {{aXCenter - aXSemidiameter, aYCenter}}) == Approx(0.0)) == aCheck);
            CHECK((aSwissCheese->get_field_value(0, {{aXCenter, aYCenter - aYSemidiameter}}) == Approx(0.0)) == aCheck);
        }
    }

    //------------------------------------------------------------------------------------------------------------------

    uint user_defined_phase_function(const ge::Geometry_Bitset& aGeometrySigns)
    {
        uint tPhaseIndex = 3;
        for (uint tGeometryIndex = 0; tGeometryIndex < 8; tGeometryIndex++)
        {
            if (aGeometrySigns.test(tGeometryIndex))
            {
                tPhaseIndex = tGeometryIndex;
            }
        }

        return tPhaseIndex;
    }

    //------------------------------------------------------------------------------------------------------------------

    real user_defined_geometry_field(const Matrix<DDRMat>& aCoordinates,
                                     const Cell<real*>&    aParameters)
    {
        return aCoordinates(0) * pow(*aParameters(0), 2) + aCoordinates(1) * pow(*aParameters(1), 3);
    }

    //------------------------------------------------------------------------------------------------------------------

    void user_defined_geometry_sensitivity(const Matrix<DDRMat>& aCoordinates,
                                           const Cell<real*>&    aParameters,
                                           Matrix<DDRMat>&       aSensitivities)
    {
        aSensitivities = {{2 * aCoordinates(0) * *aParameters(0), 3 * aCoordinates(1) * pow(*aParameters(1), 2)}};
    }

    //------------------------------------------------------------------------------------------------------------------

}
