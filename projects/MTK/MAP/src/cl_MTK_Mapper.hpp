/*
 * cl_MTK_Mapper.hpp
 *
 *  Created on: Oct 30, 2018
 *      Author: messe
 */

#ifndef PROJECTS_MTK_MAP_SRC_CL_MTK_MAPPER_HPP_
#define PROJECTS_MTK_MAP_SRC_CL_MTK_MAPPER_HPP_

#include "typedefs.hpp"
#include "cl_Cell.hpp"

#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
// #include "cl_MTK_Field.hpp"
#include "cl_MTK_Mesh.hpp"

namespace moris
{
//------------------------------------------------------------------------------

    namespace MSI
    {
        class Equation_Object;
    }

//------------------------------------------------------------------------------

    namespace fem
    {
        class IWG_L2;
        class Node_Base;
    }

//------------------------------------------------------------------------------

    namespace mdl
    {
        class Model;
    }

//------------------------------------------------------------------------------
    namespace mapper
    {
//------------------------------------------------------------------------------

     class Mapper
     {
         mtk::Mesh                          * mSourceMesh;
         mtk::Mesh                          * mTargetMesh;
         fem::IWG_L2                        * mIWG;
         mdl::Model                         * mModel;

         bool mHaveIwgAndModel = false;

//------------------------------------------------------------------------------
     public:
//------------------------------------------------------------------------------

         /**
          * constructor with only one mesh
          */
         Mapper( mtk::Mesh * aMesh );

//------------------------------------------------------------------------------

         /**
          * destructor
          */
         ~Mapper();

//------------------------------------------------------------------------------

         void
         perform_mapping(
                 const std::string & aSourceLabel,
                 const EntityRank    aSourceEntityRank,
                 const std::string & aTargetLabel,
                 const EntityRank    aTargetEntityRank );

//------------------------------------------------------------------------------

         /*void
         project_coeffs_from_node_data(
                 const Matrix< DDRMat > & aNodeValues,
                 const uint             & aBSplineOrder,
                 std::shared_ptr< Mesh >  aMesh,
                 Matrix< DDRMat >       & aCoeffs ); */

//------------------------------------------------------------------------------
     private:
//------------------------------------------------------------------------------

         void
         map_node_to_bspline_same_mesh(
                 const moris_index   aSourceIndex,
                 const moris_index   aTargetIndex,
                 const EntityRank    aBSplineRank );

//------------------------------------------------------------------------------

         void
         map_node_to_element_same_mesh(
                          const moris_index   aSourceIndex,
                          const moris_index   aTargetIndex );

//------------------------------------------------------------------------------

         void
         map_bspline_to_node_same_mesh(
                 const moris_index   aSourceIndex,
                 const EntityRank     aBSplineRank,
                 const moris_index   aTargetIndex );

//------------------------------------------------------------------------------

         void
         create_iwg_and_model();

     };

//------------------------------------------------------------------------------
    } /* namespace mtk */
} /* namespace moris */


#endif /* PROJECTS_MTK_MAP_SRC_CL_MTK_MAPPER_HPP_ */