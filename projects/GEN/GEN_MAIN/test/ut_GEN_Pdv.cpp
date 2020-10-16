#include "catch.hpp"
#include "cl_Matrix.hpp"
#include "cl_SOL_Matrix_Vector_Factory.hpp"
#include "cl_MSI_Design_Variable_Interface.hpp"

#include "fn_GEN_create_geometries.hpp"
#include "fn_GEN_create_properties.hpp"
#include "fn_PRM_GEN_Parameters.hpp"

#define protected public
#define private   public
#include "cl_GEN_Geometry_Engine.hpp"
#include "cl_GEN_Pdv_Host_Manager.hpp"
#include "cl_GEN_Intersection_Node.hpp"
#undef protected
#undef private

namespace moris
{

    //------------------------------------------------------------------------------------------------------------------

    // Dummy IQI sensitivity values so a FEM model doesn't have to be created
    uint gNumPDVs = 36;
    Matrix<DDRMat> gdIQIdPDV1(1, gNumPDVs);
    Matrix<DDRMat> gdIQIdPDV2(1, gNumPDVs);

    sol::Dist_Vector* MSI::Design_Variable_Interface::get_dQIdp()
    {
        // Factory
        sol::Matrix_Vector_Factory tDistributedFactory;

        // PDV/ADV IDs and sensitivities
        Matrix<DDSMat> tPDVIds(gNumPDVs, 1);
        for (uint tPDVIndex = 0; tPDVIndex < gNumPDVs; tPDVIndex++)
        {
            tPDVIds(tPDVIndex) = tPDVIndex;
            gdIQIdPDV1(tPDVIndex) = 1.0;
            gdIQIdPDV2(tPDVIndex) = (real)tPDVIndex;
        }

        // IQI sensitivity vector
        std::shared_ptr<sol::Dist_Map> tPDVMap = tDistributedFactory.create_map(tPDVIds);
        sol::Dist_Vector* tdIQIdPDV = tDistributedFactory.create_vector(tPDVMap, 2);

        // Fill values
        tdIQIdPDV->replace_global_values(tPDVIds, gdIQIdPDV1, 0);
        tdIQIdPDV->replace_global_values(tPDVIds, gdIQIdPDV2, 1);

        return tdIQIdPDV;
    }

