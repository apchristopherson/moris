/*
 * cl_MTK_Enums.hpp
 *
 *  Created on: Jul 25, 2018
 *      Author: messe
 */

#ifndef SRC_MESH_CL_MTK_ENUMS_HPP_
#define SRC_MESH_CL_MTK_ENUMS_HPP_

#include "assert.hpp"
#include "typedefs.hpp"
#include "cl_Map.hpp"

//------------------------------------------------------------------------------
namespace moris
{
    namespace mtk
    {
        //------------------------------------------------------------------------------

        enum class Interpolation_Type
        {
                UNDEFINED,
                CONSTANT, // constant interpolation
                LAGRANGE, // the most common finite element types
                BEZIER,   // Bezier type elements
                END_INTERPOLATION_TYPE
        };

        //------------------------------------------------------------------------------

        enum class Integration_Type
        {
                UNDEFINED,
                CONSTANT,
                GAUSS, // Gauss ( Quad and Hex ), Dunavant ( Tri ), Hammer ( Tet )
                END_INTEGRATION_TYPE
        };

        //------------------------------------------------------------------------------

        enum class Integration_Order
        {
                UNDEFINED,
                POINT,
                BAR_1,
                BAR_2,
                BAR_3,
                BAR_4,
                BAR_5,
                BAR_6,
                QUAD_1x1,
                QUAD_2x2,
                QUAD_3x3,
                QUAD_4x4,
                QUAD_5x5,
                TRI_1,
                TRI_3,
                TRI_4,
                TRI_6,
                TRI_7,
                TRI_12,
                TRI_13,
                TRI_16,
                TRI_19,
                TRI_25,
                HEX_1x1x1,
                HEX_2x2x2,
                HEX_3x3x3,
                HEX_4x4x4,
                HEX_5x5x5,
                TET_1,
                TET_4,
                TET_5,
                TET_10,
                TET_11,
                TET_15,
                TET_20,
                TET_35,
                TET_56,
                END_INTEGRATION_ORDER
        };

        //------------------------------------------------------------------------------

        enum class Geometry_Type
        {
                POINT, // point
                LINE,  // 1D line or curve
                QUAD,  // rectangle
                TRI,   // triangle
                HEX,   // quadrangle
                TET,   // tetrahedron
                PENTA, // pentahedron
                UNDEFINED
        };

        //------------------------------------------------------------------------------

        enum class Interpolation_Order
        {
                CONSTANT,
                LINEAR,
                QUADRATIC,
                SERENDIPITY,
                CUBIC,
                UNDEFINED
        };

        //------------------------------------------------------------------------------

        enum class Master_Slave
        {
                MASTER,
                SLAVE,
                UNDEFINED
        };

        moris::map< std::string, Master_Slave > get_master_type_map()
        {
            moris::map< std::string, Master_Slave > tMasterTypeMap;

            tMasterTypeMap["MASTER"]    = Master_Slave::MASTER;
            tMasterTypeMap["SLAVE"]     = Master_Slave::SLAVE;
            tMasterTypeMap["UNDEFINED"] = Master_Slave::UNDEFINED;
            return tMasterTypeMap;
        }

        //------------------------------------------------------------------------------

        enum class Primary_Void
        {
                PRIMARY,
                VOID,
                INTERP,
                UNDEFINED
        };

        moris::map< std::string, Primary_Void > get_primary_type_map()
        {
            moris::map< std::string, Primary_Void > tPrimaryTypeMap;

            tPrimaryTypeMap["PRIMARY"]   = Primary_Void::PRIMARY;
            tPrimaryTypeMap["VOID"]      = Primary_Void::VOID;
            tPrimaryTypeMap["INTERP"]    = Primary_Void::INTERP;
            tPrimaryTypeMap["UNDEFINED"] = Primary_Void::UNDEFINED;
            return tPrimaryTypeMap;
        }

        //------------------------------------------------------------------------------

        enum class Field_Type
        {
                FIELD_1,
                FIELD_2,
                FIELD_3,
                FIELD_4,
                FIELD_5,
                UNDEFINED
        };

        moris::map< std::string, Field_Type > get_field_type_map()
        {
            moris::map< std::string, Field_Type > tFieldTypeMap;

            tFieldTypeMap["FIELD_1"]   = Field_Type::FIELD_1;
            tFieldTypeMap["FIELD_2"]   = Field_Type::FIELD_2;
            tFieldTypeMap["FIELD_3"]   = Field_Type::FIELD_3;
            tFieldTypeMap["FIELD_4"]   = Field_Type::FIELD_4;
            tFieldTypeMap["FIELD_5"]   = Field_Type::FIELD_5;
            tFieldTypeMap[""]          = Field_Type::UNDEFINED;
            return tFieldTypeMap;
        }

        //------------------------------------------------------------------------------
    } /* namespace mtk */
} /* namespace moris */

#endif /* SRC_MESH_CL_MTK_ENUMS_HPP_ */
