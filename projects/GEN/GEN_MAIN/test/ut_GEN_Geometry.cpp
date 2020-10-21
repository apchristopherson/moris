#include "catch.hpp"
#include "cl_Matrix.hpp"
#include "fn_Parsing_Tools.hpp"
#include "fn_Exec_load_user_library.hpp"

#include "cl_GEN_Geometry_Engine.hpp"
#include "cl_GEN_User_Defined_Geometry.hpp"
#include "fn_GEN_create_geometries.hpp"
#include "fn_GEN_create_simple_mesh.hpp"
#include "fn_PRM_GEN_Parameters.hpp"

#include "cl_SOL_Matrix_Vector_Factory.hpp"

namespace moris
{

    //------------------------------------------------------------------------------------------------------------------

    // Dummy user-defined geometry
    real user_defined_geometry_field(const Matrix<DDRMat>& aCoordinates,
                                     const Cell<real*>&    aParameters);

    void user_defined_geometry_sensitivity(const Matrix<DDRMat>& aCoordinates,
                                           const Cell<real*>&    aParameters,
                                           Matrix<DDRMat>&       aSensitivities);

    // Approximate check for vectors
    template< typename Matrix_Type >
    void check_equal(Matrix<Matrix_Type> aMat1, Matrix<Matrix_Type> aMat2);

    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        // Class for testing (sometimes need access to geometry after GEN manipulation)
        class Geometry_Engine_Test : public Geometry_Engine
        {
        public:

            Geometry_Engine_Test(
                    Cell< std::shared_ptr<Geometry> > aGeometry,
                    Phase_Table                       aPhaseTable,
                    mtk::Interpolation_Mesh*          aMesh)
                    : Geometry_Engine(aGeometry, aPhaseTable, aMesh)
            {
            }

            std::shared_ptr<Geometry> get_geometry(uint aGeometryIndex = 0)
            {
                return mGeometries(aGeometryIndex);
            }
        };

        //--------------------------------------------------------------------------------------------------------------