    namespace ge
    {

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Interpolation PDV Creation", "[gen], [pdv], [interpolation pdv], [interpolation pdv serial]")
        {
            if( par_size() == 1)
            {
                // Create PDV_Type host manager
                Pdv_Host_Manager tPdvHostManager;

                tPdvHostManager.mPdvTypeList = {PDV_Type::DENSITY, PDV_Type::TEMPERATURE, PDV_Type::ELASTIC_MODULUS};
                tPdvHostManager.mPdvTypeMap.set_size(10, 1, -1);
                tPdvHostManager.mPdvTypeMap( 3 ) = 0;
                tPdvHostManager.mPdvTypeMap( 4 ) = 1;
                tPdvHostManager.mPdvTypeMap( 5 ) = 2;

                // ----------------- Interpolation PDVs ---------------------- //
                // Node indices per set
                Cell<Matrix<DDSMat>> tIpNodeIndicesPerSet(2);
                tIpNodeIndicesPerSet(0).resize(4, 1);
                tIpNodeIndicesPerSet(1).resize(4, 1);
                tIpNodeIndicesPerSet(0) = {{0, 1, 2, 3}};
                tIpNodeIndicesPerSet(1) = {{2, 3, 4, 5}};

                Cell<Matrix<DDSMat>> tIpNodeIdsPerSet(2);
                tIpNodeIdsPerSet(0).resize(4, 1);
                tIpNodeIdsPerSet(1).resize(4, 1);
                tIpNodeIdsPerSet(0) = {{0, 1, 2, 3}};
                tIpNodeIdsPerSet(1) = {{2, 3, 4, 5}};

                Cell<Matrix<DDSMat>> tIpNodeOwnersPerSet(2);
                tIpNodeOwnersPerSet(0).resize(4, 1);
                tIpNodeOwnersPerSet(1).resize(4, 1);
                tIpNodeOwnersPerSet(0) = {{0, 0, 0, 0}};
                tIpNodeOwnersPerSet(1) = {{0, 0, 0, 0}};

                // PDV_Type types per set
                Cell<Cell<Cell<PDV_Type>>> tIpPdvTypes(2);
                tIpPdvTypes(0).resize(2);
                tIpPdvTypes(1).resize(2);
                tIpPdvTypes(0)(0).resize(1);
                tIpPdvTypes(0)(1).resize(1);
                tIpPdvTypes(1)(0).resize(1);
                tIpPdvTypes(1)(1).resize(1);
                tIpPdvTypes(0)(0)(0) = PDV_Type::DENSITY;
                tIpPdvTypes(0)(1)(0) = PDV_Type::TEMPERATURE;
                tIpPdvTypes(1)(0)(0) = PDV_Type::TEMPERATURE;
                tIpPdvTypes(1)(1)(0) = PDV_Type::ELASTIC_MODULUS;

                // Create PDV_Type hosts
                tPdvHostManager.create_interpolation_pdv_hosts(
                        tIpNodeIndicesPerSet,
                        tIpNodeIdsPerSet,
                        tIpNodeOwnersPerSet,
                        Cell<Matrix<F31RMat>>(6),
                        tIpPdvTypes);

                // Set PDVs
                for (uint tMeshSetIndex = 0; tMeshSetIndex < 2; tMeshSetIndex++)
                {
                    for (uint tNodeIndex = 0; tNodeIndex < 4; tNodeIndex++)
                    {
                        for (uint tPdvIndex = 0; tPdvIndex < 2; tPdvIndex++)
                        {
                            tPdvHostManager.create_interpolation_pdv(
                                    (uint)tIpNodeIndicesPerSet(tMeshSetIndex)(tNodeIndex),
                                    tIpPdvTypes(tMeshSetIndex)(tPdvIndex)(0),
                                    (real)tMeshSetIndex);
                        }
                    }
                }

                tPdvHostManager.create_pdv_ids();

                // Check PDVs
                Cell<Matrix<DDRMat>> tPdvValues;
                for (uint tMeshSetIndex = 0; tMeshSetIndex < 2; tMeshSetIndex++)
                {
                    for (uint tPdvIndex = 0; tPdvIndex < 2; tPdvIndex++)
                    {
                        tPdvValues.clear();
                        tPdvHostManager.get_ip_pdv_value(tIpNodeIndicesPerSet(tMeshSetIndex), tIpPdvTypes(tMeshSetIndex)(tPdvIndex), tPdvValues);

                        for (uint tNodeIndex = 0; tNodeIndex < 4; tNodeIndex++)
                        {
                            CHECK(tPdvValues(0)(tNodeIndex) == tMeshSetIndex +
                                    (tMeshSetIndex == 0) * (tNodeIndex > 1) *
                                    (tIpPdvTypes(tMeshSetIndex)(tPdvIndex)(0) == PDV_Type::TEMPERATURE));
                        }
                    }
                }

                // ------------------- Check global map ----------------------- //
                const Matrix<DDSMat> & tLocalGlobalMap = tPdvHostManager.get_my_local_global_map();

                REQUIRE(tLocalGlobalMap.length() == 14);
                for (int tGlobalPdvIndex = 0; tGlobalPdvIndex < 14; tGlobalPdvIndex++)
                {
                    CHECK(tLocalGlobalMap(tGlobalPdvIndex) == tGlobalPdvIndex);
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Parallel Interpolation PDV Creation", "[gen], [pdv], [interpolation pdv], [interpolation pdv parallel]")
        {
            if( par_size() == 2)
            {
                // Create PDV_Type host manager
                Pdv_Host_Manager tPdvHostManager;

                tPdvHostManager.mPdvTypeList = {PDV_Type::DENSITY, PDV_Type::TEMPERATURE};
                tPdvHostManager.mPdvTypeMap.set_size(10, 1, -1);
                tPdvHostManager.mPdvTypeMap( 3 ) = 0;
                tPdvHostManager.mPdvTypeMap( 4 ) = 1;

                // ----------------- Interpolation PDVs ---------------------- //

                Cell<Matrix<DDSMat>> tIpNodeIndicesPerSet(1);
                tIpNodeIndicesPerSet(0).resize(4, 1);

                Cell<Matrix<DDSMat>> tIpNodeIdsPerSet(1);
                tIpNodeIdsPerSet(0).resize(4, 1);

                Cell<Matrix<DDSMat>> tIpNodeOwnersPerSet(1);
                tIpNodeOwnersPerSet(0).resize(4, 1);

                // PDV_Type types per set
                Cell<Cell<Cell<PDV_Type>>> tIpPdvTypes(1);
                tIpPdvTypes(0).resize(2);
                tIpPdvTypes(0)(0).resize(1);
                tIpPdvTypes(0)(1).resize(1);
                tIpPdvTypes(0)(0)(0) = PDV_Type::DENSITY;
                tIpPdvTypes(0)(1)(0) = PDV_Type::TEMPERATURE;

                if( par_rank() == 0)
                {
                    // Node indices per set
                    tIpNodeIndicesPerSet(0) = {{0, 1, 2, 3}};

                    tIpNodeIdsPerSet(0) = {{0, 1, 2, 3}};

                    tIpNodeOwnersPerSet(0) = {{0, 0, 0, 1}};

                    tPdvHostManager.mCommTable.set_size( 2, 1, 0);
                    tPdvHostManager.mCommTable( 1, 0 ) = 1;

                    tPdvHostManager.mIPVertexIdtoIndMap[ 2 ] = 2;
                }
                else if( par_rank() == 1)
                {
                    // Node indices per set
                    tIpNodeIndicesPerSet(0) = {{0, 1, 2, 3}};

                    tIpNodeIdsPerSet(0) = {{2, 3, 4, 5}};

                    tIpNodeOwnersPerSet(0) = {{0, 1, 1, 1}};

                    tPdvHostManager.mCommTable.set_size( 2, 1, 1);
                    tPdvHostManager.mCommTable( 1, 0 ) = 0;

                    tPdvHostManager.mIPVertexIdtoIndMap[ 3 ] = 1;
                }

                // Create PDV_Type hosts
                tPdvHostManager.create_interpolation_pdv_hosts(
                        tIpNodeIndicesPerSet,
                        tIpNodeIdsPerSet,
                        tIpNodeOwnersPerSet,
                        Cell<Matrix<F31RMat>>(4),
                        tIpPdvTypes);

                // Set PDVs
                for (uint tMeshSetIndex = 0; tMeshSetIndex < 1; tMeshSetIndex++)
                {
                    for (uint tNodeIndex = 0; tNodeIndex < 4; tNodeIndex++)
                    {
                        for (uint tPdvIndex = 0; tPdvIndex < 2; tPdvIndex++)
                        {
                            tPdvHostManager.create_interpolation_pdv(
                                    (uint)tIpNodeIndicesPerSet(tMeshSetIndex)(tNodeIndex),
                                    tIpPdvTypes(tMeshSetIndex)(tPdvIndex)(0),
                                    (real)tMeshSetIndex);
                        }
                    }
                }

                tPdvHostManager.create_pdv_ids();

                // ------------------- Check global map ----------------------- //
                const Matrix<DDSMat> & tLocalGlobalMap = tPdvHostManager.get_my_local_global_map();
                const Matrix<DDSMat> & tLocalGlobalOSMap = tPdvHostManager.get_my_local_global_overlapping_map();

                REQUIRE(tLocalGlobalMap.length() == 6);
                REQUIRE(tLocalGlobalOSMap.length() == 8);

                if( par_rank() == 0)
                {
                    CHECK(tLocalGlobalMap(0) == 0);                    CHECK(tLocalGlobalMap(1) == 1);
                    CHECK(tLocalGlobalMap(2) == 2);                    CHECK(tLocalGlobalMap(3) == 3);
                    CHECK(tLocalGlobalMap(4) == 4);                    CHECK(tLocalGlobalMap(5) == 5);

                    CHECK(tLocalGlobalOSMap(0) == 0);                  CHECK(tLocalGlobalOSMap(1) == 1);
                    CHECK(tLocalGlobalOSMap(2) == 2);                  CHECK(tLocalGlobalOSMap(3) == 6);
                    CHECK(tLocalGlobalOSMap(4) == 3);                  CHECK(tLocalGlobalOSMap(5) == 4);
                    CHECK(tLocalGlobalOSMap(6) == 5);                  CHECK(tLocalGlobalOSMap(7) == 9);
                }
                if( par_rank() == 1)
                {
                    CHECK(tLocalGlobalMap(0) == 6);                    CHECK(tLocalGlobalMap(1) == 7);
                    CHECK(tLocalGlobalMap(2) == 8);                    CHECK(tLocalGlobalMap(3) == 9);
                    CHECK(tLocalGlobalMap(4) == 10);                   CHECK(tLocalGlobalMap(5) == 11);

                    CHECK(tLocalGlobalOSMap(0) == 2);                  CHECK(tLocalGlobalOSMap(1) == 6);
                    CHECK(tLocalGlobalOSMap(2) == 7);                  CHECK(tLocalGlobalOSMap(3) == 8);
                    CHECK(tLocalGlobalOSMap(4) == 5);                  CHECK(tLocalGlobalOSMap(5) == 9);
                    CHECK(tLocalGlobalOSMap(6) == 10);                 CHECK(tLocalGlobalOSMap(7) == 11);
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("Intersection PDV Creation", "[gen], [pdv], [intersection pdv]")
        {
            if( par_size() == 2)
            {
                // Create PDV_Type host manager
                Pdv_Host_Manager tPdvHostManager;
                tPdvHostManager.set_num_background_nodes(0);

                Matrix<IdMat> tIpNodeIdsPerSet(4,1);

                Matrix<DDSMat> tIpNodeOwnersPerSet(4,1);

                Cell< std::shared_ptr<Intersection_Node> > tIntersectionNodes(4);

                if( par_rank() == 0)
                {
                    tIpNodeIdsPerSet = {{0}, {1}, {2}, {3}};

                    tIpNodeOwnersPerSet = {{0}, {0}, {0}, {1}};

                    tPdvHostManager.mCommTable.set_size( 2, 1, 0);
                    tPdvHostManager.mCommTable( 1, 0 ) = 1;

                    tPdvHostManager.mIGVertexIdtoIndMap[ 2 ] = 2;
                }
                else if( par_rank() == 1)
                {
                    tIpNodeIdsPerSet = {{2}, {3}, {4}, {5}};

                    tIpNodeOwnersPerSet = {{0}, {1}, {1}, {1}};

                    tPdvHostManager.mCommTable.set_size( 2, 1, 1);
                    tPdvHostManager.mCommTable( 1, 0 ) = 0;

                    tPdvHostManager.mIGVertexIdtoIndMap[ 3 ] = 1;
                }


                for( sint Ik = 0; Ik < 4; Ik++ )
                {
                    tIntersectionNodes( Ik ) = std::make_shared<Intersection_Node>();

                    tIntersectionNodes( Ik )->mGlobalCoordinates.set_size( 2, 1 );
                    tPdvHostManager.set_intersection_node( Ik, tIntersectionNodes( Ik ) );
                    tPdvHostManager.update_intersection_node( Ik, tIpNodeIdsPerSet( Ik ), tIpNodeOwnersPerSet( Ik ));
                }

                tPdvHostManager.create_pdv_ids();

                // ------------------- Check global map ----------------------- //
                const Matrix<DDSMat> & tLocalGlobalMap = tPdvHostManager.get_my_local_global_map();
                const Matrix<DDSMat> & tLocalGlobalOSMap = tPdvHostManager.get_my_local_global_overlapping_map();

                REQUIRE(tLocalGlobalMap.length() == 6);
                REQUIRE(tLocalGlobalOSMap.length() == 8);

                if (par_rank() == 0)
                {
                    CHECK(tLocalGlobalMap(0) == 0);                    CHECK(tLocalGlobalMap(1) == 1);
                    CHECK(tLocalGlobalMap(2) == 2);                    CHECK(tLocalGlobalMap(3) == 3);
                    CHECK(tLocalGlobalMap(4) == 4);                    CHECK(tLocalGlobalMap(5) == 5);

                    CHECK(tLocalGlobalOSMap(0) == 0);                  CHECK(tLocalGlobalOSMap(1) == 1);
                    CHECK(tLocalGlobalOSMap(2) == 2);                  CHECK(tLocalGlobalOSMap(3) == 3);
                    CHECK(tLocalGlobalOSMap(4) == 4);                  CHECK(tLocalGlobalOSMap(5) == 5);
                    CHECK(tLocalGlobalOSMap(6) == 6);                  CHECK(tLocalGlobalOSMap(7) == 7);
                }
                if (par_rank() == 1)
                {
                    CHECK(tLocalGlobalMap(0) == 6);                    CHECK(tLocalGlobalMap(1) == 7);
                    CHECK(tLocalGlobalMap(2) == 8);                    CHECK(tLocalGlobalMap(3) == 9);
                    CHECK(tLocalGlobalMap(4) == 10);                   CHECK(tLocalGlobalMap(5) == 11);

                    CHECK(tLocalGlobalOSMap(0) == 4);                  CHECK(tLocalGlobalOSMap(1) == 5);
                    CHECK(tLocalGlobalOSMap(2) == 6);                  CHECK(tLocalGlobalOSMap(3) == 7);
                    CHECK(tLocalGlobalOSMap(4) == 8);                  CHECK(tLocalGlobalOSMap(5) == 9);
                    CHECK(tLocalGlobalOSMap(6) == 10);                 CHECK(tLocalGlobalOSMap(7) == 11);
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        TEST_CASE("PDV Sensitivities", "[gen], [pdv], [sensitivity], [pdv sensitivity]")
        {
            if (par_size() == 1)
            {
                // Create PDV_Type host manager
                Pdv_Host_Manager tPdvHostManager;
                tPdvHostManager.mPdvTypeList = {PDV_Type::DENSITY };
                tPdvHostManager.mPdvTypeMap.set_size(10, 1, -1);
                tPdvHostManager.mPdvTypeMap( 3 ) = 0;

                // Create discrete property
                ParameterList tParameterList = moris::prm::create_gen_property_parameter_list();;
                tParameterList.set("type", "discrete");
                tParameterList.set("property_variable_indices", "all");
                tParameterList.set("adv_indices", "all");
                tParameterList.set("pdv_type", "DENSITY");

                // Create property
                Matrix<DDRMat> tADVs(gNumPDVs, 1);
                std::shared_ptr<Property> tProperty = create_property(
                        tParameterList, 
                        tADVs, 
                        Cell<std::shared_ptr<moris::ge::Property>>(0));

                // Node indices per set
                Cell<Matrix<DDSMat>> tIpNodeIndicesPerSet(1);
                Cell<Matrix<DDSMat>> tIpNodeIdsPerSet(1);
                Cell<Matrix<DDSMat>> tIpNodeOWnersPerSet(1);
                tIpNodeIndicesPerSet(0).set_size(gNumPDVs, 1);
                tIpNodeIdsPerSet(0).set_size(gNumPDVs, 1);
                tIpNodeOWnersPerSet(0).set_size(gNumPDVs, 1, 0);
                for (uint tNodeIndex = 0; tNodeIndex < gNumPDVs; tNodeIndex++)
                {
                    tIpNodeIndicesPerSet(0)(tNodeIndex) = tNodeIndex;
                    tIpNodeIdsPerSet(0)(tNodeIndex) = tNodeIndex;
                }

                // PDV_Type types per set
                Cell<Cell<Cell<PDV_Type>>> tIpPdvTypes(1);
                tIpPdvTypes(0).resize(1);
                tIpPdvTypes(0)(0).resize(1);
                tIpPdvTypes(0)(0)(0) = PDV_Type::DENSITY;

                // Create PDV_Type hosts
                tPdvHostManager.create_interpolation_pdv_hosts(
                        tIpNodeIndicesPerSet,
                        tIpNodeIdsPerSet,
                        tIpNodeOWnersPerSet,
                        Cell<Matrix<F31RMat>>(gNumPDVs),
                        tIpPdvTypes);

                // Set PDVs
                for (uint tNodeIndex = 0; tNodeIndex < gNumPDVs; tNodeIndex++)
                {
                    tPdvHostManager.create_interpolation_pdv(uint(tIpNodeIndicesPerSet(0)(tNodeIndex)), tIpPdvTypes(0)(0)(0), tProperty);
                }
                tPdvHostManager.create_pdv_ids();

                // Set ADV IDs
                Matrix<DDSMat> tADVIds(gNumPDVs, 1);
                for (uint tADVIndex = 0; tADVIndex < gNumPDVs; tADVIndex++)
                {
                    tADVIds(tADVIndex) = tADVIndex;
                }
                tPdvHostManager.set_owned_adv_ids(tADVIds);

                // Check size of sensitivities matrix
                Matrix<DDRMat> tdIQIdADV = tPdvHostManager.compute_diqi_dadv();
                REQUIRE(tdIQIdADV.n_rows() == 2);
                REQUIRE(tdIQIdADV.n_cols() == gNumPDVs);

                // Check first IQI sensitivities
                for (uint tPDVIndex = 0; tPDVIndex < gNumPDVs; tPDVIndex++)
                {
                    CHECK(tdIQIdADV(0, tPDVIndex) == Approx(gdIQIdPDV1(tPDVIndex)));
                }

                // Check second IQI sensitivities
                for (uint tPDVIndex = 0; tPDVIndex < gNumPDVs; tPDVIndex++)
                {
                    CHECK(tdIQIdADV(1, tPDVIndex) == Approx(gdIQIdPDV2(tPDVIndex)));
                }
            }
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
