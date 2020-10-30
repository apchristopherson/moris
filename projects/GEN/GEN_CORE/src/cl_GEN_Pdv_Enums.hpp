#ifndef MORIS_CL_PDV_Type_ENUMS_HPP_
#define MORIS_CL_PDV_Type_ENUMS_HPP_

#include "cl_Map.hpp"

namespace moris
{
    enum class PDV_Type
    {
        X_COORDINATE,
        Y_COORDINATE,
        Z_COORDINATE,
        DENSITY,
        TEMPERATURE,
        ELASTIC_MODULUS,
        LS1,
        LS2,
        UNDEFINED
    };

    moris::map< std::string, PDV_Type > get_pdv_type_map();

    enum class Intersection_Mode
    {
        LEVEL_SET,
        COLORING,
        END_ENUM
    };

    moris::map< std::string, Intersection_Mode > get_intersection_mode_map();
}

#endif /* MORIS_CL_PDV_ENUMS_HPP_ */
