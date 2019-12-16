/*
 * cl_XTK_Sensitivity.cpp
 *
 *  Created on: Jan 25, 2018
 *      Author: ktdoble
 */


#include <memory>
#include <mpi.h>
#include "catch.hpp"
#include "cl_Logger.hpp"

// XTKL: Linear Algebra Includes

#include "cl_Matrix.hpp"
#include "cl_XTK_Matrix_Base_Utilities.hpp"
#include "linalg_typedefs.hpp"
#include "fn_norm.hpp"
#include "op_minus.hpp"

#include "xtk/cl_XTK_Background_Mesh.hpp"
#include "cl_XTK_Model.hpp"
#include "cl_XTK_Enums.hpp"
#include "cl_Cell.hpp"

#include "geometry/cl_Discrete_Level_Set.hpp"
#include "cl_MGE_Geometry_Engine.hpp"

#include "cl_MTK_Mesh.hpp"
#include "cl_Mesh_Enums.hpp"
#include "cl_MTK_Mesh_Data_Input.hpp"
#include "cl_MTK_Scalar_Field_Info.hpp"


#include "../projects/GEN/src/geometry/cl_GEN_Discrete_Level_Set.hpp"

using namespace moris;


namespace xtk
{
  TEST_CASE("Finite differencing check on a single tet mesh","[FD_TRI]")
  {
      if(par_size() == 1)
      {
          // This is the interface node with dxdp information
          moris_index tInterfaceNodeInd = 5;

          // background node level set value to perturb
          moris_index tNodeToPerturbInd = 0;

          // Amount to perturb (for finite difference check)
          real   tPerturbVal = 1e-6;

          // Tolerance in the 2 norm
          real tTol = 1e-8;

          // Level Set Values
          moris::real tPhi1 =  0.4;
          moris::real tPhi2 =  0.3;
          moris::real tPhi3 =  0.5;
          moris::real tPhi4 = -0.0000000001;

          Matrix<DDRMat> tBaseLSF({{ tPhi1, tPhi2, tPhi3, tPhi4}});
          Matrix<DDRMat> tPertDownLSF({{ tPhi1, tPhi2-tPerturbVal, tPhi3, tPhi4}});
          Matrix<DDRMat> tPertUpLSF({{ tPhi1, tPhi2+tPerturbVal, tPhi3, tPhi4}});

          Cell<Matrix<DDRMat>> tLSF({tBaseLSF, tPertDownLSF,tPertUpLSF});

          // interface node location for each decomposition
          Cell<Matrix< DDRMat >> tInterfaceNodeLocation(2);

          // computed sensitivity
          Matrix< DDRMat > tDxDp(1,1);
          Matrix< IndexMat > tDxDpInds(1,1);

          Matrix< DDRMat > tDxDpComp(3,1,10.0);
          for(size_t i = 0; i<3; i++)
          {
              std::string tPrefix;
              tPrefix = std::getenv("MORISROOT");
              std::string tMeshFileName = tPrefix + "/TestExoFiles/single_tet_mesh.e";
              Cell<std::string> tScalarFieldNames = {"lsf"};

              // Declare scalar node field
              mtk::Scalar_Field_Info<DDRMat> tNodeField1;
              tNodeField1.set_field_name(tScalarFieldNames(0));
              tNodeField1.set_field_entity_rank(EntityRank::NODE);

              // Initialize field information container
              mtk::MtkFieldsInfo tFieldsInfo;

              // Place the node field into the field info container
              mtk::add_field_for_mesh_input(&tNodeField1,tFieldsInfo);

              // Declare some supplementary fields
              mtk::MtkMeshData tMeshDataInput;
              tMeshDataInput.FieldsInfo = &tFieldsInfo;

              moris::mtk::Interpolation_Mesh* tMeshData = moris::mtk::create_interpolation_mesh( MeshType::STK, tMeshFileName, &tMeshDataInput );

              tMeshData->add_mesh_field_real_scalar_data_loc_inds(tScalarFieldNames(0),EntityRank::NODE,tLSF(i));


              // set  up the geometry/geometry engine
              moris::ge::Discrete_Level_Set tLevelSetMesh(tMeshData,tScalarFieldNames);
              moris::ge::GEN_Phase_Table        tPhaseTable (1,  Phase_Table_Structure::EXP_BASE_2);
              moris::ge::GEN_Geometry_Engine    tGEIn(tLevelSetMesh,tPhaseTable);
              tGEIn.mComputeDxDp = true;
              tGEIn.mThresholdValue = 0.0;
              // Setup XTK Model -----------------------------
              size_t tModelDimension = 3;
              Model tXTKModel(tModelDimension,tLevelSetMesh.get_level_set_mesh(),tGEIn);
              tXTKModel.mSameMesh = true;

              //Specify your decomposition methods and start cutting
              Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::C_HIERARCHY_TET4};
              tXTKModel.decompose(tDecompositionMethods);

              // compute sensitivities
              tXTKModel.compute_sensitivity();

              moris::ge::GEN_Geometry_Engine const & tGEOut          = tXTKModel.get_geom_engine();
              Background_Mesh & tBackgroundMesh = tXTKModel.get_background_mesh();

              // store the xtk computed derivative
              if( i == 0)
              {
                  tDxDp     = tGEOut.get_node_dx_dp(tInterfaceNodeInd);
                  tDxDpInds = tGEOut.get_node_adv_indices(tInterfaceNodeInd);
              }
              else
              {
                  tInterfaceNodeLocation(i-1) = tBackgroundMesh.get_selected_node_coordinates_loc_inds({{tInterfaceNodeInd}});
              }

              delete tMeshData;
          }

          moris::real tChangeInPhi           = 2*tPerturbVal;
          Matrix< DDRMat > tChangeInLocation = tInterfaceNodeLocation(1)-tInterfaceNodeLocation(0);
          Matrix< DDRMat > tDxDpFD           = tChangeInLocation/tChangeInPhi;
          real t2Norm                        = moris::norm((tDxDpFD-tDxDp.get_row(0)));

          // check the design index is the one we expect
          CHECK(tDxDpInds(0) == 1);
          CHECK(t2Norm < tTol);

      }
  }