        // Check for ellipse location in a swiss cheese
        void check_swiss_cheese(std::shared_ptr<Geometry> aSwissCheese,
                                real aXCenter,
                                real aYCenter,
                                real aXSemidiameter,
                                real aYSemidiameter,
                                bool aCheck = true);

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Circle", "[gen], [geometry], [distributed advs], [circle]")
        {
            // Set up geometry
            ParameterList tCircle1ParameterList = prm::create_geometry_parameter_list();
            tCircle1ParameterList.set("type", "circle");
            tCircle1ParameterList.set("geometry_variable_indices", "all");
            tCircle1ParameterList.set("adv_indices", "0, 1, 3");

            ParameterList tCircle2ParameterList = prm::create_geometry_parameter_list();
            tCircle2ParameterList.set("type", "circle");
            tCircle2ParameterList.set("geometry_variable_indices", "all");
            tCircle2ParameterList.set("adv_indices", "0, 2, 4");

            // ADV vector
            Matrix<DDRMat> tADVs;

            // Distributed ADVs
            sol::Matrix_Vector_Factory tDistributedFactory;
            Matrix<DDSMat> tADVIds = {{0}, {1}, {2}, {3}, {4}};
            std::shared_ptr<sol::Dist_Map> tADVMap = tDistributedFactory.create_map(tADVIds);
            sol::Dist_Vector* tDistributedADVs = tDistributedFactory.create_vector(tADVMap);

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
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Superellipse", "[gen], [geometry], [superellipse]")
        {
            // Set up geometry
            ParameterList tSuperellipseParameterList = prm::create_geometry_parameter_list();
            tSuperellipseParameterList.set("type", "superellipse");
            tSuperellipseParameterList.set("geometry_variable_indices", "all");
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
            tSphereParameterList.set("geometry_variable_indices", "all");
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
            tSuperellipsoidParameterList.set("geometry_variable_indices", "all");
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

        TEST_CASE("Linear B-spline Geometry", "[gen], [geometry], [distributed advs], [B-spline geometry], [B-spline geometry linear]")
        {
            // Create mesh
            mtk::Interpolation_Mesh* tMesh = create_simple_mesh(6, 6);

            // Circle parameters
            real tRadius = 0.5;
            real tLowerBound = -2.0;
            real tUpperBound = 2.0;

            // Level set circle parameter list
            ParameterList tCircleParameterList = prm::create_geometry_parameter_list();
            tCircleParameterList.set("type", "circle");
            tCircleParameterList.set("constant_parameters", "0.0, 0.0, " + std::to_string(tRadius));
            tCircleParameterList.set("bspline_mesh_index", 0);
            tCircleParameterList.set("bspline_lower_bound", tLowerBound);
            tCircleParameterList.set("bspline_upper_bound", tUpperBound);

            // Set up geometry
            Matrix<DDRMat> tADVs(0, 0);
            std::shared_ptr<Geometry> tBSplineCircle = create_geometry(tCircleParameterList, tADVs);

            // Create geometry engine
            Phase_Table tPhaseTable(1);
            Geometry_Engine_Test tGeometryEngine({tBSplineCircle}, tPhaseTable, tMesh);

            // Get ADVs/bounds
            tADVs = tGeometryEngine.get_advs();
            Matrix<DDRMat> tLowerBounds = tGeometryEngine.get_lower_bounds();
            Matrix<DDRMat> tUpperBounds = tGeometryEngine.get_upper_bounds();

            // Check that ADVs and bounds were created (specific values not checked)
            if (par_rank() == 0)
            {
                REQUIRE(tADVs.length() == 49);
                REQUIRE(tLowerBounds.length() == 49);
                REQUIRE(tUpperBounds.length() == 49);
                for (uint tBSplineIndex = 0; tBSplineIndex < 49; tBSplineIndex++)
                {
                    CHECK(tADVs(tBSplineIndex) != Approx(0.0));
                    CHECK(tLowerBounds(tBSplineIndex) == Approx(tLowerBound));
                    CHECK(tUpperBounds(tBSplineIndex) == Approx(tUpperBound));
                }
            }
            else
            {
                REQUIRE(tADVs.length() == 0);
                REQUIRE(tLowerBounds.length() == 0);
                REQUIRE(tUpperBounds.length() == 0);
            }

            // Get geometry back
            tBSplineCircle = tGeometryEngine.get_geometry(0);

            // Check field values at all nodes
            for (uint tNodeIndex = 0; tNodeIndex < tMesh->get_num_nodes(); tNodeIndex++)
            {
                // Get node coordinates
                Matrix<DDRMat> tNodeCoordinates = tMesh->get_node_coordinate(tNodeIndex);

                // Set approximate target
                Approx tApproxTarget =
                        Approx(sqrt(pow(tNodeCoordinates(0), 2) + pow(tNodeCoordinates(1), 2)) - tRadius);

                // Check field value
                CHECK(tBSplineCircle->get_field_value(tNodeIndex, {{}}) == tApproxTarget);

                // Check sensitivities
                if (par_rank() == tMesh->get_entity_owner(tNodeIndex, EntityRank::NODE, 0))
                {
                    check_equal(
                            tBSplineCircle->get_field_sensitivities(tNodeIndex, {{}}),
                            tMesh->get_t_matrix_of_node_loc_ind(tNodeIndex, EntityRank::BSPLINE));
                    check_equal(
                            tBSplineCircle->get_determining_adv_ids(tNodeIndex, {{}}),
                            tMesh->get_bspline_inds_of_node_loc_ind(tNodeIndex, EntityRank::BSPLINE));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Nonlinear B-spline Geometry", "[gen], [geometry], [distributed advs], [B-spline geometry], [B-spline geometry nonlinear]")
        {
            if (par_size() == 1)
            {
                // Loop over possible cases
                for (uint tCaseNumber = 0; tCaseNumber <= 1; tCaseNumber++)
                {
                    // Determine mesh orders
                    uint tLagrangeOrder = 1;
                    uint tBSplineOrder = 1;
                    switch (tCaseNumber)
                    {
                        case 0:
                        {
                            tLagrangeOrder = 2;
                            break;
                        }
                        case 1:
                        {
                            tBSplineOrder = 2;
                            break;
                        }
                        default:
                        {
                            MORIS_ERROR(false, "Case not defined.");
                        }
                    }

                    // Create mesh
                    mtk::Interpolation_Mesh* tMesh = create_simple_mesh(2, 2, tLagrangeOrder, tBSplineOrder);

                    // Level set circle parameter list
                    real tRadius = 0.5;
                    ParameterList tCircleParameterList = prm::create_geometry_parameter_list();
                    tCircleParameterList.set("type", "circle");
                    tCircleParameterList.set("constant_parameters", "0.0, 0.0, " + std::to_string(tRadius));
                    tCircleParameterList.set("bspline_mesh_index", 0);

                    // Set up geometry
                    Matrix<DDRMat> tADVs(0, 0);
                    std::shared_ptr<Geometry> tBSplineCircle = create_geometry(tCircleParameterList, tADVs);

                    // Create geometry engine
                    Phase_Table tPhaseTable(1);
                    Geometry_Engine_Test tGeometryEngine({tBSplineCircle}, tPhaseTable, tMesh);

                    // Check that ADVs were created and L2 was performed
                    tADVs = tGeometryEngine.get_advs();
                    uint tNumADVs = pow(2 + tBSplineOrder, 2);
                    REQUIRE(tADVs.length() == tNumADVs);
                    for (uint tBSplineIndex = 0; tBSplineIndex < tNumADVs; tBSplineIndex++)
                    {
                        CHECK(tADVs(tBSplineIndex) != Approx(0.0));
                    }

                    // Set epsilon for checking 
                    real tEpsilon = std::numeric_limits<real>::epsilon()*100;
                    
                    // Epsilon must be much greater if Lagrange order is greater than B-spline order
                    if (tLagrangeOrder > tBSplineOrder)
                    {
                        tEpsilon = 0.5;
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
                        Approx tTargetFieldValue =
                                Approx(sqrt(pow(tNodeCoordinates(0), 2) + pow(tNodeCoordinates(1), 2)) - tRadius)
                                .epsilon(tEpsilon);

                        // Check field value
                        CHECK(tBSplineCircle->get_field_value(tNodeIndex, {{}}) == tTargetFieldValue);

                        // Check sensitivities
                        check_equal(
                                tBSplineCircle->get_field_sensitivities(tNodeIndex, {{}}),
                                tMesh->get_t_matrix_of_node_loc_ind(tNodeIndex, EntityRank::BSPLINE));
                        check_equal(
                                tBSplineCircle->get_determining_adv_ids(tNodeIndex, {{}}),
                                tMesh->get_bspline_inds_of_node_loc_ind(tNodeIndex, EntityRank::BSPLINE));
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
                                .epsilon(tEpsilon);

                        // Check field value
                        CHECK(tGeometryEngine.get_geometry_field_value(tNodeIndex, {{}}, 0) == tApproxTarget);
                    }

                    // Clean up
                    delete tMesh;
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("User-defined Geometry", "[gen], [geometry], [user-defined geometry]")
        {
            // Create user-defined geometry
            Matrix<DDRMat> tADVs = {{-1.0, 0.5}};
            std::shared_ptr<Geometry> tUserDefinedGeometry = std::make_shared<User_Defined_Geometry>(
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
            tCircleParameterLists(0).set("geometry_variable_indices", "all");
            tCircleParameterLists(0).set("adv_indices", tADVIndices1);
            tCircleParameterLists(0).set("multigeometry_id", "circles");

            tCircleParameterLists(1) = prm::create_geometry_parameter_list();
            tCircleParameterLists(1).set("type", "circle");
            tCircleParameterLists(1).set("geometry_variable_indices", "all");
            tCircleParameterLists(1).set("adv_indices", tADVIndices2);
            tCircleParameterLists(1).set("multigeometry_id", "circles");

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

    template< typename Matrix_Type >
    void check_equal(Matrix<Matrix_Type> aMat1, Matrix<Matrix_Type> aMat2)
    {
        REQUIRE(aMat1.n_rows() == aMat2.n_rows());
        REQUIRE(aMat1.n_cols() == aMat2.n_cols());
        for (uint tRowIndex = 0; tRowIndex < aMat1.n_rows(); tRowIndex++)
        {
            for (uint tColumnIndex = 0; tColumnIndex < aMat1.n_cols(); tColumnIndex++)
            {
                CHECK(aMat1(tRowIndex, tColumnIndex) == Approx(aMat2(tRowIndex, tColumnIndex)));
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------

}
