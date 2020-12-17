/*
 * cl_XTK_Model_Export_UT.cpp
 *
 *  Created on: Mar 4, 2019
 *      Author: doble
 */

#ifndef PROJECTS_XTK_TEST_XTK_CL_XTK_MODEL_EXPORT_UT_CPP_
#define PROJECTS_XTK_TEST_XTK_CL_XTK_MODEL_EXPORT_UT_CPP_

#include "catch.hpp"

#include "paths.hpp"
//#include "cl_Sphere.hpp"
#include "cl_XTK_Model.hpp"
//#include "cl_MGE_Geometry_Engine.hpp"
#include "cl_GEN_Geometry.hpp"
#include "cl_GEN_Sphere.hpp"

namespace xtk
{

TEST_CASE("Outputting XTK Model","[EXPORT]")
{
    if(par_size() == 1)
    {
    /*
     * Loads an exodus file with a Block Set and Side Set already populated
     * Performs regular subdivison method and then checks to see if the
     * children XTK mesh entities have the same as their parents
     */

    /*
     * Set up:
     * Geometry,
     * Geometry Engine,
     * Mesh
     */

    real tRadius = 5.1;
    real tXCenter = 0.0;
    real tYCenter = 0.0;
    real tZCenter = 0.0;
    Cell<std::shared_ptr<moris::ge::Geometry>> tGeometry(1);
    tGeometry(0) = std::make_shared<moris::ge::Sphere>(tXCenter, tYCenter, tZCenter, tRadius);

    /*
     * Load Mesh which has 3 block sets. These blocks are named:
     *  - top_bread
     *  - meat
     *  - bottom_bread
     *
     * Side Sets will eventually be named
     *  - top_crust
     *  - bottom_crust
     */
    std::string tPrefix = moris::get_base_moris_dir();
    std::string tMeshFileName = tPrefix + "/projects/XTK/test/test_exodus_files/sandwich.e";
    moris::Cell<std::string> tFieldNames;

    moris::mtk::Interpolation_Mesh* tMeshData = moris::mtk::create_interpolation_mesh( MeshType::STK, tMeshFileName, NULL );

    moris::ge::Geometry_Engine tGeometryEngine(tGeometry, tMeshData);

    /*
     * Setup XTK Model and tell it how to cut
     */
    size_t tModelDimension = 3;
    Cell<enum Subdivision_Method> tDecompositionMethods = {Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8,Subdivision_Method::C_HIERARCHY_TET4};

    Model tXTKModel(tModelDimension,tMeshData,&tGeometryEngine);
    tXTKModel.mVerbose  =  false;
    /*
     * Decompose
     */
    tXTKModel.decompose(tDecompositionMethods);


    // Output with standard options
    moris::mtk::Mesh* tOutputMeshData = tXTKModel.get_output_mesh();
    tPrefix = std::getenv("MORISOUTPUT");
    std::string tMeshOutputFile = tPrefix + "/xtk_export/xtk_export_base.e";
    tOutputMeshData->create_output_mesh(tMeshOutputFile);
    delete tOutputMeshData;

    // with background mesh sets propogated
    Output_Options tOutputOptions;
    tOutputOptions.mAddNodeSets = true;
    tOutputOptions.mAddSideSets = true;
    tOutputMeshData = tXTKModel.get_output_mesh(tOutputOptions);
    tPrefix = std::getenv("MORISOUTPUT");
    tMeshOutputFile = tPrefix + "/xtk_export/xtk_export_background_sets.e";
    tOutputMeshData->create_output_mesh(tMeshOutputFile);
    delete tOutputMeshData;

    // with only phase 0 output
    tOutputOptions = Output_Options();
    size_t tNumPhases = 2;
    Cell<size_t> tPhasesToOutput = {0};
    tOutputOptions.change_phases_to_output(tNumPhases,tPhasesToOutput);
    tOutputMeshData = tXTKModel.get_output_mesh(tOutputOptions);
    tMeshOutputFile ="./xtk_exo/xtk_export/xtk_export_phase_0.e";
    tOutputMeshData->create_output_mesh(tMeshOutputFile);
    delete tOutputMeshData;



   delete tMeshData;
}
}
}


#endif /* PROJECTS_XTK_TEST_XTK_CL_XTK_MODEL_EXPORT_UT_CPP_ */