//   TEST_CASE("Finite differencing check on a single hex mesh","[FD_HEX]")
//   {
//       if(par_size() == 1)
//       {
//       // This is the interface node with dxdp information
//       moris_index tInterfaceNodeInd = 15;
//
//       // background node level set value to perturb
//       moris_index tNodeToPerturbInd = 0;
//
//       // Amount to perturb (for finite difference check)
//       real   tPerturbVal = 1e-6;
//
//       // Tolerance in the 2 norm
//       real tTol = 1e-4;
//
//       // Set up perturbations
//       Matrix<DDRMat> tBaseLSF(     {{0.4, 0.3, 0.5, -0.5,               0.2, 0.1, 0.3, 0.4}});
//       Matrix<DDRMat> tPertUpLSF(   {{0.4, 0.3, 0.5, -0.5 + tPerturbVal, 0.2, 0.1, 0.3, 0.4}});
//       Matrix<DDRMat> tPertDownLSF( {{0.4, 0.3, 0.5, -0.5 - tPerturbVal, 0.2, 0.1, 0.3, 0.4}});
//       Cell<Matrix<DDRMat>> tLSF({tBaseLSF, tPertUpLSF,tPertDownLSF});
//
//       // interface node location for each decomposition
//       Cell<Matrix< DDRMat >> tInterfaceNodeLocation(2);
//
//       // computed sensitivity
//       Matrix< DDRMat > tDxDp(1,1);
//       Matrix< IndexMat > tDxDpInds(1,1);
//
//       // Random threshold value between 0.3 and -0.5
//       real tMax =  0.1;
//       real tMin = -0.5;
//       real tF = (real)rand() / RAND_MAX;
//       real tThreshold = (real) (tMin + tF * (tMax - tMin));
//
//       Matrix< DDRMat > tDxDpComp(3,1,10.0);
//       for(size_t i = 0; i<3; i++)
//       {
//           std::string tMeshFileName = "generated:1x1x1";
//           Cell<std::string> tScalarFieldNames = {"lsf"};
//
//           // Declare scalar node field
//           mtk::Scalar_Field_Info<DDRMat> tNodeField1;
//           tNodeField1.set_field_name(tScalarFieldNames(0));
//           tNodeField1.set_field_entity_rank(EntityRank::NODE);
//
//           // Initialize field information container
//           mtk::MtkFieldsInfo tFieldsInfo;
//
//           // Place the node field into the field info container
//           mtk::add_field_for_mesh_input(&tNodeField1,tFieldsInfo);
//
//           // Declare some supplementary fields
//           mtk::MtkMeshData tMeshDataInput;
//           tMeshDataInput.FieldsInfo = &tFieldsInfo;
//
//           moris::mtk::Mesh* tMeshData = moris::mtk::create_mesh( MeshType::STK, tMeshFileName, &tMeshDataInput );
//
//           tMeshData->add_mesh_field_real_scalar_data_loc_inds(tScalarFieldNames(0),EntityRank::NODE,tLSF(i));
//
//           Discrete_Level_Set tLevelSetMesh(tMeshData,tScalarFieldNames);
//           Phase_Table        tPhaseTable (1, Phase_Table_Structure::EXP_BASE_2);
//           Geometry_Engine    tGEIn(tLevelSetMesh,tPhaseTable);
//           tGEIn.mComputeDxDp    = false;
//           tGEIn.mThresholdValue = tThreshold;
//
//           // Setup XTK Model -----------------------------
//           size_t tModelDimension = 3;
//           Model tXTKModel(tModelDimension,tLevelSetMesh.get_level_set_mesh(),tGEIn);
//           tXTKModel.mSameMesh = true;
//
//           //Specify your decomposition methods and start cutting
//           Cell<enum Subdivision_Method> tDecompositionMethods =
//           {Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8,
//            Subdivision_Method::C_HIERARCHY_TET4};
//
//
//           // Verify we get a throw if the interface sensitivity is computed prior to decomposition
//           CHECK_THROWS(tXTKModel.compute_sensitivity());
//
//           // decompose
//           tXTKModel.decompose(tDecompositionMethods);
//
//           // compute sensitivities
//           tXTKModel.compute_sensitivity();
//
//           Geometry_Engine const & tGEOut    = tXTKModel.get_geom_engine();
//           Background_Mesh & tBackgroundMesh = tXTKModel.get_background_mesh();
//
//           // store the xtk computed derivative
//           if( i == 0)
//           {
//               tDxDp     = tGEOut.get_node_dx_dp(tInterfaceNodeInd);
//               tDxDpInds = tGEOut.get_node_adv_indices(tInterfaceNodeInd);
//
//
//               moris::mtk::Mesh* tCutMeshData = tXTKModel.get_output_mesh();
//
//               std::string tPrefix = std::getenv("MORISOUTPUT");
//               std::string tMeshOutputFile = tPrefix + "/xtk_sensitivity_check_hex8.e";
//               tCutMeshData->create_output_mesh(tMeshOutputFile);
//               delete tCutMeshData;
//           }
//           else
//           {
//               tInterfaceNodeLocation(i-1) = tBackgroundMesh.get_selected_node_coordinates_loc_inds({{tInterfaceNodeInd}});
//           }
//
//           delete tMeshData;
//       }
//
//       DDRMat tDxDpFdMat = (tInterfaceNodeLocation(1).matrix_data()-tInterfaceNodeLocation(0).matrix_data())/(2*tPerturbVal);
//       Matrix< DDRMat > tDxDpFD(tDxDpFdMat);
//       Matrix< DDRMat > tDxDpRow = tDxDp.get_row(1);
//       real tL2Norm = moris::norm((tDxDpFD-tDxDpRow));
//
//       CHECK(tL2Norm < tTol);
//   }
//   }


}
