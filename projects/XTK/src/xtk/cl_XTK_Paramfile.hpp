/*
 * cl_XTK_Paramfile.hpp
 *
 *  Created on: Mar 22, 2019
 *      Author: doble
 */

#ifndef PROJECTS_XTK_SRC_XTK_CL_XTK_PARAMFILE_HPP_
#define PROJECTS_XTK_SRC_XTK_CL_XTK_PARAMFILE_HPP_

#include "cl_XML_Parser.hpp"
#include "cl_Mesh_Enums.hpp"
#include "cl_Geometry_Enums.hpp"
#include "cl_XTK_Enums.hpp"

#include <string>
#include <sstream>
#include <iostream>

using namespace moris;

namespace xtk
{

class XTK_Problem_Params
{
public:
    XTK_Problem_Params():
    mInputMeshFile(""),
    mMeshType(MeshType::END_ENUM),
    mGeometryType(Geometry_Type::INVALID),
    mRealGeomParams(0),
    mIntGeomParams(0),
    mSubdivisionMethods(0),
    mComputeSens(false),
    mUnzip(false),
    mEnrich(false),
    mGhost(false),
    mExport(false),
    mOutputMeshFile("")
    {};

    // input mesh
    std::string       mInputMeshFile;
    enum MeshType     mMeshType;

    // geometry
    enum Geometry_Type mGeometryType;
    std::string        mGeometryName;
    Cell<real>         mRealGeomParams;
    Cell<std::string>  mRealGeomLabels;
    Cell<moris_index>  mIntGeomParams;
    Cell<std::string>  mIntGeomLabels;

    // decomposition
    Cell<enum Subdivision_Method> mSubdivisionMethods;
    Cell<std::string> mSubdivisionStrings;

    // compute sens
    bool mComputeSens;

    // unzip
    bool mUnzip;

    //enrich
    bool mEnrich;

    // ghost
    bool mGhost;

    // export mesh file
    bool mExport;
    std::string mOutputMeshFile;

    // dump data to hdf5
    bool mOutputData;
    std::string mDataFile; /*HDF5*/

    // print geometry parameters
    void print_geom()
    {
        //TODO: support discrete with a field name
        std::cout<<"Geometry:  "<<mGeometryName<< std::endl;
        for(moris::uint  i = 0; i < mRealGeomParams.size(); i++)
        {
          std::cout<<"    "<<mRealGeomLabels(i)<<" = "<<mRealGeomParams(i)<<std::endl;
        }
    }

    void print_operations()
    {
      std::cout<<"Operations to Perform:"<<std::endl;

      for(moris::uint i = 0; i <mSubdivisionStrings.size(); i++)
      {
        std::cout<<"    "<<mSubdivisionStrings(i)<<std::endl;
      }

      if(mComputeSens)
      {
        std::cout<<"    Sensitivity Computation"<<std::endl;
      }

      if(mUnzip)
      {
        std::cout<<"    Unzip Interface"<<std::endl;
      }

      if(mEnrich)
      {
        std::cout<<"    Basis Enrichment"<<std::endl;
      }

      if(mGhost)
      {
        std::cout<<"    Ghost Penalization"<<std::endl;
      }
    }
};


class Paramfile
{
public:
    Paramfile( const std::string & aPath );

    ~Paramfile();

    Cell<XTK_Problem_Params> &
    get_xtk_problem_params()
    {
        return mXTKProblems;
    }

private:
    // the xml parser
    XML_Parser * mParser = nullptr;

    // XTK problems to run
    Cell<XTK_Problem_Params> mXTKProblems;

    /*!
     * Load mesh parameters from XML file
     */
    void
    load_xtk_problems();

    /*!
    * Parse the problems mesh
    */
    void
    parse_xtk_problem_input_mesh(moris::uint aProblemIndex);

    /*!
    * Parse the problems geometry
    */
    void
    parse_xtk_problem_geometry(moris::uint aProblemIndex);

    /*!
    * Parse the problems geometry
    */
    void
    parse_xtk_problem_decomp(moris::uint aProblemIndex);

    /*!
    * Parse the problems geometry
    */
    void
    parse_xtk_problem_operators(moris::uint aProblemIndex);

    /*!
    * Parse the problems geometry
    */
    void
    parse_xtk_problem_output(moris::uint aProblemIndex);




    enum MeshType
    get_mesh_type_enum(std::string const & aMeshStr)
    {
      if(aMeshStr == "STK")
      {
        return MeshType::STK;
      }

      else
      {
        MORIS_ERROR(0,"Mesh str not recognized.");
        return MeshType::STK;
      }
    }

    enum Geometry_Type
    get_geometry_enum(std::string const & aGeometryStr)
    {
      if(aGeometryStr == "Sphere")
      {
        return Geometry_Type::SPHERE;
      }

      else if(aGeometryStr == "Plane")
      {
        return Geometry_Type::PLANE;
      }

      else
      {
        MORIS_ERROR(0,"Geometry str not recognized: %s.", aGeometryStr);
        return Geometry_Type::INVALID;
      }
    }

    enum Subdivision_Method
    get_decomp_enum(std::string const & aDecompStr)
    {
      if(aDecompStr == "Hex8 Regular Subdivision")
      {
        return Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8;
      }
      else if(aDecompStr == "Tet4 Node Hierarchy")
      {
        return Subdivision_Method::C_HIERARCHY_TET4;
      }
      else
      {
        MORIS_ERROR(0,"Decomposition str not recognized: %s. Please use Hex8 Regular Subdivision or Tet4 Node Hierarchy.", aDecompStr);
        return Subdivision_Method::NC_REGULAR_SUBDIVISION_HEX8;
      }
    }

    Cell<real>
    convert_str_to_cell_real(std::string const & aStr)
    {
      std::stringstream ss( aStr );
      Cell<real> result;

      while( ss.good() )
      {
          std::string substr;
          std::getline( ss, substr, ',' );
          result.push_back( std::stod(substr) );
      }

      return result;
    }

    Cell<std::string>
    convert_str_to_cell_str(std::string const & aStr)
    {
      std::stringstream ss( aStr );
      Cell<std::string> result;

      while( ss.good() )
      {
          std::string substr;
          std::getline( ss, substr, ',' );
          result.push_back( substr );
      }

      return result;
    }


};
}
#endif /* PROJECTS_XTK_SRC_XTK_CL_XTK_PARAMFILE_HPP_ */
