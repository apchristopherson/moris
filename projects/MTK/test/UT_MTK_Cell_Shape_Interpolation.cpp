/*
 * UT_MTK_Cell_Shape_Interpolation.cpp
 *
 *  Created on: Apr. 8, 2021
 *      Author: gates
 */

#include "catch.hpp"
#include "cl_Communication_Tools.hpp"

// base class
#include "cl_MTK_Mesh_Factory.hpp"
#include "cl_MTK_Cell.hpp"
#include "cl_MTK_Mesh_Core_STK.hpp"
#include "cl_MTK_Cell_STK.hpp"
#include "cl_Mesh_Enums.hpp"
#include "cl_MTK_Vertex_STK.hpp"
#include "cl_MTK_Cell_Info_Quad4.hpp"
#include "cl_MTK_Cell_Info_Tri3.hpp"
#include "cl_MTK_Cell_Info_Hex8.hpp"
#include "cl_MTK_Cell_Info_Tet4.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Cell_Info.hpp"
#include "IP/cl_MTK_Space_Interpolator.hpp"

// linalg includes
#include "cl_Matrix.hpp"
#include "op_equal_equal.hpp"
#include "fn_all_true.hpp"
#include "fn_reindex_mat.hpp"

namespace moris
{
    namespace mtk
    {

        TEST_CASE("MTK Hex8 Rectangular", "[MTK],[Rect_Hex8]")
        {
            if (par_size() <= 1)
            {
                // define a HEX8 space element, i.e. space coordinates xHat
                Matrix<DDRMat> tXHat = {{0.0, 0.0, 0.0},
                                        {1.2, 0.0, 0.0},
                                        {1.2, 1.0, 0.0},
                                        {0.0, 1.0, 0.0},
                                        {0.0, 0.0, 1.5},
                                        {1.2, 0.0, 1.5},
                                        {1.2, 1.0, 1.5},
                                        {0.0, 1.0, 1.5}};

                // the HEX8 interpolation element in space and time param coordinates xiHat
                Matrix<DDRMat> tXiHat = {{-1.0, -1.0, -1.0},
                                         {1.0, -1.0, -1.0},
                                         {1.0, 1.0, -1.0},
                                         {-1.0, 1.0, -1.0},
                                         {-1.0, -1.0, 1.0},
                                         {1.0, -1.0, 1.0},
                                         {1.0, 1.0, 1.0},
                                         {-1.0, 1.0, 1.0}};

                // integration mesh interpolation rule
                Interpolation_Rule tInterpRule(Geometry_Type::HEX,
                                               Interpolation_Type::LAGRANGE,
                                               Interpolation_Order::LINEAR,
                                               Interpolation_Type::LAGRANGE,
                                               Interpolation_Order::QUADRATIC);

                // create a space interpolator
                Space_Interpolator tSpaceInterpolator(tInterpRule);

                // set the coefficients xHat, tHat
                tSpaceInterpolator.set_space_coeff(tXHat);

                // set the coefficients xiHat, tauHat
                tSpaceInterpolator.set_space_param_coeff(tXiHat);

                // Interpolation location
                Matrix<DDRMat> tX = {{0.4}, {0.4}, {0.4}};

                // Set the space
                tSpaceInterpolator.set_space(tX);

                // calculating det J and inv J
                real tDetJ = tSpaceInterpolator.space_det_J();
                Matrix<DDRMat> tInvJac = tSpaceInterpolator.inverse_space_jacobian();

                // setting rectangular flag
                tSpaceInterpolator.set_cell_shape( CellShape::RECTANGULAR );
                tSpaceInterpolator.reset_eval_flags();

                // calculating det J and inv J using rectangular eval
                real tDetJ_Rect = tSpaceInterpolator.space_det_J();
                Matrix<DDRMat> tInvJac_Rect = tSpaceInterpolator.inverse_space_jacobian();

                // checking det J calc
                CHECK(tDetJ == Approx(tDetJ_Rect));

                // checking inverse jacobian
                for (uint i = 0; i <= 2; i++)
                {
                    for (uint j = 0; j <= 2; j++)
                    {
                        CHECK(tInvJac(i, j) == Approx(tInvJac_Rect(i, j)));
                    }
                }
            }
        }

        TEST_CASE("MTK Quad4 Rectangular", "[MTK],[Rect_Quad4]")
        {
            if (par_size() <= 1)
            {
                // define a QUAD4 space element, i.e. space coordinates xHat
                Matrix<DDRMat> tXHat = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.5}, {0.0, 1.5}};

                // the QUAD4 interpolation element in space and time param coordinates xiHat
                Matrix<DDRMat> tXiHat = {{-1.0, -1.0}, {1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0}};

                // integration mesh interpolation rule
                Interpolation_Rule tInterpRule(Geometry_Type::QUAD,
                                               Interpolation_Type::LAGRANGE,
                                               Interpolation_Order::LINEAR,
                                               Interpolation_Type::LAGRANGE,
                                               Interpolation_Order::QUADRATIC);

                // create a space interpolator
                Space_Interpolator tSpaceInterpolator(tInterpRule);

                // set the coefficients xHat, tHat
                tSpaceInterpolator.set_space_coeff(tXHat);

                // set the coefficients xiHat, tauHat
                tSpaceInterpolator.set_space_param_coeff(tXiHat);

                // Interpolation location
                Matrix<DDRMat> tX = {{0.4}, {0.4}};

                // Set the space
                tSpaceInterpolator.set_space(tX);

                // calculating detJ and the inverse jacobian
                real tDetJ = tSpaceInterpolator.space_det_J();
                Matrix<DDRMat> tInvJac = tSpaceInterpolator.inverse_space_jacobian();

                // setting rectangular flag
                tSpaceInterpolator.set_cell_shape( CellShape::RECTANGULAR );
                tSpaceInterpolator.reset_eval_flags();

                // calculating detJ and inv J using rectangular calc
                real tDetJ_Rect = tSpaceInterpolator.space_det_J();
                Matrix<DDRMat> tInvJac_Rect = tSpaceInterpolator.inverse_space_jacobian();

                // checking det j
                CHECK(tDetJ == Approx(tDetJ_Rect));

                // checking inverse jacobian
                for (uint i = 0; i <= 1; i++)
                {
                    for (uint j = 0; j <= 1; j++)
                    {
                        CHECK(tInvJac(i, j) == Approx(tInvJac_Rect(i, j)));
                    }
                }
            }
        }

    } // namespace mtk
} // namespace moris
