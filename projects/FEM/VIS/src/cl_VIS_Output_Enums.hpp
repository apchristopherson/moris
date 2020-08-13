/*
 * cl_VIS_Enums.hpp
 *
 *  Created on: Dec 10, 2019
 *      Author: schmidt
 */
#ifndef SRC_CL_VIS_ENUMS_HPP_
#define SRC_CL_VIS_ENUMS_HPP_

#include "cl_Map.hpp"

namespace moris
{
namespace vis
{
    enum class Output_Type
    {
        UNDEFINED, //< Undefined
        STRAIN_ENERGY,
	STRESS,
        VOLUME,
        SIGY,   //< Y-stress
        VOLUME_FRACTION,
        UX,     //< X-Displacement
        UY,     //< Y-Displacement
        UZ,     //< Z-Displacement
        TEMP,   //< Temperature degree of freedom
        L2,     //< Least Squares type
        MAPPING_DOF,
        LS1,    //< Level set
        LS2,    //< Level set
        NLSX,   //< X-Level set normal
        NLSY,   //< Y-Level set normal
        NLSZ,   //< Z-Level set normal
        VX,     //< X-Velocity
        VY,     //< Y-Velocity
        VZ,     //< Z-Velocity
        P,
        DOF, // Dof
        MAX_DOF,
        MAX_STRESS,
        Analytic, // Analytic
        L2_ERROR_ANALYTIC, // L2 error for dof
        H1_ERROR_ANALYTIC, // H1 error for dof
        H1_SEMI_ERROR, // H1-semi error for dof
        PROPERTY, //Property
        J_INTEGRAL,  // J-Integral
        K1_SENT,     // 1st stress intensity factor for the case of a single edge notch tension analysis
        DRAG_COEFF,
        LIFT_COEFF,
        LATENT_HEAT_ABSORPTION,
        VISCOSITY,
        TURBULENT_KINEMATIC_VISCOSITY,
        END_ENUM//
    };

    moris::map< std::string, enum vis::Output_Type > get_vis_output_type_map()
    {
        moris::map< std::string, enum vis::Output_Type > tVisOutputTypeMap;

        tVisOutputTypeMap["UNDEFINED"]              = vis::Output_Type::UNDEFINED;
        tVisOutputTypeMap["STRAIN_ENERGY"]          = vis::Output_Type::STRAIN_ENERGY;
        tVisOutputTypeMap["STRESS"]                 = vis::Output_Type::STRESS;
        tVisOutputTypeMap["VOLUME"]                 = vis::Output_Type::VOLUME;
        tVisOutputTypeMap["VOLUME_FRACTION"]        = vis::Output_Type::VOLUME_FRACTION;
        tVisOutputTypeMap["UX"]                     = vis::Output_Type::UX;
        tVisOutputTypeMap["UY"]                     = vis::Output_Type::UY;
        tVisOutputTypeMap["UZ"]                     = vis::Output_Type::UZ;
        tVisOutputTypeMap["TEMP"]                   = vis::Output_Type::TEMP;
        tVisOutputTypeMap["L2"]                     = vis::Output_Type::L2;
        tVisOutputTypeMap["MAPPING_DOF"]            = vis::Output_Type::MAPPING_DOF;
        tVisOutputTypeMap["LS1"]                    = vis::Output_Type::LS1;
        tVisOutputTypeMap["LS2"]                    = vis::Output_Type::LS2;
        tVisOutputTypeMap["NLSX"]                   = vis::Output_Type::NLSX;
        tVisOutputTypeMap["NLSY"]                   = vis::Output_Type::NLSY;
        tVisOutputTypeMap["NLSZ"]                   = vis::Output_Type::NLSZ;
        tVisOutputTypeMap["VX"]                     = vis::Output_Type::VX;
        tVisOutputTypeMap["VY"]                     = vis::Output_Type::VY;
        tVisOutputTypeMap["VZ"]                     = vis::Output_Type::VZ;
        tVisOutputTypeMap["P"]                      = vis::Output_Type::P;
        tVisOutputTypeMap["DOF"]                    = vis::Output_Type::DOF;
        tVisOutputTypeMap["MAX_DOF"]                = vis::Output_Type::MAX_DOF;
        tVisOutputTypeMap["MAX_STRESS"]             = vis::Output_Type::MAX_STRESS;
        tVisOutputTypeMap["L2_ERROR_ANALYTIC"]      = vis::Output_Type::L2_ERROR_ANALYTIC;
        tVisOutputTypeMap["H1_ERROR_ANALYTIC"]      = vis::Output_Type::H1_ERROR_ANALYTIC;
        tVisOutputTypeMap["H1_SEMI_ERROR"]          = vis::Output_Type::H1_SEMI_ERROR;
        tVisOutputTypeMap["PROPERTY"]               = vis::Output_Type::PROPERTY;
        tVisOutputTypeMap["J_INTEGRAL"]             = vis::Output_Type::J_INTEGRAL;
        tVisOutputTypeMap["K1_SENT"]                = vis::Output_Type::K1_SENT;
        tVisOutputTypeMap["DRAG_COEFF"]             = vis::Output_Type::DRAG_COEFF;
        tVisOutputTypeMap["LIFT_COEFF"]             = vis::Output_Type::LIFT_COEFF;
        tVisOutputTypeMap["LATENT_HEAT_ABSORPTION"] = vis::Output_Type::LATENT_HEAT_ABSORPTION;
        tVisOutputTypeMap["VISCOSITY"]              = vis::Output_Type::VISCOSITY;
        tVisOutputTypeMap["TURBULENT_KINEMATIC_VISCOSITY"] = vis::Output_Type::TURBULENT_KINEMATIC_VISCOSITY;
        tVisOutputTypeMap["END_ENUM"]               = vis::Output_Type::END_ENUM;

        return tVisOutputTypeMap;
    }

    enum class VIS_Mesh_Type
    {
        UNDEFINED,
        STANDARD,
        OVERLAPPING_INTERFACE,
        FULL_DISCONTINOUS,
        END_ENUM//
    };

    enum class Field_Type
    {
        UNDEFINED,
        NODAL,
        NODAL_IP,
        ELEMENTAL,
        GLOBAL,
        END_ENUM//
    };

    moris::map< std::string, enum vis::Field_Type > get_vis_field_type_map()
    {
        moris::map< std::string, enum vis::Field_Type > tVisFieldTypeMap;

        tVisFieldTypeMap["UNDEFINED"] = vis::Field_Type::UNDEFINED;
        tVisFieldTypeMap["NODAL"]     = vis::Field_Type::NODAL;
        tVisFieldTypeMap["NODAL_IP"]  = vis::Field_Type::NODAL_IP;
        tVisFieldTypeMap["ELEMENTAL"] = vis::Field_Type::ELEMENTAL;
        tVisFieldTypeMap["GLOBAL"]    = vis::Field_Type::GLOBAL;
        tVisFieldTypeMap["END_ENUM"]  = vis::Field_Type::END_ENUM;

        return tVisFieldTypeMap;
    }
}
}

#endif /* SRC_CL_VIS_ENUMS_HPP_ */
